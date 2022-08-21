#ifndef __IMG_H
#define __IMG_H
#include "main.h"

// 1024*21
#define img_buf_size 1024*16  			//�������ݻ���img_buf�Ĵ�С(*4�ֽ�)

extern uint8_t camera_mode;  			// ��ǰ����ͷģʽ��ʵʱ��Ƶ/����
extern uint8_t camera_workmode; 		// ��ǰ����ͷ����ģʽ��0:rgb565,1:jpeg  
extern uint8_t camera_contrast;  	// �Աȶȵȼ�
extern uint8_t camera_saturation;  // ���Ͷȵȼ� 
extern uint8_t camera_effect;  		// ������ЧЧ��
extern uint8_t camera_autoexposure_level;// �Զ��ع�ȼ�
extern uint8_t camera_light_mode;	// ��ƽ��ģʽ 
extern uint8_t camera_brightness;	// ��������
extern uint8_t camera_color_bar;		// ��������
extern uint8_t camera_size;     	 	// ����ͷ���ͼƬ�ߴ磬Ĭ����QVGA 320*240�ߴ�
extern uint8_t camera_is_abort;		//��;�˳�����ͷ�ɼ���־

extern __align(4) uint32_t img_buf[img_buf_size]; 			//�������ݻ���img_buf
extern volatile uint32_t img_data_len; 			//buf�е�JPEG��Ч���ݳ��� 
extern volatile uint8_t img_data_ok;					//JPEG���ݲɼ���ɱ�־ 
extern uint8_t is_start;

void set_camera_config(void);
void init_camera(void);
void deInit_camera(void);
void collect_image_data(void);
// ֹͣ�����ϴ�ͼƬ����
void suspend_and_send_imgdata(void);
// �ָ����񲢿�ʼ��һ�ֲɼ�
void resume_and_start_next(void);


#endif


