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
#include "photoready.h"	

void photo(void)
{   
			if(sd_ok)//SD�������ſ�������
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

}

