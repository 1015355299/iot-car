
#include "pid.h"
#include "at_log.h"

//// ��ȡ��ǰֵ
int16_t Get_New_Value(PID * pid)
{
	if(pid->Init_Count>=2)
	{
		pid->His_Err[pid->Cur_Index] = pid->Set_Val - pid->His_Val[pid->Cur_Index];//������ʷƫ��
		//������ʷ���ֵ������ʽ
		pid->His_Out[pid->Cur_Index] =  pid->His_Out[pid->Pre_Index] + \
		pid->Kp * (pid->His_Err[pid->Cur_Index] - pid->His_Err[pid->Pre_Index]) + \
		pid->Ki * pid->His_Err[pid->Cur_Index] + \
		pid->Kd * (pid->His_Err[pid->Cur_Index] - 2*pid->His_Err[pid->Pre_Index] + pid->His_Err[pid->PPre_Index]);
	}else{// ��һ��
		pid->Init_Count++;
		pid->His_Err[pid->Cur_Index] = pid->Set_Val;//������ʷƫ��
		pid->His_Out[pid->Cur_Index] = pid->Set_Val;//������ʷ���ֵ
	}
	
	// �������ж�
	if(pid->His_Out[pid->Cur_Index] > pid->Upper_Limit){
		pid->His_Out[pid->Cur_Index] = pid->Upper_Limit;
	}else if(pid->His_Out[pid->Cur_Index] <= pid->Lower_Limit){
		pid->His_Out[pid->Cur_Index] = pid->Lower_Limit;
	}

	// У�㣬������ʷ���Ӱ��
	if(pid->Set_Val==0&&pid->His_Val[pid->Cur_Index]==0) pid->His_Out[pid->Cur_Index]=0;
	
	//��������
	if((++(pid->Cur_Index)) >= pid->His_Len)
	{
		pid->Cur_Index = 0;
		pid->Pre_Index++;
		pid->PPre_Index++;
	}else{
		pid->Pre_Index = pid->Cur_Index - 1;
		if(pid->Pre_Index>0)
			pid->PPre_Index = pid->Pre_Index - 1;
		else
			pid->PPre_Index = pid->Init_Count>=2 ? (pid->PPre_Index + 1) : 0;
	}
	
	return pid->His_Out[pid->Pre_Index];
}
