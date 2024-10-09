#include "key.h"
#include "ui.h"
#include "ui_helpers.h"

/************************************
引脚说明：

KEY0连接在PA0
按键未按下，PA0为高电平(1)
按键 按下，PA0为低电平(0)

KEY1连接在PE2
按键未按下，PE0为高电平(1)
按键 按下，PE0为低电平(0)

KEY2连接在PE3
按键未按下，PE3为高电平(1)
按键 按下，PE3为低电平(0)

KEY3连接在PE4
按键未按下，PE4为高电平(1)
按键 按下，PE4为低电平(0)

*************************************/

/* 事件标志组句柄 */
EventGroupHandle_t key_event_group;

void key_init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	EXTI_InitTypeDef	EXTI_InitStructure;	
	NVIC_InitTypeDef	NVIC_InitStructure;
	
	//使能端口A的硬件时钟，端口A才能工作，说白了就是对端口A上电
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	//使能端口E的硬件时钟，端口E才能工作，说白了就是对端口E上电
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
		
	
	
	//使能系统配置硬件时钟，说白了就是对系统配置的硬件供电
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	//配置硬件，配置GPIO，端口A，第0个引脚
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;							//第0 个引脚
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;						//输入模式
	GPIO_InitStructure.GPIO_Speed=GPIO_High_Speed;					//引脚高速工作，收到指令立即工作；缺点：功耗高
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;					//由于外部已有上拉电阻，内部不需要上下拉电阻
	GPIO_Init(GPIOA,&GPIO_InitStructure);		

	//配置硬件，配置GPIO，端口E，第2 3 4个引脚
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;	//第2 3 4个引脚
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN;						//输入模式
	GPIO_InitStructure.GPIO_Speed=GPIO_High_Speed;					//引脚高速工作
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;					//由于外部已有上拉电阻，内部不需要上下拉电阻。矩阵键盘要使用内部上拉电阻
	GPIO_Init(GPIOE,&GPIO_InitStructure);			
	
	
	//将PA0引脚连接到EXTI0
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA,EXTI_PinSource0);
	
	//将PE2引脚连接到EXTI2
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource2);
	
	//将PE3引脚连接到EXTI3
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource3);	

	//将PE4引脚连接到EXTI4
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource4);	
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line0|EXTI_Line2|EXTI_Line3|EXTI_Line4;	//EXTI0/2/3/4
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;							//中断触发
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 					//下降沿，按键按下，就立即触发中断请求，通知CPU立即处理 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;									//使能
	EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;							//外部中断0的中断号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;						//响应优先级 0x05
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//NVIC使能
	NVIC_Init(&NVIC_InitStructure);	
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;							//外部中断2的中断号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;						//响应优先级 0x05
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//NVIC使能
	NVIC_Init(&NVIC_InitStructure);		
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;							//外部中断3的中断号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;						//响应优先级 0x05
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//NVIC使能
	NVIC_Init(&NVIC_InitStructure);		
	
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;							//外部中断3的中断号
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;						//响应优先级 0x05
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//NVIC使能
	NVIC_Init(&NVIC_InitStructure);	
}

void EXTI0_IRQHandler(void)  //按下按键1切换心率界面
{
	
	//判断中断标志位是否置1
	if(EXTI_GetITStatus(EXTI_Line0) == SET)	//判断是否是外部中断0号线触发的中断
	{
		
		
		/*如果出现数据乱跳的现象，可再次判断引脚电平，以避免抖动*/
		if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0)
		{
			 _ui_screen_change(&ui_heartScreen, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, &ui_heartScreen_screen_init); 
		}
		EXTI_ClearITPendingBit(EXTI_Line0);		//清除外部中断0号线的中断标志位
												//中断标志位必须清除
												//否则中断将连续不断地触发，导致主程序卡死
	}
}

void EXTI2_IRQHandler(void)		//按下按键2切换睡眠质量检测页面
{

	if (EXTI_GetITStatus(EXTI_Line2) == SET)		//判断是否是外部中断2号线触发的中断
	{
		
		/*如果出现数据乱跳的现象，可再次判断引脚电平，以避免抖动*/
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2) == 0)
		{
			_ui_screen_change(&ui_SleepScreen, LV_SCR_LOAD_ANIM_FADE_IN, 50, 0, &ui_SleepScreen_screen_init);
			
			
		}											
		EXTI_ClearITPendingBit(EXTI_Line2);		
			
	}
}


void EXTI3_IRQHandler(void)				//按下按键3切换日历页面
{

	if (EXTI_GetITStatus(EXTI_Line3) == SET)		//判断是否是外部中断3号线触发的中断
	{
		
		/*如果出现数据乱跳的现象，可再次判断引脚电平，以避免抖动*/
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3) == 0)
		{
			
			_ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_IN, 50, 0, &ui_Screen1_screen_init);
				
		}	

		EXTI_ClearITPendingBit(EXTI_Line3);		
														
	}
}

void EXTI4_IRQHandler(void)			//按下按键4切换主界面
{
	
	if (EXTI_GetITStatus(EXTI_Line4) == SET)		//判断是否是外部中断4号线触发的中断
	{
		
		/*如果出现数据乱跳的现象，可再次判断引脚电平，以避免抖动*/
		if (GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4) == 0)
		{	
			
			_ui_screen_change(&ui_MainScreen, LV_SCR_LOAD_ANIM_FADE_ON, 200, 0, &ui_MainScreen_screen_init);
															
		}	
		EXTI_ClearITPendingBit(EXTI_Line4);				
	}
}

