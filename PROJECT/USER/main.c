#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h" 
#include "sram.h"   
#include "malloc.h" 
#include "usmart.h"  
#include "spi_sdcard.h"    
#include "malloc.h" 
#include "w25qxx.h"    
#include "ff.h"  
#include "exfuns.h"    
#include "fontupd.h"
#include "string.h"	
#include "math.h"	
#include "dcmi.h"	
#include "ov5640.h"	
#include "pwm.h"
#include "road.h"
#include "carkey.h"
#include "direction.h"
#include "usart.h"
#include "dht11.h"
//------------------------------------------------------------------------------------------PART ONE(摄像头部分预载)
																										
			








#define jpeg_dma_bufsize	5*1024		//定义JPEG DMA接收时数据缓存jpeg_buf0/1的大小(*4字节)
volatile u32 jpeg_data_len=0; 			//buf中的JPEG有效数据长度(*4字节)
volatile u8 jpeg_data_ok=0;				//JPEG数据采集完成标志 
										//0,数据没有采集完;
										//1,数据采集完了,但是还没处理;
										//2,数据已经处理完成了,可以开始下一帧接收
										
u32 *jpeg_buf0;							//JPEG数据缓存buf,通过malloc申请内存
u32 *jpeg_buf1;							//JPEG数据缓存buf,通过malloc申请内存
u32 *jpeg_data_buf;						//JPEG数据缓存buf,通过malloc申请内存


//zigbee
u32 Pow(u8 m,u8 n); 
u32 ds=0;


//处理JPEG数据
//当采集完一帧JPEG数据后,调用此函数,切换JPEG BUF.开始下一帧采集.
void jpeg_data_process(void)
{
	u16 i;
	u16 rlen;//剩余数据长度
	u32 *pbuf;

	if(jpeg_data_ok==0)	//jpeg数据还未采集完?
	{	
		DMA_Cmd(DMA2_Stream1, DISABLE);//停止当前传输 
		while (DMA_GetCmdStatus(DMA2_Stream1) != DISABLE){}//等待DMA2_Stream1可配置  
		rlen=jpeg_dma_bufsize-DMA_GetCurrDataCounter(DMA2_Stream1);//得到此次数据传输的长度
		pbuf=jpeg_data_buf+jpeg_data_len;//偏移到有效数据末尾,继续添加
		if(DMA2_Stream1->CR&(1<<19))for(i=0;i<rlen;i++)pbuf[i]=jpeg_buf1[i];//读取buf1里面的剩余数据
		else for(i=0;i<rlen;i++)pbuf[i]=jpeg_buf0[i];//读取buf0里面的剩余数据 
		jpeg_data_len+=rlen;			//加上剩余长度
		jpeg_data_ok=1; 				//标记JPEG数据采集完按成,等待其他函数处理
	}
	if(jpeg_data_ok==2)	//上一次的jpeg数据已经被处理了
	{
		DMA_SetCurrDataCounter(DMA2_Stream1,jpeg_dma_bufsize);//传输长度为jpeg_buf_size*4字节
		DMA_Cmd(DMA2_Stream1,ENABLE); //重新传输
		jpeg_data_ok=0;					//标记数据未采集
		jpeg_data_len=0;				//数据重新开始
	}
} 
//jpeg数据接收回调函数
void jpeg_dcmi_rx_callback(void)
{ 
	u16 i;
	u32 *pbuf;
	pbuf=jpeg_data_buf+jpeg_data_len;//偏移到有效数据末尾
	if(DMA2_Stream1->CR&(1<<19))//buf0已满,正常处理buf1
	{ 
		for(i=0;i<jpeg_dma_bufsize;i++)pbuf[i]=jpeg_buf0[i];//读取buf0里面的数据
		jpeg_data_len+=jpeg_dma_bufsize;//偏移
	}else //buf1已满,正常处理buf0
	{
		for(i=0;i<jpeg_dma_bufsize;i++)pbuf[i]=jpeg_buf1[i];//读取buf1里面的数据
		jpeg_data_len+=jpeg_dma_bufsize;//偏移 
	} 	
}

//文件名自增（避免覆盖）
//mode:0,创建.bmp文件;1,创建.jpg文件.
//bmp组合成:形如"0:PHOTO/PIC13141.bmp"的文件名
//jpg组合成:形如"0:PHOTO/PIC13141.jpg"的文件名
void camera_new_pathname(u8 *pname)
{	 
	u8 res;					 
	u16 index=0;
	while(index<0XFFFF)
	{
		sprintf((char*)pname,"0:PHOTO/PIC%05d.jpg",index);
		res=f_open(ftemp,(const TCHAR*)pname,FA_READ);//尝试打开这个文件
		if(res==FR_NO_FILE)break;		//该文件名不存在=正是我们需要的.
		index++;
	}
}
//OV5640拍照jpg图片
//返回值:0,成功
//    其他,错误代码
u8 ov5640_jpg_photo(u8 *pname)
{
	FIL* f_jpg; 
	u8 res=0;
	u32 bwr;
	u32 i;
	u8* pbuf;
	f_jpg=(FIL *)mymalloc(SRAMIN,sizeof(FIL));	//开辟FIL字节的内存区域 
	if(f_jpg==NULL)return 0XFF;				//内存申请失败.
	jpeg_data_ok=0;
	OV5640_OutSize_Set(0,0,1440,1080);		//设置输出尺寸,不能调大可以调小，调大了数据量太大会写入失败
	dcmi_rx_callback=jpeg_dcmi_rx_callback;	//JPEG接收数据回调函数
	DCMI_DMA_Init((u32)jpeg_buf0,(u32)jpeg_buf1,jpeg_dma_bufsize,DMA_MemoryDataSize_Word,DMA_MemoryInc_Enable);//DCMI DMA配置(双缓冲模式)
	DCMI_Start(); 			//启动传输 
	while(jpeg_data_ok!=1);	//等待第一帧图片采集完
	jpeg_data_ok=2;			//忽略本帧图片,启动下一帧采集 
	while(jpeg_data_ok!=1);	//等待第二帧图片采集完,第二帧,才保存到SD卡去. 

	DCMI_Stop(); 			//停止DMA搬运
	res=f_open(f_jpg,(const TCHAR*)pname,FA_WRITE|FA_CREATE_NEW);//模式0,或者尝试打开失败,则创建新文件	 
	if(res==0)
	{
		printf("jpeg data size:%d\r\n",jpeg_data_len*4);//串口打印JPEG文件大小
		pbuf=(u8*)jpeg_data_buf;
		for(i=0;i<jpeg_data_len*4;i++)//查找0XFF,0XD8和0XFF,0XD9,获取jpg文件大小
		{
			if((pbuf[i]==0XFF)&&(pbuf[i+1]==0XD8))break;//找到FF D8
		}
		if(i==jpeg_data_len*4)res=0XFD;//没找到0XFF,0XD8
		else//找到了
		{
			pbuf+=i;//偏移到0XFF,0XD8处
			res=f_write(f_jpg,pbuf,jpeg_data_len*4-i,&bwr);
			if(bwr!=(jpeg_data_len*4-i))res=0XFE; 
		}
	}
	jpeg_data_len=0;
	f_close(f_jpg); 
	myfree(SRAMIN,f_jpg); 
	return res;
}
































//----------------------------------------------------------------------------------------------主函数
int main(void)
{ 
	int a = 1;//开始时磁轨检测器不在磁轨上a为0，反之为1
	int i = 3;//i为前进磁轨条数，在第几条磁轨后掉头
	int x = 2*i;//x=x+a,a为回去的磁轨条数
	int y=1;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  //初始化延时函数
	uart_init(115200);//初始化串口波特率为115200
	TIM13_PWM_Init(500-1,84-1);//84M/84=1Mhz的计数频率,重装载值500，所以PWM频率为 1M/500=2Khz.
  TIM14_PWM_Init(500-1,84-1);	//84M/84=1Mhz的计数频率,重装载值500，所以PWM频率为 1M/500=2Khz.
	ROAD_Init();
	CARKEY_Init();//摄像头预载入	
	
	
	
	
	//小车
	 
	stop();
	delay_ms(5000);
	if(KEY0==1&&KEY1==1&&KEY2==1&&KEY3==1&&KEY4==1&&KEY5==1&&KEY6==1&&KEY7==1)//--------------------检测是否充电状态
	{
	stop();
		
	
 		
	}
	else//-------------------------------------------------------工作状态
	{
		u8 res;							 
	u8 *pname;				//带路径的文件名    
	u8 q;						 
	u8 sd_ok=1;				//0,sd卡不正常;1,SD卡正常.
u32 t=0;			    
u8 temperature;  u8 t1=0;	    
u8 humidity;    	
		DHT11_Init();//DHT11初始化		
	
 
	while(OV5640_Init())
	{
		printf("OV5640 Init error\r\n");
		delay_ms(500);
	}
	if(SD_Initialize())
	{
		printf("SD Init error！\r\n");
		delay_ms(500);
	}
	res = f_mount(fs[0],"0:",1); 		//挂载SD卡
	printf("f_mount = %d\r\n",res);	 		
	printf("Explorer STM32F4开发板\r\n");	 			    	 
	printf("OV5640照相机实验\r\n");				    	 
	printf("KEY_UP:拍照\r\n");		
	res=f_mkdir("0:/PHOTO");		//创建PHOTO文件夹
	if(res!=FR_EXIST&&res!=FR_OK) 	//发生了错误
	{		    
		printf("SD卡错误!!!!!\r\n");
		delay_ms(200);				  
		printf("拍照功能将不可用!\r\n");
		sd_ok=0;  	
	} 	
	jpeg_buf0=mymalloc(SRAMIN,jpeg_dma_bufsize*4);	//为jpeg dma接收申请内存	
	jpeg_buf1=mymalloc(SRAMIN,jpeg_dma_bufsize*4);	//为jpeg dma接收申请内存	
	jpeg_data_buf=mymalloc(SRAMEX,300*1024);		//为jpeg文件申请内存(最大300KB)
 	pname=mymalloc(SRAMIN,30);//为带路径的文件名分配30个字节的内存	 
 	while(pname==NULL||!jpeg_buf0||!jpeg_buf1||!jpeg_data_buf)	//内存分配出错
 	{	    
		printf("内存分配失败!\r\n");
		delay_ms(500);					  
	}	
	printf("OV5640 正常\r\n");
	//自动对焦初始化
	OV5640_JPEG_Mode();						//JPEG模式  
	OV5640_Focus_Init(); 
	OV5640_Light_Mode(0);	//自动模式
	OV5640_Color_Saturation(3);//色彩饱和度0
	OV5640_Brightness(4);	//亮度0
	OV5640_Contrast(3);		//对比度0
	OV5640_Sharpness(33);	//自动锐度
	OV5640_Focus_Constant();//启动持续对焦
	My_DCMI_Init();			//DCMI配置
	
		
	
	
	
	
	

	
	
	                                 //------------------循环体--------------------
	
	
	
	
	
	
	while(1)
	{
	
				if(a<i||(a>i&&a<x))//----------------------------------------------------普通拍照节点
		{
			if(KEY0==0||KEY1==0||KEY2==0||KEY3==0||KEY4==0||KEY5==0||KEY6==0||KEY7==0)
			{
				Move_forward();
			}
			else
			{
				stop();
				delay_ms(3000);
				//------------------------------------------------------------------------拍照
					
					if(t1%100==0)//每100ms读取一次
		{	
			ds=0;
			DHT11_Read_Data(&temperature,&humidity);		//读取温湿度值
			for(t=1;t<3;t++)
			{
				USART_RX_BUF[t]=((temperature/Pow(10,2-t))%10+48);
				USART_SendData(USART1, USART_RX_BUF[t]);         //向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
				delay_ms(50);
				
				
			}
			for(t=3;t<5;t++)
			{
				USART_RX_BUF[t]=((humidity/Pow(10,4-t))%10+48);
				USART_SendData(USART1, USART_RX_BUF[t]);         //向串口1发送数据
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
				delay_ms(50);
				
				
			}
			
			printf("\r\n");//插入换行
			USART_RX_STA=0;
		}				   
			delay_ms(10);t1++;
		if(sd_ok)//SD卡正常才可以拍照
					{    
						camera_new_pathname(pname);//得到文件名	
						res=ov5640_jpg_photo(pname);
						if(res)//拍照有误
						{
							printf("写入文件错误!\r\n");		 
						}
						else 
						{
							printf("拍照成功!\r\n");
							printf("保存为:%s\r\n",pname);    
						}  					
					}
					
					else //提示SD卡错误
						{					    
							printf("SD卡错误!\r\n");
							printf("拍照功能不可用!\r\n");			    
						}   		
					
					delay_ms(10);
					q++;
					if(q==20)//DS0闪烁.
					{
						q=0;
						LED0=!LED0;
					}	  
				delay_ms(5000);
				
				do
				{
					Move_forward();
				}
				while(KEY0==1&&KEY1==1&&KEY2==1&&KEY3==1&&KEY4==1&&KEY5==1&&KEY6==1&&KEY7==1);
				a=a+1;
			}
		}
		if(a==i||a==x)//a=i与a=x的状态，i,x为调头节点
		{
			if(KEY0==0||KEY1==0||KEY2==0||KEY3==0||KEY4==0||KEY5==0||KEY6==0||KEY7==0)
			{
				Move_forward();
			}
			else
			{
				stop();
				delay_ms(3000);
				do
				{
					diaotou();
				}
				while(KEY0==1&&KEY1==1&&KEY2==1&&KEY3==1&&KEY4==1&&KEY5==1&&KEY6==1&&KEY7==1);
				a=a+1;
			  }
		}
	 		
		if(a==x+1)//修正充电姿态行程
		{
			if(KEY0==0||KEY1==0||KEY2==0||KEY3==0||KEY4==0||KEY5==0||KEY6==0||KEY7==0)
			{
				Move_forward();
			}
			else
			{
				stop();
				delay_ms(3000);
				do
				{
					Move_back();
				}
				while(KEY0==1&&KEY1==1&&KEY2==1&&KEY3==1&&KEY4==1&&KEY5==1&&KEY6==1&&KEY7==1);
				a=a+1;
			}
		}
		
		if(a==x+2)//充电行程
		{
				if(y==1)
				{
					Move_back();//倒车充电
					delay_ms(1000);
					y=y+1;
				}
				else
				{
					stop();//结束运行，充电待机状态
				}
		}
		
	} 
	
	}//else
}//main
u32 Pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}

/*if(sd_ok)//SD卡正常才可以拍照
			{    
					camera_new_pathname(pname);//得到文件名	
					res=ov5640_jpg_photo(pname);
	
					if(res)//拍照有误
					{
						printf("写入文件错误!\r\n");		 
					}else 
					{
						printf("拍照成功!\r\n");
						printf("保存为:%s\r\n",pname);    
					}  					
			}
				else //提示SD卡错误
				{					    
					printf("SD卡错误!\r\n");
					printf("拍照功能不可用!\r\n");			    
				}   		
					delay_ms(10);
					q++;
				if(q==20)//DS0闪烁.
				{
					q=0;
					LED0=!LED0;
				}	  
*/

