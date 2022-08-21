
#include "sccb.h"
#include "tim.h"


//��ʼ��SCCB�ӿ� 
void SCCB_Init(void)
{											      	 
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_6|GPIO_PIN_7,GPIO_PIN_SET);
	SCCB_SDA_OUT();	   
}			 

//SCCB��ʼ�ź�
//��ʱ��Ϊ�ߵ�ʱ��,�����ߵĸߵ���,ΪSCCB��ʼ�ź�
//�ڼ���״̬��,SDA��SCL��Ϊ�͵�ƽ
void SCCB_Start(void)
{
    SCCB_SDA(GPIO_PIN_SET);     //�����߸ߵ�ƽ	   
    SCCB_SCL(GPIO_PIN_SET);	    //��ʱ���߸ߵ�ʱ���������ɸ�����
    HAL_DelayXus(50);  
    SCCB_SDA(GPIO_PIN_RESET);
    HAL_DelayXus(50);	 
    SCCB_SCL(GPIO_PIN_RESET);	    //�����߻ָ��͵�ƽ��������������Ҫ	  
}

//SCCBֹͣ�ź�
//��ʱ��Ϊ�ߵ�ʱ��,�����ߵĵ͵���,ΪSCCBֹͣ�ź�
//����״����,SDA,SCL��Ϊ�ߵ�ƽ
void SCCB_Stop(void)
{
    SCCB_SDA(GPIO_PIN_RESET);
    HAL_DelayXus(50);	 
    SCCB_SCL(GPIO_PIN_SET);	
    HAL_DelayXus(50); 
    SCCB_SDA(GPIO_PIN_SET);	
    HAL_DelayXus(50);
}  
//����NA�ź�
void SCCB_No_Ack(void)
{
	HAL_DelayXus(50);
	SCCB_SDA(GPIO_PIN_SET);	
	SCCB_SCL(GPIO_PIN_SET);	
	HAL_DelayXus(50);
	SCCB_SCL(GPIO_PIN_RESET);	
	HAL_DelayXus(50);
	SCCB_SDA(GPIO_PIN_RESET);	
	HAL_DelayXus(50);
}
//SCCB,д��һ���ֽ�
//����ֵ:0,�ɹ�;1,ʧ��. 
uint8_t SCCB_WR_Byte(uint8_t dat)
{
	uint8_t j,res;	 
	for(j=0;j<8;j++) //ѭ��8�η�������
	{
		if(dat&0x80)SCCB_SDA(GPIO_PIN_SET);	
		else SCCB_SDA(GPIO_PIN_RESET);
		dat<<=1;
		HAL_DelayXus(50);
		SCCB_SCL(GPIO_PIN_SET);	
		HAL_DelayXus(50);
		SCCB_SCL(GPIO_PIN_RESET);		   
	}			 
	SCCB_SDA_IN();		//����SDAΪ���� 
	HAL_DelayXus(50);
	SCCB_SCL(GPIO_PIN_SET);			//���յھ�λ,���ж��Ƿ��ͳɹ�
	HAL_DelayXus(50);
	if(SCCB_READ_SDA)res=1;  //SDA=1����ʧ�ܣ�����1
	else res=0;         //SDA=0���ͳɹ�������0
	SCCB_SCL(GPIO_PIN_RESET);		 
	SCCB_SDA_OUT();		//����SDAΪ���    
	return res;  
}	 
//SCCB ��ȡһ���ֽ�
//��SCL��������,��������
//����ֵ:����������
uint8_t SCCB_RD_Byte(void)
{
	uint8_t temp=0,j;    
	SCCB_SDA_IN();		//����SDAΪ����  
	for(j=8;j>0;j--) 	//ѭ��8�ν�������
	{		     	  
		HAL_DelayXus(50);
		SCCB_SCL(GPIO_PIN_SET);
		temp=temp<<1;
		if(SCCB_READ_SDA)temp++;   
		HAL_DelayXus(50);
		SCCB_SCL(GPIO_PIN_RESET);
	}	
	SCCB_SDA_OUT();		//����SDAΪ���    
	return temp;
} 							    
//д�Ĵ���
//����ֵ:0,�ɹ�;1,ʧ��.
uint8_t SCCB_WR_Reg(uint8_t reg,uint8_t data)
{
	uint8_t res=0;
	SCCB_Start(); 					//����SCCB����
	if(SCCB_WR_Byte(SCCB_ID))res=1;	//д����ID	  
	HAL_DelayXus(100);
  if(SCCB_WR_Byte(reg))res=1;		//д�Ĵ�����ַ	  
	HAL_DelayXus(100);
	if(SCCB_WR_Byte(data))res=1; 	//д����	 
	SCCB_Stop();	  
	return	res;
}		  					    
//���Ĵ���
//����ֵ:�����ļĴ���ֵ
uint8_t SCCB_RD_Reg(uint8_t reg)
{
	uint8_t val=0;
	SCCB_Start(); 				//����SCCB����
	SCCB_WR_Byte(SCCB_ID);		//д����ID	  
	HAL_DelayXus(100);	 
  SCCB_WR_Byte(reg);			//д�Ĵ�����ַ	  
	HAL_DelayXus(100);	  
	SCCB_Stop();   
	HAL_DelayXus(100);	   
	//���üĴ�����ַ�󣬲��Ƕ�
	SCCB_Start();
	SCCB_WR_Byte(SCCB_ID|0X01);	//���Ͷ�����	  
	HAL_DelayXus(100);
	val=SCCB_RD_Byte();		 	//��ȡ����
	SCCB_No_Ack();
	SCCB_Stop();
	return val;
}



