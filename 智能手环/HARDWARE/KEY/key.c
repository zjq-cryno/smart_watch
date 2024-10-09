#include "key.h"
#include "ui.h"
#include "ui_helpers.h"

/************************************
����˵����

KEY0������PA0
����δ���£�PA0Ϊ�ߵ�ƽ(1)
���� ���£�PA0Ϊ�͵�ƽ(0)

KEY1������PE2
����δ���£�PE0Ϊ�ߵ�ƽ(1)
���� ���£�PE0Ϊ�͵�ƽ(0)

KEY2������PE3
����δ���£�PE3Ϊ�ߵ�ƽ(1)
���� ���£�PE3Ϊ�͵�ƽ(0)

KEY3������PE4
����δ���£�PE4Ϊ�ߵ�ƽ(1)
���� ���£�PE4Ϊ�͵�ƽ(0)

*************************************/

/* �¼���־���� */
EventGroupHandle_t key_event_group;

void key_init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	EXTI_InitTypeDef	EXTI_InitStructure;	
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	//ʹ�ܶ˿�A��Ӳ��ʱ�ӣ��˿�A���ܹ�����˵���˾��ǶԶ˿�A�ϵ�
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	//ʹ�ܶ˿�E��Ӳ��ʱ�ӣ��˿�E���ܹ�����˵���˾��ǶԶ˿�E�ϵ�
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
		
	
	
	//ʹ��ϵͳ����Ӳ��ʱ�ӣ�˵���˾��Ƕ�ϵͳ���õ�Ӳ������
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	//����Ӳ��������GPIO���˿�A����0������
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;							//��0 ������
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;						//����ģʽ
	GPIO_InitStructure.GPIO_Speed=GPIO_High_Speed;					//���Ÿ��ٹ������յ�ָ������������ȱ�㣺���ĸ�
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;					//�����ⲿ�����������裬�ڲ�����Ҫ����������
	GPIO_Init(GPIOA,&GPIO_InitStructure);		

	//����Ӳ��������GPIO���˿�E����2 3 4������
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;	//��2 3 4������
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;						//����ģʽ
	GPIO_InitStructure.GPIO_Speed=GPIO_High_Speed;					//���Ÿ��ٹ���
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;					//�����ⲿ�����������裬�ڲ�����Ҫ���������衣�������Ҫʹ���ڲ���������
	GPIO_Init(GPIOE,&GPIO_InitStructure);			
	
	
	//��PA0�������ӵ�EXTI0
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA,EXTI_PinSource0);
	
	//��PE2�������ӵ�EXTI2
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource2);
	
	//��PE3�������ӵ�EXTI3
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource3);	

	//��PE4�������ӵ�EXTI4
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource4);	
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line0|EXTI_Line2|EXTI_Line3|EXTI_Line4;	//EXTI0/2/3/4
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;							//�жϴ���
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 					//�½��أ��������£������������ж�����֪ͨCPU�������� 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;									//ʹ��
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;							//�ⲿ�ж�0���жϺ�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;						//��Ӧ���ȼ� 0x05
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//NVICʹ��
	NVIC_Init(&NVIC_InitStructure);	
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;							//�ⲿ�ж�2���жϺ�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;						//��Ӧ���ȼ� 0x05
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//NVICʹ��
	NVIC_Init(&NVIC_InitStructure);		
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;							//�ⲿ�ж�3���жϺ�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;						//��Ӧ���ȼ� 0x05
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//NVICʹ��
	NVIC_Init(&NVIC_InitStructure);		
	
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;							//�ⲿ�ж�3���жϺ�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;						//��Ӧ���ȼ� 0x05
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//NVICʹ��
	NVIC_Init(&NVIC_InitStructure);	
}

void EXTI0_IRQHandler(void)  //���°���1�л����ʽ���
{
	
	//�ж��жϱ�־λ�Ƿ���1
	if(EXTI_GetITStatus(EXTI_Line0) == SET)	//�ж��Ƿ����ⲿ�ж�0���ߴ������ж�
	{
		
		
		/*��������������������󣬿��ٴ��ж����ŵ�ƽ���Ա��ⶶ��*/
		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0)
		{
			 _ui_screen_change(&ui_heartScreen, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, &ui_heartScreen_screen_init); 
		}
		EXTI_ClearITPendingBit(EXTI_Line0);		//����ⲿ�ж�0���ߵ��жϱ�־λ
												//�жϱ�־λ�������
												//�����жϽ��������ϵش�����������������
	}
}

void EXTI2_IRQHandler(void)		//���°���2�л�˯���������ҳ��
{

	if (EXTI_GetITStatus(EXTI_Line2) == SET)		//�ж��Ƿ����ⲿ�ж�2���ߴ������ж�
	{
		
		/*��������������������󣬿��ٴ��ж����ŵ�ƽ���Ա��ⶶ��*/
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2) == 0)
		{
			_ui_screen_change(&ui_SleepScreen, LV_SCR_LOAD_ANIM_FADE_IN, 50, 0, &ui_SleepScreen_screen_init);
			
			
		}											
		EXTI_ClearITPendingBit(EXTI_Line2);		
			
	}
}


void EXTI3_IRQHandler(void)				//���°���3�л�����ҳ��
{

	if (EXTI_GetITStatus(EXTI_Line3) == SET)		//�ж��Ƿ����ⲿ�ж�3���ߴ������ж�
	{
		
		/*��������������������󣬿��ٴ��ж����ŵ�ƽ���Ա��ⶶ��*/
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3) == 0)
		{
			
			_ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_IN, 50, 0, &ui_Screen1_screen_init);
				
		}	

		EXTI_ClearITPendingBit(EXTI_Line3);		
														
	}
}

void EXTI4_IRQHandler(void)			//���°���4�л�������
{
	
	if (EXTI_GetITStatus(EXTI_Line4) == SET)		//�ж��Ƿ����ⲿ�ж�4���ߴ������ж�
	{
		
		/*��������������������󣬿��ٴ��ж����ŵ�ƽ���Ա��ⶶ��*/
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4) == 0)
		{	
			
			_ui_screen_change(&ui_MainScreen, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, &ui_MainScreen_screen_init);
															
		}	
		EXTI_ClearITPendingBit(EXTI_Line4);				
	}
}

