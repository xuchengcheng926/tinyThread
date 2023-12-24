#ifndef __SYS_H__
#define __SYS_H__
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f10x.h"

int delay_init(void);
int delay_ms(uint32_t cnt);
int delay_us(uint32_t cnt);

int usart_init(void);
int usart_send(uint8_t* data, uint32_t len);
int usart_print(const char* data);

int led_init(void);
int led_flash(void);
int led_set(uint8_t enable);

#endif // !__SYS_H__
