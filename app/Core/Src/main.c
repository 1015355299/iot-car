/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "dcmi.h"
#include "dma.h"
#include "iwdg.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "move.h"
#include "pwm.h"
#include "sccb.h"
#include "ov2640.h"
#include "img.h"
#include "esp8266.h"
#include "at_log.h"
#include "qcloud_iot_api_export.h"
#include "init.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t p;
uint8_t frame_count=0;
uint8_t ENTRY_NETWORK_METHOD = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
CAR car={
	// �ٶ�
	0,// ��ǰ���峵�٣�cm/s
	0,// �ƶ��Ƕ�
	0,// ��ǰ���ٶ�
	0,// ��ǰ���ٶ�
	0,// ������ٶ�
	0,// �Һ����ٶ�
	// �����ٶ�
	0,// ��ǰ���峵�٣�cm/s
	0,// �ƶ��Ƕ�
	0,// ��ת���� 0����ת��1��ת��-1��ת
	20,// ��ת�ٶ�
	// ����Ƕ�
	0,// һάƽ̨��ת�Ƕȣ�������ǰ���Ƕ�,���Ϊ�����ұ�Ϊ��-135~135
	45,// ��άƽ̨��ת�Ƕȣ�������ǰ���Ƕ�,����Ϊ��������Ϊ��-90~90
	// ����ͷ״̬
	0,// ��ǰ����ͷģʽ��ʵʱ��Ƶ/����
	1,// ��ǰ����ͷ����ģʽ��0:rgb565,1:jpeg  
	2,// �Աȶȵȼ�
	4,// ���Ͷȵȼ� 
	0,// ������ЧЧ��
	0,// �Զ��ع�ȼ�
	4,// ��ƽ��ģʽ 
	3,// ��������
	0,// ��������
	4,// ����ͷ���ͼƬ�ߴ� 8 5 4 2 5
	1,// ��;�˳�����ͷ�ɼ���־
	// ������־
	0,// ֡��
	0,// С�����ӿ���״̬
	1,// С�����
	"VlzbGfFuvRV8o67sTlRLMw==",// �豸��Կ
	0,// ����wifi2
};
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	//SCB->VTOR = FLASH_BASE|0x10000;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
	
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_USART3_UART_Init();
  MX_DCMI_Init();
  MX_TIM8_Init();
  MX_TIM11_Init();
  MX_TIM7_Init();
  MX_UART5_Init();
  MX_TIM13_Init();
  MX_TIM6_Init();
  MX_IWDG_Init();
  MX_TIM12_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
	Log_d("-----------��begin��-----------");

	// ��ʼ��������ʱ��-���л�����ʱ��
	init_base_all();

	// ��ʼ��ov2640ģ��
	init_ov2640();
	
	// ��ʼ������ʹ��ͨ�����ж�
	init_enable_channel_or_it();
	
	// ��ʼ��ѡ��������ʽ
	init_net_method();// ����led��˸3s��ѡ��������ʽ����key0ѡ��SmartConfig������Ĭ��SoftAP

	atk_8266_init();

////		// ��������ͷ
//	init_camera();
//	collect_image_data();
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();
  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	// ����ִ��ʧ�ܣ��ܷ�
  while (1)
  {
		//NVIC_SystemReset();// ϵͳ��λ
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* DMA1_Stream7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);
  /* DMA2_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
  /* DCMI_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DCMI_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DCMI_IRQn);
  /* TIM8_BRK_TIM12_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM8_BRK_TIM12_IRQn, 11, 0);
  HAL_NVIC_EnableIRQ(TIM8_BRK_TIM12_IRQn);
  /* TIM6_DAC_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 12, 0);
  HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
  /* USART1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART1_IRQn, 9, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USART3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART3_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* TIM1_TRG_COM_TIM11_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM1_TRG_COM_TIM11_IRQn, 8, 0);
  HAL_NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);
  /* UART5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(UART5_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(UART5_IRQn);
}

/* USER CODE BEGIN 4 */
// ����С�����ݣ�ֻ��mqtt������ָ֪ͨ�������״̬��С������ͨ������5
void update_state()
{
	if(car.reset_wifi2){
		car.reset_wifi2=0;
		atk_8266_init();
	}
	if(car.camera_is_abort==1&&is_start==1){
		deInit_camera();// �ر�����ͷ
	}else if(is_start==0&&car.camera_is_abort==0){
		init_camera();// ��������ͷ
	}
	if(!car.camera_is_abort) set_camera_config();// ��������ͷ����
	update_duoji();// ���¶������
}



// ֡ɨ���ж�
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	__HAL_DCMI_CLEAR_FLAG(hdcmi,DCMI_FLAG_FRAMERI);//���֡�ж�
	if(img_data_ok==0) img_data_ok=1;// ���һ��ɨ�����
	frame_count++;// ֡����
}


// ���ڽ����ж�
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	// ����δ���
	if(huart->Instance==UART5 && (UART5_RX_STA&0x8000)==0)
	{		
		// С��ָ�����
		if(init_wifi2_finish)
		{	
			switch(UART5_RX_BUF[UART5_RX_STA])
			{
				case '@':HAL_UART_Receive_IT(&huart5,&UART5_RX_BUF[0],1);UART5_RX_STA=0;break;// ����ָ�ʼ
				case '$':UART5_RX_BUF[UART5_RX_STA]='\0';UART5_RX_STA|=0x8000;break;// ����ָ���β
				default:
					if(UART5_RX_STA+1>=UART5_MAX_RECV_LEN){
						UART5_RX_BUF[UART5_RX_STA]='\0';UART5_RX_STA|=0x8000;
					}else{
						HAL_UART_Receive_IT(&huart5,&UART5_RX_BUF[++UART5_RX_STA],1);// ������������
					}
			}
		}else{
			// wifi2��ʼ��
			// ֱ�����50msû����������ɽ���
			if(UART5_RX_STA==0) HAL_TIM_Base_Start_IT(&htim11);
			__HAL_TIM_SET_COUNTER(&htim11,0);
			HAL_UART_Receive_IT(&huart5,&UART5_RX_BUF[++UART5_RX_STA],1);// �����´ν���
			if(UART5_RX_STA+1>=UART5_MAX_RECV_LEN) UART5_RX_STA=0;// �������գ����ý���
		}
	}
}
/* USER CODE END 4 */

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM10 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
	//printf("12\r\n");
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM10) {
		//printf("22\r\n");
    HAL_IncTick();
		
  }
  /* USER CODE BEGIN Callback 1 */
	// TIM12���ٸ����ж�
	if (htim->Instance == TIM12) {
    Actual_Speed();// ����
		return;
  }
	// ����5�õ��Ķ�ʱ��
	if(htim->Instance == TIM11)
	{
		HAL_TIM_Base_Stop_IT(&htim11);
		HAL_UART_AbortReceive_IT(&huart5);// ��ֹ����5�����ж�
		UART5_RX_BUF[UART5_RX_STA&0x7fff]='\0';
		UART5_RX_STA|=0x8000;	//��ǽ������
		return;
	}
	// TIM6һ��֡���ж�
	if (htim->Instance == TIM6) {
		//Log_d("fps:%d",frame_count);
    car.camera_frame = frame_count;
		frame_count = 0;
		return;
  }
  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
		flicker_led(2,2,1,2,100);
		flicker_led(2,2,1,2,100);
		flicker_led(2,2,1,2,100);
		flicker_led(2,2,1,2,100);
		flicker_led(2,2,1,0,100);
		NVIC_SystemReset();// ϵͳ��λ
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */

  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
