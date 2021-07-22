#include "foxport.h"
#include "thread.h"
//switch Current to Next
foxTCB *volatile pCurrentTCB,*volatile pNextTCB;

/*-----------------------------------------------------------*/
/*
设置msp，出发SVC异常
*/
 void foxPortStartFirstTask( void )
{
	__asm volatile(
					" ldr r0, =0xE000ED08 	\n" /* Use the NVIC offset register to locate the stack. */
					" ldr r0, [r0] 			\n"
					" ldr r0, [r0] 			\n"
					" msr msp, r0			\n" /* Set the msp back to the start of the stack. */
					" cpsie i				\n" /* Globally enable interrupts. */
					" cpsie f				\n"
					" dsb					\n"
					" isb					\n"
					" svc 0					\n" /* System call to start first task. */
					" nop					\n"
				);
}
/*-----------------------------------------------------------*/

/*
用于启动第一个任务
*/
void vPortSVCHandler( void )
{
	__asm volatile (
					"	ldr	r3, =pCurrentTCB		\n" /* Restore the context. */
					"	ldr r1, [r3]					\n" /* Use pxCurrentTCBConst to get the pxCurrentTCB address. */
					"	ldr r0, [r1]					\n" /* The first item in pxCurrentTCB is the task top of stack. */
					"	ldmia r0!, {r4-r11, r14}		\n" /* Pop the registers that are not automatically saved on exception entry and the critical nesting count. */
					"	msr psp, r0						\n" /* Restore the task stack pointer. */
					"	isb								\n"
					"	mov r0, #0 						\n"
					"	msr	basepri, r0					\n"
					"									\n"
					"	bx r14							\n" /*这里R14已经设置为使用线程模式*/
				);
}
/*-----------------------------------------------------------*/

/*
PendSV中断服务函数
实现任务切换
pCurrtnt-->>pNextTCB

//stack frame
//--------------high address
>no name reg
>FPSCR
>{s15-s0}
>xPSR
>r15(PC)
>r14(LR)
>r12
>{r3-r0}
>r14
>{r11-r4}
>{s31-s16}
//--------------low address
*/
void xPortPendSVHandler( void )
{
	/* This is a naked function. */
#if 1
	__asm volatile
	(
	/*保存当前任务上下文*/
	"	mrs r0, psp							\n" /*获取当前psp*/
	"	isb									\n"
	"										\n"
	"	tst r14, #0x10						\n" /* Is the task using the FPU context?  If so, push high vfp registers. */
	"	it eq								\n"
	"	vstmdbeq r0!, {s16-s31}				\n"
	"										\n"
	"	stmdb r0!, {r4-r11, r14}			\n" /* Save the core registers. */
	"										\n"
	"	ldr	r3, =pCurrentTCB				\n" /*获得当前TCB地址  */
	"	ldr	r1, [r3]						\n"
	"	str r0, [r1]						\n" /* Save the new top of stack into the first member of the TCB. */
	"										\n"
	"	mov r0, #0 							\n"
	"	msr basepri, r0						\n"
	"	dsb									\n"
	"	isb									\n"

	/*加载目标任务上下文*/
	"	ldr r3, =pNextTCB					\n"
	"	ldr r1, [r3]						\n" /* The first item in pxCurrentTCB is the task top of stack. */
	"	ldr r0, [r1]						\n"
	"	ldmia r0!, {r4-r11, r14}			\n" /* Pop the core registers. */
	"										\n"
	"	tst r14, #0x10						\n" /* Is the task using the FPU context?  If so, pop the high vfp registers too. */
	"	it eq								\n"
	"	vldmiaeq r0!, {s16-s31}				\n"
	"										\n"
	"	msr psp, r0							\n" /*将新的栈顶地址加载到psp*/
	"	isb									\n"
	"	mov r0, #0							\n"
	"	msr basepri, r0						\n"
	"	dsb									\n"
	"	isb									\n"
	"	bx r14								\n"
	"										\n"
	);
#endif
}
/*-----------------------------------------------------------*/
/*
任务异常退出捕获函数
任务正常情况下永远不会跳转到此函数
*/
static void TaskExitError( void )
{
	while(1);
}

/*-----------------------------------------------------------*/
/*
TCB初始化
pTCB->TCB指针
stack->任务栈起始地址
stackdepth->任务栈大小（字节）
pFunction->任务入口地址
返回初始化结果
*/
uint8_t  FoxInitStack(foxTCB* pTCB, void *pStack,uint32_t iStackDepth,void (*pFunction)(void))
{
    uint32_t *pxTopOfStack;
    int i;
//-------------------------------------------
    /*初始化任务栈*/
    pxTopOfStack=(uint32_t *)((uint8_t*)pStack+iStackDepth);
	 
	//栈顶指针8字节对齐
	 pxTopOfStack=ADDR_ALIGNED8((uint32_t)pxTopOfStack);
	
    /*初始化浮点寄存器*/
    #if (__FPU_PRESENT == 1)
    pxTopOfStack--;
    *pxTopOfStack=0;
    pxTopOfStack--;
    *pxTopOfStack=0x03000000;    /* FPSCR		*/
    for(i=0;i<16;i++)
    {
        pxTopOfStack--;
        *pxTopOfStack=0;
    }
    #endif
   
	/* Offset added to account for the way the MCU uses the stack on entry/exit
	of interrupts, and to ensure alignment. */
	pxTopOfStack--;
	*pxTopOfStack = 0x01000000;	/* xPSR */
	pxTopOfStack--;
	*pxTopOfStack = ( (uint32_t) pFunction )&0xfffffffe;	/* PC */
	pxTopOfStack--;
	*pxTopOfStack = ( uint32_t ) TaskExitError;	/* LR */

	/* Save code space by skipping register initialisation. */
	pxTopOfStack -= 5;	/* R12, R3, R2 and R1. */
	*pxTopOfStack =0;	/* R0 */

	/* A save method is being used that requires each task to maintain its
	own exec return value. */
	pxTopOfStack--;
	*pxTopOfStack = 0xfffffffd;/*R14*/
	pxTopOfStack -= 8;	/* R11, R10, R9, R8, R7, R6, R5 and R4. */

	//set TCB
	pTCB->p_foxTopOfStack=pxTopOfStack;
	pTCB->p_foxStartOfStack=(uint32_t*)pStack;

	return 0;
}

/*-----------------------------------------------------------*/