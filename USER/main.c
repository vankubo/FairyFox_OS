#include "usart.h"
#include "delay.h"
#include "sys.h"
#include "arm_math.h"
#include "arm_const_structs.h"
#include "thread.h"
#include "timer.h"

void task1(void);
void task2(void);
int main()
{
    int i;
    RCC_ClocksTypeDef rcc_clock;    //获取系统时钟状态
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	uart_init(115200);		//初始化串口波特率为115200
    RCC_GetClocksFreq(&rcc_clock);

    printf("stm32f4xx run at:%dMHZ\n",rcc_clock.SYSCLK_Frequency/1000000);
    delay_init(84);  //初始化延时函数
   //设置pendSV优先级
   /* Make PendSV and SysTick the lowest priority interrupts. */
	portNVIC_SYSPRI2_REG |= portNVIC_PENDSV_PRI;
    //初始化
    //TIM3_Int_Init(5000-1,8400-1);	//定时器时钟84M，分频系数8400，所以84M/8400=10Khz的计数频率，计数5000次为500ms

    fThreadInit();
    printf("init done\n");
    //创建任务
    fCreateThread(task1,1,1024);
   fCreateThread(task2,2,1024);
    printf("create done\n");
    //启动os
   FoxStart();

    while(1)
    {
       printf("main\n");
        delay_ms(2000);
    }  
  
	return 0;
}



#pragma GCC push_options
#pragma GCC optimize("O0")
void task1(void)
{

    while(1)
    {
       
        printf("task1\n");
        FoxSleep(2000);
    }
}


void task2(void)
{
    while(1)
    {
        
        printf("task2\n");
        FoxSleep(5000);
    }
}
#pragma GCC pop_options
/*
浮点计算测试
*/
//禁止gcc优化此函数
#pragma GCC push_options
#pragma GCC optimize("O0")
#pragma GCC pop_options