#include "road.h" 

void ROAD_Init(void)
{   
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);//ʹ��GPIOEʱ��
  

  GPIO_InitStructure.GPIO_Pin =GPIO_Pin_2|GPIO_Pin_3 |GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_0|GPIO_Pin_1;//2,3ʹ��5,6�ź�0,1�ź�
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//����
  GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��GPIO
	
  GPIO_ResetBits(GPIOE,GPIO_Pin_2); 
  GPIO_ResetBits(GPIOE,GPIO_Pin_3);
}





