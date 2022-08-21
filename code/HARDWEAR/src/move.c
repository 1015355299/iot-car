
#include "move.h"
#include "math.h"
#include "at_log.h"

// 4��������߼�io
//      ��  ��        ��  ��
// ǰ�� LQ1 RQ1  ���� LQ1 RQ1 ǰ��
//      LH2 RH2       LH2 RH2 ����
// ����4���������
#define LH1(x) HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,x) // ����� PB3  PB10
#define LH2(x) HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,x)
#define LQ1(x) HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,x) // ��ǰ�� PD0  PD3
#define LQ2(x) HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,x)
#define RQ1(x) HAL_GPIO_WritePin(GPIOD,GPIO_PIN_5,x) // ��ǰ�� PD5  PG10
#define RQ2(x) HAL_GPIO_WritePin(GPIOG,GPIO_PIN_10,x)
#define RH1(x) HAL_GPIO_WritePin(GPIOE,GPIO_PIN_0,x) // �Һ��� PE0  PC1
#define RH2(x) HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,x)

// L298N
// EN  IN1  IN2  ��Ϊ
// 0    x    x   ֹͣ
// 1    0    0   �ƶ�
// 1    0    1   ǰ��
// 1    1    0   ����
// 1    1    1   �ƶ�

uint8_t is_braking=0;

// С���ƶ�
void move_braking(uint16_t v)
{
	is_braking=1;
	LQ1(GPIO_PIN_RESET);// ��ǰ��
	LQ2(GPIO_PIN_RESET);
	
	RQ1(GPIO_PIN_RESET);// ��ǰ��
	RQ2(GPIO_PIN_RESET);
	
	LH1(GPIO_PIN_RESET);// �����
	LH2(GPIO_PIN_RESET);
	
	RH1(GPIO_PIN_RESET);// �Һ���
	RH2(GPIO_PIN_RESET);
	Adjust_Speed(v,v,v,v);
}


// С��ԭ����ת����ʱ�룩
void move_turn_left(uint16_t v)
{
	LQ1(GPIO_PIN_SET);// ��ǰ��
	LQ2(GPIO_PIN_RESET);
	
	RQ1(GPIO_PIN_RESET);// ��ǰ��
	RQ2(GPIO_PIN_SET);
	
	LH1(GPIO_PIN_SET);// �����
	LH2(GPIO_PIN_RESET);
	
	RH1(GPIO_PIN_RESET);// �Һ���
	RH2(GPIO_PIN_SET);
	Adjust_Speed(v,v,v,v);
}

// С��ԭ����ת��˳ʱ�룩
void move_turn_right(uint16_t v)
{
	LQ1(GPIO_PIN_RESET);// ��ǰ��
	LQ2(GPIO_PIN_SET);
	
	RQ1(GPIO_PIN_SET);// ��ǰ��
	RQ2(GPIO_PIN_RESET);
	
	LH1(GPIO_PIN_RESET);// �����
	LH2(GPIO_PIN_SET);
	
	RH1(GPIO_PIN_SET);// �Һ���
	RH2(GPIO_PIN_RESET);
	Adjust_Speed(v,v,v,v);
}


// �̶���������Ƕ�ƽ��
// �ƶ��ٶ�v cm���ƶ�������ǰ���������ʱ��н�angle ��
// �������ٶ�  ��  ��
//             v1  v2  ǰ��
//             v3  v4  ����
// С��ȫ���ƶ�
void move_all_dir_fix(uint16_t v,uint16_t angle)
{
	int16_t v1,v2,v3,v4;
	float cosVal,sinVal;
	cosVal = cosf(angle*0.017453);
	sinVal = sinf(angle*0.017453);
	v1 = v4 = (int16_t)(v*(cosVal - sinVal));
	v2 = v3 = (int16_t)(v*(cosVal + sinVal));
	
	// ��ǰ��
	if(v1>=0)
	{
		LQ1(GPIO_PIN_RESET);// ǰ��
		LQ2(GPIO_PIN_SET);
	}else{
		LQ1(GPIO_PIN_SET);// ����
		LQ2(GPIO_PIN_RESET);
	}

	// ��ǰ��
	if(v2>=0)
	{
		RQ1(GPIO_PIN_RESET);// ǰ��
		RQ2(GPIO_PIN_SET);
	}else{
		RQ1(GPIO_PIN_SET);// ����
		RQ2(GPIO_PIN_RESET);
	}
	
	// �����
	if(v3>=0)
	{
		LH1(GPIO_PIN_RESET);// ǰ��
		LH2(GPIO_PIN_SET);
	}else{
		LH1(GPIO_PIN_SET);// ����
		LH2(GPIO_PIN_RESET);
	}
	
	// �Һ���
	if(v4>=0)
	{
		RH1(GPIO_PIN_RESET);// ǰ��
		RH2(GPIO_PIN_SET);
	}else{
		RH1(GPIO_PIN_SET);// ����
		RH2(GPIO_PIN_RESET);
	}
	Adjust_Speed(v1,v2,v3,v4);
}

// ���٣�С�������ٶ�
void Adjust_Speed(int16_t v1,int16_t v2,int16_t v3,int16_t v4)
{
	//Log_d("speed:%d,%d,%d,%d",v1,v2,v3,v4);
	pid[0].Set_Val = v1 < 0 ? -v1 : v1;
	pid[1].Set_Val = v2 < 0 ? -v2 : v2;
	pid[2].Set_Val = v3 < 0 ? -v3 : v3;
	pid[3].Set_Val = v4 < 0 ? -v4 : v4;
}
// ����С���ƶ�
void update_move(void)
{
	is_braking=0;
	// ��ת
	if(car.set_speed_dir)
	{
		if(car.set_dir_speed_whole==0)
		{
			move_braking(0);// �ƶ�
		}else{
			car.set_speed_dir==1?move_turn_left(car.set_dir_speed_whole):move_turn_right(car.set_dir_speed_whole);
		}
	}else{// ƽ��
		if(car.set_speed_whole==0)
		{
			move_braking(0);// �ƶ�
		}else{
			move_all_dir_fix(car.set_speed_whole,car.set_speed_angle);
		}
	}
}

