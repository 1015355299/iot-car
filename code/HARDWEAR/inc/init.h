#ifndef __INIT_H
#define __INIT_H
#include "main.h"

// led�Ƶ�����˸
void flicker_led(uint8_t group,uint8_t flicker,uint8_t sync,uint8_t keep,uint16_t delay);
// ��ʼ��ѡ��������ʽ
void init_net_method(void);
// ��ʼ��������ʱ��-������������
void init_base_encode(void);
	// ��ʼ��������ʱ��-���pwm
void init_base_move_pwm(void);
	// ��ʼ��������ʱ��-���pwm
void init_base_duoji_pwm(void);
// ��ʼ��������ʱ��-����������ʱ�� us
void init_base_other(void);

// ��ʼ��������ʱ��-���л�����ʱ��
void init_base_all(void);

// ��ʼ��ov2640ģ��
void init_ov2640(void);
// ��ʼ�����pwmʹ��ͨ��
void init_enable_pwm_start_move(void);
// ��ʼ�����pwmʹ��ͨ��
void init_enable_pwm_start_duoji(void);
// ��ʼ��������ʹ��ͨ��
void init_enable_pwm_start_encode(void);
// ��ʼ������3ʹ�ܽ����ж�
void init_enable_uart3_receive(void);

// ��ʼ������ʹ��ͨ�����ж�
void init_enable_channel_or_it(void);



// ȡ����ʼ��������ʱ��-������������
void deInit_base_encode(void);
// ȡ����ʼ��������ʱ��-���pwm
void deInit_base_move_pwm(void);
// ȡ����ʼ��������ʱ��-���pwm
void deInit_base_duoji_pwm(void);	
// ȡ����ʼ��������ʱ��-����������ʱ�� us
void deInit_base_other(void);

// ȡ����ʼ��������ʱ��-���л�����ʱ��
void deInit_base_all(void);

// ȡ����ʼ�����pwmʹ��ͨ��
void deInit_enable_pwm_start_move(void);
// ȡ����ʼ�����pwmʹ��ͨ��
void deInit_enable_pwm_start_duoji(void);
// ȡ����ʼ��������ʹ��ͨ��
void deInit_enable_pwm_start_encode(void);
// ȡ����ʼ������3ʹ�ܽ����ж�
void deInit_enable_uart3_receive(void);

// ȡ����ʼ������ʹ��ͨ�����ж�
void deInit_enable_channel_or_it(void);



#endif


