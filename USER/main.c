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
    RCC_ClocksTypeDef rcc_clock;    //��ȡϵͳʱ��״̬
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	uart_init(115200);		//��ʼ�����ڲ�����Ϊ115200
    RCC_GetClocksFreq(&rcc_clock);

    printf("stm32f4xx run at:%dMHZ\n",rcc_clock.SYSCLK_Frequency/1000000);
    delay_init(84);  //��ʼ����ʱ����
   //����pendSV���ȼ�
   /* Make PendSV and SysTick the lowest priority interrupts. */
	portNVIC_SYSPRI2_REG |= portNVIC_PENDSV_PRI;
    //��ʼ��
    //TIM3_Int_Init(5000-1,8400-1);	//��ʱ��ʱ��84M����Ƶϵ��8400������84M/8400=10Khz�ļ���Ƶ�ʣ�����5000��Ϊ500ms

    fThreadInit();
    printf("init done\n");
    //��������
    fCreateThread(task1,1,1024);
   fCreateThread(task2,2,1024);
    printf("create done\n");
    //����os
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
����������
*/
//��ֹgcc�Ż��˺���
#pragma GCC push_options
#pragma GCC optimize("O0")
#pragma GCC pop_options