#ifndef __DHT11_H
#define __DHT11_H

#include "stdint.h"

#define PGin(n) (*(volatile uint32_t *)(0x42000000 + (GPIOG_BASE + 0x10 - 0x40000000) * 32 + (n) * 4))
#define PGout(n) (*(volatile uint32_t *)(0x42000000 + (GPIOG_BASE + 0x14 - 0x40000000) * 32 + (n) * 4))

extern void dht11_init(void);
int8_t dht11_read(uint8_t *buf);

#endif