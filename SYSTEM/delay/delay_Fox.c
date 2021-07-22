#include "delay.h"
#include "sys.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��OS,����������ͷ�ļ�����
#if SYSTEM_SUPPORT_OS
#include "thread.h"		  
#endif

static long int tickcount=0;
static uint8_t irqed=0;
static int mscnt=0;
//systick�жϷ�����,ʹ��OSʱ�õ�
extern uint8_t threadrun;

void SysTick_Handler(void)
{	
	if(threadrun==1)
		{
			//��ʱ��TickTock����
			SleepTickTock();
			//������
			FoxScheduler();
		}	
}
			   
//��ʼ���ӳٺ���
//SYSTICK��ʱ�ӹ̶�ΪAHBʱ�ӣ�������������SYSTICKʱ��Ƶ��ΪAHB/8
//����Ϊ�˼���FreeRTOS�����Խ�SYSTICK��ʱ��Ƶ�ʸ�ΪAHB��Ƶ�ʣ�
//SYSCLK:ϵͳʱ��Ƶ��
void delay_init(u8 SYSCLK)
{
	u32 reload;
 	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK); 
	reload=SYSCLK;							//ÿ���ӵļ������� ��λΪM	   
	reload*=1000000/1000;		//����delay_ostickspersec�趨���ʱ��
											//reloadΪ24λ�Ĵ���,���ֵ:16777216,��168M��,Լ��0.0998s����	
		   
	SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;//����SYSTICK�ж�
	SysTick->LOAD=reload; 					//ÿ1/configTICK_RATE_HZ��һ��	
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk; //����SYSTICK     
}								    


//��ʱnms
//nms:Ҫ��ʱ��ms��
//nms:0~65535
void delay_ms(uint32_t nms)
{	
	mscnt=0;
	while(nms>mscnt);
}

			 



































