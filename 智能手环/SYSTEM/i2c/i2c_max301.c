#include "stm32f4xx.h"
#include "sys.h"
#include "i2c_max301.h"
#include "delay.h"
#include "sys.h"

static GPIO_InitTypeDef GPIO_InitStructure;


void i2c_init(void)
{

	//ʹ�ܶ˿�B��Ӳ��ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	


	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8|GPIO_Pin_9;	//�� 8 9 ������
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT;		//���ģʽ
	GPIO_InitStructure.GPIO_Speed=GPIO_High_Speed;		//���Ÿ��ٹ������յ�ָ������������ȱ�㣺���ĸ�
	GPIO_InitStructure.GPIO_OType=GPIO_OType_OD;		//��©
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;		//����Ҫ����������
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	
	//ֻҪ�����ģʽ���϶����г�ʼ��ƽ״̬����ʱ��ͼ������״̬Ϊ�ߵ�ƽ
	SCL_W=1;
	SDA_W=1;

}

void sda_pin_mode(GPIOMode_TypeDef pin_mode)
{
//����Ӳ��������GPIO���˿�F����9������
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;			//��9 ������
	GPIO_InitStructure.GPIO_Mode=pin_mode;				//���ģʽ
	GPIO_InitStructure.GPIO_Speed=GPIO_High_Speed;		//���Ÿ��ٹ������յ�ָ������������ȱ�㣺���ĸ�
	GPIO_InitStructure.GPIO_OType=GPIO_OType_OD;		//��©
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_NOPULL;		//����Ҫ����������
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}


void i2c_start(void)
{	
	//��֤SDA����Ϊ���ģʽ
	sda_pin_mode(GPIO_Mode_OUT);
	
	SDA_W=1;
	SCL_W=1;
	delay_us(1);
	
	SDA_W=0;
	delay_us(1);
	
	SCL_W=0;//���߽���æ״̬
	delay_us(1);
}


void i2c_stop(void)
{
	//��֤SDA����Ϊ���ģʽ
	sda_pin_mode(GPIO_Mode_OUT);
	
	SDA_W=0;
	SCL_W=1;
	delay_us(1);
	
	SDA_W=1;
	delay_us(1);

}


void i2c_send_byte(uint8_t byte)
{
	int32_t i=0;
	
	//��֤SDA����Ϊ���ģʽ
	sda_pin_mode(GPIO_Mode_OUT);

	SDA_W=0;
	SCL_W=0;
	delay_us(1);
	
	//�����Чλ���ȴ��䣬ͨ��ʱ��ͼ�۲쵽
	for(i=7; i>=0; i--)
	{
		//����Ӧ��bitλ��1����0
		if(byte & (1<<i))
			SDA_W=1;
		else
			SDA_W=0;
	
		delay_us(1);
	
		//ʱ�������ߣ�������Ч
		SCL_W=1;
		delay_us(1);

	
		//ʱ�������ͣ����ݱ��
		SCL_W=0;
		delay_us(1);		
	
	}
}


uint8_t i2c_recv_byte(void)
{
	uint8_t d=0;
	int32_t i;
	
	//��֤SDA����Ϊ����ģʽ
	sda_pin_mode(GPIO_Mode_IN);

	for(i=7; i>=0; i--)
	{
		//ʱ�������ߣ�������Ч
		SCL_W=1;
		delay_us(1);
		
		//��ȡSDA���ŵ�ƽ
		if(SDA_R)
			d|=1<<i;


		//ʱ�������ͣ�����ռ�����ߣ�������æ״̬
		SCL_W=0;
		delay_us(1);	
	
	}

	return d;
}


void i2c_ack(uint8_t ack)
{

	//��֤SDA����Ϊ���ģʽ
	sda_pin_mode(GPIO_Mode_OUT);

	SDA_W=0;
	SCL_W=0;
	delay_us(1);
	

	if(ack)
		SDA_W=1;
	else
		SDA_W=0;

	delay_us(1);

	//ʱ�������ߣ�������Ч
	SCL_W=1;
	delay_us(1);


	//ʱ�������ͣ����ݱ��
	SCL_W=0;
	delay_us(1);		
}

uint8_t i2c_wait_ack(void)
{
	uint8_t ack=0;
	//��֤SDA����Ϊ����ģʽ
	sda_pin_mode(GPIO_Mode_IN);

	//ʱ�������ߣ�������Ч
	SCL_W=1;
	delay_us(1);
	
	//��ȡSDA���ŵ�ƽ
	if(SDA_R)
		ack=1;//��Ӧ��
	else
		ack=0;//��Ӧ��

	//ʱ�������ͣ�����ռ�����ߣ�������æ״̬
	SCL_W=0;
	delay_us(1);


	return ack;
}

void IIC_WriteBytes(u8 WriteAddr,u8* data,u8 dataLength)
{		
	u8 i;	
    i2c_start();  

	i2c_send_byte(WriteAddr);	    //����д����
	i2c_wait_ack();
	
	for(i=0;i<dataLength;i++)
	{
		i2c_send_byte(data[i]);
		i2c_wait_ack();
	}				    	   
    i2c_stop();//����һ��ֹͣ���� 
	delay_ms(10);	 
}

void IIC_ReadBytes(u8 deviceAddr, u8 writeAddr,u8* data,u8 dataLength)
{		
	u8 i;	
    i2c_start();  

	i2c_send_byte(deviceAddr);	    //����д����
	i2c_wait_ack();
	i2c_send_byte(writeAddr);
	i2c_wait_ack();
	i2c_send_byte(deviceAddr|0X01);//�������ģʽ			   
	i2c_wait_ack();
	
	for(i=0;i<dataLength-1;i++)
	{
		data[i] = i2c_recv_byte();
		i2c_ack(0);
	}		
	data[dataLength-1] = i2c_recv_byte();	
	i2c_ack(1);
    i2c_stop();//����һ��ֹͣ���� 
	delay_ms(10);	 
}

void IIC_Read_One_Byte(u8 daddr,u8 addr,u8* data)
{				  	  	    																 
    i2c_start();  
	
	i2c_send_byte(daddr);	   //����д����
	i2c_wait_ack();
	i2c_send_byte(addr);//���͵�ַ
	i2c_wait_ack();		 
	i2c_start();  	 	   
	i2c_send_byte(daddr|0X01);//�������ģʽ			   
	i2c_wait_ack();	 
    *data = i2c_recv_byte();
	i2c_ack(1);
    i2c_stop();//����һ��ֹͣ����	    
}

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data)
{				   	  	    																 
    i2c_start();  
	
	i2c_send_byte(daddr);	    //����д����
	i2c_wait_ack();
	i2c_send_byte(addr);//���͵�ַ
	i2c_wait_ack();	   	 										  		   
	i2c_send_byte(data);     //�����ֽ�							   
	i2c_wait_ack();  		    	   
    i2c_stop();//����һ��ֹͣ���� 
	delay_ms(10);	 
}
