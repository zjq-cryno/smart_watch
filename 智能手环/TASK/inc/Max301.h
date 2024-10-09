#ifndef __MAX301_H__
#define __MAX301_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx.h"
#include "usart.h"
#include "max30102.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "algorithm.h"

extern int32_t n_heart_rate;
extern int32_t n_sp02; 
extern void HeartSpo2_getvalue_task(void *pvParameters);

#endif // __MAX301_H__
