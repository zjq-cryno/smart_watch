#include "stm32f4xx.h"
#include "sys.h"
#include "usart.h"
#include "../lvgl.h"
#include "FreeRTOSConfig.h"
#include "tim.h"
#include "stm32f4xx_rcc.h"

void tim3_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// ��TIM3Ӳ��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = 10000 / 1000 - 1;		// ����ֵ�����ڶ�ʱʱ�������
	TIM_TimeBaseStructure.TIM_Prescaler = 8399;					// Ԥ��Ƶֵ������
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;				// F407û�У�����Ҫ����
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // ���ϼ���������0 -> TIM_Period��Ȼ��ͻᴥ��ʱ������ж�
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/* ��ʱ��ʱ������ж�ʹ�� */
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	// ����NVIC�����ڴ�TIM3���ж�����ͨ��
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;														 // TIM3������ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY; // ��ռ���ȼ�0xF
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;												 // ��Ӧ���ȼ�0xF
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;														 // ��������ͨ��
	NVIC_Init(&NVIC_InitStructure);

	/* ʹ��TIM3���� */
	TIM_Cmd(TIM3, ENABLE);
}

/************************************
定时器说明：

TIM3：16定时器(65535)
TIM3挂在APB1下，定时器频率：84MHZ
*************************************/
void Tim1_Init(void)
{
	
	TIM_TimeBaseInitTypeDef	TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef   NVIC_InitStructure;
	
	
	
	
	//1、能定时器时钟。
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	
	TIM_TimeBaseInitStruct.TIM_Prescaler	= (168-1);  //预分频器  168MHZ/168 = 1MHZ
	TIM_TimeBaseInitStruct.TIM_Period		= (1000-1);//在1MHZ下，计1000个用时1ms
	TIM_TimeBaseInitStruct.TIM_CounterMode	= TIM_CounterMode_Up; 	//向上计数
	TIM_TimeBaseInitStruct.TIM_ClockDivision= TIM_CKD_DIV1; 		//分频因子
	
	
	//1、能定时器时钟。
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
//	
//	TIM_TimeBaseInitStruct.TIM_Prescaler	= (16800-1);  //预分频器  168MHZ/16800 = 10000HZ
//	TIM_TimeBaseInitStruct.TIM_Period		= (10000-1); //在10000HZ下，计10000个用时1s
//	TIM_TimeBaseInitStruct.TIM_CounterMode	= TIM_CounterMode_Up; 	//向上计数
//	TIM_TimeBaseInitStruct.TIM_ClockDivision= TIM_CKD_DIV1; 		//分频因子


	//2、初始化定时器，配置ARR,PSC。
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStruct);
	
	
	//3、启定时器中断，配置NVIC。
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_TIM10_IRQn;    		//中断通道，只能在stm32f4xx.h 查阅
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  	//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 			//响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//通道使能
	NVIC_Init(&NVIC_InitStructure);
	
	
	//4、设置 TIM3_DIER  允许更新中断
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
	
	//5、使能定时器。--开始向上计数
	TIM_Cmd(TIM1, ENABLE);

}
/************************************

用于处理TIM1和TIM10定时器的中断
当TIM1或TIM10定时器中断发生时，CPU会自动跳转到这个函数中进行处理，函数名中的UP表示该定时器的更新事件触发了中断，即计数器溢出

************************************/

void TIM1_UP_TIM10_IRQHandler(void)          //TIM1 Update and TIM10
{
	
	//判断更新标志位是否置1
	if(TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{

		
		
	
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
	
}
void TIM3_IRQHandler(void)
{
	static uint32_t usart1_rx_cnt_last = 0;
	static uint32_t irq_cnt = 0;

	// ���ʱ������жϵı�־λ�Ƿ���λ
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		irq_cnt++;

		if (irq_cnt >= 10 && g_usart1_rx_cnt)
		{
			// ����ȣ����ʾ����3Ŀǰ�����������
			if (g_usart1_rx_cnt == usart1_rx_cnt_last)
			{
				g_usart1_rx_end = 1;
			}
			else
			{
				usart1_rx_cnt_last = g_usart1_rx_cnt;
			}

			irq_cnt = 0;
		}

		lv_tick_inc(1);

		// ��ձ�־λ
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}
