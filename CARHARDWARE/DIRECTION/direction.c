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
	if(KEY0==1&&KEY7==1)               //两边不在磁轨，使能
	{ 
	GPIO_SetBits(GPIOC,GPIO_Pin_0);//E5高电平
	GPIO_ResetBits(GPIOC,GPIO_Pin_1);//E6低电平
	GPIO_SetBits(GPIOE,GPIO_Pin_2);//E0高电平
	GPIO_ResetBits(GPIOE,GPIO_Pin_3);//E1低电平
	pwmval=250;pwmva2=250;//正常
	TIM_SetCompare1(TIM13,pwmval);//F8
	TIM_SetCompare1(TIM14,pwmva2);//A7
	}	 
else{ //偏移
		while(KEY7==0) 
		{//E5 E6电机
		GPIO_SetBits(GPIOC,GPIO_Pin_1);//56电机反转//E6高电平
		GPIO_ResetBits(GPIOC,GPIO_Pin_0);//E5低电平
		GPIO_SetBits(GPIOE,GPIO_Pin_2);//01电机正转//E0高电平
		GPIO_ResetBits(GPIOE,GPIO_Pin_3);//E1低电平
		pwmval=50;pwmva2=50;//偏快
		TIM_SetCompare1(TIM13,pwmval);//F8
		TIM_SetCompare1(TIM14,pwmva2);//A7
		if(KEY7==1)break;
		} 
		while(KEY0==0) 
		{//E0 E1电机
		GPIO_SetBits(GPIOE,GPIO_Pin_3);//01电机反转//E1高电平
		GPIO_ResetBits(GPIOE,GPIO_Pin_2);//E0低电平
		GPIO_SetBits(GPIOC,GPIO_Pin_0);//56电机正转//E5高电平
		GPIO_ResetBits(GPIOC,GPIO_Pin_1);//E6低电平
		pwmval=50;pwmva2=50;//偏快
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
	GPIO_SetBits(GPIOC,GPIO_Pin_1);//E6高电平   信号端口反向 
	GPIO_ResetBits(GPIOC,GPIO_Pin_0);//E5低电平
	GPIO_SetBits(GPIOE,GPIO_Pin_3);//E1高电平
	GPIO_ResetBits(GPIOE,GPIO_Pin_2);//E0低电平
	pwmval=250;pwmva2=250;//正常
	TIM_SetCompare1(TIM13,pwmval);//F8
	TIM_SetCompare1(TIM14,pwmva2);//A7
	
}

void diaotou(void){
		u16 pwmval; 
		u16 pwmva2;
		GPIO_SetBits(GPIOC,GPIO_Pin_1);//01电机正转//E1高电平
		GPIO_ResetBits(GPIOC,GPIO_Pin_0);//E0低电平
		GPIO_SetBits(GPIOE,GPIO_Pin_2);//56电机反转//E5高电平
		GPIO_ResetBits(GPIOE,GPIO_Pin_3);//E6低电平
		pwmval=50;pwmva2=50;
		delay_ms(300);
		stop();
		delay_ms(300);
		TIM_SetCompare1(TIM13,pwmval);//F8
		TIM_SetCompare1(TIM14,pwmva2);//A7

}
