#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__
#include "includes.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "bluetoothTask.h"

extern EventGroupHandle_t xBluetoothEventGroup;

extern void bluetooth_init(uint32_t baud);
extern void Bluetooth_SendData(const char *str);

#endif
