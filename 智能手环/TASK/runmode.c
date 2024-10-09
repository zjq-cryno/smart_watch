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
	
	float pitch,roll,yaw; 		//ŷ����
	short aacx,aacy,aacz;		//���ٶȴ�����ԭʼ����
	short gyrox,gyroy,gyroz;	//������ԭʼ����
	
		// ��ʼ��MPU6050
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
	
	/* ���ò�����ֵΪ0*/
	
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
			if(cnt == 20)      //ÿ2000ms�ж�һ��step_flag
			{
				if(step_flag == true)         //�ж��ֻ��������͵��Ƿ�Ϊ��ȡ����ָ��
				{
					sprintf(str,"%ld",step_cnt);
					Bluetooth_SendData(str);    //���Ͳ���ֵ���ֻ�
					step_flag = false;    //������Ϊ0
					
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
		 vTaskDelay(1000); // �ӳ��Ա�������ֹѭ��
	 }
}
	
