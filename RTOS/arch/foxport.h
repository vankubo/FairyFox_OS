#ifndef FOXPORT_H
#define FOXPORT_H
#include <stdint.h>
#include "stm32f4xx.h"

#define configPRIO_BITS       		4 
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY			15                      //中断最低优先级
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY	5                       //系统可管理的最高中断优先级
#define configKERNEL_INTERRUPT_PRIORITY 		( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

#define portNVIC_SYSPRI2_REG				( * ( ( volatile uint32_t * ) 0xe000ed20 ) )
#define portNVIC_PENDSV_PRI					( ( ( uint32_t ) configKERNEL_INTERRUPT_PRIORITY ) << 16UL )



#define portNVIC_INT_CTRL_REG		( * ( ( volatile uint32_t * ) 0xe000ed04 ) )
#define portNVIC_PENDSVSET_BIT		( 1UL << 28UL )
#define TRIGSWITCH	(portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT)

#define ADDR_ALIGNED8(addr)  (((addr)) & (0xfffffff8))
/*
任务控制块
*/
typedef struct{
	volatile uint32_t *p_foxTopOfStack; //栈顶指针
	volatile uint32_t *p_foxStartOfStack;//任务栈起始地址
}foxTCB;


/*
中断宏
*/
#define xPortPendSVHandler 	PendSV_Handler
#define vPortSVCHandler 	SVC_Handler
void foxPortStartFirstTask( void );
uint8_t  FoxInitStack(foxTCB* pTCB, void *pStack,uint32_t iStackDepth,void (*pFunction)(void));
#endif