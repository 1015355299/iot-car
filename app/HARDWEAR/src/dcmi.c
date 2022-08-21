
#include "dcmi.h" 
#include "ov2640.h" 


DCMI_HandleTypeDef  DCMI_Handler;           //DCMI���
DMA_HandleTypeDef   DMADMCI_Handler;        //DMA���

uint8_t ov_frame=0;  							//֡��
extern void jpeg_data_process(void);		//JPEG���ݴ�����


//DCMI DMA����
//memaddr:�洢����ַ  ��Ҫ�洢����ͷ���ݵ��ڴ��ַ(Ҳ�����������ַ)
//memblen:�洢��λ��,����Ϊ:DMA_MDATAALIGN_BYTE/DMA_MDATAALIGN_HALFWORD/DMA_MDATAALIGN_WORD
//meminc:�洢��������ʽ,����Ϊ:DMA_MINC_ENABLE/DMA_MINC_DISABLE
void DCMI_DMA_Init(u32 memaddr,uint16_t memsize,u32 memblen,u32 meminc)
{ 
    __HAL_RCC_DMA2_CLK_ENABLE();                                    //ʹ��DMA2ʱ��
    __HAL_LINKDMA(&DCMI_Handler,DMA_Handle,DMADMCI_Handler);        //��DMA��DCMI��ϵ����
	
    DMADMCI_Handler.Instance=DMA2_Stream1;                          //DMA2������1                     
    DMADMCI_Handler.Init.Channel=DMA_CHANNEL_1;                     //ͨ��1
    DMADMCI_Handler.Init.Direction=DMA_PERIPH_TO_MEMORY;            //���赽�洢��
    DMADMCI_Handler.Init.PeriphInc=DMA_PINC_DISABLE;                //���������ģʽ
    DMADMCI_Handler.Init.MemInc=meminc;                             //�洢������ģʽ
    DMADMCI_Handler.Init.PeriphDataAlignment=DMA_PDATAALIGN_WORD;   //�������ݳ���:32λ
    DMADMCI_Handler.Init.MemDataAlignment=memblen;                  //�洢�����ݳ���:8/16/32λ
    DMADMCI_Handler.Init.Mode=DMA_CIRCULAR;                         //ʹ��ѭ��ģʽ 
    DMADMCI_Handler.Init.Priority=DMA_PRIORITY_HIGH;                //�����ȼ�
    DMADMCI_Handler.Init.FIFOMode=DMA_FIFOMODE_ENABLE;              //ʹ��FIFO
    DMADMCI_Handler.Init.FIFOThreshold=DMA_FIFO_THRESHOLD_HALFFULL; //ʹ��1/2��FIFO 
    DMADMCI_Handler.Init.MemBurst=DMA_MBURST_SINGLE;                //�洢��ͻ������
    DMADMCI_Handler.Init.PeriphBurst=DMA_PBURST_SINGLE;             //����ͻ�����δ��� 
    HAL_DMA_DeInit(&DMADMCI_Handler);                               //�������ǰ������
    HAL_DMA_Init(&DMADMCI_Handler);	                                //��ʼ��DMA
    
    //�ڿ���DMA֮ǰ��ʹ��__HAL_UNLOCK()����һ��DMA,��ΪHAL_DMA_Statrt()HAL_DMAEx_MultiBufferStart()
    //����������һ��ʼҪ��ʹ��__HAL_LOCK()����DMA,������__HAL_LOCK()���жϵ�ǰ��DMA״̬�Ƿ�Ϊ����״̬�������
    //����״̬�Ļ���ֱ�ӷ���HAL_BUSY�������ᵼ�º���HAL_DMA_Statrt()��HAL_DMAEx_MultiBufferStart()������DMA����
    //����ֱ�ӱ�������DMAҲ�Ͳ�������������Ϊ�˱���������������������DMA֮ǰ�ȵ���__HAL_UNLOC()�Ƚ���һ��DMA��
    __HAL_UNLOCK(&DMADMCI_Handler);

	HAL_DMA_Start(&DMADMCI_Handler,(u32)&DCMI->DR,memaddr,memsize);
}






   








