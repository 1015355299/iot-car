
#include "pwm.h"
#include "tim.h"
#include "at_log.h"

// ���pwm��ʼֵ
Dj_Degree dj_degree[2]={
	{1500,135},// һά
	{2000,135},// ��ά
};


// ��ȡ��ƽ̨�Ƕȶ�Ӧ�ļ���ֵ
uint16_t Get_Degree_Count(uint8_t pos)
{
	// ��ά
	if(pos)
		return (uint16_t)((0.5 + 0.01111*dj_degree[1].Set_Degree)*1000);
	else 
		return (uint16_t)((0.493 + 0.0074062*dj_degree[0].Set_Degree)*1000);
}

// ���õ�һάƽ̨����Ƕ�
void Set_1_degree(int16_t degree)
{
	// 0.5==0 1 == 70 1.5ms == 135 , 2.0==205  ,2.5 == 270 ,3==70
	// count = 0.493 + 0.0074062*deg
	dj_degree[0].Set_Degree = degree + 135;
	dj_degree[0].Degree_Count = Get_Degree_Count(0);// ��ȡ�Ƕȶ�Ӧ��pwm����ֵ
	
	// ���ö��pwm�趨ֵ
	__HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_3, dj_degree[0].Degree_Count);
	// ���¶����ת�Ƕ�
	car.cur_duoji_1_angle = dj_degree[0].Set_Degree - 135;
}

// ���õڶ�άƽ̨����Ƕ�
void Set_2_degree(int16_t degree)
{
	// 0.5==0, 1==45,1.5==90,2.0==135,2.5==180
	// count = 0.5 + 0.01111*deg
	// ������ǣ���ֹ��ͷ��
	degree = degree < -45? -45 : degree;
	dj_degree[1].Set_Degree = degree + 90;
	dj_degree[1].Degree_Count = Get_Degree_Count(1);// ��ȡ�Ƕȶ�Ӧ��pwm����ֵ
	
	// ���ö��pwm�趨ֵ
	__HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_4, dj_degree[1].Degree_Count);
	// ���¶����ת�Ƕ�
	car.cur_duoji_2_angle = dj_degree[1].Set_Degree - 90;
}

// ���¶���Ƕ�
void update_duoji(void)
{
	// ���¶��1
	Set_1_degree(car.cur_duoji_1_angle);
	// ���¶��2
	Set_2_degree(car.cur_duoji_2_angle);
}

