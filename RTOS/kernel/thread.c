#include "thread.h"
#include "memfox.h"
#include "stdio.h"
/*
线程树
调度器可见状态：就绪态、运行态、阻塞态
调度器不可见状态：挂起态、僵尸态
*/
static fThreadTCB pThreadReady;//就绪线程队列
static fThreadTCB pThreadRun;//运行线程
static fThreadTCB pThreadBlock;//阻塞线程队列
static fThreadTCB pThreadPend;//挂起线程队列
static fThreadTCB pThreadZombie;//僵尸线程队列
static fTimer pThreadTimer;//定时器队列


/*
向线程队列插入一个节点
按照线程优先级降序排序
*/
void InsertNode(fThreadTCB* node,fThreadTCB* head)
{
 fThreadTCB *work,*pwork;
 work=head->pNextTCB;
 pwork=head;
 while(work!=NULL)
 {
	if((*work).tPriority<=(*node).tPriority)
		break;
	pwork=work;
	work=work->pNextTCB;
 }
 node->pNextTCB=work;
 pwork->pNextTCB=node;
}

/*
移除进程队列中的某个节点
*/
void RemoveNode(fThreadTCB* node,fThreadTCB *head)
{
	fThreadTCB *work,*pwork;
	work=head->pNextTCB;
	pwork=head;
	while(work!=NULL)
	{
		if(work==node)
			break;
		pwork=work;
		work=work->pNextTCB;
	}
	if(work==NULL)
		pwork->pNextTCB=work;
	else if(work!=NULL)
		pwork->pNextTCB=work->pNextTCB;

}
/*
将某个线程移动到另一个队列，同时改变线程的状态
*/
void MoveThread(fThreadTCB* psrc,fThreadTCB* pdst,fThreadTCB* pThread,uint8_t status)
{
	RemoveNode(pThread,psrc);
	(*pThread).tStatus=status;
	InsertNode(pThread,pdst);
}

void PrintList(fThreadTCB* head)
{
	fThreadTCB* node;
	int i=0;
	node=head;
	while(node!=NULL)
	{
		printf("node[%d]:pori[%d]\n",i,node->tPriority);
		node=node->pNextTCB;
		i++;
	}
	printf("--------\n");
}

/*
系统状态表
*不知道有什么用，总之先放在上面
*/
static uint8_t tID=0;
/*
空闲任务
*/
void VoidThread(void)
{
	while(1)
	{
	}
}

/*
创建任务,动态创建
pFunction->任务指针
Pori->任务优先级
SatckSize->任务堆栈大小
*/
uint8_t fCreateThread(void (*pFunction)(void),uint8_t Pori,uint32_t SatckSize)
{
	fThreadTCB* pTempTCB;
	uint8_t* pStack;
	//申请TCB空间
	pTempTCB=(fThreadTCB*)memfox_malloc(sizeof(fThreadTCB));
	//初始化任务参数
	pTempTCB->tID=tID;
	tID++;
	pTempTCB->tPriority=Pori;
	pTempTCB->tStatus=THREAD_READY;
	pTempTCB->pfoxTCB=(foxTCB*)memfox_malloc(sizeof(foxTCB));
	pStack=(uint8_t*)memfox_malloc(SatckSize);
	FoxInitStack(pTempTCB->pfoxTCB, (void*)pStack,SatckSize,pFunction);
	//将任务TCB挂载到就绪列表
	InsertNode(pTempTCB,&pThreadReady);
	return 0;

}



/*
初始化OS
*初始化内存池
*创建空闲任务
*/
uint8_t FoxHole[25*1024] __attribute__ ((aligned (4)));//内存池,4字节对齐
uint8_t fThreadInit(void)
{
	//初始化系统变量
	//初始化内存池
	if(memfox_init(FoxHole,25*1024)!=0)
	{
		return -1;
	}
	//初始化任务树
	//就绪列表
	pThreadReady.tPriority=0;
	pThreadReady.tID=0;
	pThreadReady.pfoxTCB=NULL;
	pThreadReady.pNextTCB=NULL;
	pThreadReady.tStatus=THREAD_ZOMBIE;
	//运行任务
	pThreadRun.tPriority=0;
	pThreadRun.tID=0;
	pThreadRun.pfoxTCB=NULL;
	pThreadRun.pNextTCB=NULL;
	pThreadRun.tStatus=THREAD_ZOMBIE;
	
	//阻塞列表
	pThreadBlock.tPriority=0;
	pThreadBlock.tID=0;
	pThreadBlock.pfoxTCB=NULL;
	pThreadBlock.pNextTCB=NULL;
	pThreadBlock.tStatus=THREAD_ZOMBIE;
	//挂起列表
	//pThreadPend=NULL;
	//僵尸列表
	//pThreadZombie=NULL;
	//定时器列表
	pThreadTimer.count=0;
	pThreadTimer.pNextTimer=NULL;
	pThreadTimer.pThread=NULL;
	//创建空闲任务
	fCreateThread(VoidThread,0,512);
	return 0;
}



/*
实现调度器
*检查当前任务状态
*将当前任务挂载到就绪列表
*选取最高优先级任务运行
*切换任务状态
*调度器需要在特权模式下运行
*/
extern uint32_t *volatile pCurrentTCB,*volatile pNextTCB;
void FoxScheduler(void)
{
	
	fThreadTCB* pThread;
	//pThread=pThreadRun.pNextTCB;
	pThread=pThreadRun.pNextTCB;
	//检查当前任务状态，只将处在运行态的任务挂载到就绪列表
	switch((*pThread).tStatus)
	{
		case THREAD_RUNNING:
		{
			MoveThread(&pThreadRun,&pThreadReady,pThread,THREAD_READY);
		}break;
		case THREAD_BLOCKED:
		{
			MoveThread(&pThreadRun,&pThreadBlock,pThread,THREAD_READY);
		}break;
	}
	//从就绪列表中摘取最高优先级任务，列表的第一个节点永远是优先级最高的任务
	pCurrentTCB=pThread->pfoxTCB;
	//printf("curr id:%d\n",(*pThread).tPriority);
	pThread=pThreadReady.pNextTCB;
	//printf("next id:%d\n",(*pThread).tPriority);
	MoveThread(&pThreadReady,&pThreadRun,pThread,THREAD_RUNNING);
	pNextTCB=pThread->pfoxTCB;
	//触发任务切换 
	TRIGSWITCH;
}

/*
启动系统
*/
uint8_t threadrun=0;
void FoxStart(void)
{
	fThreadTCB* pThread;
	threadrun=1;
	//配置第一个任务
	pThread=pThreadReady.pNextTCB;
	MoveThread(&pThreadReady,&pThreadRun,pThread,THREAD_RUNNING);
	pCurrentTCB=pThread->pfoxTCB;
	 //启动第一个任务
	foxPortStartFirstTask();
}



/*
插入一个定时器节点
节点总是插在链表头
*/
void InsertTimer(fTimer* tim,fTimer* head)
{
	tim->pNextTimer=head->pNextTimer;
	head->pNextTimer=tim;
}

/*
移除一个定时器节点
*/
void RemoveTimer(fTimer* tim,fTimer* head)
{
	fTimer *work,*pwork;
	work=head->pNextTimer;
	pwork=head;
	while(work!=NULL)
	{
		if(work==tim)
			break;
		pwork=work;
		work=work->pNextTimer;
	}
	if(work==NULL)
		pwork->pNextTimer=work;
	else if(work!=NULL)
		pwork->pNextTimer=work->pNextTimer;
}

/*
sleep
*将当前任务挂载到阻塞列表
*阻塞当前任务，等待调度器进行调度
*/
void FoxSleep(uint32_t time)
{
	fTimer* pTimer;
	fThreadTCB* pThread;
	pThread=pThreadRun.pNextTCB;
	(*pThread).tStatus=THREAD_BLOCKED;
	//创建定时器节点，将定时器节点挂载到定时器列表
	pTimer=(fTimer*)memfox_malloc(sizeof(fTimer));
	pTimer->count=time+TIMBASE;
	pTimer->pThread=pThread;
	InsertTimer(pTimer,&pThreadTimer);
	//阻塞任务，等待下一次任务切换
	while((pTimer->count)>TIMBASE);
	//退出定时，删除定时器节点，释放资源
	RemoveTimer(pTimer,&pThreadTimer);
	memfox_free(pTimer);
}

/*
sleep tick tock
每次调用减少定时器队列中的count值，
当count=0时将任务从阻塞列表中摘除，挂载到就绪列表并改变任务状态
*/
void SleepTickTock(void)
{
	#if 1
	int i=0;
	fTimer* pWork;
	fThreadTCB *pThread;
	//遍历定时器列表
	pWork=pThreadTimer.pNextTimer;
	while(pWork!=NULL)
	{
		(pWork->count)--;
		if((pWork->count)<=TIMBASE)
		{
			pThread=pWork->pThread;
			//从阻塞列表摘除
			//挂载到就绪列表
			 MoveThread(&pThreadBlock,&pThreadReady,pThread,THREAD_READY);
		}
		pWork=pWork->pNextTimer;
	}
	#endif
}
