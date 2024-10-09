
#include "includes.h"
#include "inv_mpu.h"

#include "step.h"
#include "TemHum.h"
#include "detectionAngle.h"
#include "screenControls.h"
#include "bluetoothTask.h"
#include "runmode.h"
#include "Max301.h"
#include "mpu6050.h"


GPIO_InitTypeDef GPIO_InitStructure;
NVIC_InitTypeDef NVIC_InitStructure;
SPI_InitTypeDef SPI_InitStructure;

bool is_screen_off = false; // 屏幕是否熄屏标志位
uint8_t page_Num = 1;		// 当前显示的页面

/* 互斥型信号量句柄 */
SemaphoreHandle_t xMutex;
static SemaphoreHandle_t g_mutex_printf;



/* 任务句柄 */
static void app_task_setup(void *pvParameter);
static void screen_control_task(void *pvParameters);
static void app_task_lvgl(void *pvParameter);
static void app_task_dht11(void *pvParameter);
static void app_task_bluetooth(void *pvParameter);
static void app_task_max301(void *pvParameter);
static void app_task_mpu6050(void *pvParameter);

static void lv_timer_back(); // 创建lv_timer_create()定时器函数




/* 主函数 */
int main(void)
{
	xMutex = xSemaphoreCreateMutex(); // 创建互斥锁

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	SysTick_Config(SystemCoreClock / configTICK_RATE_HZ);
	
	// 创建system_init任务
	xTaskCreate(app_task_setup, "app_task_setup", 512, NULL, 4, NULL);

	vTaskStartScheduler();
	
	while (true)
	{
	}
}

/*任务函数*/
/*------------------------START-------------------------------*/
/* 初始化任务 */
static void app_task_setup(void *pvParameters)
{

	rtc_init();
	beep_init();
	key_init();
	Led_Init();
	usart1_init(115200);
	dht11_init();
	tim3_init();
	max30102_init();

	// 初始化蓝牙
	bluetooth_init(9600);    //设置蓝牙波特率
	Bluetooth_SendData("AT\r\n");  //发送指令
	vTaskDelay(100); 
	Bluetooth_SendData("AT+NAMEZJQ\r\n");//发送指令,修改蓝牙名称为ZJQ
	
	// 串口延迟一会，确保芯片内部完成全部初始化,printf无乱码输出
	delay_ms(1000);
	
	
	/* 初始化lvgl */
	lv_init();
	lv_port_disp_init();
	lv_port_indev_init();
	ui_init();
	
	// 创建lvgl定时器，lv_timer_back，定时周期为100ms
	lv_timer_create(lv_timer_back, 1000, NULL);


	

	
	/*************************各模块记得要在初始化函数里调用初始化函数******************************/

	/* 创建互斥锁（互斥量、互斥型互斥锁） */
	g_mutex_printf=xSemaphoreCreateMutex();
	
	/*创建lvgl任务*/
		xTaskCreate(app_task_lvgl, "app_task_lvgl", 1024, NULL, 5, NULL);

		/*创建dht11任务 */
		xTaskCreate(app_task_dht11, "app_task_dht11", 256, NULL, 5, NULL);

		/*创建key任务 */
		//xTaskCreate(app_task_key, "app_task_key", 256, NULL, 5, NULL);
		
		// 创建蓝牙任务
		xTaskCreate(app_task_bluetooth, "app_task_bluetooth", 256, NULL, 5, NULL);
		
		/*创建max301任务 */
		xTaskCreate(app_task_max301, "app_task_max301", 256, NULL, 5, NULL);
		
		
		/*创建mpu6050任务 */
		xTaskCreate(app_task_mpu6050, "app_task_mpu6050", 256, NULL, 6, NULL);

		// 创建屏幕控制任务
		xTaskCreate(screen_control_task, "screen_control_task", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

	vTaskDelete(NULL);
}

/* lvgl任务函数 */
static void app_task_lvgl(void *pvParameters)
{
	while (true)
	{
		lv_task_handler();
		vTaskDelay(5);
	}
}

/* dht11任务函数 */
static void app_task_dht11(void *pvParameter)
{
	TemHum_init();

	// 创建任务
	xTaskCreate(TemHum__sensor_task, "TemHum Sensor", 256, NULL, 5, NULL);
	xTaskCreate(TemHum_ui_update_task, "TemHum UI Update", 256, NULL, 5, NULL);

	vTaskDelete(NULL);
}


/* max301任务函数 */
static void app_task_max301(void *pvParameter)
{
	
	// 创建任务
	xTaskCreate(HeartSpo2_getvalue_task, "HeartSpo2 Sensor", 256, NULL, 5, NULL);
	
	vTaskDelete(NULL);
}

/* mpu6050任务函数 */
static void app_task_mpu6050(void *pvParameter)
{
	// 创建任务
	xTaskCreate(mpu6050_runmode_task, "mpu6050 Sensor", 256, NULL, 5, NULL);	
	xTaskCreate(Send_stepcnt_task, "StepCnt Uptophone", 256, NULL, 5, NULL);
	xTaskCreate(Update_StepNum_ui, "Update StepNum", 256, NULL, 5, NULL);
	vTaskDelete(NULL);
}




// 蓝牙任务函数
static void app_task_bluetooth(void *pvParameter)
{
	bluetoothTask_init();
	// 蓝牙状态监控任务
	xTaskCreate(bluetooth_status_task, "BT Status", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
	// UI更新任务
	xTaskCreate(bluetooth_ui_update_task, "BT UI Update", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
	// 蓝牙数据处理任务
	xTaskCreate(bluetooth_data_task, "BT Data", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
	
	vTaskDelete(NULL);
}

// 屏幕控制任务
void screen_control_task(void *pvParameters)
{
	screen_control_init();
	xTaskCreate(screen_event_task, "ScreenEvent", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

	vTaskDelete(NULL);
}


/*-------------------------END-------------------------------*/

/* lv_timer的回调函数 */
/*------------------------START-------------------------------*/
void lv_timer_back()
{
	// 获取RTC时钟信息
	RTC_TimeTypeDef RTC_TimeStructure;
	RTC_DateTypeDef RTC_DateStructure;
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);

	// 更新时间标签
	lv_label_set_text_fmt(ui_LabelTime, "%02d:%02d:%02d",
						  RTC_TimeStructure.RTC_Hours,
						  RTC_TimeStructure.RTC_Minutes,
						  RTC_TimeStructure.RTC_Seconds);

	// 更新日期标签
	lv_label_set_text_fmt(ui_LabelDate, "20%02d-%02d-%02d",
						  RTC_DateStructure.RTC_Year,
						  RTC_DateStructure.RTC_Month,
						  RTC_DateStructure.RTC_Date);

	// 更新星期标签
	const char *week_days[] = {
		"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

	lv_label_set_text_fmt(ui_LabelWeek, "%s", week_days[RTC_DateStructure.RTC_WeekDay - 1]);
}

/*-------------------------END-------------------------------*/



void vApplicationMallocFailedHook(void)
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for (;;)
		;
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook(void)
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
	(void)pcTaskName;
	(void)pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for (;;)
		;
}

void vApplicationTickHook(void)
{
}
