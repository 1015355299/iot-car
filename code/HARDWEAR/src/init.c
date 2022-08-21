
#include "init.h"
#include "tim.h"
#include "ov2640.h"
#include "at_log.h"
#include "usart.h"
#include "pwm.h"

// ���ε���˸ 0 led9,1 led10,2 ����,flicker�Ƿ���˸��0��1����2��һ��,sync �Ƿ�ͬʱ��˸˫��ʱ��Ч
// keep ���ձ��ֵ�״̬��0Ϩ��1����2ԭ��״̬,delay�ӳ�ms
void flicker_led(uint8_t group,uint8_t flicker,uint8_t sync,uint8_t keep,uint16_t delay)
{
	// ֻ��
	if(flicker==1||flicker==2){
		switch(group){
			case 2:
				if(sync){
					HAL_GPIO_WritePin(LED9_GPIO_Port,LED9_Pin,GPIO_PIN_RESET);
					HAL_GPIO_WritePin(LED10_GPIO_Port,LED10_Pin,GPIO_PIN_RESET);
				}else{
					HAL_GPIO_WritePin(LED9_GPIO_Port,LED9_Pin,GPIO_PIN_RESET);
					HAL_GPIO_WritePin(LED10_GPIO_Port,LED10_Pin,GPIO_PIN_SET);
				}
				break;
			case 1:
				HAL_GPIO_WritePin(LED10_GPIO_Port,LED10_Pin,GPIO_PIN_RESET);break;
			case 0:
				HAL_GPIO_WritePin(LED9_GPIO_Port,LED9_Pin,GPIO_PIN_RESET);break;
			default:;
		}
		HAL_Delay(delay);
	}
	// ֻ��
	if(flicker==0||flicker==2){
		switch(group){
			case 2:
				if(sync){
					HAL_GPIO_WritePin(LED9_GPIO_Port,LED9_Pin,GPIO_PIN_SET);
					HAL_GPIO_WritePin(LED10_GPIO_Port,LED10_Pin,GPIO_PIN_SET);
				}else{
					HAL_GPIO_WritePin(LED9_GPIO_Port,LED9_Pin,GPIO_PIN_SET);
					HAL_GPIO_WritePin(LED10_GPIO_Port,LED10_Pin,GPIO_PIN_RESET);
				}
				break;
			case 1:
				HAL_GPIO_WritePin(LED10_GPIO_Port,LED10_Pin,GPIO_PIN_SET);break;
			case 0:
				HAL_GPIO_WritePin(LED9_GPIO_Port,LED9_Pin,GPIO_PIN_SET);break;
			default:;
		}
		HAL_Delay(delay);
	}
	switch(keep){
		case 1:
			HAL_GPIO_WritePin(LED9_GPIO_Port,LED9_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(LED10_GPIO_Port,LED10_Pin,GPIO_PIN_RESET);
			break;
		case 0:
			HAL_GPIO_WritePin(LED9_GPIO_Port,LED9_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(LED10_GPIO_Port,LED10_Pin,GPIO_PIN_SET);
			break;
		default:;
	}
}

// ��ʼ��ѡ��������ʽ
void init_net_method()
{	
	Log_d("��select net start��");
	p=20;// 10�ν�����˸��������˸��2����ʱ���� 4s
	while(--p&&ENTRY_NETWORK_METHOD==0)
	{
		flicker_led(2,2,0,0,100);
		// softAp����,���¸ߵ�ƽ
		if(HAL_GPIO_ReadPin(KEY0_NETWORK_0_GPIO_Port,KEY0_NETWORK_0_Pin)==GPIO_PIN_SET){
			ENTRY_NETWORK_METHOD = SmartConfig;
		}else if(HAL_GPIO_ReadPin(KEY1_NETWORK_1_GPIO_Port,KEY1_NETWORK_1_Pin)==GPIO_PIN_SET){
			// SmartConfig����
			ENTRY_NETWORK_METHOD = SoftAP;
		}
	}
	p=0;
	Log_d("��select net end��");
}


/* ��ʱ����ʼ�� */

// ��ʼ��������ʱ��-������������
void init_base_encode()
{
	HAL_TIM_Base_Start_IT(&htim12);// ���ٶ�ʱ��
	
	HAL_TIM_Base_Start(&htim1);// ������
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim3);
	HAL_TIM_Base_Start(&htim4);
}


// ��ʼ��������ʱ��-���pwm
void init_base_move_pwm()
{
	HAL_TIM_Base_Start(&htim8);// ���1-3pwm
	HAL_TIM_Base_Start(&htim13);// ���4pwm
}

// ��ʼ��������ʱ��-���pwm
void init_base_duoji_pwm()
{
	update_duoji();// ��ʼ�����λ�ã�������ƫ
	HAL_TIM_Base_Start(&htim5);// ���pwm
}

// ��ʼ��������ʱ��-����������ʱ�� us������5��ʱ��
void init_base_other()
{
	HAL_TIM_Base_Start(&htim7);// us��ʱ��
	HAL_TIM_Base_Start_IT(&htim11);
	HAL_TIM_Base_Stop_IT(&htim11);// ����5��ʱ��
}

// ��ʼ��������ʱ��-���л�����ʱ��
void init_base_all()
{
	Log_d("��init base start��");
	// ��ʼ��������ʱ��-������������
	init_base_encode();
	// ��ʼ��������ʱ��-���pwm
	init_base_move_pwm();
	// ��ʼ��������ʱ��-���pwm
	init_base_duoji_pwm();
	// ��ʼ��������ʱ��-us
	init_base_other();
	HAL_Delay(200);
	//Log_d("��init base end��");
}

// ��ʼ��ov2640ģ��
void init_ov2640()
{
	Log_d("��init ov2640 start��");
	while(OV2640_Init());
	Log_d("��init ov2640 end��");
}


// ��ʼ�����pwmʹ��ͨ��
void init_enable_pwm_start_move()
{
	// ����4�����pwm�жϣ���ʹ��ͨ��
	HAL_TIM_PWM_Start(&htim8,TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim8,TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim8,TIM_CHANNEL_3);
	HAL_TIMEx_PWMN_Start(&htim8,TIM_CHANNEL_1); // PA5
	HAL_TIMEx_PWMN_Start(&htim8,TIM_CHANNEL_2);	// PB0
	HAL_TIMEx_PWMN_Start(&htim8,TIM_CHANNEL_3);	// PB1
	HAL_TIM_PWM_Start(&htim13,TIM_CHANNEL_1);		// PF8
}

// ��ʼ�����pwmʹ��ͨ��
void init_enable_pwm_start_duoji()
{
	// �������pwmʹ��ͨ��
	HAL_TIM_PWM_Start(&htim5,TIM_CHANNEL_3);		// PA2
	HAL_TIM_PWM_Start(&htim5,TIM_CHANNEL_4);		// PA3
}

// ��ʼ��������ʹ��ͨ��
void init_enable_pwm_start_encode()
{
	// �����������жϣ���ʹ��
	HAL_TIM_Encoder_Start(&htim1,TIM_CHANNEL_1);		// PE9
	HAL_TIM_Encoder_Start(&htim1,TIM_CHANNEL_2);		// PE11
	HAL_TIM_Encoder_Start(&htim2,TIM_CHANNEL_1);		// PA1
	HAL_TIM_Encoder_Start(&htim2,TIM_CHANNEL_2);		// PA15
	HAL_TIM_Encoder_Start(&htim3,TIM_CHANNEL_1);		// PB4
	HAL_TIM_Encoder_Start(&htim3,TIM_CHANNEL_2);		// PA7
	HAL_TIM_Encoder_Start(&htim4,TIM_CHANNEL_1);		// PD12
	HAL_TIM_Encoder_Start(&htim4,TIM_CHANNEL_2);		// PD13
}

// ��ʼ������3ʹ�ܽ����ж�
void init_enable_uart3_receive()
{
	// �����ȿ���usart3�����ж�
	HAL_UART_Receive_IT(&huart3,&p,1);
}

// ��ʼ������ʹ��ͨ�����ж�
void init_enable_channel_or_it()
{
	//Log_d("��init enbale channel or it start��");
	init_enable_pwm_start_move();// ��ʼ�����pwmʹ��ͨ��
	init_enable_pwm_start_duoji();// ��ʼ�����pwmʹ��ͨ��
	init_enable_pwm_start_encode();// ��ʼ��������ʹ��ͨ��
	init_enable_uart3_receive();// ��ʼ������3ʹ�ܽ����ж�
	//Log_d("��init enbale channel or it end��");
}


/* ȡ����ʱ����ʼ�� */

// ȡ����ʼ��������ʱ��-������������
void deInit_base_encode()
{
	HAL_TIM_Base_Stop_IT(&htim12);// ���ٶ�ʱ��
	
	HAL_TIM_Base_Stop(&htim1);// ������
	HAL_TIM_Base_Stop(&htim2);
	HAL_TIM_Base_Stop(&htim3);
	HAL_TIM_Base_Stop(&htim4);
}


// ȡ����ʼ��������ʱ��-���pwm
void deInit_base_move_pwm()
{
	HAL_TIM_Base_Stop(&htim8);// ���1-3pwm
	HAL_TIM_Base_Stop(&htim13);// ���4pwm
}

// ȡ����ʼ��������ʱ��-���pwm
void deInit_base_duoji_pwm()
{
	HAL_TIM_Base_Stop(&htim5);// ���pwm
}

// ȡ����ʼ��������ʱ��-����������ʱ�� us������5��ʱ��
void deInit_base_other()
{
	HAL_TIM_Base_Stop(&htim7);// us��ʱ��
	HAL_TIM_Base_Stop(&htim11);// ����5��ʱ��
}

// ȡ����ʼ��������ʱ��-���л�����ʱ��
void deInit_base_all()
{
	Log_d("��deInit base start��");
	// ȡ����ʼ��������ʱ��-������������
	deInit_base_encode();
	// ȡ����ʼ��������ʱ��-���pwm
	deInit_base_move_pwm();
	// ȡ����ʼ��������ʱ��-���pwm
	deInit_base_duoji_pwm();
	// ȡ����ʼ��������ʱ��-us
	deInit_base_other();
	Log_d("��deInit base end��");
}


// ȡ����ʼ�����pwmʹ��ͨ��
void deInit_enable_pwm_start_move()
{
	// ����4�����pwm�жϣ���ʹ��ͨ��
	HAL_TIM_PWM_Stop(&htim8,TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop(&htim8,TIM_CHANNEL_2);
	HAL_TIM_PWM_Stop(&htim8,TIM_CHANNEL_3);
	HAL_TIMEx_PWMN_Stop(&htim8,TIM_CHANNEL_1); // PA5
	HAL_TIMEx_PWMN_Stop(&htim8,TIM_CHANNEL_2);	// PB0
	HAL_TIMEx_PWMN_Stop(&htim8,TIM_CHANNEL_3);	// PB1
	HAL_TIM_PWM_Stop(&htim13,TIM_CHANNEL_1);		// PF8
}

// ȡ����ʼ�����pwmʹ��ͨ��
void deInit_enable_pwm_start_duoji()
{
	// �������pwmʹ��ͨ��
	HAL_TIM_PWM_Stop(&htim5,TIM_CHANNEL_3);		// PA2
	HAL_TIM_PWM_Stop(&htim5,TIM_CHANNEL_4);		// PA3
}

// ȡ����ʼ��������ʹ��ͨ��
void deInit_enable_pwm_start_encode()
{
	// �����������жϣ���ʹ��
	HAL_TIM_Encoder_Stop(&htim1,TIM_CHANNEL_1);		// PE9
	HAL_TIM_Encoder_Stop(&htim1,TIM_CHANNEL_2);		// PE11
	HAL_TIM_Encoder_Stop(&htim2,TIM_CHANNEL_1);		// PA1
	HAL_TIM_Encoder_Stop(&htim2,TIM_CHANNEL_2);		// PA15
	HAL_TIM_Encoder_Stop(&htim3,TIM_CHANNEL_1);		// PB4
	HAL_TIM_Encoder_Stop(&htim3,TIM_CHANNEL_2);		// PA7
	HAL_TIM_Encoder_Stop(&htim4,TIM_CHANNEL_1);		// PD12
	HAL_TIM_Encoder_Stop(&htim4,TIM_CHANNEL_2);		// PD13
}

// ȡ����ʼ������3ʹ�ܽ����ж�
void deInit_enable_uart3_receive()
{
	HAL_UART_Abort_IT(&huart3);
}

// ȡ����ʼ������ʹ��ͨ�����ж�
void deInit_enable_channel_or_it()
{
	Log_d("��deInit enbale channel or it start��");
	deInit_enable_pwm_start_move();// ȡ����ʼ�����pwmʹ��ͨ��
	deInit_enable_pwm_start_duoji();// ȡ����ʼ�����pwmʹ��ͨ��
	deInit_enable_pwm_start_encode();// ȡ����ʼ��������ʹ��ͨ��
	deInit_enable_uart3_receive();// ȡ����ʼ������3ʹ�ܽ����ж�
	Log_d("��deInit enbale channel or it end��");
}
