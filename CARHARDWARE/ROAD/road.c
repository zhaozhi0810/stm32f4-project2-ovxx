#include "road.h" 

void ROAD_Init(void)
{   
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE|RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOE,C时钟
  

  GPIO_InitStructure.GPIO_Pin =GPIO_Pin_2|GPIO_Pin_3;//2,3使能5,6信号0,1信号
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//下拉
  GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIO
	
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1;//2,3使能5,6信号0,1信号
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;//下拉
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
  GPIO_ResetBits(GPIOE,GPIO_Pin_2); 
  GPIO_ResetBits(GPIOE,GPIO_Pin_3);
}





