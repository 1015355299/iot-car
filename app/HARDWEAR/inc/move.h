#ifndef __MOVE_H
#define __MOVE_H

#include "encode.h"
void move_braking(uint16_t v);// С���ƶ�

void move_turn_left(uint16_t v);// С��ԭ����ת����ʱ�룩
void move_turn_right(uint16_t v);// С��ԭ����ת��˳ʱ�룩
void move_all_dir_fix(uint16_t v,uint16_t angle);// ȫ���ƶ������򲻱�
void Adjust_Speed(int16_t v1,int16_t v2,int16_t v3,int16_t v4);//����
void update_move(void);

extern uint8_t is_braking;
#endif
