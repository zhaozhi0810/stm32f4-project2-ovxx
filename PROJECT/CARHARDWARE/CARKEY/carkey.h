#ifndef __CARKEY_H
#define __CARKEY_H	 
#include "sys.h" 

/*����ķ�ʽ��ͨ��ֱ�Ӳ����⺯����ʽ��ȡIO*/
#define KEY0 		GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_0) //PF0
#define KEY1 	  GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_1)	//PF1
#define KEY2 		GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_2)	//PF2 
#define KEY3 		GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_3) //PF3
#define KEY4 		GPIO_ReadInputDataBit(GPIOF,GPIO_Pin_4) //PF4
#define KEY5 		GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_8) //PD8
#define KEY6 		GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_9) //PD9
#define KEY7 		GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_10) //PD10



#define KEY0_PRES 	1	//KEY0����
#define KEY1_PRES	2	//KEY1����
#define KEY2_PRES	3	//KEY2����
#define WKUP_PRES   4	//KEY_UP����(��WK_UP)

void CARKEY_Init(void);	//IO��ʼ��
u8 KEY_Scan(u8);  		//����ɨ�躯��	

#endif
