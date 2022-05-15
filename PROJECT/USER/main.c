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
//------------------------------------------------------------------------------------------PART ONE(����ͷ����Ԥ��)
																										
			








#define jpeg_dma_bufsize	5*1024		//����JPEG DMA����ʱ���ݻ���jpeg_buf0/1�Ĵ�С(*4�ֽ�)
volatile u32 jpeg_data_len=0; 			//buf�е�JPEG��Ч���ݳ���(*4�ֽ�)
volatile u8 jpeg_data_ok=0;				//JPEG���ݲɼ���ɱ�־ 
										//0,����û�вɼ���;
										//1,���ݲɼ�����,���ǻ�û����;
										//2,�����Ѿ����������,���Կ�ʼ��һ֡����
										
u32 *jpeg_buf0;							//JPEG���ݻ���buf,ͨ��malloc�����ڴ�
u32 *jpeg_buf1;							//JPEG���ݻ���buf,ͨ��malloc�����ڴ�
u32 *jpeg_data_buf;						//JPEG���ݻ���buf,ͨ��malloc�����ڴ�


//zigbee
u32 Pow(u8 m,u8 n); 
u32 ds=0;


//����JPEG����
//���ɼ���һ֡JPEG���ݺ�,���ô˺���,�л�JPEG BUF.��ʼ��һ֡�ɼ�.
void jpeg_data_process(void)
{
	u16 i;
	u16 rlen;//ʣ�����ݳ���
	u32 *pbuf;

	if(jpeg_data_ok==0)	//jpeg���ݻ�δ�ɼ���?
	{	
		DMA_Cmd(DMA2_Stream1, DISABLE);//ֹͣ��ǰ���� 
		while (DMA_GetCmdStatus(DMA2_Stream1) != DISABLE){}//�ȴ�DMA2_Stream1������  
		rlen=jpeg_dma_bufsize-DMA_GetCurrDataCounter(DMA2_Stream1);//�õ��˴����ݴ���ĳ���
		pbuf=jpeg_data_buf+jpeg_data_len;//ƫ�Ƶ���Ч����ĩβ,�������
		if(DMA2_Stream1->CR&(1<<19))for(i=0;i<rlen;i++)pbuf[i]=jpeg_buf1[i];//��ȡbuf1�����ʣ������
		else for(i=0;i<rlen;i++)pbuf[i]=jpeg_buf0[i];//��ȡbuf0�����ʣ������ 
		jpeg_data_len+=rlen;			//����ʣ�೤��
		jpeg_data_ok=1; 				//���JPEG���ݲɼ��갴��,�ȴ�������������
	}
	if(jpeg_data_ok==2)	//��һ�ε�jpeg�����Ѿ���������
	{
		DMA_SetCurrDataCounter(DMA2_Stream1,jpeg_dma_bufsize);//���䳤��Ϊjpeg_buf_size*4�ֽ�
		DMA_Cmd(DMA2_Stream1,ENABLE); //���´���
		jpeg_data_ok=0;					//�������δ�ɼ�
		jpeg_data_len=0;				//�������¿�ʼ
	}
} 
//jpeg���ݽ��ջص�����
void jpeg_dcmi_rx_callback(void)
{ 
	u16 i;
	u32 *pbuf;
	pbuf=jpeg_data_buf+jpeg_data_len;//ƫ�Ƶ���Ч����ĩβ
	if(DMA2_Stream1->CR&(1<<19))//buf0����,��������buf1
	{ 
		for(i=0;i<jpeg_dma_bufsize;i++)pbuf[i]=jpeg_buf0[i];//��ȡbuf0���������
		jpeg_data_len+=jpeg_dma_bufsize;//ƫ��
	}else //buf1����,��������buf0
	{
		for(i=0;i<jpeg_dma_bufsize;i++)pbuf[i]=jpeg_buf1[i];//��ȡbuf1���������
		jpeg_data_len+=jpeg_dma_bufsize;//ƫ�� 
	} 	
}

//�ļ������������⸲�ǣ�
//mode:0,����.bmp�ļ�;1,����.jpg�ļ�.
//bmp��ϳ�:����"0:PHOTO/PIC13141.bmp"���ļ���
//jpg��ϳ�:����"0:PHOTO/PIC13141.jpg"���ļ���
void camera_new_pathname(u8 *pname)
{	 
	u8 res;					 
	u16 index=0;
	while(index<0XFFFF)
	{
		sprintf((char*)pname,"0:PHOTO/PIC%05d.jpg",index);
		res=f_open(ftemp,(const TCHAR*)pname,FA_READ);//���Դ�����ļ�
		if(res==FR_NO_FILE)break;		//���ļ���������=����������Ҫ��.
		index++;
	}
}
//OV5640����jpgͼƬ
//����ֵ:0,�ɹ�
//    ����,�������
u8 ov5640_jpg_photo(u8 *pname)
{
	FIL* f_jpg; 
	u8 res=0;
	u32 bwr;
	u32 i;
	u8* pbuf;
	f_jpg=(FIL *)mymalloc(SRAMIN,sizeof(FIL));	//����FIL�ֽڵ��ڴ����� 
	if(f_jpg==NULL)return 0XFF;				//�ڴ�����ʧ��.
	jpeg_data_ok=0;
	OV5640_OutSize_Set(0,0,1440,1080);		//��������ߴ�,���ܵ�����Ե�С��������������̫���д��ʧ��
	dcmi_rx_callback=jpeg_dcmi_rx_callback;	//JPEG�������ݻص�����
	DCMI_DMA_Init((u32)jpeg_buf0,(u32)jpeg_buf1,jpeg_dma_bufsize,DMA_MemoryDataSize_Word,DMA_MemoryInc_Enable);//DCMI DMA����(˫����ģʽ)
	DCMI_Start(); 			//�������� 
	while(jpeg_data_ok!=1);	//�ȴ���һ֡ͼƬ�ɼ���
	jpeg_data_ok=2;			//���Ա�֡ͼƬ,������һ֡�ɼ� 
	while(jpeg_data_ok!=1);	//�ȴ��ڶ�֡ͼƬ�ɼ���,�ڶ�֡,�ű��浽SD��ȥ. 

	DCMI_Stop(); 			//ֹͣDMA����
	res=f_open(f_jpg,(const TCHAR*)pname,FA_WRITE|FA_CREATE_NEW);//ģʽ0,���߳��Դ�ʧ��,�򴴽����ļ�	 
	if(res==0)
	{
		printf("jpeg data size:%d\r\n",jpeg_data_len*4);//���ڴ�ӡJPEG�ļ���С
		pbuf=(u8*)jpeg_data_buf;
		for(i=0;i<jpeg_data_len*4;i++)//����0XFF,0XD8��0XFF,0XD9,��ȡjpg�ļ���С
		{
			if((pbuf[i]==0XFF)&&(pbuf[i+1]==0XD8))break;//�ҵ�FF D8
		}
		if(i==jpeg_data_len*4)res=0XFD;//û�ҵ�0XFF,0XD8
		else//�ҵ���
		{
			pbuf+=i;//ƫ�Ƶ�0XFF,0XD8��
			res=f_write(f_jpg,pbuf,jpeg_data_len*4-i,&bwr);
			if(bwr!=(jpeg_data_len*4-i))res=0XFE; 
		}
	}
	jpeg_data_len=0;
	f_close(f_jpg); 
	myfree(SRAMIN,f_jpg); 
	return res;
}
































//----------------------------------------------------------------------------------------------������
int main(void)
{ 
	int a = 1;//��ʼʱ�Ź��������ڴŹ���aΪ0����֮Ϊ1
	int i = 3;//iΪǰ���Ź��������ڵڼ����Ź���ͷ
	int x = 2*i;//x=x+a,aΪ��ȥ�ĴŹ�����
	int y=1;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);  //��ʼ����ʱ����
	uart_init(115200);//��ʼ�����ڲ�����Ϊ115200
	TIM13_PWM_Init(500-1,84-1);//84M/84=1Mhz�ļ���Ƶ��,��װ��ֵ500������PWMƵ��Ϊ 1M/500=2Khz.
  TIM14_PWM_Init(500-1,84-1);	//84M/84=1Mhz�ļ���Ƶ��,��װ��ֵ500������PWMƵ��Ϊ 1M/500=2Khz.
	ROAD_Init();
	CARKEY_Init();//����ͷԤ����	
	
	
	
	
	//С��
	 
	stop();
	delay_ms(5000);
	if(KEY0==1&&KEY1==1&&KEY2==1&&KEY3==1&&KEY4==1&&KEY5==1&&KEY6==1&&KEY7==1)//--------------------����Ƿ���״̬
	{
	stop();
		
	
 		
	}
	else//-------------------------------------------------------����״̬
	{
		u8 res;							 
	u8 *pname;				//��·�����ļ���    
	u8 q;						 
	u8 sd_ok=1;				//0,sd��������;1,SD������.
u32 t=0;			    
u8 temperature;  u8 t1=0;	    
u8 humidity;    	
		DHT11_Init();//DHT11��ʼ��		
	
 
	while(OV5640_Init())
	{
		printf("OV5640 Init error\r\n");
		delay_ms(500);
	}
	if(SD_Initialize())
	{
		printf("SD Init error��\r\n");
		delay_ms(500);
	}
	res = f_mount(fs[0],"0:",1); 		//����SD��
	printf("f_mount = %d\r\n",res);	 		
	printf("Explorer STM32F4������\r\n");	 			    	 
	printf("OV5640�����ʵ��\r\n");				    	 
	printf("KEY_UP:����\r\n");		
	res=f_mkdir("0:/PHOTO");		//����PHOTO�ļ���
	if(res!=FR_EXIST&&res!=FR_OK) 	//�����˴���
	{		    
		printf("SD������!!!!!\r\n");
		delay_ms(200);				  
		printf("���չ��ܽ�������!\r\n");
		sd_ok=0;  	
	} 	
	jpeg_buf0=mymalloc(SRAMIN,jpeg_dma_bufsize*4);	//Ϊjpeg dma���������ڴ�	
	jpeg_buf1=mymalloc(SRAMIN,jpeg_dma_bufsize*4);	//Ϊjpeg dma���������ڴ�	
	jpeg_data_buf=mymalloc(SRAMEX,300*1024);		//Ϊjpeg�ļ������ڴ�(���300KB)
 	pname=mymalloc(SRAMIN,30);//Ϊ��·�����ļ�������30���ֽڵ��ڴ�	 
 	while(pname==NULL||!jpeg_buf0||!jpeg_buf1||!jpeg_data_buf)	//�ڴ�������
 	{	    
		printf("�ڴ����ʧ��!\r\n");
		delay_ms(500);					  
	}	
	printf("OV5640 ����\r\n");
	//�Զ��Խ���ʼ��
	OV5640_JPEG_Mode();						//JPEGģʽ  
	OV5640_Focus_Init(); 
	OV5640_Light_Mode(0);	//�Զ�ģʽ
	OV5640_Color_Saturation(3);//ɫ�ʱ��Ͷ�0
	OV5640_Brightness(4);	//����0
	OV5640_Contrast(3);		//�Աȶ�0
	OV5640_Sharpness(33);	//�Զ����
	OV5640_Focus_Constant();//���������Խ�
	My_DCMI_Init();			//DCMI����
	
		
	
	
	
	
	

	
	
	                                 //------------------ѭ����--------------------
	
	
	
	
	
	
	while(1)
	{
	
				if(a<i||(a>i&&a<x))//----------------------------------------------------��ͨ���սڵ�
		{
			if(KEY0==0||KEY1==0||KEY2==0||KEY3==0||KEY4==0||KEY5==0||KEY6==0||KEY7==0)
			{
				Move_forward();
			}
			else
			{
				stop();
				delay_ms(3000);
				//------------------------------------------------------------------------����
					
					if(t1%100==0)//ÿ100ms��ȡһ��
		{	
			ds=0;
			DHT11_Read_Data(&temperature,&humidity);		//��ȡ��ʪ��ֵ
			for(t=1;t<3;t++)
			{
				USART_RX_BUF[t]=((temperature/Pow(10,2-t))%10+48);
				USART_SendData(USART1, USART_RX_BUF[t]);         //�򴮿�1��������
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
				delay_ms(50);
				
				
			}
			for(t=3;t<5;t++)
			{
				USART_RX_BUF[t]=((humidity/Pow(10,4-t))%10+48);
				USART_SendData(USART1, USART_RX_BUF[t]);         //�򴮿�1��������
				while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
				delay_ms(50);
				
				
			}
			
			printf("\r\n");//���뻻��
			USART_RX_STA=0;
		}				   
			delay_ms(10);t1++;
		if(sd_ok)//SD�������ſ�������
					{    
						camera_new_pathname(pname);//�õ��ļ���	
						res=ov5640_jpg_photo(pname);
						if(res)//��������
						{
							printf("д���ļ�����!\r\n");		 
						}
						else 
						{
							printf("���ճɹ�!\r\n");
							printf("����Ϊ:%s\r\n",pname);    
						}  					
					}
					
					else //��ʾSD������
						{					    
							printf("SD������!\r\n");
							printf("���չ��ܲ�����!\r\n");			    
						}   		
					
					delay_ms(10);
					q++;
					if(q==20)//DS0��˸.
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
		if(a==i||a==x)//a=i��a=x��״̬��i,xΪ��ͷ�ڵ�
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
	 		
		if(a==x+1)//���������̬�г�
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
		
		if(a==x+2)//����г�
		{
				if(y==1)
				{
					Move_back();//�������
					delay_ms(1000);
					y=y+1;
				}
				else
				{
					stop();//�������У�������״̬
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

/*if(sd_ok)//SD�������ſ�������
			{    
					camera_new_pathname(pname);//�õ��ļ���	
					res=ov5640_jpg_photo(pname);
	
					if(res)//��������
					{
						printf("д���ļ�����!\r\n");		 
					}else 
					{
						printf("���ճɹ�!\r\n");
						printf("����Ϊ:%s\r\n",pname);    
					}  					
			}
				else //��ʾSD������
				{					    
					printf("SD������!\r\n");
					printf("���չ��ܲ�����!\r\n");			    
				}   		
					delay_ms(10);
					q++;
				if(q==20)//DS0��˸.
				{
					q=0;
					LED0=!LED0;
				}	  
*/

