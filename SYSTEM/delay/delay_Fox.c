#include "delay.h"
#include "sys.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用OS,则包括下面的头文件即可
#if SYSTEM_SUPPORT_OS
#include "thread.h"		  
#endif

static long int tickcount=0;
static uint8_t irqed=0;
static int mscnt=0;
//systick中断服务函数,使用OS时用到
extern uint8_t threadrun;

void SysTick_Handler(void)
{	
	if(threadrun==1)
		{
			//定时器TickTock调用
			SleepTickTock();
			//调度器
			FoxScheduler();
		}	
}
			   
//初始化延迟函数
//SYSTICK的时钟固定为AHB时钟，基础例程里面SYSTICK时钟频率为AHB/8
//这里为了兼容FreeRTOS，所以将SYSTICK的时钟频率改为AHB的频率！
//SYSCLK:系统时钟频率
void delay_init(u8 SYSCLK)
{
	u32 reload;
 	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK); 
	reload=SYSCLK;							//每秒钟的计数次数 单位为M	   
	reload*=1000000/1000;		//根据delay_ostickspersec设定溢出时间
											//reload为24位寄存器,最大值:16777216,在168M下,约合0.0998s左右	
		   
	SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;//开启SYSTICK中断
	SysTick->LOAD=reload; 					//每1/configTICK_RATE_HZ断一次	
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk; //开启SYSTICK     
}								    


//延时nms
//nms:要延时的ms数
//nms:0~65535
void delay_ms(uint32_t nms)
{	
	mscnt=0;
	while(nms>mscnt);
}

			 



































