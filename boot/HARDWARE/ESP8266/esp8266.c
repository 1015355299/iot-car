
#include "esp8266.h"
#include "usart.h"
#include "tim.h"
#include "stdarg.h"
#include "iwdg.h"
#include "main.h"


//�û�������

//���Ӷ˿ں�:8888,�������޸�Ϊ�����˿�.
uint8_t* remotePort="58266";		
uint8_t* remoteIp="118.25.95.166";	

//uint8_t* remotePort="8888";		 
//uint8_t* remoteIp="192.168.137.1";	

// ���ض˿ں�tcp���ض˿�9999
uint8_t* localTcpPort="9998";	
uint8_t* keepAliveTime="60";


//WIFI STAģʽ,����Ҫȥ���ӵ�·�������߲���,��������Լ���·��������,�����޸�.
//uint8_t* wifista_ssid="ChinaNet-9rrY";			//·����SSID��
//uint8_t* wifista_encryption="wpawpa2_aes";	//wpa/wpa2 aes���ܷ�ʽ
//uint8_t* wifista_password="jc2ttcwb"; 	//��������

uint8_t* wifista_ssid="mycar";			//·����SSID��
uint8_t* wifista_encryption="wpawpa2_aes";	//wpa/wpa2 aes���ܷ�ʽ
uint8_t* wifista_password="44466666"; 	//��������


//���ڷ��ͻ����� 	
__align(4) uint8_t UART5_TX_BUF[UART5_MAX_SEND_LEN]; //���ͻ���,���USART3_MAX_SEND_LEN�ֽ�
uint8_t UART5_RX_BUF[UART5_MAX_RECV_LEN] __attribute__ ((at(0X20001000))); //���ջ���,���USART3_MAX_RECV_LEN���ֽ�.

//���յ�������״̬
//[31]:0,û�н��յ�����;1,���յ���һ������.
//[30:0]:���յ������ݳ���
uint32_t UART5_RX_STA=0;   	 

uint8_t init_wifi2_finish = 0;

void u5_printf(char* fmt,...)  
{  
	uint16_t i;
	va_list ap;// ����ɱ�����б�
	va_start(ap,fmt);// �׸��ɱ������ַ��ֵ��ap
	vsprintf((char*)UART5_TX_BUF,fmt,ap);// ��ap��fmt�ĸ�ʽ���뷢�ͻ���
	va_end(ap);// �����ɱ����
	// ��ȡ��η��͵����ݳ��Ȳ�����
	i=strlen((const char*)UART5_TX_BUF);//�˴η������ݵĳ���
	// ��������
	HAL_UART_Transmit_IT(&huart5,UART5_TX_BUF,i);
}

// ������Ӧ���ݸ�pc
void atk_8266_at_response(uint8_t mode)
{
	if((UART5_RX_STA&0x80000000)==0x80000000)		//���յ�һ��������
	{ 
		printf("\r\n-----response-----\r\n%s\r\n-----end----\r\n",UART5_RX_BUF);	//���͵�����pc
		if(mode) UART5_RX_STA=0;
	} 
}

//ATK-ESP8266���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//    ����,�ڴ�Ӧ������λ��(str��λ��)
uint8_t* atk_8266_check_cmd(uint8_t *str)
{
	char *strx=0;
	if((UART5_RX_STA&0x80000000)==0x80000000)		//���յ�һ������������
	{ 
		// ���Ҳ����ؽ��������е�һ�γ���str��λ���������ַ���ָ��
		strx=strstr((const char*)UART5_RX_BUF,(const char*)str);
	} 
	return (uint8_t*)strx;
}

//��ATK-ESP8266��������
//0,���ͳɹ�(�õ����ڴ���Ӧ����) 1,����ʧ��
uint8_t atk_8266_send_cmd(uint8_t *cmd,uint8_t *ack,uint16_t waittime)
{
	printf("\r\nSEND CMD:%s\r\n",cmd);
	uint8_t res=0; 
	UART5_RX_STA=0;
	if(cmd[0]=='+'&&cmd[1]=='+'&&cmd[2]=='+')
	{
		// �������
		u5_printf("%s",cmd);
		return 0;
	}else{
		HAL_UART_Receive_IT(&huart5,UART5_RX_BUF,1);// ��������
		u5_printf("%s\r\n",cmd);
	}
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			HAL_Delay(10);
			
			if((UART5_RX_STA&0x80000000)==0x80000000)//���յ��ڴ���Ӧ����
			{
				// �õ��ڴ�Ӧ��
				if(atk_8266_check_cmd(ack)) break;
			} 
		}
		if(waittime==0) res=1; // δ�ȵ�Ԥ�ڽ��
	}
	atk_8266_at_response(0);// ������Ӧ��pc
	return res;
} 


//ATK-ESP8266�˳�͸��ģʽ
//0,�˳��ɹ�;1,�˳�ʧ��
uint8_t atk_8266_quit_trans(void)
{
	atk_8266_send_cmd("+++","",0);
	HAL_Delay(1100);					//�ȴ�1100ms
	atk_8266_send_cmd("AT","OK",100);
	return atk_8266_send_cmd("AT","OK",100);
}

//ATK-ESP8266����͸��ģʽ
//0�ɹ�;1ʧ��
uint8_t atk_8266_set_trans(void)
{
	uint8_t restart=0;
	// ����͸��ģʽ
	if(atk_8266_send_cmd("AT+CIPMODE=1","OK",200)==0) return 0;
	
	while(atk_8266_send_cmd("AT+CIPMODE?","CIPMODE:1",500))
	{
		if(atk_8266_send_cmd("AT+CIPMODE=1","OK",200)==0) break;
		if(++restart>2){return 1;}
		HAL_Delay(200);
	}
	return 0;
}

//ATK-ESP8266���ò�����
//0�ɹ�;1ʧ��
uint8_t atk_8266_set_uart(uint8_t *p)
{
	uint8_t restart=0,i=0;
	uint8_t cmd[] = "AT+UART_CUR=115200,8,1,0,0";
	for(i=0;i<=5;i++)
	{
		cmd[i+12]=p[i];
	}
	while(atk_8266_send_cmd(cmd,"OK",100))
	{
		if(++restart>2){return 1;}
		HAL_Delay(200);
	}
	printf("###��esp8266���������óɹ���###\r\n");
	return 0;
}


// ģ�����߲��ԣ�����ATģʽ	 0���ߣ�1����
uint8_t atk_8266_online_test(void)
{
	uint8_t restart=0;
	if(atk_8266_send_cmd("AT","OK",100)==0) return 0;
	// ���Ͳ���ָ���ģ�飬���WIFIģ���Ƿ�����
	while(atk_8266_quit_trans())// �������ʧ�ܣ���һֱ���ͣ�ֱ���ɹ�
	{
		if(++restart>1){return 1;}// ATָ����Ч���˳�ʧ�ܻ�������
		HAL_Delay(200);
	} 
	return 0;
}


// ����8266ģʽ1 sta��2 ap,3 ap+sta; 0�ɹ���1ʧ��
uint8_t atk_8266_set_mode(uint8_t mode)
{
	uint8_t restart=0;
	uint8_t ack[] = "CWMODE_DEF:1";
	uint8_t cmd[] = "AT+CWMODE_DEF=1";
	ack[11] = mode + '0';
	cmd[14] = mode + '0';

	if(atk_8266_send_cmd(cmd,"OK",100)==0) return 0;
	// ��⣬����Ϊģʽ1
	while(atk_8266_send_cmd("AT+CWMODE_DEF?",ack,500))
	{
		if(atk_8266_send_cmd(cmd,"OK",100)==0) break;
		if(++restart>2){restart=0;return 1;}
		HAL_Delay(200);
	}
	return 0;
}


// staģʽ�����ӵ�wifi
uint8_t sta_connect_wifi(void)
{
	uint8_t restart=0;
	uint8_t p[30];
	
	// �������ã�Ĭ�����õ�����Ա���
	sprintf((char*)p,"AT+CWJAP_DEF=\"%s\",\"%s\"",wifista_ssid,wifista_password);
	
	// ����sta ���ӵ�wifi
	while(atk_8266_send_cmd("AT+CWJAP_DEF?","CWJAP_DEF:\"",800)) // ����Ĭ������
	{
		if(atk_8266_check_cmd(wifista_ssid) && atk_8266_check_cmd(wifista_password)) break;
		// �Ͽ��ȵ�
		atk_8266_send_cmd("AT+CWQAP","OK",400);
		HAL_IWDG_Refresh(&hiwdg);
		if(atk_8266_send_cmd(p,"CONNECTED",2000)==0) break;					//����APģʽ����
		if(++restart>2) return 1;
		HAL_Delay(200);
	}
	return 0;
}

//staģʽ����staΪtcp��0 �ɹ���1ʧ��
uint8_t set_sta_tcp(void)
{
	uint8_t restart=0;
	uint8_t p[60];
	atk_8266_send_cmd("AT+CIPCLOSE","OK",100);
	// ����sta tcp�ͻ��˹���ģʽ
	sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s,300",remoteIp,remotePort);
	
	// ���ж��Ƿ��������
	while(atk_8266_send_cmd("AT+CIPSTATUS",remoteIp,100)||!atk_8266_check_cmd(remotePort))
	{
		if(atk_8266_check_cmd(remoteIp))
		{
			atk_8266_send_cmd("AT+CIPCLOSE","OK",100);
		}
		if(atk_8266_send_cmd(p,"OK",500)==0) break;
		if(++restart>2) return 1;
		HAL_Delay(200);
	}
	printf("###��STA TCP��ע�᡿###\r\n");
	return 0;
}

// ����sta�����ӽ���͸��
uint8_t atk_8266_sta_start(uint8_t netpro)
{
	// ����������ģʽ3
	if(atk_8266_set_mode(netpro))
	{
		printf("###������ģʽ1ʧ�ܡ�###\r\n");return 1;
	}
	
	// ���õ�����
	if(atk_8266_send_cmd("AT+CIPMUX=0","OK",100))
	{
		printf("###��staģʽ���õ�����ʧ�ܡ�###\r\n");
	}
	

_RESET:
	// �˳�smartCOnfig����ģʽ
	if(atk_8266_send_cmd("AT+CWSTOPSMART","OK",500))
	{
		printf("###��AT+CWSTOPSMARTʧ�ܡ�###\r\n");
	}
	
	if(ENTRY_NETWORK_METHOD){
		flicker_led(2,1,1,1,1);
		HAL_IWDG_Refresh(&hiwdg);
		// ����smartConfig����ģʽ
		if(atk_8266_send_cmd("AT+CWSTARTSMART=3","connected",2000))
		{
			printf("###��SmartConfig����ʧ�ܡ�###\r\n");
		}
		flicker_led(2,0,1,0,1);
		
		// �˳�smartCOnfig����ģʽ
		if(atk_8266_send_cmd("AT+CWSTOPSMART","OK",500))
		{
			printf("###��AT+CWSTOPSMARTʧ�ܡ�###\r\n");
		}
		HAL_IWDG_Refresh(&hiwdg);
		// �ж��Ƿ�������
		if(atk_8266_send_cmd("AT+CWJAP?","CWJAP",1000)){
			goto _RESET;
		}
	}else{
		// Ĭ������
		if(sta_connect_wifi())
		{
			printf("###��staģʽ����wifiʧ�ܡ�###\r\n");return 1;
		}
	}
	HAL_IWDG_Refresh(&hiwdg);
	// ����wifi���ӳ٣��Ա��ʼ������
	HAL_Delay(1000);
	
	// ��ѯip
	atk_8266_send_cmd("AT+CIFSR","OK",100);

	
	// ����sta����ģʽ tcp
	if(set_sta_tcp()) 
	{
		printf("###��sta tcpע��ʧ�ܡ�###\r\n");return 1;
	}
	
	// ֻ�ڵ�����tcp�ͻ��˺�upd�¿�����͸��ģʽ��δ����
	if(atk_8266_set_trans()) 
	{
		printf("###������͸��ʧ�ܡ�###\r\n");
		return 1;
	}
	return 0;		
} 

// ���ô��ڲ�����
uint8_t set_uart_baudRate(uint32_t baudRate)
{
		// �޸Ĵ���5������
	__HAL_UART_DISABLE(&huart5);
	huart5.Init.BaudRate = baudRate;
	if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    printf("###��change uart5 BaudRate fail��###\r\n");
		return 1;
  }
	printf("###��change uart5 BaudRate:%d success��###",baudRate);
	return 0;
}

// ����esp8266������
uint8_t set_esp8266_baudRate(uint8_t * baudRate)
{
	// ���ò����� 921600 460800 115200 
	if(atk_8266_set_uart(baudRate))
	{
		printf("###��change esp8266 BaudRate fail��###\r\n");
		return 1;
	}
	printf("###��change esp8266 BaudRate:%s success��###",baudRate);
	return 0;
}

// ���Ժ�wifi2��������ͨ��
uint8_t atc_8266_connect()
{
	// AT ָ����Ч
	if(atk_8266_send_cmd("AT","OK",100))
	{
		// �˳�͸��������ATģʽ��Ч
		if(atk_8266_quit_trans())
		{
			// ���ô��ڲ�����
			set_uart_baudRate(115200);
			HAL_Delay(100);
			if(atk_8266_send_cmd("AT","OK",100))
			{
				if(atk_8266_quit_trans()) 
				{
					return 1;
				}
			}
		}
	}
	// ����esp8266������ 1382400 921600 691200 460800 115200 57600 
	set_esp8266_baudRate("115200");
	printf("###��wifi2ͨ�ųɹ���###\r\n");
	return 0;
}

//ATK-ESP8266ģ�����������
void atk_8266_init(void)
{

	uint8_t restart=2;
	printf("###��bootloader��ʼ����###\r\n");
	printf("###����ʼ��ʼ��wifi2��###\r\n");

	// ��ʼͨ��
	if(atc_8266_connect())
	{
		printf("###��wifi2ͨ��ʧ��,��ʼ��λ��###\r\n");
		// uart5 Ӳ����λ
		HAL_GPIO_WritePin(WIFI2_RESET_GPIO_Port,WIFI2_RESET_Pin,GPIO_PIN_RESET);
		HAL_Delay(300);
		HAL_GPIO_WritePin(WIFI2_RESET_GPIO_Port,WIFI2_RESET_Pin,GPIO_PIN_SET);
		printf("###����λ��ɡ�###\r\n");
	}
	
	// ���ô��ڲ����ʣ�����esp8266�Ͳ������ȶ�
	set_uart_baudRate(115200);
	HAL_IWDG_Refresh(&hiwdg);
RESET:
	
	while(--restart)
	{
		// ���Ͳ���ָ���ģ�飬���WIFIģ���Ƿ�����
		if(atk_8266_online_test())
		{
			printf("###���豸���߻��޷����ӣ���ʼ��λ��###\r\n");
			// uart5 ��λ
			HAL_GPIO_WritePin(WIFI2_RESET_GPIO_Port,WIFI2_RESET_Pin,GPIO_PIN_RESET);
			HAL_Delay(300);
			HAL_GPIO_WritePin(WIFI2_RESET_GPIO_Port,WIFI2_RESET_Pin,GPIO_PIN_SET);
			printf("###����λ��ɡ�###\r\n");
		}else{
			printf("###���豸���ߡ�###\r\n");
			break;
		}
	}
	
	//�رջ���
	while(atk_8266_send_cmd("ATE0","OK",100))
	{
		if(++restart>2){break;}
		HAL_Delay(200);
	}
	HAL_IWDG_Refresh(&hiwdg);

	
	// ����staģʽ
	if(atk_8266_sta_start(1))
	{
		printf("###��init tcp esp8266 fail��###\r\n");
		goto RESET;
	}
	printf("��init tcp esp8266 success��\r\n");
	HAL_IWDG_Refresh(&hiwdg);
	
	// ����esp8266������ 1382400 921600 806400 748800 691200 460800 115200 57600 
	set_esp8266_baudRate("691200");
	
	// ���ô��ڲ�����
	set_uart_baudRate(691200);
	
	if(atk_8266_send_cmd("AT+CIPSEND","OK",500))
	{
		printf("###��͸������ʧ�ܡ�###\r\n");
		goto RESET;
	}
	printf("###��͸���ѿ��������롿###\r\n");
	printf("###��wifi2��ʼ���ɹ���###\r\n");
	printf("###��bootloader��ʼ���ɹ���###\r\n");
	
	init_wifi2_finish=1;
	HAL_TIM_Base_Stop_IT(&htim14);// ��ֹ����5���ռ�ʱ��
	UART5_RX_STA=0;
	HAL_Delay(500);
	HAL_UART_Receive_IT(&huart5,&UART5_RX_BUF[0],1);
	flicker_led(2,2,1,0,1000);
}

