#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "pwm.h"
#include "road.h"
#include "carkey.h"
#include "direction.h" 

void stop(void)
{   
	u16 pwmval=500; 
	u16 pwmva2=500;
	TIM_SetCompare1(TIM13,pwmval);//GPIOF8
	TIM_SetCompare1(TIM14,pwmva2);//GPIOA7
}


void Move_forward(void)
{	
	
	u16 pwmval; 
	u16 pwmva2;
	if(KEY0==1&&KEY7==1)               //���߲��ڴŹ죬ʹ��
	{ 
	GPIO_SetBits(GPIOC,GPIO_Pin_0);//E5�ߵ�ƽ
	GPIO_ResetBits(GPIOC,GPIO_Pin_1);//E6�͵�ƽ
	GPIO_SetBits(GPIOE,GPIO_Pin_2);//E0�ߵ�ƽ
	GPIO_ResetBits(GPIOE,GPIO_Pin_3);//E1�͵�ƽ
	pwmval=250;pwmva2=250;//����
	TIM_SetCompare1(TIM13,pwmval);//F8
	TIM_SetCompare1(TIM14,pwmva2);//A7
	}	 
else{ //ƫ��
		while(KEY7==0) 
		{//E5 E6���
		GPIO_SetBits(GPIOC,GPIO_Pin_1);//56�����ת//E6�ߵ�ƽ
		GPIO_ResetBits(GPIOC,GPIO_Pin_0);//E5�͵�ƽ
		GPIO_SetBits(GPIOE,GPIO_Pin_2);//01�����ת//E0�ߵ�ƽ
		GPIO_ResetBits(GPIOE,GPIO_Pin_3);//E1�͵�ƽ
		pwmval=50;pwmva2=50;//ƫ��
		TIM_SetCompare1(TIM13,pwmval);//F8
		TIM_SetCompare1(TIM14,pwmva2);//A7
		if(KEY7==1)break;
		} 
		while(KEY0==0) 
		{//E0 E1���
		GPIO_SetBits(GPIOE,GPIO_Pin_3);//01�����ת//E1�ߵ�ƽ
		GPIO_ResetBits(GPIOE,GPIO_Pin_2);//E0�͵�ƽ
		GPIO_SetBits(GPIOC,GPIO_Pin_0);//56�����ת//E5�ߵ�ƽ
		GPIO_ResetBits(GPIOC,GPIO_Pin_1);//E6�͵�ƽ
		pwmval=50;pwmva2=50;//ƫ��
		TIM_SetCompare1(TIM13,pwmval);//F8
		TIM_SetCompare1(TIM14,pwmva2);//A7
		if(KEY0==1)break;
		}
	}
}


void Move_back(void)
{
	u16 pwmval; 
	u16 pwmva2;
	GPIO_SetBits(GPIOC,GPIO_Pin_1);//E6�ߵ�ƽ   �źŶ˿ڷ��� 
	GPIO_ResetBits(GPIOC,GPIO_Pin_0);//E5�͵�ƽ
	GPIO_SetBits(GPIOE,GPIO_Pin_3);//E1�ߵ�ƽ
	GPIO_ResetBits(GPIOE,GPIO_Pin_2);//E0�͵�ƽ
	pwmval=250;pwmva2=250;//����
	TIM_SetCompare1(TIM13,pwmval);//F8
	TIM_SetCompare1(TIM14,pwmva2);//A7
	
}

void diaotou(void){
		u16 pwmval; 
		u16 pwmva2;
		GPIO_SetBits(GPIOC,GPIO_Pin_1);//01�����ת//E1�ߵ�ƽ
		GPIO_ResetBits(GPIOC,GPIO_Pin_0);//E0�͵�ƽ
		GPIO_SetBits(GPIOE,GPIO_Pin_2);//56�����ת//E5�ߵ�ƽ
		GPIO_ResetBits(GPIOE,GPIO_Pin_3);//E6�͵�ƽ
		pwmval=50;pwmva2=50;
		delay_ms(300);
		stop();
		delay_ms(300);
		TIM_SetCompare1(TIM13,pwmval);//F8
		TIM_SetCompare1(TIM14,pwmva2);//A7

}
