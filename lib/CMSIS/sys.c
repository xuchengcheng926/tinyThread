#include "sys.h"

static uint16_t fac_ms, fac_us;
int delay_init(void)
{
 	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); 
	fac_us = SystemCoreClock / 8 / 1000000;	
	fac_ms = (uint16_t)fac_us * 1000;

    return 0;
}

int delay_ms(uint32_t cnt)
{
    uint32_t temp;
    SysTick->LOAD = cnt * fac_ms;             
    SysTick->VAL = 0x00;                     
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; 
    do
    {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16))); 
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;      
    SysTick->VAL = 0X00;   
    return 0;
}

int delay_us(uint32_t cnt)
{
    uint32_t temp;
    SysTick->LOAD = cnt * fac_us;             
    SysTick->VAL = 0x00;                     
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; 
    do
    {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16))); 
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;      
    SysTick->VAL = 0X00;                            
    return 0;
}

struct __FILE
{
    int handle;
};
FILE __stdout;
int fputc(int ch, FILE *f)
{
    USART1->DR = ((uint8_t)ch & (uint16_t)0x01FF);
    while ((USART1->SR & USART_FLAG_TXE) == RESET);
    return (ch);
}

static int usart_sendc(uint8_t data)
{
    USART1->DR = (data & (uint16_t)0x01FF);
    while ((USART1->SR & USART_FLAG_TXE) == RESET);
    return 0;
}

int usart_send(uint8_t* data, uint32_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        usart_sendc(data[i]);
    }
    return 0;
}

int usart_print(const char* data)
{
    usart_send((uint8_t*)data, strlen(data));
    return 0;
}

int usart_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_DeInit(GPIOA);
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    USART_InitTypeDef USART_InitStruct;
    USART_DeInit(USART1);
    USART_StructInit(&USART_InitStruct);
    USART_InitStruct.USART_BaudRate = 115200;
    USART_Init(USART1, &USART_InitStruct);
    USART_Cmd(USART1, ENABLE);
    return 0;
}

int led_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_DeInit(GPIOC);
    GPIO_StructInit(&GPIO_InitStruct);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
    return 0;
}

int led_flash(void)
{
    uint8_t enable = GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13);
    if (enable)
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
    }
    else
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_13);
    }
    return 0;
}

int led_set(uint8_t enable)
{
    if (enable)
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_13);
    }
    else
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
    }
    return 0;
}
