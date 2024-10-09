/*��    ��:��Ƕ.�¹� */

#include "includes.h"

/*�������ýṹ��*/
static NVIC_InitTypeDef NVIC_InitStructure;
static EXTI_InitTypeDef EXTI_InitStructure;
static GPIO_InitTypeDef GPIO_InitStructure;

volatile uint32_t g_tp_event = 0;

uint16_t g_tp_x, g_tp_y;
uint8_t g_tp_finger_num = 0;

extern EventGroupHandle_t screenEventGroup;
extern bool is_screen_off;
extern TickType_t last_activity_time;

#define SCREEN_ON_BIT (1 << 0)
#define SCREEN_OFF_BIT (1 << 1)

void tp_sda_pin_mode(GPIOMode_TypeDef pin_mode)
{
#if TP_PIN_DEF == TP_PIN_DEF_1

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;		   // ��14������
	GPIO_InitStructure.GPIO_Mode = pin_mode;		   // �������/����ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;	   // ��©ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // ����IO���ٶ�Ϊ100MHz��Ƶ��Խ������Խ�ã�Ƶ��Խ�ͣ�����Խ��
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   // ����Ҫ��������
	GPIO_Init(GPIOD, &GPIO_InitStructure);

#endif

#if TP_PIN_DEF == TP_PIN_DEF_2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;		   // ��7������
	GPIO_InitStructure.GPIO_Mode = pin_mode;		   // �������/����ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;	   // ��©ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // ����IO���ٶ�Ϊ100MHz��Ƶ��Խ������Խ�ã�Ƶ��Խ�ͣ�����Խ��
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   // ����Ҫ��������
	GPIO_Init(GPIOD, &GPIO_InitStructure);

#endif
}
void tp_i2c_start(void)
{
	// ��֤SDA����Ϊ���ģʽ
	tp_sda_pin_mode(GPIO_Mode_OUT);

	TP_SCL_W = 1;
	TP_SDA_W = 1;
	delay_us(1);

	TP_SDA_W = 0;
	delay_us(1);

	TP_SCL_W = 0;
	delay_us(1);
}

void tp_i2c_stop(void)
{
	// ��֤SDA����Ϊ���ģʽ
	tp_sda_pin_mode(GPIO_Mode_OUT);

	TP_SCL_W = 1;
	TP_SDA_W = 0;
	delay_us(1);

	TP_SDA_W = 1;
	delay_us(1);
}

void tp_i2c_send_byte(uint8_t byte)
{
	int32_t i;

	// ��֤SDA����Ϊ���ģʽ
	tp_sda_pin_mode(GPIO_Mode_OUT);

	TP_SCL_W = 0;
	TP_SDA_W = 0;
	delay_us(1);

	for (i = 7; i >= 0; i--)
	{
		if (byte & (1 << i))
			TP_SDA_W = 1;
		else
			TP_SDA_W = 0;

		delay_us(1);

		TP_SCL_W = 1;
		delay_us(1);

		TP_SCL_W = 0;
		delay_us(1);
	}
}

void tp_i2c_ack(uint8_t ack)
{
	// ��֤SDA����Ϊ���ģʽ
	tp_sda_pin_mode(GPIO_Mode_OUT);

	TP_SCL_W = 0;
	TP_SDA_W = 0;
	delay_us(1);

	if (ack)
		TP_SDA_W = 1;
	else
		TP_SDA_W = 0;

	delay_us(1);

	TP_SCL_W = 1;
	delay_us(1);

	TP_SCL_W = 0;
	delay_us(1);
}

uint8_t tp_i2c_wait_ack(void)
{
	uint8_t ack;
	// ��֤SDA����Ϊ����ģʽ
	tp_sda_pin_mode(GPIO_Mode_IN);

	// �����ŵھŸ�ʱ�����ڣ���SCL����
	TP_SCL_W = 1;
	delay_us(1);

	if (TP_SDA_R)
		ack = 1;
	else
		ack = 0;

	// ��������ռ������
	TP_SCL_W = 0;
	delay_us(1);

	return ack;
}

uint8_t tp_i2c_recv_byte(void)
{

	uint8_t d = 0;
	int32_t i;

	// ��֤SDA����Ϊ����ģʽ
	tp_sda_pin_mode(GPIO_Mode_IN);

	for (i = 7; i >= 0; i--)
	{
		TP_SCL_W = 1;
		delay_us(1);

		if (TP_SDA_R)
			d |= 1 << i;

		// ��������ռ������
		TP_SCL_W = 0;
		delay_us(1);
	}

	return d;
}

void tp_lowlevel_init(void)
{

#if TP_PIN_DEF == TP_PIN_DEF_1

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

	// ʹ��ϵͳ���õ�Ӳ��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_4 | GPIO_Pin_14; // ��0 4 14������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;						 // �������ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;						 // ��©ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;					 // ����IO���ٶ�Ϊ100MHz��Ƶ��Խ������Խ�ã�Ƶ��Խ�ͣ�����Խ��
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;					 // ����Ҫ��������
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	// ֻҪ�����ģʽ���г�ʼ��ƽ״̬
	TP_SCL_W = 1;
	TP_SDA_W = 1;

	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOF, &GPIO_InitStructure);

	/* �����ⲿ�ж�12��صĲ��� */
	EXTI_InitStructure.EXTI_Line = EXTI_Line12;				// �ⲿ�ж�12
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		// �ж�
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // ��ⰴ���İ��£�ʹ���½��ش���
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;				// ʹ��
	EXTI_Init(&EXTI_InitStructure);

	/* NVIC���ⲿ�ж�12��ͨ������Ϊ���������ȼ� */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;												 // �жϺ�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY; // ��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;												 // ��Ӧ(��)���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;														 // �򿪸�ͨ��
	NVIC_Init(&NVIC_InitStructure);
	// ��ʼ��
	TP_RST = 1;

#endif

#if TP_PIN_DEF == TP_PIN_DEF_2

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	// ʹ��ϵͳ���õ�Ӳ��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; // ��6 7������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		   // �������ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		   // ��©ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	   // ����IO���ٶ�Ϊ100MHz��Ƶ��Խ������Խ�ã�Ƶ��Խ�ͣ�����Խ��
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	   // ����Ҫ��������
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	// ֻҪ�����ģʽ���г�ʼ��ƽ״̬
	TP_SCL_W = 1;
	TP_SDA_W = 1;

	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource8);

	/* �����ⲿ�ж�8��صĲ��� */
	EXTI_InitStructure.EXTI_Line = EXTI_Line8;				// �ⲿ�ж�8
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		// �ж�
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // ��ⰴ���İ��£�ʹ���½��ش���
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;				// ʹ��
	EXTI_Init(&EXTI_InitStructure);

	/* NVIC���ⲿ�ж�0��ͨ������Ϊ���������ȼ� */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;													 // �жϺ�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY; // ��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;												 // ��Ӧ(��)���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;														 // �򿪸�ͨ��
	NVIC_Init(&NVIC_InitStructure);

	// ��ʼ��
	TP_RST = 1;

#endif
}

void tp_send_byte(uint8_t addr, uint8_t *data)
{
	tp_i2c_start();
	tp_i2c_send_byte(0x2A);
	tp_i2c_wait_ack();
	tp_i2c_send_byte(addr);
	tp_i2c_wait_ack();
	tp_i2c_send_byte(*data);
	tp_i2c_wait_ack();
	tp_i2c_stop();
}

void tp_recv_byte(uint8_t addr, uint8_t *data)
{
	tp_i2c_start();
	tp_i2c_send_byte(0x2A);
	tp_i2c_wait_ack();
	tp_i2c_send_byte(addr);
	tp_i2c_wait_ack();
	tp_i2c_start();
	tp_i2c_send_byte(0x2B);
	tp_i2c_wait_ack();
	*data = tp_i2c_recv_byte();
	tp_i2c_ack(1);
	tp_i2c_stop();
}

void tp_recv(uint8_t addr, uint8_t *data, uint32_t len)
{
	uint8_t *p = data;

	tp_i2c_start();
	tp_i2c_send_byte(0x2A);
	tp_i2c_wait_ack();
	tp_i2c_send_byte(addr);
	tp_i2c_wait_ack();
	tp_i2c_start();
	tp_i2c_send_byte(0x2B);
	tp_i2c_wait_ack();

	len = len - 1;

	while (len--)
	{
		*p = tp_i2c_recv_byte();
		tp_i2c_ack(0);
		p++;
	}

	*p = tp_i2c_recv_byte();
	tp_i2c_ack(1);

	tp_i2c_stop();
}

void tp_reset(void)
{
	TP_RST = 0;
	delay_ms(10);

	TP_RST = 1;
	delay_ms(60);
}

void tp_init(void)
{
	uint8_t Data = 0;
	uint8_t ChipID = 0;
	uint8_t FwVersion = 0;

	tp_lowlevel_init();

	tp_reset(); // оƬ�ϵ��ʼ��
	tp_recv_byte(0xa7, &ChipID);
	tp_recv_byte(0xa9, &FwVersion);
}

uint8_t tp_finger_num_get(void)
{
	return g_tp_finger_num;
}

uint8_t tp_read(uint16_t *screen_x, uint16_t *screen_y)
{
	uint8_t buf[7];
	uint16_t x = 0, y = 0, tmp;

	tp_recv(0, buf, 7);

	x = (uint16_t)((buf[3] & 0x0F) << 8) | buf[4];
	y = (uint16_t)((buf[5] & 0x0F) << 8) | buf[6];

	g_tp_finger_num = buf[2];

	if ((x < g_lcd_width) && (y < g_lcd_height))
	{

		if (lcd_get_direction() == 1)
		{
			tmp = x;
			x = y;
			y = g_lcd_height - tmp;
		}

		if (lcd_get_direction() == 2)
		{
			x = g_lcd_width - x;
			y = g_lcd_height - y;
		}

		if (lcd_get_direction() == 3)
		{
			tmp = y;
			y = x;
			x = g_lcd_width - tmp;
		}

		*screen_x = x;
		*screen_y = y;

		/*
			0x00��������
			0x01���»�
			0x02���ϻ�
			0x03����
			0x04���һ�
			0x05������
			0x0B��˫��
			0x0C������
		*/

		return buf[1];
	}

	return 0;
}

#if TP_PIN_DEF == TP_PIN_DEF_1
void EXTI15_10_IRQHandler(void)
{
	// ��ȡ�ⲿ�ж�12�Ƿ��д���
	if (EXTI_GetITStatus(EXTI_Line12) != RESET)
	{
		/* �����¼�����*/
		g_tp_event = 1;

		/* tp_read���жϵ���ʵʱ�Ի���ã����ᵼ�������ж��ӳ� */
		// tp_read(&g_tp_x,&g_tp_y);

		/* ����ⲿ�ж�12�ı�־λ������CPU�Ѿ�������� */
		EXTI_ClearITPendingBit(EXTI_Line12);
	}
}
#endif

#if TP_PIN_DEF == TP_PIN_DEF_2
void EXTI9_5_IRQHandler(void)
{
	// ��ȡ�ⲿ�ж�8�Ƿ��д���
	if (EXTI_GetITStatus(EXTI_Line8) != RESET)
	{
		/* �����¼�����*/

		g_tp_event = 1;

		/* tp_read���жϵ���ʵʱ�Ի���ã����ᵼ�������ж��ӳ� */
		// tp_read(&g_tp_x,&g_tp_y);

		/* ����ⲿ�ж�8�ı�־λ������CPU�Ѿ�������� */
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
}
#endif