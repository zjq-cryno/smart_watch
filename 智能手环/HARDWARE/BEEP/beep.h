#ifndef __BEEP_H
#define __BEEP_H

#include "stm32f4xx.h"

#define  Beep(x)  x?GPIO_SetBits(GPIOF, GPIO_Pin_8):GPIO_ResetBits(GPIOF, GPIO_Pin_8)

extern void beep_init(void);
extern void beep_on(void);
extern void beep_off(void);

#endif