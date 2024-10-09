#include "Bluetooth.h"
#include "runmode.h"
#include "rtc.h"

extern bool step_flag;

void bluetooth_init(uint32_t baud)
{
	//结构体变量
	GPIO_InitTypeDef	GPIO_InitStructure;
	USART_InitTypeDef	USART_InitStruct;
	NVIC_InitTypeDef   NVIC_InitStructure;	
	
	
	//GPIOA 时钟使能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	
	//串口时钟使能，
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	//设置引脚复用器映射：调用 GPIO_PinAFConfig 函数。
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);		
	
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_10|GPIO_Pin_11;  	//引脚
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;		//复用功能
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_25MHz; //速度
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;	//推挽
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP ;	//上拉
	//GPIO 初始化设置：要设置模式为复用功能。
	GPIO_Init(GPIOB, &GPIO_InitStructure); 	
	
	
	USART_InitStruct.USART_BaudRate		= baud;   			//波特率
	USART_InitStruct.USART_Mode			= USART_Mode_Rx|USART_Mode_Tx;  //双全工
	USART_InitStruct.USART_Parity		= USART_Parity_No;  		//无奇偶校验位
	USART_InitStruct.USART_StopBits		= USART_StopBits_1; 		//1位停止位
	USART_InitStruct.USART_WordLength	= USART_WordLength_8b;		//8位数据
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件控制流
	//5、串口参数初始化：设置波特率，字长，奇偶校验等参数。
	USART_Init(USART3, &USART_InitStruct);	
	
	//配置NVIC
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;    		//中断通道，只能在stm32f4xx.h 查阅
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;  	//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 			//响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//通道使能
	NVIC_Init(&NVIC_InitStructure);		
	
	//配置为接收中断（表示有数据过来，CPU要中断进行接收）
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);    
	
	//使能串口。
	USART_Cmd(USART3, ENABLE);	
}

//蓝牙发送数据
void Bluetooth_SendData(const char *str)
{
	while (*str != '\0')
	{
		USART_SendData(USART3, *str++);
		while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
	}
}

//蓝牙显示状态
bool Bluetooth_CheckStatus(void)
{
	return GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_11) == 1;
}

//串口中断处理函数
void USART3_IRQHandler(void)
{
	static uint8_t rx_buffer[32] = {0};
	static uint8_t count = 0;
	static bool flag = false;
	//判断接受标志位是否置1
	if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
	{
		
		rx_buffer[count] = USART_ReceiveData(USART3); 

		
		//判断输入的是否为日期，日期格式如2024-05-21-2#  2024年5月21号星期2  
		if(count ==11 &&(rx_buffer[4] == '-')&& (rx_buffer[7] == '-')&& (rx_buffer[10] == '-'))
		{
			flag = true;
		}
		//判断输入的是否为时间，时间格式如20:44:15#
		if(count == 7 && (rx_buffer[2] == ':') && (rx_buffer[5] == ':'))
		{
			flag = true;
		}
		//判断输入的是否为要设置的闹钟时间，时间格式如20:44:15-2#  20分44分15秒星期2
		if(count == 9 && (rx_buffer[2] == ':') && (rx_buffer[5] == ':') && (rx_buffer[8] == '-'))
		{
			flag = true;
		}	
		//判断输入的是否为关闭闹钟指令，格式为alarm#
		if(count == 4 && (rx_buffer[0] == 'a') && (rx_buffer[1] == 'l') && (rx_buffer[2] == 'a') && (rx_buffer[3] == 'r') && (rx_buffer[4] == 'm'))
		{
			flag = true;
		}	
		//判断输入的是否为获取步数指令，格式为step#
		if(count == 3 && (rx_buffer[0] == 's') && (rx_buffer[1] == 't') && (rx_buffer[2] == 'e')	&& (rx_buffer[3] == 'p'))
		{
			step_flag = true;
			flag = true;
		}	
		
		//判断是否为结束标志'#'
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
		
		//清除中断标志位
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	}
}
