/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2019 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "qcloud_iot_api_export.h"
#include "lite-utils.h"
#include "at_client.h"
#include "string.h"
#include "data_config.c"

#include "img.h"
#include "dcmi.h"
#include "ov2640.h"
#include "usart.h"
#include "esp8266.h"
#include "iwdg.h"
#include "move.h"
#include "init.h"


static bool sg_control_msg_arrived = false;
static char sg_data_report_buffer[AT_CMD_MAX_LEN];// �ϱ����ݻ�����
static size_t sg_data_report_buffersize = sizeof(sg_data_report_buffer) / sizeof(sg_data_report_buffer[0]);

/* �¼� */
#ifdef EVENT_POST_ENABLED
#include "events_config.c"
#ifdef	EVENT_TIMESTAMP_USED
// �����¼� ʱ���
static void update_events_timestamp(sEvent *pEvents, int count)
{
	int i;
	
	for(i = 0; i < count; i++){
        if (NULL == (&pEvents[i])) { 
	        Log_e("null event pointer"); 
	        return; 
        }
		pEvents[i].timestamp = HAL_GetTimeSeconds();
	}
}
#endif 
// �¼��ظ�֮��Ļص�������¼���־(�ظ��������·����¼�ָ��)
static void event_post_cb(char *msg, void *context)
{
	Log_d("eventReply:%s", msg);
	IOT_Event_clearFlag(context, FLAG_EVENT0|FLAG_EVENT1|FLAG_EVENT2|FLAG_EVENT3);
}

#endif

/* ��Ϊ */
#ifdef ACTION_ENABLED
#include "action_config.c"

// �������·��յ���Ϊָ��֮��Ļص��������߼�������ִ�н����Ӧ��������
// action : regist action and set the action handle callback, add your aciton logic here
static void OnActionCallback(void *pClient, const char *pClientToken, DeviceAction *pAction) 
{	
	int i;
	sReplyPara replyPara;

	//do something base on input, just print as an sample
	DeviceProperty *pActionInput = pAction->pInput;
	for (i = 0; i < pAction->input_num; i++) {		
		if (JSTRING == pActionInput[i].type) {
			Log_d("Input:[%s], data:[%s]",  pActionInput[i].key, pActionInput[i].data);
			HAL_Free(pActionInput[i].data);
		} else {
			if(JINT32 == pActionInput[i].type) {
				Log_d("Input:[%s], data:[%d]",  pActionInput[i].key, *((int*)pActionInput[i].data));
			} else if( JFLOAT == pActionInput[i].type) {
				Log_d("Input:[%s], data:[%f]",  pActionInput[i].key, *((float*)pActionInput[i].data));
			} else if( JUINT32 == pActionInput[i].type) {
				Log_d("Input:[%s], data:[%u]",  pActionInput[i].key, *((uint32_t*)pActionInput[i].data));
			}
		}
	}	

	// construct output 
	memset((char *)&replyPara, 0, sizeof(sReplyPara));
	replyPara.code = eDEAL_SUCCESS;
	replyPara.timeout_ms = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;						
	strcpy(replyPara.status_msg, "action execute success!"); //add the message about the action resault 
	
	
	DeviceProperty *pActionOutnput = pAction->pOutput;	
	(void)pActionOutnput; //elimate warning
	//TO DO: add your aciont logic here and set output properties which will be reported by action_reply
	
	// �ظ�
	IOT_ACTION_REPLY(pClient, pClientToken, sg_data_report_buffer, sg_data_report_buffersize, pAction, &replyPara); 	
}

// ע����Ϊ������ģ�壬��������Ϊ��ӻص�����
static int _register_data_template_action(void *pTemplate_client)
{
	int i,rc;
	
    for (i = 0; i < TOTAL_ACTION_COUNTS; i++) {
	    rc = IOT_Template_Register_Action(pTemplate_client, &g_actions[i], OnActionCallback);
	    if (rc != QCLOUD_RET_SUCCESS) {
	        rc = IOT_Template_Destroy(pTemplate_client);
	        Log_e("register device data template action failed, err: %d", rc);
	        return rc;
	    } else {
	        Log_i("data template action=%s registered.", g_actions[i].pActionId);
	    }
    }

	return QCLOUD_RET_SUCCESS;
}
#endif

// ��������Ļص�����
static void OnControlMsgCallback(void *pClient, const char *pJsonValueBuffer, uint32_t valueLength, DeviceProperty *pProperty) 
{
    int i = 0;

    for (i = 0; i < TOTAL_PROPERTY_COUNT; i++) {		
        if (strcmp(sg_DataTemplate[i].data_property.key, pProperty->key) == 0) {
            sg_DataTemplate[i].state = eCHANGED;
            //Log_i("Property=%s changed", pProperty->key);
            sg_control_msg_arrived = true;
            return;
        }
    }

    Log_e("Property=%s changed no match", pProperty->key);
}

// ע������ģ������
static int _register_data_template_property(void *ptemplate_client)
{
	int i,rc;
	
    for (i = 0; i < TOTAL_PROPERTY_COUNT; i++) {
	    rc = IOT_Template_Register_Property(ptemplate_client, &sg_DataTemplate[i].data_property, OnControlMsgCallback);
	    if (rc != QCLOUD_RET_SUCCESS) {
	        rc = IOT_Template_Destroy(ptemplate_client);
	        Log_e("register device data template property failed, err: %d", rc);
	        return rc;
	    } else {
	        //Log_i("data template property=%s registered.", sg_DataTemplate[i].data_property.key);
	    }
    }

	return QCLOUD_RET_SUCCESS;
}

// �ϱ��ظ�����Ļص�����
static void OnReportReplyCallback(void *pClient, Method method, ReplyAck replyAck, const char *pJsonDocument, void *pUserdata) {
	//Log_i("recv report_reply(ack=%d): %s", replyAck,pJsonDocument);
}


/*�û���Ҫʵ�ֵ��������ݵ�ҵ���߼�,���û�ʵ��*/
static void deal_down_stream_user_logic(void *pClient, ProductDataDefine   * pData)
{
	// ��������
	car.set_speed_whole = pData->m_set_speed_whole;// ��ǰ���峵�٣�cm/s
	car.set_speed_angle = pData->m_set_speed_angle;// �ƶ��Ƕ�	
	car.set_speed_dir = pData->m_set_speed_dir;// ��ת����
	car.set_dir_speed_whole = pData->m_set_dir_speed_whole;// ��ת�ٶ�
	car.cur_duoji_1_angle = -pData->m_cur_duoji_1_angle;// һάƽ̨��ת�Ƕȣ�������ǰ���Ƕ�,���Ϊ�����ұ�Ϊ��-135~135
	car.cur_duoji_2_angle = -pData->m_cur_duoji_2_angle+45;// ��άƽ̨��ת�Ƕȣ�������ǰ���Ƕ�,����Ϊ��������Ϊ��-90~90
	car.camera_mode = pData->m_camera_mode;// ��ǰ����ͷģʽ��ʵʱ��Ƶ/����
	car.camera_workmode = pData->m_camera_workmode;// ��ǰ����ͷ����ģʽ��0:rgb565,1:jpeg  
	car.camera_contrast = pData->m_camera_contrast;// �Աȶȵȼ�
	car.camera_saturation = pData->m_camera_saturation;// ���Ͷȵȼ� 
	car.camera_effect = pData->m_camera_effect;// ������ЧЧ��
	car.camera_autoexposure_level = pData->m_camera_autoexposure_level;// �Զ��ع�ȼ�
	car.camera_light_mode = pData->m_camera_light_mode;// ��ƽ��ģʽ 
	car.camera_brightness = pData->m_camera_brightness;// ��������
	car.camera_color_bar = pData->m_camera_color_bar;// ��������
	car.camera_size = pData->m_camera_size;// ����ͷ���ͼƬ�ߴ�
	car.camera_is_abort = pData->m_camera_is_abort;// ��;�˳�����ͷ�ɼ���־
	car.reset_wifi2 = pData->m_reset_wifi2;// ����wifi2��־λ
	//Log_d("��user data is update��");
}

/*�û���Ҫʵ�ֵ��������ݵ�ҵ���߼�,�˴�����ʾ��*/
static int deal_up_stream_user_logic(void *pClient, DeviceProperty *pReportDataList[], int *pCount,ProductDataDefine   * pData)
{
	int i, j;

	// ��������
	pData->m_cur_speed_whole = car.cur_speed_whole;// ��ǰ���峵�٣�cm/s
	pData->m_cur_speed_angle=car.cur_speed_angle ;// �ƶ��Ƕ�	
	pData->m_set_speed_dir = car.set_speed_dir;// ��ǰ�ƶ�����
	pData->m_set_dir_speed_whole=car.set_dir_speed_whole ;// ��ת�ٶ�
	pData->m_cur_speed_lq=car.cur_speed_lq ;// ��ǰ���ٶ�
	pData->m_cur_speed_rq=car.cur_speed_rq ;// ��ǰ���ٶ�
	pData->m_cur_speed_lh=car.cur_speed_lh ;// ������ٶ�
	pData->m_cur_speed_rh=car.cur_speed_rh ;// �Һ����ٶ�
	pData->m_cur_duoji_1_angle=-car.cur_duoji_1_angle ;// һάƽ̨��ת�Ƕȣ�������ǰ���Ƕ�,���Ϊ�����ұ�Ϊ��-135~135
	pData->m_cur_duoji_2_angle=-car.cur_duoji_2_angle+45 ;// ��άƽ̨��ת�Ƕȣ�������ǰ���Ƕ�,����Ϊ��������Ϊ��-90~90
	pData->m_camera_mode=car.camera_mode ;// ��ǰ����ͷģʽ��ʵʱ��Ƶ/����
	pData->m_camera_workmode=car.camera_workmode ;// ��ǰ����ͷ����ģʽ��0:rgb565,1:jpeg  
	pData->m_camera_contrast=car.camera_contrast ;// �Աȶȵȼ�
	pData->m_camera_saturation=car.camera_saturation ;// ���Ͷȵȼ� 
	pData->m_camera_effect=car.camera_effect ;// ������ЧЧ��
	pData->m_camera_autoexposure_level=car.camera_autoexposure_level ;// �Զ��ع�ȼ�
	pData->m_camera_light_mode=car.camera_light_mode ;// ��ƽ��ģʽ 
	pData->m_camera_brightness=car.camera_brightness ;// ��������
	pData->m_camera_color_bar=car.camera_color_bar ;// ��������
	pData->m_camera_size=car.camera_size ;// ����ͷ���ͼƬ�ߴ�
	pData->m_camera_is_abort=car.camera_is_abort ;// ��;�˳�����ͷ�ɼ���־
	pData->m_camera_frame=car.camera_frame ;// ֡��
	pData->m_car_control_transmission_state=car.car_control_transmission_state;// С�����ӿ���״̬
	strcpy( pData->m_car_secret, car.car_secret);// �豸��Կ
	pData->m_reset_wifi2=car.reset_wifi2;// ����wifi2��־λ
	//check local property state
	//_refresh_local_property. if property changed, set sg_DataTemplate[i].state = eCHANGED;
		// �����Ը����Ƿ���ı�־
    for (i = 0, j = 0; i < TOTAL_PROPERTY_COUNT; i++) {       
        //if(eCHANGED == sg_DataTemplate[i].state) {
            pReportDataList[j++] = &(sg_DataTemplate[i].data_property);
						//sg_DataTemplate[i].state = eNOCHANGE;
						sg_DataTemplate[i].state = eCHANGED;
        //}
				//Log_d("��user data is update��%s:%lf",pReportDataList[i]->key,pReportDataList[i]->data);
    }
	*pCount = j;
	return (*pCount > 0)?QCLOUD_RET_SUCCESS:QCLOUD_ERR_FAILURE;
}

// ���ӷ�����������׼��
static eAtResault net_prepare(void)
{
	eAtResault Ret;
	DeviceInfo sDevInfo;
	
	// ��ȡ�ͻ���ʵ��
	at_client_t pclient = at_client_get();	
	
	// ��ȡ�豸��Ϣ
	memset((char *)&sDevInfo, '\0', sizeof(DeviceInfo));
	Ret = (eAtResault)HAL_GetDevInfo(&sDevInfo);
	if(QCLOUD_RET_SUCCESS != Ret){
		Log_e("Get device info err");
		return QCLOUD_ERR_FAILURE;
	}
	
	
	// ����ģ���ʼ��
	if(QCLOUD_RET_SUCCESS != module_init(eMODULE_ESP8266)) 
	{
		Log_e("module init failed");
		goto exit;
	}else{
		Log_d("module init success");	
	}

	
	// �ȴ����������̳߳�ʼ��
	while(AT_STATUS_INITIALIZED != pclient->status)
	{	
		HAL_SleepMs(1000);
	}
	
	
	// �ȴ���������ģ��
	Log_d("Start shakehands with module...");
	Ret = module_handshake(CMD_TIMEOUT_MS);
	if(QCLOUD_RET_SUCCESS != Ret)
	{
		Log_e("module connect fail,Ret:%d", Ret);
		goto exit;
	}else{
		Log_d("module connect success");
	}
	
	// ��ʼ��iot�豸��Ϣ��������ȡ
	Ret = iot_device_info_init(sDevInfo.product_id, sDevInfo.device_name, sDevInfo.devSerc);
	if(QCLOUD_RET_SUCCESS != Ret)
	{
		Log_e("dev info init fail,Ret:%d", Ret);
		goto exit;
	}
	
	
	// ����ģ����Ϣ
	Ret = module_info_set(iot_device_info_get(), ePSK_TLS);
	if(QCLOUD_RET_SUCCESS != Ret)
	{
		Log_e("module info set fail,Ret:%d", Ret);
	}

exit:

	return Ret;
}

// �¼��ϱ������
static void eventPostCheck(void *client)
{
#ifdef EVENT_POST_ENABLED	
	int rc;
	int i;
	uint32_t eflag;
	sEvent *pEventList[EVENT_COUNTS];
	uint8_t event_count;
		
	// �¼��ϱ�
	IOT_Event_setFlag(client, FLAG_EVENT0|FLAG_EVENT1|FLAG_EVENT2|FLAG_EVENT3);
	eflag = IOT_Event_getFlag(client);
	if((EVENT_COUNTS > 0 )&& (eflag > 0))
	{	
		event_count = 0;
		for(i = 0; i < EVENT_COUNTS; i++)
		{
		
			if((eflag&(1<<i))&ALL_EVENTS_MASK)
			{
				 pEventList[event_count++] = &(g_events[i]);				 
				 IOT_Event_clearFlag(client, 1<<i);
#ifdef	EVENT_TIMESTAMP_USED				 
				 update_events_timestamp(&g_events[i], 1);
#endif
			}			
		}	

		rc = IOT_Post_Event(client, sg_data_report_buffer, sg_data_report_buffersize, \
											event_count, pEventList, event_post_cb);
		if(rc < 0)
		{
			Log_e("events post failed: %d", rc);
		}
	}
#endif

}

// ��ȡ�豸ϵͳ��Ϣ�������������Ϣ�ֶΣ��ƶ˽������豸��Ϣ�ֶ�
static int _get_sys_info(void *handle, char *pJsonDoc, size_t sizeOfBuffer)
{
	/*ƽ̨�����ֶΣ���ѡ�ֶ�������һ��*/
    DeviceProperty plat_info[] = {
     	{.key = "module_hardinfo", .type = TYPE_TEMPLATE_STRING, .data = "ESP8266"},
     	{.key = "module_softinfo", .type = TYPE_TEMPLATE_STRING, .data = "V1.0"},
     	{.key = "fw_ver", 		   .type = TYPE_TEMPLATE_STRING, .data = QCLOUD_IOT_AT_SDK_VERSION},
//     	{.key = "imei", 		   .type = TYPE_TEMPLATE_STRING, .data = "11-22-33-44"},
//     	{.key = "lat", 			   .type = TYPE_TEMPLATE_STRING, .data = "22.546015"},
//     	{.key = "lon", 			   .type = TYPE_TEMPLATE_STRING, .data = "113.941125"},
        {NULL, NULL, JINT32}  //����
	};
		
	/*�Զ��帽����Ϣ*/
	DeviceProperty self_info[] = {
        {.key = "test2", .type = TYPE_TEMPLATE_STRING, .data = "debug"},
        {NULL, NULL, JINT32}  //����
	};

	return IOT_Template_JSON_ConstructSysInfo(handle, pJsonDoc, sizeOfBuffer, plat_info, self_info); 	
}

// ����������ģ������
void data_template_demo_task(void *arg)
{
	eAtResault Ret;
	int rc;
	uint8_t retry_count=0;

	void *client = NULL;
	// ��ȡ�ͻ���ʵ��
	at_client_t pclient = at_client_get();	
	
	int ReportCont;
	DeviceProperty *pReportDataList[TOTAL_PROPERTY_COUNT];

/**
 * �������߼�
 */
	Log_d("��data_template_demo_task init start��");
	do  
	{
		// ����֮ǰ��׼��
		Ret = net_prepare();
		if(QCLOUD_RET_SUCCESS != Ret)
		{
			Log_e("��net prepare fail,Ret:%d��", Ret);
		}
		Log_d("��init net prepare success��");
				// ���¿��Ź�
		HAL_IWDG_Refresh(&hiwdg);
		
RESET:	
		Log_d("��----entry net----��");
		

		// ����ģ���ʼ������������mqtt������
		Ret = module_register_network(eMODULE_ESP8266);
		if(QCLOUD_RET_SUCCESS != Ret)
		{			
			Log_e("��network connect fail,Ret:%d��", Ret);
			goto RESET;
		}
		Log_d("��init network connect success��");
		// �����ɹ�
		
		// ��ʼ��mqttģ��
		Ret = (eAtResault)IOT_Template_Construct(&client);
		if(QCLOUD_RET_SUCCESS != Ret)
		{
			Log_e("��data template construct fail,Ret:%d��", Ret);
			goto RESET;
		}
		Log_d("��init data template construct success��");

		
		// ��ʼ������ģ�壬�ڲ�ʵ��
		_init_data_template();

				
		// ע������ģ������
		rc = _register_data_template_property(client);
		if (rc != QCLOUD_RET_SUCCESS) 
		{
			Log_e("��Register data template propertys Failed: %d��", rc);
			goto RESET;
		}
		Log_d("��init Register template propertys success��");

		
//		// ʹ����Ϊ
//#ifdef ACTION_ENABLED
//		// ע����Ϊ������ģ��
//		rc = _register_data_template_action(client);
//		if (rc != QCLOUD_RET_SUCCESS) {
//			Log_e("��Register data template actions Failed: %d��", rc);
//			goto RESET;
//		}Log_d("��init Register data template actions success��");
//#endif
			
		
//		// �ϱ��豸��Ϣ,ƽ̨���������Ϣ�ṩ��Ʒ��������ݷ���,Ʃ��λ�÷����
//		rc = _get_sys_info(client, sg_data_report_buffer, sg_data_report_buffersize);
//		if(QCLOUD_RET_SUCCESS == rc)
//		{
//			rc = IOT_Template_Report_SysInfo_Sync(client, sg_data_report_buffer, sg_data_report_buffersize, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);	
//			if (rc != QCLOUD_RET_SUCCESS) 
//			{
//				Log_e("��Report system info fail, err: %d��", rc);
//				goto RESET;
//			}
//		}else{
//			Log_e("��Get system info fail, err: %d��", rc);
//			goto RESET;
//		}
//		Log_d("��init Report system info success��");

		
//		// ��ȡ�����ڼ����ݣ�״̬
//		rc = IOT_Template_GetStatus_sync(client, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
//		if (rc != QCLOUD_RET_SUCCESS) 
//		{
//			Log_e("��Get data status fail, err: %d��", rc);
//			goto RESET;
//		}
//		Log_d("��init Get data status success��");


		
		
		Log_d("\r\n-----------��logic handle start��-----------");
		
		// ģʽ����
		rc = control_mode(client,pReportDataList,ReportCont);
		if(rc!=QCLOUD_RET_SUCCESS)
		{
			Log_e("��logic handle fail, err: %d��", rc);// �����г������³�ʼ��
			HAL_Delay(1000);
			retry_count++;
			if(retry_count>3) NVIC_SystemReset();// ϵͳ��λ
			goto RESET;
		}
		
//		while(1)
//		{
//			// ��������¼�
//			eventPostCheck(client);
//			
//		}
		
		
	// �������߼�ѭ��
	}while (0);
	
	
	// ������������񣬱���
	hal_thread_destroy(NULL);
	Log_e("Task teminated,Something goes wrong!!!");
}


// ����С��״̬
void handleMove(void)
{
	// ָ��ӳ���
	// set_speed_whole:w-0
	// set_speed_angle:a-0
	// set_speed_dir:d-0
	// set_dir_speed_whole:w-1
	// wifi2_reset: r-1
	// system_reset: r-2 
	// receive_app: r-3 
	// entry_app: a-1 

	char *p;
	
	// ת���ٶ�
	p = strstr((const char*)UART5_RX_BUF,"w-0");
	if(p) car.set_speed_whole=atoi(p+5);
	
	// ת���Ƕ�
	p = strstr((const char*)UART5_RX_BUF,"a-0");
	if(p) car.set_speed_angle=atoi(p+5);
	
	// ת���ƶ�����
	p = strstr((const char*)UART5_RX_BUF,"d-0");
	if(p) car.set_speed_dir=atoi(p+5);
	
	// ת����ת�ٶ�
	p = strstr((const char*)UART5_RX_BUF,"w-1");
	if(p) car.set_dir_speed_whole=atoi(p+5);
	
	// ����bootloader
	p = strstr((const char*)UART5_RX_BUF,"r-2");
	if(p){
		NVIC_SystemReset();
	}
	
	
	update_move();// ����С���ƶ�����
	
	//Log_d("set_speed_whole:%d,set_speed_angle:%d,set_speed_dir:%d,set_dir_speed_whole:%d",car.set_speed_whole,car.set_speed_angle,car.set_speed_dir,car.set_dir_speed_whole);

	// ������һ�ν���
	UART5_RX_STA=0;
	HAL_UART_Receive_IT(&huart5,&UART5_RX_BUF[UART5_RX_STA],1);
}

// ѡ�����ģʽ
int control_mode(void *client,DeviceProperty *pReportDataList[],int ReportCont)
{
	int rc;
	uint8_t fail_count=0;
	uint16_t i=0;
	uint16_t n=80;
	uint16_t x=80;
	uint16_t c=160;
	uint8_t * car_data="{\"car_secret\":\"VlzbGfFuvRV8o67sTlRLMw==\"}";
	
	// ���¿��Ź�
	HAL_IWDG_Refresh(&hiwdg);
	flicker_led(2,1,1,0,2000);//˫�Ƴ���3��

	Log_d("\r\n#####��start handle cloud logic��#####");
	while(1)
	{
		// 25ms ����
		if(++i>=50000) i=0,n++,c++,x++;
		
		//ά������
		if(c>=60){
			c=0;
			HAL_DCMI_Suspend(&hdcmi);// ��ͣ����ͼ��
			while(HAL_UART_Transmit_IT(&huart5,car_data,strlen((const char *)car_data)));
			HAL_DCMI_Resume(&hdcmi);// �ָ�����
		}

		if(is_start)
		{
			// ����ͼ��׶�
			if(img_data_ok){
				HAL_DCMI_Suspend(&hdcmi);// ��ͣ����ͼ��
				__HAL_DMA_DISABLE(&hdma_dcmi);//�ر�DMA
				while(DMA2_Stream1->CR&0X01);	//�ȴ�DMA2_Stream1������   
				// ��ȡ���βɼ���һ֡���ݳ��ȣ�����С��buf
				img_data_len=(img_buf_size-__HAL_DMA_GET_COUNTER(&hdma_dcmi))/2+1;
				while(HAL_UART_Transmit_DMA(&huart5,(uint8_t *)img_buf,img_data_len*4));
				if(n>=40){// 2s���ڣ�����С��״̬���߼�����
					
					n=0;
					rc = logic_handle(client,pReportDataList,ReportCont);
					if(rc!=QCLOUD_RET_SUCCESS&&++fail_count>3) return rc;
					else fail_count=0;
					
				}else if(x>=60){
					
					x=0;
					update_state();// ��ո��¶��������ͷ��wifi2����
					
				}
				while(HAL_UART_Transmit_DMA(&huart5,(uint8_t *)img_buf+img_data_len*4,img_data_len*4));
				// ����5���մ���
				if((UART5_RX_STA&0x8000)!=0) handleMove();
				while(huart5.gState!=HAL_UART_STATE_READY);
				resume_and_start_next();
			}
			HAL_IWDG_Refresh(&hiwdg);
		}else{
			// ����5���մ���
			if((UART5_RX_STA&0x8000)!=0) handleMove();
			if(n>=80){// 2s���ڣ�����С��״̬���߼�����
			
				n=0;
				HAL_IWDG_Refresh(&hiwdg);
				rc = logic_handle(client,pReportDataList,ReportCont);
				if(rc!=QCLOUD_RET_SUCCESS&&++fail_count>3) return rc;
				else fail_count=0;
				update_state();// ��ո��¶��������ͷ��wifi2����
				
			}
		}
		
	}
}

// �������߼�����
int logic_handle(void *client,DeviceProperty *pReportDataList[],int ReportCont)
{
	int rc;
	
	IOT_Template_Yield(client, 1);
	// �����������Ϣ������Կ�ʼ���������ָ��
	if (sg_control_msg_arrived) {	
		
		// ����ҵ���߼�����
		deal_down_stream_user_logic(client, &sg_ProductData);	
		
		
		//ҵ���߼����������Ҫ֪ͨ�����control msg ���յ���������ɾ��control msg���������˻ᱣ��control msg(ͨ��Get status������Եõ�δɾ����Control����)
		sReplyPara replyPara;
		memset((char *)&replyPara, 0, sizeof(sReplyPara));
		replyPara.code = eDEAL_SUCCESS;
		replyPara.timeout_ms = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;						
		replyPara.status_msg[0] = '\0';			//����ͨ�� replyPara.status_msg ��Ӹ�����Ϣ��һ����ʧ������²����
		
		// �ظ�����control��Ϣ
		rc = IOT_Template_ControlReply(client, sg_data_report_buffer, sg_data_report_buffersize, &replyPara);
		if (rc == QCLOUD_RET_SUCCESS) {
			//Log_d("Contol msg reply success");
			sg_control_msg_arrived = false;   
		}else{
			Log_e("��Contol msg reply failed, err: %d��", rc);
			return rc;
		}				
	}	

//HAL_Delay(1000);

	// �豸���и����������ݣ������ϱ�
	if(QCLOUD_RET_SUCCESS == deal_up_stream_user_logic(client, pReportDataList, &ReportCont,&sg_ProductData)){
		
		// ���ϱ�������ӻظ���Ϣ�����reported�ֶΣ������Ǹ���
		rc = IOT_Template_JSON_ConstructReportArray(client, sg_data_report_buffer, sg_data_report_buffersize, ReportCont, pReportDataList);
		if (rc == QCLOUD_RET_SUCCESS) {
			// ����ģ���첽��ʽ�ϱ�����
			rc = IOT_Template_Report(client, sg_data_report_buffer, sg_data_report_buffersize, 
										OnReportReplyCallback, NULL, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
			if (rc == QCLOUD_RET_SUCCESS) {
				//Log_d("data template reporte success");
			}else{
				Log_e("��data template reporte failed, err: %d��", rc);
				return rc;
			}
		}else{
			Log_e("��construct reporte data failed, err: %d��", rc);
			return rc;
		}
	}
	return QCLOUD_RET_SUCCESS;
}


// �����������߳�
void data_template_sample(void)
{
	osThreadId demo_threadId;
	
#ifdef OS_USED
	hal_thread_create(&demo_threadId, 512, osPriorityNormal, data_template_demo_task, NULL);
	hal_thread_destroy(NULL);
#else
	#error os should be used just now
#endif
}
