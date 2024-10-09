/*��    ��:��Ƕ.�¹�*/
#include "includes.h"

#define RED 	0XF800   	// 红色
#define GREEN 	0X07E0 		// 绿色
#define GREY 	0X001F  	// 蓝色
#define WHITE 	0XFFFF 		// 白色
#define BLACK 	0X0000 		// 黑色



uint32_t g_lcd_width = LCD_WIDTH;
uint32_t g_lcd_height = LCD_HEIGHT;
uint32_t g_lcd_direction = 0;


void spi1_send_byte(uint8_t byte)
{
#if LCD_SOFT_SPI_ENABLE
	unsigned char counter;

	for (counter = 0; counter < 8; counter++)
	{
		SPI_SCK_0;
		if ((byte & 0x80) == 0)
		{
			SPI_SDA_0;
		}
		else
			SPI_SDA_1;
		byte = byte << 1;
		SPI_SCK_1;
	}
	SPI_SCK_0;

#else
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
		;
	SPI_I2S_SendData(SPI1, byte);

	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
		;
	SPI_I2S_ReceiveData(SPI1);

#endif
}

void lcd_send_cmd(uint8_t cmd)
{
	SPI_CS_0;
	LCD_DC_0;
	spi1_send_byte(cmd);
	SPI_CS_1;
}

// ��Һ����дһ��8λ����
void lcd_send_data(uint8_t dat)
{
	SPI_CS_0;
	LCD_DC_1;
	spi1_send_byte(dat);
	SPI_CS_1;
}

void lcd_addr_set(uint32_t x_s, uint32_t y_s, uint32_t x_e, uint32_t y_e)
{
	/* ����tft����Ҫƫ���� */

	if (lcd_get_direction() == 0 || lcd_get_direction() == 2)
	{
		y_s = y_s + Y_OFFSET;
		y_e = y_e + Y_OFFSET;
	}

	if (lcd_get_direction() == 1 || lcd_get_direction() == 3)
	{
		x_s = x_s + X_OFFSET;
		x_e = x_e + X_OFFSET;
	}

	lcd_send_cmd(0x2a);		 // �е�ַ����
	lcd_send_data(x_s >> 8); // ��ʼ��
	lcd_send_data(x_s);
	lcd_send_data(x_e >> 8); // ������
	lcd_send_data(x_e);

	lcd_send_cmd(0x2b);		 // �е�ַ����
	lcd_send_data(y_s >> 8); // ��ʼ��
	lcd_send_data(y_s);
	lcd_send_data(y_e >> 8); // ������
	lcd_send_data(y_e);
	lcd_send_cmd(0x2C); // д�Դ�
}

void lcd_fill(uint32_t x_s, uint32_t y_s, uint32_t x_len, uint32_t y_len, uint32_t color)
{

	uint32_t x, y;

	lcd_addr_set(x_s, y_s, x_s + x_len - 1, y_s + y_len - 1);

	SPI_CS_0;
	LCD_DC_1;

	for (y = y_s; y < y_s + y_len; y++)
	{
		for (x = x_s; x < x_s + x_len; x++)
		{

			spi1_send_byte(color >> 8);
			spi1_send_byte(color);
		}
	}

	SPI_CS_1;
}

void lcd_clear(uint32_t color)
{
	lcd_fill(0, 0, g_lcd_width, g_lcd_height, color);
}

void lcd_draw_picture(uint32_t x_s, uint32_t y_s, uint32_t width, uint32_t height, const uint8_t *pic)
{

	const uint8_t *p = pic;
	uint32_t i = 0;

	lcd_addr_set(x_s, y_s, x_s + width - 1, y_s + height - 1);

	SPI_CS_0;
	LCD_DC_1;
	for (i = 0; i < width * height * 2; i += 2)
	{

		spi1_send_byte(p[i]);
		spi1_send_byte(p[i + 1]);
	}
	SPI_CS_1;
}

void lcd_show_char(uint32_t x, uint32_t y, uint8_t ch, uint32_t fc, uint32_t bc, uint32_t font_size, uint32_t mode)
{

	u8 temp, sizex, t, m = 0;
	u16 i, TypefaceNum; // һ���ַ���ռ�ֽڴ�С

	u16 x0 = x;

	sizex = font_size / 2;

	TypefaceNum = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * font_size;

	ch = ch - ' '; // �õ�ƫ�ƺ��ֵ

	lcd_addr_set(x, y, x + sizex - 1, y + font_size - 1); // ���ù��λ��

	for (i = 0; i < TypefaceNum; i++)
	{
		if (font_size == 12)
			temp = ascii_1206[ch][i]; // ����6x12����
		else if (font_size == 16)
			temp = ascii_1608[ch][i]; // ����8x16����
		else if (font_size == 24)
			temp = ascii_2412[ch][i]; // ����12x24����
		else if (font_size == 32)
			temp = ascii_3216[ch][i]; // ����16x32����
		else
			return;
		for (t = 0; t < 8; t++)
		{
			if (!mode) // �ǵ���ģʽ
			{
				if (temp & (0x01 << t))
				{
					lcd_send_data(fc >> 8);
					lcd_send_data(fc);
				}
				else
				{
					lcd_send_data(bc >> 8);
					lcd_send_data(bc);
				}
				m++;
				if (m % sizex == 0)
				{
					m = 0;
					break;
				}
			}
			else // ����ģʽ
			{
				if (temp & (0x01 << t))
					lcd_draw_point(x, y, fc); // ��һ����
				x++;
				if ((x - x0) == sizex)
				{
					x = x0;
					y++;
					break;
				}
			}
		}
	}
}

void lcd_show_string(u16 x, u16 y, const u8 *p, u16 fc, u16 bc, u8 font_size, u8 mode)
{
	while (*p != '\0')
	{
		lcd_show_char(x, y, *p, fc, bc, font_size, mode);
		x += font_size / 2;
		p++;
	}
}

/******************************************************************************
 *����˵������ʾ����
 *������ݣ�m������nָ��
 *����ֵ��  ��
 ******************************************************************************/
u32 mypow(u8 m, u8 n)
{
	u32 result = 1;
	while (n--)
		result *= m;
	return result;
}

/******************************************************************************
*����˵������ʾ��������
*������ݣ�x,y��ʾ����
	num Ҫ��ʾ��������
	len Ҫ��ʾ��λ��
	fc �ֵ���ɫ
	bc �ֵı���ɫ
	font_size �ֺ�
*����ֵ��  ��
******************************************************************************/
void lcd_show_integer(uint32_t x, uint32_t y, uint32_t num, uint32_t len, uint32_t fc, uint32_t bc, uint32_t font_size)
{
	u8 t, temp;
	u8 enshow = 0;
	u8 sizex = font_size / 2;
	for (t = 0; t < len; t++)
	{
		temp = (num / mypow(10, len - t - 1)) % 10;
		if (enshow == 0 && t < (len - 1))
		{
			if (temp == 0)
			{
				lcd_show_char(x + t * sizex, y, ' ', fc, bc, font_size, 0);
				continue;
			}
			else
				enshow = 1;
		}
		lcd_show_char(x + t * sizex, y, temp + 48, fc, bc, font_size, 0);
	}
}

/******************************************************************************
*����˵������ʾ��λС������
*������ݣ�x,y��ʾ����
		num Ҫ��ʾС������
		len Ҫ��ʾ��λ��
		fc �ֵ���ɫ
		bc �ֵı���ɫ
		font_size �ֺ�
*����ֵ��  ��
******************************************************************************/
void lcd_show_float(uint32_t x, uint32_t y, float num, uint32_t len, uint32_t fc, uint32_t bc, uint32_t font_size)
{
	uint32_t t, temp, sizex;
	uint32_t num1;
	sizex = font_size / 2;
	num1 = num * 100;
	for (t = 0; t < len; t++)
	{
		temp = (num1 / mypow(10, len - t - 1)) % 10;
		if (t == (len - 2))
		{
			lcd_show_char(x + (len - 2) * sizex, y, '.', fc, bc, font_size, 0);
			t++;
			len += 1;
		}
		lcd_show_char(x + t * sizex, y, temp + 48, fc, bc, font_size, 0);
	}
}

void lcd_init(void) ////ST7789V2
{

#if LCD_SOFT_SPI_ENABLE

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);

	// ���ŵ�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	   // ���ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // �ٶ����ø���
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	   // ���������Push Pull��ʹ����PMOS����NMOS��
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   // ��ʹ������������
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_9 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	   // ���ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // �ٶ����ø���
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	   // ���������Push Pull��ʹ����PMOS����NMOS��
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;   // ��ʹ������������
	GPIO_Init(GPIOE, &GPIO_InitStructure);

#else
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	// SCK=PB3,  MOSI=PB5
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	 // ���ù���ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed; // ���Ÿ��ٹ������յ�ָ������������ȱ�㣺���ĸ�
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	 // �����������������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // ����Ҫ����������
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	 // ���ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed; // ���Ÿ��ٹ������յ�ָ������������ȱ�㣺���ĸ�
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	 // �����������������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // ����Ҫ����������
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	 // ���ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed; // ���Ÿ��ٹ������յ�ָ������������ȱ�㣺���ĸ�
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	 // �����������������
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // ����Ҫ����������
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	// PB3 PB5���ӵ�SPI1Ӳ��
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);

	// �ر�SPI1
	SPI_Cmd(SPI1, DISABLE);

	// ����SPI
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // ȫ˫���շ�
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;					   // ��Ϊ����
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;				   // 8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;						   // ����ʱʱ��Ϊ��
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;					   // ��1��ʱ���ز�������
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;						   // CS��SSIλ���ƣ��Կأ�
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2; // ������Ϊ2��Ƶ
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;				   // ��λ�ȴ���
	SPI_InitStructure.SPI_CRCPolynomial = 0;						   // ��ʹ��CRC
	SPI_Init(SPI1, &SPI_InitStructure);								   // ��ʼ��SPI1

	// ����SPI1
	SPI_Cmd(SPI1, ENABLE);

#endif

	// ��1.69Ӣ����Ļ����Ϊ�͵�ƽ����
	LCD_BLK_0;

	// SPI_SCK_0;

	SPI_SCK_1; // �ر�ע�⣡��
	LCD_RST_0;
	delay_ms(100);
	LCD_RST_1;
	delay_ms(100);
	lcd_send_cmd(0x11); // Sleep Out
	delay_ms(120);		// DELAY120ms
	//-----------------------ST7789V Frame rate setting-----------------//
	//************************************************
	lcd_send_cmd(0x3A); // 65k mode
	lcd_send_data(0x05);
	lcd_send_cmd(0xC5); // VCOM
	lcd_send_data(0x1A);
	lcd_send_cmd(0x36); // ��Ļ��ʾ��������
	lcd_send_data(0x00);
	//-------------ST7789V Frame rate setting-----------//
	lcd_send_cmd(0xb2); // Porch Setting
	lcd_send_data(0x05);
	lcd_send_data(0x05);
	lcd_send_data(0x00);
	lcd_send_data(0x33);
	lcd_send_data(0x33);

	lcd_send_cmd(0xb7);	 // Gate Control
	lcd_send_data(0x05); // 12.2v   -10.43v
	//--------------ST7789V Power setting---------------//
	lcd_send_cmd(0xBB); // VCOM
	lcd_send_data(0x3F);

	lcd_send_cmd(0xC0); // Power control
	lcd_send_data(0x2c);

	lcd_send_cmd(0xC2); // VDV and VRH Command Enable
	lcd_send_data(0x01);

	lcd_send_cmd(0xC3);	 // VRH Set
	lcd_send_data(0x0F); // 4.3+( vcom+vcom offset+vdv)

	lcd_send_cmd(0xC4);	 // VDV Set
	lcd_send_data(0x20); // 0v

	lcd_send_cmd(0xC6);	 // Frame Rate Control in Normal Mode
	lcd_send_data(0X01); // 111Hz

	lcd_send_cmd(0xd0); // Power Control 1
	lcd_send_data(0xa4);
	lcd_send_data(0xa1);

	lcd_send_cmd(0xE8); // Power Control 1
	lcd_send_data(0x03);

	lcd_send_cmd(0xE9); // Equalize time control
	lcd_send_data(0x09);
	lcd_send_data(0x09);
	lcd_send_data(0x08);
	//---------------ST7789V gamma setting-------------//
	lcd_send_cmd(0xE0); // Set Gamma
	lcd_send_data(0xD0);
	lcd_send_data(0x05);
	lcd_send_data(0x09);
	lcd_send_data(0x09);
	lcd_send_data(0x08);
	lcd_send_data(0x14);
	lcd_send_data(0x28);
	lcd_send_data(0x33);
	lcd_send_data(0x3F);
	lcd_send_data(0x07);
	lcd_send_data(0x13);
	lcd_send_data(0x14);
	lcd_send_data(0x28);
	lcd_send_data(0x30);

	lcd_send_cmd(0XE1); // Set Gamma
	lcd_send_data(0xD0);
	lcd_send_data(0x05);
	lcd_send_data(0x09);
	lcd_send_data(0x09);
	lcd_send_data(0x08);
	lcd_send_data(0x03);
	lcd_send_data(0x24);
	lcd_send_data(0x32);
	lcd_send_data(0x32);
	lcd_send_data(0x3B);
	lcd_send_data(0x14);
	lcd_send_data(0x13);
	lcd_send_data(0x28);
	lcd_send_data(0x2F);

	lcd_send_cmd(0x21); // ����

	lcd_send_cmd(0x29); // ������ʾ
}

void lcd_show_chn(uint32_t x, uint32_t y, uint8_t no, uint32_t fc, uint32_t bc, uint32_t font_size)
{
	uint32_t i, j;
	uint8_t tmp;

	lcd_addr_set(x, y, x + font_size - 1, y + font_size - 1);

	for (i = 0; i < (font_size * font_size / 8); i++) // column loop
	{
		if (font_size == 16)
			tmp = chinese_tbl_16[no][i];
		if (font_size == 24)
			tmp = chinese_tbl_24[no][i];
		if (font_size == 32)
			tmp = chinese_tbl_32[no][i];

		for (j = 0; j < 8; j++)
		{
			if (tmp & (1 << j))
			{
				lcd_send_data(fc >> 8);
				lcd_send_data(fc);
			}

			else
			{
				lcd_send_data(bc);
				lcd_send_data(bc);
			}
		}
	}
}

void lcd_draw_point(uint32_t x, uint32_t y, uint32_t color)
{
	lcd_addr_set(x, y, x, y);

	lcd_send_data(color >> 8);
	lcd_send_data(color);
}

void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	uint16_t t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;
	delta_x = x2 - x1; // ������������
	delta_y = y2 - y1;
	uRow = x1; // �����������
	uCol = y1;
	if (delta_x > 0)
		incx = 1; // ���õ�������
	else if (delta_x == 0)
		incx = 0; // ��ֱ��
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}
	if (delta_y > 0)
		incy = 1;
	else if (delta_y == 0)
		incy = 0; // ˮƽ��
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}
	if (delta_x > delta_y)
		distance = delta_x; // ѡȡ��������������
	else
		distance = delta_y;
	for (t = 0; t < distance + 1; t++)
	{
		lcd_draw_point(uRow, uCol, color); // ����
		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if (yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
}

void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	lcd_draw_line(x1, y1, x2, y1, color);
	lcd_draw_line(x1, y1, x1, y2, color);
	lcd_draw_line(x1, y2, x2, y2, color);
	lcd_draw_line(x2, y1, x2, y2, color);
}

void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
	int a, b;

	a = 0;
	b = r;

	while (a <= b)
	{
		lcd_draw_point(x0 - b, y0 - a, color); // 3
		lcd_draw_point(x0 + b, y0 - a, color); // 0
		lcd_draw_point(x0 - a, y0 + b, color); // 1
		lcd_draw_point(x0 - a, y0 - b, color); // 2
		lcd_draw_point(x0 + b, y0 + a, color); // 4
		lcd_draw_point(x0 + a, y0 - b, color); // 5
		lcd_draw_point(x0 + a, y0 + b, color); // 6
		lcd_draw_point(x0 - b, y0 + a, color); // 7
		a++;
		if ((a * a + b * b) > (r * r)) // �ж�Ҫ���ĵ��Ƿ��Զ
		{
			b--;
		}
	}
}

void lcd_set_direction(uint32_t dir)
{
	g_lcd_direction = dir;

	/* 0��*/
	if (dir == 0)
	{
		lcd_send_cmd(0x36);
		lcd_send_data(0x00);
		g_lcd_width = LCD_WIDTH;
		g_lcd_height = LCD_HEIGHT;
	}

	/* 90��*/
	if (dir == 1)
	{
		lcd_send_cmd(0x36);
		lcd_send_data((1 << 6) | (1 << 5) | (1 << 4));
		g_lcd_width = LCD_HEIGHT;
		g_lcd_height = LCD_WIDTH;
	}

	/* 180��*/
	if (dir == 2)
	{
		lcd_send_cmd(0x36);
		lcd_send_data((1 << 7) | (1 << 6));
		g_lcd_width = LCD_WIDTH;
		g_lcd_height = LCD_HEIGHT;
	}

	/* 270��*/
	if (dir == 3)
	{
		lcd_send_cmd(0x36);
		lcd_send_data((1 << 7) | (0 << 6) | (1 << 5) | (1 << 4));
		g_lcd_width = LCD_HEIGHT;
		g_lcd_height = LCD_WIDTH;
	}
}

// lcd��ʾ��/�ر�
void lcd_display_on(uint16_t on)
{
	if (on)
		lcd_send_cmd(0x29); // ����
	else
		lcd_send_cmd(0x28); // ����
}

uint32_t lcd_get_direction(void)
{
	return g_lcd_direction;
}

void spi1_tx_dma_init(uint32_t DMA_Memory0BaseAddr, uint16_t DMA_BufferSize, uint32_t DMA_MemoryDataSize, uint32_t DMA_MemoryInc)
{
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE); // DMA2ʱ��ʹ��

	DMA_DeInit(DMA2_Stream3);

	// �ȴ�DMA2_Stream1������
	while (DMA_GetCmdStatus(DMA2_Stream3) != DISABLE)
		;

	/* ���� DMA Stream */
	DMA_InitStructure.DMA_Channel = DMA_Channel_3;							// ͨ��3 SPI1ͨ��
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;			// �����ַΪ:SPI1->DR
	DMA_InitStructure.DMA_Memory0BaseAddr = DMA_Memory0BaseAddr;			// DMA �洢��0��ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;					// �洢��������ģʽ
	DMA_InitStructure.DMA_BufferSize = DMA_BufferSize;						// ���ݴ�����
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// ���������ģʽ
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc;						// �洢������ģʽ
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // �������ݳ���:8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize;				// �洢�����ݳ���:8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							// ����ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;						// �����ȼ�
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;					// ����FIFOģʽ
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;			//
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;				// ����ͻ�����δ���
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;		// �洢��ͻ�����δ���
	DMA_Init(DMA2_Stream3, &DMA_InitStructure);

	DMA_ClearFlag(DMA2_Stream3, DMA_FLAG_TCIF3);

	DMA_Cmd(DMA2_Stream3, ENABLE);

	/* ������������ж�  */
	DMA_ITConfig(DMA2_Stream3, DMA_IT_TC, ENABLE);

	// �жϳ�ʼ��
	/* DMA�����ж�Դ */
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream3_IRQn;
	/* �������ȼ� */
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY;
	/* ��Ӧ���ȼ� */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	/* ʹ���ⲿ�ж�ͨ�� */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	/* ����NVIC */
	NVIC_Init(&NVIC_InitStructure);
}

void spi1_tx_dma_start(void)
{
	DMA_Cmd(DMA2_Stream3, ENABLE);
}

void spi1_tx_dma_stop(void)
{
	DMA_Cmd(DMA2_Stream3, DISABLE);
}

extern lv_disp_drv_t *g_disp_drvp;

void DMA2_Stream3_IRQHandler(void)
{
	// DMA �������
	if (DMA_GetITStatus(DMA2_Stream3, DMA_IT_TCIF3))
	{
		// ���DMA������ɱ�־
		DMA_ClearITPendingBit(DMA2_Stream3, DMA_IT_TCIF3);

		// Ƭѡ���ߣ����ݷ������
		SPI_CS_1;

		lv_disp_flush_ready(g_disp_drvp);
	}
}
