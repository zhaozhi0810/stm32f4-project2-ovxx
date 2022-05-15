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
			if(sd_ok)//SD卡正常才可以拍照
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

}

