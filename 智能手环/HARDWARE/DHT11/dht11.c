#include "dht11.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "delay.h"

extern GPIO_InitTypeDef GPIO_InitStructure;

void dht11_init(void)
{
    /* 使能端口G的硬件时钟，实际上就是对端口G供电 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

    /* 对GPIOG的9 号引脚配置为开漏输出模式*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;        // 9 号引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;    // 输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // 引脚工作速度2MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;   // 开漏，Open Drain
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOG, &GPIO_InitStructure);

    GPIO_WriteBit(GPIOG, GPIO_Pin_9, Bit_SET);
}

int8_t dht11_read(uint8_t *buf)
{
    uint32_t t = 0;
    int32_t i, j;
    uint8_t d = 0;

    /* 对GPIOG的9 号引脚配置为开漏输出模式*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;        // 9 号引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;    // 输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // 引脚工作速度2MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;   // 开漏，Open Drain
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    PGout(9) = 0;
    delay_ms(18);

    PGout(9) = 1;
    delay_us(30);

    /* 对GPIOG的9 号引脚配置为输入模式*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;        // 9 号引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;     // 输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // 引脚工作速度2MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;   // 开漏，Open Drain
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    /* 等待dht11的响应信号低电平出现 */
    t = 0;
    while (PGin(9))
    {
        t++;
        delay_us(1);
        if (t >= 4000)
            return -1;
    }

    /* 测量dht11的响应信号低电平持续是否合法 */
    t = 0;
    while (PGin(9) == 0)
    {
        t++;
        delay_us(1);
        if (t >= 100)
            return -2;
    }

    /* 测量dht11的响应信号高电平持续是否合法 */
    t = 0;
    while (PGin(9))
    {
        t++;
        delay_us(1);
        if (t >= 100)
            return -3;
    }

    /* 循环接收5个字节 */
    for (j = 0; j < 5; j++)
    {
        /* 循环接收一个字节，最高有效位优先传输 */
        d = 0;
        for (i = 7; i >= 0; i--)
        {

            /* 测量dht11的数据低电平持续是否合法 */
            t = 0;
            while (PGin(9) == 0)
            {
                t++;
                delay_us(1);
                if (t >= 100)
                    return -4;
            }

            delay_us(40);

            if (PGin(9))
            {
                d |= 1 << i;

                /* 等待dht11的数据1等下的高电平持续完毕 */
                t = 0;
                while (PGin(9))
                {
                    t++;
                    delay_us(1);
                    if (t >= 100)
                        return -5;
                }
            }
        }

        buf[j] = d;
    }

    /* 接收完5个字节后，延时50us */
    delay_us(50);

    if (((buf[0] + buf[1] + buf[2] + buf[3]) & 0xFF) != buf[4])
        return -6;

    return 0;
}
