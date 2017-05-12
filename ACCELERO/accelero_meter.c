#include "accelero_meter.h"
#include "delay.h"

//#if defined T36CTA
#define ACCELERO_DELAY  5

uint16_t axis_value[3] = {0, 0, 0};
uint8_t asix_sequence = 0;



void ACCELERO_IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);//ʹ��GPIOBʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_11);
}

void ACCELERO_I2C_init(void)
{
	ACCELERO_SDA_OUT();
	ACCELERO_SDA = 1;
	ACCELERO_SCL = 1;
}

void ACCELERO_I2C_start(void)
{
	ACCELERO_SDA_OUT();
	ACCELERO_SDA = 1;
	delay_us(ACCELERO_DELAY);
	ACCELERO_SCL = 1;
	delay_us(ACCELERO_DELAY);
	ACCELERO_SDA = 0;
	delay_us(ACCELERO_DELAY);
	ACCELERO_SCL = 0;
}

void ACCELERO_I2C_stop(void)
{
	ACCELERO_SDA_OUT();
	ACCELERO_SCL = 0;
	ACCELERO_SDA = 0;
	delay_us(ACCELERO_DELAY);
	ACCELERO_SCL = 1;
	ACCELERO_SDA = 1;
	delay_us(ACCELERO_DELAY);;
}

u8 ACCELERO_I2C_wait_for_ack(void)
{
	u16 ucErrTime = 0;
	ACCELERO_SDA_IN();		//SDA����Ϊ���� 	
	ACCELERO_SCL = 0;
	ACCELERO_SDA = 1;
	delay_us(20);
	for(ucErrTime = 0; ucErrTime < 250; ucErrTime++)
	{
		if(READ_ACCELERO_SDA == 0)
		{
			ACCELERO_SDA_OUT();
			ACCELERO_SDA = 0 ;
			delay_us(ACCELERO_DELAY);
			ACCELERO_SCL = 1;
			return ACK;
		}
	}
	return NACK ;
}
//����ACKӦ��
void ACCELERO_I2C_Ack(void)
{	ACCELERO_SDA_OUT();
	ACCELERO_SCL = 0;	
	ACCELERO_SDA = 0;
	delay_us(ACCELERO_DELAY);
	ACCELERO_SCL = 1;
	delay_us(ACCELERO_DELAY);
	ACCELERO_SCL = 0;
}

//������ACKӦ��		    
void ACCELERO_I2C_NAck(void)
{
	ACCELERO_SCL = 0;
	ACCELERO_SDA_OUT();
	ACCELERO_SDA = 1;
	delay_us(ACCELERO_DELAY);
	ACCELERO_SCL = 1;
	delay_us(ACCELERO_DELAY);
	ACCELERO_SCL = 0;
}

void ACCELERO_I2C_write_byte(u8 txd)
{
    u8 t;   
	ACCELERO_SDA_OUT(); 
	ACCELERO_SCL = 0;
    for(t = 0; t < 8; t++)
    {  		 		
		ACCELERO_SDA = (txd & 0x80)>>7;
		txd <<= 1;
		delay_us(1);
		ACCELERO_SCL = 1;
		delay_us(5);
		ACCELERO_SCL = 0;
		delay_us(5);
    }	
}


u8 ACCELERO_I2C_read_byte(u8 ack)
{
	u8 i, ret = 0;	
	ACCELERO_SDA_IN();			//SDA����Ϊ����
	ACCELERO_SCL = 0;
	ACCELERO_SDA = 1 ;
	delay_us(10) ;
	for(i = 0; i < 8; i++)
	{	
		ACCELERO_SCL = 1;
		delay_us(5);
        if(READ_ACCELERO_SDA)	ret |= (1 << (7 - i));
		delay_us(5);  
		ACCELERO_SCL = 0;
		delay_us(5); 		
    }
	if(ack == ACK)
	{
		ACCELERO_I2C_Ack();
	}
	else
	{
		ACCELERO_I2C_NAck();
	}
//	ACCELERO_SCL = 1;
		   
    return ret;
}

void ACCELERO_Write_Data(u8 Addr,u8 Value)
{
  ACCELERO_I2C_start(); //make the I2C bus begin
  
  ACCELERO_I2C_write_byte(ACCELERO_ADDR_WRITE);//master send slaver address
  ACCELERO_I2C_wait_for_ack();             //we can get Ackknowledge from slaver if equal to 0,means right
 
  ACCELERO_I2C_write_byte(Addr);//master send register to slaver
  ACCELERO_I2C_wait_for_ack();            
  
  ACCELERO_I2C_write_byte(Value);//master send value to slaver
  if( !ACCELERO_I2C_wait_for_ack() )            
  {                          
    ACCELERO_I2C_stop();
  }
  delay_us(ACCELERO_DELAY);
  ACCELERO_I2C_stop();
}

u8 ACCELERO_Read_Data(u8 Addr)
{
  u8 Value;
  
  ACCELERO_I2C_start(); //make the I2C bus begin

  ACCELERO_I2C_write_byte(ACCELERO_ADDR_WRITE ); // write address 1100100 and make the R/w with 0
  ACCELERO_I2C_wait_for_ack();

  ACCELERO_I2C_write_byte(Addr);// write register to slaver
  ACCELERO_I2C_wait_for_ack();

  ACCELERO_I2C_start(); //repeat make the I2C bus begin

  ACCELERO_I2C_write_byte(ACCELERO_ADDR_READ);// write address 1100100 and make the R/w with 1
  ACCELERO_I2C_wait_for_ack();
  
  Value = ACCELERO_I2C_read_byte(NACK); // we recive the content from slaver 
  ACCELERO_I2C_stop();

  return Value;
}
//#endif