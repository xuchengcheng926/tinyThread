#include "sys.h"
#include "stm32f10x.h"
#include "tinyThread.h"

int arg[10] = {0};

void delay(uint16_t cnt)
{
	for (size_t i = 0; i < cnt; i++)
	{
		int j = 4000;
		while (j--);
	}
	
}

#define HELLOWORLD(i)              \
	void *helloworld##i(void *args) \
	{                              \
		while (1)                  \
		{                          \
			arg[i]++;              \
			delay(100 * (i + 1));        \
		}                          \
	}

HELLOWORLD(0)
HELLOWORLD(9)

char buff[200] = {0};
void *myMain(void *args)
{
	while (1)
	{
		sprintf(buff, "cnt100: %03d cnt1000: %03d\n", arg[0], arg[9]);
		usart_send(buff, strlen(buff));
		delay(100);
	}
}

int main(void)
{
	usart_init();
	delay_init();
	led_init();
	TIM_DeInit(TIM1);

	tinythread_create(helloworld0, NULL);
	tinythread_create(helloworld9, NULL);

	tinythread_create(myMain, NULL);
	tinythread_run();

	while (1)
	{
	}
}
