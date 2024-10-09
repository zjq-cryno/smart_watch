#include "delay.h"
#include "sys.h"
#include "FreeRTOS.h"
#include "task.h"

void delay_us(uint32_t nus)
{
	uint32_t ticks;
	uint32_t told, tnow, tcnt = 0;
	uint32_t reload = SysTick->LOAD;		   // ϵͳ��ʱ��������ֵ
	ticks = nus * (SystemCoreClock / 1000000); // ��Ҫ�Ľ�����
	told = SysTick->VAL;					   // �ս���ʱ�ļ�����ֵ

	/* ���������[��ѡ,�ᵼ�¸����ȼ������޷���ռ��ǰ���񣬵��ܹ���ߵ�ǰ����ʱ��ľ�ȷ��] */
	vTaskSuspendAll();

	while (1)
	{
		tnow = SysTick->VAL;

		if (tnow != told)
		{
			/* SYSTICK��һ���ݼ��ļ����� */
			if (tnow < told)
				tcnt += told - tnow;
			else
				tcnt += reload - tnow + told;

			told = tnow;

			/* ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳���*/
			if (tcnt >= ticks)
				break;
		}
	}

	/* �ָ�������[��ѡ] */
	xTaskResumeAll();
}

void delay_ms(uint32_t nms)
{
	vTaskDelay(nms);
}
