
#include "img.h"
#include "dcmi.h"
#include "ov2640.h"
#include "usart.h"
#include "esp8266.h"
#include "at_log.h"
#include "tim.h"
#include "iwdg.h"

uint8_t camera_mode=0;  			// ��ǰ����ͷģʽ��ʵʱ��Ƶ/����
uint8_t camera_workmode=1; 		// ��ǰ����ͷ����ģʽ��0:rgb565,1:jpeg  
uint8_t camera_contrast=2;  	// �Աȶȵȼ�
uint8_t camera_saturation=4;  // ���Ͷȵȼ� 
uint8_t camera_effect=0;  		// ������ЧЧ��
uint8_t camera_autoexposure_level=0;// �Զ��ع�ȼ�
uint8_t camera_light_mode=4;	// ��ƽ��ģʽ 
uint8_t camera_brightness=3;	// ��������
uint8_t camera_color_bar=0;		// ��������
uint8_t camera_size=4;     	 	// ����ͷ���ͼƬ�ߴ磬Ĭ����QVGA 320*240�ߴ�
uint8_t camera_is_abort=1;		//��;�˳�����ͷ�ɼ���־


__align(4) uint32_t img_buf[img_buf_size];	//���ݻ���buf

volatile uint32_t img_data_len=0; 			//buf�е�JPEG��Ч���ݳ��� 
volatile uint8_t img_data_ok=0;					//JPEG���ݲɼ���ɱ�־ 
uint8_t is_start=0;// ����ͷ������־λ


//JPEG�ߴ�֧���б�
const uint16_t jpeg_img_size_tbl[][2]=
{
	// 91260/x = fps
	160,120,	//QQVGA 0 512=512*4=2048
	176,144,	//QCIF 1 768=768*4=3072     15
	320,240,	//QVGA 2 1536=1536*4=6144   14.8
	400,240,	//WQVGA 3 1792=1792*4=7168  12.7
	352,288,	//CIF 4 2048 = 2048*4=8192   11.1
	640,480,	//VGA 5 4096 = 4096*4=16384  5.6
	800,600,	//SVGA 6 5888=5888*4=23552   3.8
	1024,768,	//XGA  7 8704=8704*4=34816   2.6
	1280,800,	//WXGA 8 10752=10752*4=43008  2.1
	1280,960,	//XVGA 9 12544=12544*4=50176
	1440,900,	//WXGA+ 10 18176=72704
	1280,1024,	//SXGA	11 14080=56320
	1600,1200,	//UXGA	12 20480=81920
};


// �������
void set_camera_config(void)
{
	// �޸ĶԱȶ�
	if(car.camera_contrast!=camera_contrast) 
	{
		camera_contrast = car.camera_contrast;
		OV2640_Contrast(camera_contrast);
	}
	// �޸ı��Ͷ�
	if(car.camera_saturation!=camera_saturation) 
	{
		camera_saturation = car.camera_saturation;
		OV2640_Color_Saturation(camera_saturation);
	}
	// �޸���Ч
	if(car.camera_effect!=camera_effect) 
	{
		camera_effect = car.camera_effect;
		OV2640_Special_Effects(camera_effect);
	}
	// �޸��ع�ȼ�
	if(car.camera_autoexposure_level!=camera_autoexposure_level) 
	{
		camera_autoexposure_level = car.camera_autoexposure_level;
		OV2640_Auto_Exposure(camera_autoexposure_level);
	}
	// �޸İ�ƽ��
	if(car.camera_light_mode!=camera_light_mode) 
	{
		camera_light_mode = car.camera_light_mode;
		OV2640_Light_Mode(camera_light_mode);
	}
	// �޸�����
	if(car.camera_brightness!=camera_brightness) 
	{
		camera_brightness = car.camera_brightness;
		OV2640_Brightness(camera_brightness);
	}
	// �޸Ĳ�������
	if(car.camera_color_bar!=camera_color_bar) 
	{
		camera_color_bar = car.camera_color_bar;
		OV2640_Color_Bar(camera_color_bar);
	}
	// �޸�ͼ��ߴ��С
	if(car.camera_size!=camera_size) 
	{
		camera_size = car.camera_size;
		// �������ͼ��ߴ�
		OV2640_OutSize_Set(jpeg_img_size_tbl[car.camera_size][0],jpeg_img_size_tbl[car.camera_size][1]);
	}
}

// ����ͼ�����ݵ�����
void send_img_data(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
	uint8_t i=0;
	uint8_t min = 1;// ���ٴ������
	uint16_t res;
	if(Size>=16384)//512 16384
	{
		while((Size/++min)>=16384);// ����������ٴ������
	}
	res = Size/min*4 + 1;
	while(i<=min)
	{
		while(HAL_UART_Transmit_DMA(huart,pData+i*res,res));
		i++;
	}
	HAL_UART_AbortTransmit_IT(huart);
}


// ��ʼ������ͷ
void init_camera()
{
	Log_d("��init camera start��");

	HAL_TIM_Base_Start_IT(&htim6);// ����֡���ж϶�ʱ��
	
	if(car.camera_workmode)
		OV2640_JPEG_Mode();		// ����ov2640 jpegģʽ
	else
		OV2640_RGB565_Mode(); // ov2640����RGB565ģʽ
	
	// �������ͼ��ߴ�
	OV2640_OutSize_Set(jpeg_img_size_tbl[car.camera_size][0],jpeg_img_size_tbl[car.camera_size][1]);
	Log_d("��init OV2640_OutSize_Set success��");

	// ���òɼ�ģʽ,������˫����
	HAL_DCMI_Start_DMA(&hdcmi,(car.camera_mode?DCMI_MODE_SNAPSHOT:DCMI_MODE_CONTINUOUS),(uint32_t)&img_buf, img_buf_size);
	is_start=1;// �������ͷ��ʼ�����
	Log_d("��init camera end��");
}

// ֹͣ����ͷ
void deInit_camera()
{
	Log_d("��deInit camera start��");
	
	HAL_UART_DMAStop(&huart5);// ֹͣ����5DMA����
	
	__HAL_DCMI_DISABLE_IT(&hdcmi, DCMI_IT_FRAME);
	
	HAL_TIM_Base_Stop_IT(&htim6);// ֹͣ֡�ʼ����ж�
	
	// esp8266����

	HAL_DCMI_Stop(&hdcmi);

	car.camera_frame=0;
	img_data_ok=0;
	is_start=0;
	UART5_RX_STA=0;
	Log_d("��deInit camera end��");
}

// ֹͣ�����ϴ�ͼƬ����
void suspend_and_send_imgdata()
{
	HAL_DCMI_Suspend(&hdcmi);// ��ͣ����ͼ��
	__HAL_DMA_DISABLE(&hdma_dcmi);//�ر�DMA
	while(DMA2_Stream1->CR&0X01);	//�ȴ�DMA2_Stream1������   
	// ��ȡ���βɼ���һ֡���ݳ��ȣ�����С��buf
	img_data_len=img_buf_size-__HAL_DMA_GET_COUNTER(&hdma_dcmi);
	//Log_d("%d",img_data_len);
	// ����ͼ������
	send_img_data(&huart5,(uint8_t *)img_buf,img_data_len);
}

// �ָ����񲢿�ʼ��һ�ֲɼ�
void resume_and_start_next()
{
	__HAL_DMA_SET_COUNTER(&hdma_dcmi,img_buf_size);// ���ü���
	__HAL_DMA_ENABLE(&hdma_dcmi); //��DMA
	__HAL_DCMI_ENABLE_IT(&hdcmi,DCMI_IT_FRAME);// ������һ֡�ж�
	HAL_DCMI_Resume(&hdcmi);// �ָ�����
	img_data_ok=0;// �����һ�ֲɼ���ʼ
}

//JPEGģʽ�ɼ�ͼ��
void collect_image_data(void)
{
	Log_d("��collect_image_data start��");
	while(1)
	{
		// ֡�ж��л���Ϊ1
		if(img_data_ok==1)	//jpeg���ݲɼ���ɵ�buf�У�׼������buf->uart5��dma����
		{
//			// ֹͣ�����ϴ�ͼƬ����
			suspend_and_send_imgdata();
			//HAL_IWDG_Refresh(&hiwdg);
			//Log_d("%d  %d",img_buf_size*4 ,img_data_len*4);
			
			// �ָ����񲢿�ʼ��һ�ֲɼ�
			resume_and_start_next();
		}

	}    
} 

