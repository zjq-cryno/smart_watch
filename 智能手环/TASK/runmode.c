#include "runmode.h"
#include "stm32f4xx_rtc.h"
#include "ui.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "led.h"
#include "beep.h"
#include "Bluetooth.h"
#include <stdbool.h>
#include "includes.h"
#include "inv_mpu.h"

bool step_flag = false;
uint32_t t=0;


unsigned long step_cnt;

void Send_stepcnt_task(void *pvParameter);
void Update_StepNum_ui(void *pvParameter);

void mpu6050_runmode_task(void *pvParameter)
{
	uint8_t res;
	
	float pitch,roll,yaw; 		//欧拉角
	short aacx,aacy,aacz;		//加速度传感器原始数据
	short gyrox,gyroy,gyroz;	//陀螺仪原始数据
	
		// 初始化MPU6050
	while (MPU_Init())
	{
		printf("[ERROR] MPU6050 Initialization failed\r\n");
		vTaskDelay(500);
	}
	printf("[OK] MPU_Init success\r\n");
	
	while(mpu_dmp_init())
	{
		printf("[ERROR] MPU6050 DMP Initialization failed \r\n");
		printf("error num:%d\r\n",mpu_dmp_init());
		vTaskDelay(500); 
	}
	printf("[OK] mpu_dmp_init success\r\n");
	
	/* 设置步数初值为0*/
	
	while(dmp_set_pedometer_step_count(0))
	{
		//delay_ms(500);
		vTaskDelay(500);

	}
	
	printf("[OK] dmp_set_pedometer_step_count success\r\n");
	
	while(1)
	{
			dmp_get_pedometer_step_count(&step_cnt);
		res=mpu_dmp_get_data(&pitch,&roll,&yaw);
		//printf("res:%d\r\n",res);
		if(res==0)
		{ 
			printf("step_cnt:%ld\r\n",step_cnt);
			printf("pitch=%f,roll=%f,yaw=%f\r\n",pitch,roll,yaw);
			
			//delay_ms(100);
			vTaskDelay(1000);
		
		}	
	}
}
void Send_stepcnt_task(void *pvParameter)
{
	 int cnt = 1;
	 char str[20];
	 while(1)
	 {
			cnt++;
			vTaskDelay(100);
			if(cnt == 20)      //每2000ms判断一次step_flag
			{
				if(step_flag == true)         //判断手机蓝牙发送的是否为获取步数指令
				{
					sprintf(str,"%ld",step_cnt);
					Bluetooth_SendData(str);    //发送步数值到手机
					step_flag = false;    //重新置为0
					
				}
				cnt = 1;
			}
	}
}
void Update_StepNum_ui(void *pvParameter)
{
	 while(1)
	 {
		lv_label_set_text_fmt(ui_LabelStepNum, "%d", step_cnt);
		lv_arc_set_value(ui_ArcStep, step_cnt);
		 vTaskDelay(1000); // 延迟以避免无休止循环
	 }
}
	
