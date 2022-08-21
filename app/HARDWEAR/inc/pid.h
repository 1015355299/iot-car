#ifndef __PID_H
#define __PID_H

#include "main.h"

typedef struct
{
	uint16_t Cur_Index;        //��ǰֵ����
	uint16_t Pre_Index;        //ǰһ������
	uint16_t PPre_Index;       //ǰ��������
	uint8_t Init_Count;        //������ʼ��
	int16_t Set_Val;      	//�趨Ҫ�ﵽ��ֵ���ⲿ�ṩֵ��
	float Kp;							//����ϵ�����ⲿ�ṩֵ��
  float Ki;							//����ϵ�����ⲿ�ṩֵ��
	float Kd;							//΢��ϵ�����ⲿ�ṩֵ��
	int16_t *His_Out;   		//��ʷϵͳ���ֵ
	int16_t *His_Val;				//��ʷ����ֵ���ⲿ���£�
	int16_t *His_Err;				//��ʷƫ��
	uint16_t His_Len;        //��ʷ���鳤��
	int16_t Upper_Limit;      //����
	int16_t Lower_Limit;      //����
}PID;

int16_t Get_New_Value(PID * pid);


#endif
