#include "tinyThread.h"
#include "tinyList.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "sys.h"

#define TINY_THREAD_STACK_SIZE  (0x100)

typedef struct tagtinyThread
{
    tinyListItem head;
    TINY_THREAD_FUNC func;
    uint32_t* stackTop;
    uint32_t* stackBottom;
    uint32_t stackSize;
    void *args;
    int tid;
    uint32_t regs[8];
    uint8_t isRun;
} tinyThread;

typedef struct
{
    uint8_t isInit;
    tinyListHead head;
    int tid;
    tinyThread* currentThread;
} tinyThreadMgr;

static tinyThreadMgr s_tinyThreadMgr = {0};

static void threadSourceFree(void* p)
{
    tinyThread* thread = (tinyThread*)p;
    free(thread->stackTop);
    free(thread);
}

/*
    执行中断后自动将下列寄存器压入堆栈
    R0 R1 R2 R3 R12 R14 R15 xPSR 
*/
static void tinythread_save_environment(tinyThread* thd, uint32_t stack)
{
    uint32_t* staskPtr = (uint32_t*)(stack);
    thd->stackTop = (uint32_t*)((uint32_t)stack + 0x20);
    memcpy(thd->regs, staskPtr, 0x20);
}

static void tinythread_restore_environment(tinyThread* thd, uint32_t stack)
{
    uint32_t currStack = __get_MSP();
    uint32_t currStackChange = stack - currStack;

    uint32_t restoreStack = (uint32_t)thd->stackTop - 0x20 - currStackChange;
    memcpy((uint32_t*)((uint32_t)thd->stackTop - 0x20), thd->regs, 0x20);
    memcpy((uint32_t*)restoreStack, (uint32_t*)currStack, currStackChange);

    __set_MSP(restoreStack);
}

static void tinythread_work(uint32_t stack)
{
    // 保护当前执行线程的现场
    if (s_tinyThreadMgr.currentThread)
    {
        tinythread_save_environment(s_tinyThreadMgr.currentThread, stack);
    }

    tinyThread* thd = s_tinyThreadMgr.currentThread = (tinyThread*)(tinyList_next(s_tinyThreadMgr.currentThread)? tinyList_next(s_tinyThreadMgr.currentThread): tinyList_head(s_tinyThreadMgr.head));
    if (thd == NULL)
    {
        printf("No thread found!!!\n");
        return;
    }

    if (thd->isRun == 0)
    {
        thd->regs[0] = (uint32_t)thd->args;
        thd->regs[1] = (uint32_t)0x00000000;
        thd->regs[2] = (uint32_t)0x00000000;
        thd->regs[3] = (uint32_t)0x00000000;
        thd->regs[4] = (uint32_t)0x00002000;
        thd->regs[5] = (uint32_t)0x080001E9;
        thd->regs[6] = (uint32_t)thd->func + 0x2;
        thd->regs[7] = (uint32_t)0x61000000;
        thd->isRun = 1;
    }
    
    tinythread_restore_environment(thd, stack);
}


void TIM2_IRQHandler(void)  //TM2的中断函数
{
    uint32_t stack = __get_MSP() + 0x8; 

    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        tinythread_work(stack);
    }
}

int tinythread_init(void)
{
    if (s_tinyThreadMgr.isInit)
    {
        return -1;
    }
    
    tinyList_init(s_tinyThreadMgr.head);
    s_tinyThreadMgr.isInit = 1;
    s_tinyThreadMgr.tid = 0;
    s_tinyThreadMgr.currentThread = NULL;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);//TIM2挂在APB2上
	TIM_InternalClockConfig(TIM2);//使用RCC内部时钟72MHZ
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;  //  是否划分72MHZ时钟频率
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;  //向上计次
	TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;  //  自动重装器的值  最大为65535
	TIM_TimeBaseInitStructure.TIM_Prescaler = 7200 - 1;  //预分频系数  最大为65535 1/(72000000/7200-1+1)*10000=1s  也就是说每一秒进一次中断
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;  //是否重复计数
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
	
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);  //清除标志位
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);  //启用或者禁用定时器中断，中断输出控制模块：开启更新中断到NVIC的通路
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
    return 0;
}

int tinythread_destroy(void)
{
    if (s_tinyThreadMgr.isInit == 0)
    {
        return -1;
    }

    tinyList_destroy_free(s_tinyThreadMgr.head, threadSourceFree);
    s_tinyThreadMgr.isInit = 0;
    return 0;
}

int tinythread_run(void)
{
    if (s_tinyThreadMgr.isInit == 0)
    {
        return -1;
    }
	TIM_Cmd(TIM2, ENABLE);
	return 0;
}

int tinythread_create(TINY_THREAD_FUNC func, void *args)
{
    if (s_tinyThreadMgr.isInit == 0)
    {
        tinythread_init();
    }

    tinyThread* thread = (tinyThread*)malloc(sizeof(tinyThread));
    thread->func = func;
    thread->args = args;
    thread->stackSize = TINY_THREAD_STACK_SIZE;
    thread->stackBottom = malloc(thread->stackSize);
    thread->stackTop = (uint32_t*)((uint32_t)thread->stackBottom + thread->stackSize);
    thread->tid = ++s_tinyThreadMgr.tid;
    thread->isRun = 0;

    printf("%s threadID %d\n", __FUNCTION__, thread->tid);

    tinyList_pushback(s_tinyThreadMgr.head, thread);
    return thread->tid;
}

int tinythread_join(int tid, void* ret)
{
    if (s_tinyThreadMgr.isInit == 0)
    {
        tinythread_init();
    }

    return 0;
}

int tinythread_detach(int tid)
{
    if (s_tinyThreadMgr.isInit == 0)
    {
        tinythread_init();
    }

    return 0;
}



