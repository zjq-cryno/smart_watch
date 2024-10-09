#include "Bluetooth.h"
#include "runmode.h"
#include "rtc.h"

extern bool step_flag;

void bluetooth_init(uint32_t baud)
{
	//�ṹ�����
	GPIO_InitTypeDef	GPIO_InitStructure;
	USART_InitTypeDef	USART_InitStruct;
	NVIC_InitTypeDef   NVIC_InitStructure;	
	
	
	//GPIOA ʱ��ʹ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	
	//����ʱ��ʹ�ܣ�
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	//�������Ÿ�����ӳ�䣺���� GPIO_PinAFConfig ������
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);		
	
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_10|GPIO_Pin_11;  	//����
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;		//���ù���
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_25MHz; //�ٶ�
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;	//����
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP ;	//����
	//GPIO ��ʼ�����ã�Ҫ����ģʽΪ���ù��ܡ�
	GPIO_Init(GPIOB, &GPIO_InitStructure); 	
	
	
	USART_InitStruct.USART_BaudRate		= baud;   			//������
	USART_InitStruct.USART_Mode			= USART_Mode_Rx|USART_Mode_Tx;  //˫ȫ��
	USART_InitStruct.USART_Parity		= USART_Parity_No;  		//����żУ��λ
	USART_InitStruct.USART_StopBits		= USART_StopBits_1; 		//1λֹͣλ
	USART_InitStruct.USART_WordLength	= USART_WordLength_8b;		//8λ����
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //��Ӳ��������
	//5�����ڲ�����ʼ�������ò����ʣ��ֳ�����żУ��Ȳ�����
	USART_Init(USART3, &USART_InitStruct);	
	
	//����NVIC
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;    		//�ж�ͨ����ֻ����stm32f4xx.h ����
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;  	//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 			//��Ӧ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//ͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);		
	
	//����Ϊ�����жϣ���ʾ�����ݹ�����CPUҪ�жϽ��н��գ�
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);    
	
	//ʹ�ܴ��ڡ�
	USART_Cmd(USART3, ENABLE);	
}

//������������
void Bluetooth_SendData(const char *str)
{
	while (*str != '\0')
	{
		USART_SendData(USART3, *str++);
		while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
	}
}

//������ʾ״̬
bool Bluetooth_CheckStatus(void)
{
	return GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_11) == 1;
}

//�����жϴ�����
void USART3_IRQHandler(void)
{
	static uint8_t rx_buffer[32] = {0};
	static uint8_t count = 0;
	static bool flag = false;
	//�жϽ��ܱ�־λ�Ƿ���1
	if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
	{
		
		rx_buffer[count] = USART_ReceiveData(USART3); 

		
		//�ж�������Ƿ�Ϊ���ڣ����ڸ�ʽ��2024-05-21-2#  2024��5��21������2  
		if(count ==11 &&(rx_buffer[4] == '-')&& (rx_buffer[7] == '-')&& (rx_buffer[10] == '-'))
		{
			flag = true;
		}
		//�ж�������Ƿ�Ϊʱ�䣬ʱ���ʽ��20:44:15#
		if(count == 7 && (rx_buffer[2] == ':') && (rx_buffer[5] == ':'))
		{
			flag = true;
		}
		//�ж�������Ƿ�ΪҪ���õ�����ʱ�䣬ʱ���ʽ��20:44:15-2#  20��44��15������2
		if(count == 9 && (rx_buffer[2] == ':') && (rx_buffer[5] == ':') && (rx_buffer[8] == '-'))
		{
			flag = true;
		}	
		//�ж�������Ƿ�Ϊ�ر�����ָ���ʽΪalarm#
		if(count == 4 && (rx_buffer[0] == 'a') && (rx_buffer[1] == 'l') && (rx_buffer[2] == 'a') && (rx_buffer[3] == 'r') && (rx_buffer[4] == 'm'))
		{
			flag = true;
		}	
		//�ж�������Ƿ�Ϊ��ȡ����ָ���ʽΪstep#
		if(count == 3 && (rx_buffer[0] == 's') && (rx_buffer[1] == 't') && (rx_buffer[2] == 'e')	&& (rx_buffer[3] == 'p'))
		{
			step_flag = true;
			flag = true;
		}	
		
		//�ж��Ƿ�Ϊ������־'#'
		if (rx_buffer[count] == '#')
		{
			if (flag == true)
			{
				flag = false;
				memcpy(BlueTooth_Rx_Buffer, rx_buffer, count + 1);
				BlueTooth_Rx_Buffer[count] = '\0'; 
				memset(rx_buffer, 0, sizeof(rx_buffer));
				xEventGroupSetBitsFromISR(xBluetoothEventGroup, BLUETOOTH_RX_BIT, NULL);
			}
			
			count = 0; 
		}
		else
		{
			count++;
			if (count >= sizeof(rx_buffer))
			{
				count = 0;
			}
		}
		
		//����жϱ�־λ
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	}
}
