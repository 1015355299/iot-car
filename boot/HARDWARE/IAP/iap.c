#include "main.h"
#include "usart.h"
#include "stmflash.h"
#include "iap.h" 


iapfun jump2app; 
uint32_t iapbuf[512]; 	//2K�ֽڻ���  
//appxaddr:Ӧ�ó������ʼ��ַ
//appbuf:Ӧ�ó���CODE.
//appsize:Ӧ�ó����С(�ֽ�).
void iap_write_appbin(uint32_t appxaddr,uint8_t *appbuf,uint32_t appsize)
{
	uint32_t t;
	uint16_t i=0;
	uint32_t temp;
	uint32_t fwaddr=appxaddr;//��ǰд��ĵ�ַ
	uint8_t *dfu=appbuf;
	for(t=0;t<appsize;t+=4)
	{						   
		temp=(uint32_t)dfu[3]<<24;   
		temp|=(uint32_t)dfu[2]<<16;    
		temp|=(uint32_t)dfu[1]<<8;
		temp|=(uint32_t)dfu[0];	  
		dfu+=4;//ƫ��4���ֽ�
		iapbuf[i++]=temp;	    
		if(i==512)
		{
			i=0; 
			STMFLASH_Write(fwaddr,iapbuf,512);
			fwaddr+=2048;//ƫ��2048  512*4=2048
		}
	} 
	if(i)STMFLASH_Write(fwaddr,iapbuf,i);//������һЩ�����ֽ�д��ȥ.  
}

//��ת��Ӧ�ó����
//appxaddr:�û�������ʼ��ַ.
void iap_load_app(uint32_t appxaddr)
{ 
	typedef void (*_func)(void);
	
	//__disable_irq();
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	RCC->AHB1ENR = (1<<20); 
	RCC->AHB2ENR = 0; 
	RCC->AHB3ENR = 0; 
	RCC->APB1ENR = 0; 
	RCC->APB2ENR = 0; 
	RCC->CFGR = 0;
	MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk; 

	SCB->VTOR = appxaddr; 

	__set_BASEPRI(0); 
	__set_FAULTMASK(0); 

	__set_MSP(*(uint32_t*)appxaddr); 
	__set_PSP(*(uint32_t*)appxaddr);
	__set_CONTROL(0); 

	__ISB(); 
	//__disable_irq(); 0x08010000
	((_func)(*(uint32_t*)(appxaddr + 4)))(); 
//	if(((*(vuint32_t*)appxaddr)&0x2FFE0000)==0x20000000)	//���ջ����ַ�Ƿ�Ϸ�.
//	{ 
//		jump2app=(iapfun)*(vuint32_t*)(appxaddr+4);		//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)		
//		MSR_MSP(*(vuint32_t*)appxaddr);					//��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
//		jump2app();									//��ת��APP.
//	}
}		 



