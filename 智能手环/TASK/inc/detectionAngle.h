#ifndef __DETECTION_ANGLE_H__
#define __DETECTION_ANGLE_H__

#define MPU_QUEUE_LENGTH 10
#define MPU_POLL_DELAY pdMS_TO_TICKS(50) // 50ms
#define SCREEN_TILT_THRESHOLD -45
#define DISPLAY_ON_BIT (1 << 0)

#include <stdio.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "event_groups.h"
#include "tft.h"
#include "MPU6050.h"
#include "inv_mpu.h"

typedef struct
{
    float pitch, roll, yaw;    // 欧拉角
    short gyroX, gyroY, gyroZ; // 陀螺仪原始数据
    short accX, accY, accZ;    // 加速度传感器原始数据
    short temp;                // 温度
} mpu_data_t;

extern void detectionAngle_init(void);
extern void mpu_sensor_task(void *pvParameters);
extern void mpu_processing_task(void *pvParameters);
extern void system_control_task(void *pvParameters);

#endif // !__DETECTION_ANGLE_H__
