
#include "encode.h"
#include "math.h"
#include "move.h"
#include "at_log.h"


uint16_t Previous_Count[4] = {0,0,0,0};// ������һ�μ���ֵ

TIM_TypeDef* count_timer[4] = {TIM1,TIM2,TIM3,TIM4};

// ��ʱ����
uint16_t Current_Count[4]={0,0,0,0};// ��ǰ����ֵ
uint8_t Current_Direction[4]={0,0,0,0};
uint16_t Current_Speed[4]={0.0,0.0,0.0,0.0};

// ÿ�����*0.01867 = �ٶ�
// ��Ȧ��*30.458 = �ٶ�
//float K=1.0f/12.0f/4.0f/0.01f/34.0f*9.7f*3.14f;// ����ֵ*K = speed cm/s  1.8672456
double K=0.1867;// ����ֵ*K = speed cm/s  0.1sϵ��

uint8_t dir =0;
uint16_t currun = 0;
uint16_t count =0;
uint8_t i;
uint16_t tmp=800;
double new_count;

#define His_Len 5
int16_t His_Out1[His_Len],His_Out2[His_Len],His_Out3[His_Len],His_Out4[His_Len];   		//��ʷϵͳ���ֵ
int16_t His_Val1[His_Len],His_Val2[His_Len],His_Val3[His_Len],His_Val4[His_Len];			//��ʷ����ֵ���ⲿ���£�
int16_t His_Err1[His_Len],His_Err2[His_Len],His_Err3[His_Len],His_Err4[His_Len];			//��ʷƫ��
                     
PID pid[4] ={                
	{0,0,0,0,0,0.2,0.6,0.2,His_Out1,His_Val1,His_Err1,His_Len,180,3},
	{0,0,0,0,0,0.2,0.6,0.2,His_Out2,His_Val2,His_Err2,His_Len,180,3},
	{0,0,0,0,0,0.2,0.6,0.2,His_Out3,His_Val3,His_Err3,His_Len,180,3},
	{0,0,0,0,0,0.2,0.6,0.2,His_Out4,His_Val4,His_Err4,His_Len,180,3}
};

// ��ȡ��Ӧ���һ��ʱ��ļ���ֵ����������ȡ����TIM12���ٶ�ʱ������ 0.1s
uint16_t Read_Encoder_count(uint8_t pos)
{
  uint16_t Count,Current_Count;         

	// ���浱ǰ����ֵ
  Current_Count=count_timer[pos]->CNT;

	if((count_timer[pos]->CR1&0x0010) == 0x0010) //�����ת                               
	{
		Current_Direction[pos]=1;//���·���
		if(Previous_Count[pos] >= Current_Count){
			Count = Previous_Count[pos] - Current_Count;
		}else{
			// �������
			Count = 65536 - Current_Count + Previous_Count[pos];
		}
	}else{//�����ת
		Current_Direction[pos]=0;//���·���
		if(Current_Count >= Previous_Count[pos]){
			Count = Current_Count - Previous_Count[pos];
		}else{
			// �������
			Count = 65536 - Previous_Count[pos] + Current_Count;
		}
	}          
	//Log_d("Count:%d",Count);
  Previous_Count[pos]=Current_Count;// ��Ϊǰһ������ֵ
	return Count>60000?(65536-Count):Count;// �����������
}


// ��ȡ���ֵ�pwm�¼���ֵ
uint16_t Get_New_Count(uint8_t pos)
{
	int16_t tmpVal;
	
	// �ƶ�Ч��
	if(is_braking)
	{
		// 200ms�ƶ�Ч��
		if(count>=3) is_braking=0;// ȡ���ƶ����ָ�pid����
		count++;
		tmpVal=3000;// �ƶ���ѹ
	}else{
		count=0;
		tmpVal = Get_New_Value(&pid[pos]); // ��ȡpid��������趨�ٶ� cm/s
	}
	
	//Log_d("pos:%d-%d",pos,tmpVal);
	new_count = (double)tmpVal/18.67;// ʵ�ʲ��ټ���ֵ0-9,�趨�ٶ�ת���ɶ�Ӧ����ֵ/1000

	// ʵ�ʲ��ټ���ֵ����ɶ�Ӧ��pwm����ֵ����ϼ���ֵ
	new_count=141.1+593.7538*new_count-424.61377*pow(new_count,2)+142.173195*pow(new_count,3)-20.423*pow(new_count,4)+1.0779*pow(new_count,5);
	//Log_d("res:%f",new_count);
	
	// ����ֵ�߽��ж�
	if(new_count<300){// �޷�����
		return 0;
	}else if(new_count>3999){// ����pwm����趨ֵ
		return 3999;
	}else{
		return (uint16_t)new_count;
	}
}
// ���µ�ǰ�����ֵ�ʵ���ٶ�
void Actual_Speed(void)
{
	//Log_d("-----------------------------------------------");
	// 100ms��ȡһ�ε������ֵ��������
	for(i=0;i<4;i++)
	{
		// ��ȡ�������ٲ������ڵļ���ֵ
		Current_Count[i]=Read_Encoder_count(i);
		//Log_d("Current_Count[1]:%d",Current_Count[i]);
		
		// ʵ���ٶȣ�cm/s
		new_count = ((double)Current_Count[i])*K;
		
		// �洢��ǰ�ٶ�
		Current_Speed[i]=(uint16_t)(new_count+0.5);
		
		// �洢��ǰ�ٶȲ���ֵ
		pid[i].His_Val[pid[i].Cur_Index] = Current_Speed[i];
	}
	

	// �����ٶ�����
	car.cur_speed_lq = Current_Direction[0]?-Current_Speed[0]:Current_Speed[0];
	car.cur_speed_rq = Current_Direction[1]?-Current_Speed[1]:Current_Speed[1];
	car.cur_speed_lh = Current_Direction[2]?-Current_Speed[2]:Current_Speed[2];
	car.cur_speed_rh = Current_Direction[3]?-Current_Speed[3]:Current_Speed[3];

	// ���������ٶ�,���ı�Ƕ�ʱ��cm/s
	car.cur_speed_whole = (uint16_t)((Current_Speed[0]+Current_Speed[1]+Current_Speed[2]+Current_Speed[3])/4);
	car.cur_speed_angle = car.set_speed_angle;
	
	// ����pwm�趨ֵ
	TIM8->CCR1 = Get_New_Count(0);
	TIM8->CCR2 = Get_New_Count(1);
	TIM8->CCR3 = Get_New_Count(2);
	TIM13->CCR1 = Get_New_Count(3);
	//Log_d("TIM-COUNT:%d,%d,%d,%d",TIM8->CCR1,TIM8->CCR2,TIM8->CCR3,TIM13->CCR1);
}
