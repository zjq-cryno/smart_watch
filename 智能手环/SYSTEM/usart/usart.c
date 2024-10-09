#include "stm32f4xx.h"
#include "sys.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "FreeRTOSConfig.h"


static USART_InitTypeDef USART_InitStructure;
extern GPIO_InitTypeDef GPIO_InitStructure;
extern NVIC_InitTypeDef NVIC_InitStructure;

volatile uint8_t g_usart1_rx_buf[1280];
volatile uint32_t g_usart1_rx_cnt = 0;
volatile uint32_t g_usart1_rx_end = 0;

#pragma import(__use_no_semihosting_swi)

struct __FILE
{
	int handle; /* Add whatever you need here */
};
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, ch);

	// �ȴ����ݷ��ͳɹ�
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
		;
	USART_ClearFlag(USART1, USART_FLAG_TXE);

	return ch;
}

void _sys_exit(int return_code)
{
}

void usart1_init(uint32_t baud)
{
	// ʹ�ܶ˿�AӲ��ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	// ʹ�ܴ���1Ӳ��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	// ����PA9��PA10Ϊ���ù�������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// ��PA9��PA10���ӵ�USART1��Ӳ��
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

	// ����USART1����ز����������ʡ�����λ��У��λ
	USART_InitStructure.USART_BaudRate = baud;										// ������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 8λ����λ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 1λֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;								// ����żУ��
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // ��Ӳ��������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// �������ڷ��ͺͽ�������
	USART_Init(USART1, &USART_InitStructure);

	// ʹ�ܴ��ڽ��յ����ݴ����ж�
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// ʹ�ܴ���1����
	USART_Cmd(USART1, ENABLE);
}

void usart1_send_bytes(uint8_t *buf, uint32_t len)
{
	uint8_t *p = buf;

	while (len--)
	{
		USART_SendData(USART1, *p);

		p++;

		// �ȴ����ݷ��ͳɹ�
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		USART_ClearFlag(USART1, USART_FLAG_TXE);
	}
	memset(buf, 0, sizeof(buf));
}

void USART1_IRQHandler(void)
{
	uint8_t d = 0;

	// ����Ƿ���յ�����
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		d = USART_ReceiveData(USART1);

		g_usart1_rx_buf[g_usart1_rx_cnt++] = d;

		if (g_usart1_rx_cnt >= sizeof(g_usart1_rx_buf))
		{
			g_usart1_rx_end = 1;
		}

		// ��ձ�־λ��������Ӧ�µ��ж�����
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}
