#include "thread.h"
#include "memfox.h"
#include "stdio.h"
/*
�߳���
�������ɼ�״̬������̬������̬������̬
���������ɼ�״̬������̬����ʬ̬
*/
static fThreadTCB pThreadReady;//�����̶߳���
static fThreadTCB pThreadRun;//�����߳�
static fThreadTCB pThreadBlock;//�����̶߳���
static fThreadTCB pThreadPend;//�����̶߳���
static fThreadTCB pThreadZombie;//��ʬ�̶߳���
static fTimer pThreadTimer;//��ʱ������


/*
���̶߳��в���һ���ڵ�
�����߳����ȼ���������
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
�Ƴ����̶����е�ĳ���ڵ�
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
��ĳ���߳��ƶ�����һ�����У�ͬʱ�ı��̵߳�״̬
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
ϵͳ״̬��
*��֪����ʲô�ã���֮�ȷ�������
*/
static uint8_t tID=0;
/*
��������
*/
void VoidThread(void)
{
	while(1)
	{
	}
}

/*
��������,��̬����
pFunction->����ָ��
Pori->�������ȼ�
SatckSize->�����ջ��С
*/
uint8_t fCreateThread(void (*pFunction)(void),uint8_t Pori,uint32_t SatckSize)
{
	fThreadTCB* pTempTCB;
	uint8_t* pStack;
	//����TCB�ռ�
	pTempTCB=(fThreadTCB*)memfox_malloc(sizeof(fThreadTCB));
	//��ʼ���������
	pTempTCB->tID=tID;
	tID++;
	pTempTCB->tPriority=Pori;
	pTempTCB->tStatus=THREAD_READY;
	pTempTCB->pfoxTCB=(foxTCB*)memfox_malloc(sizeof(foxTCB));
	pStack=(uint8_t*)memfox_malloc(SatckSize);
	FoxInitStack(pTempTCB->pfoxTCB, (void*)pStack,SatckSize,pFunction);
	//������TCB���ص������б�
	InsertNode(pTempTCB,&pThreadReady);
	return 0;

}



/*
��ʼ��OS
*��ʼ���ڴ��
*������������
*/
uint8_t FoxHole[25*1024] __attribute__ ((aligned (4)));//�ڴ��,4�ֽڶ���
uint8_t fThreadInit(void)
{
	//��ʼ��ϵͳ����
	//��ʼ���ڴ��
	if(memfox_init(FoxHole,25*1024)!=0)
	{
		return -1;
	}
	//��ʼ��������
	//�����б�
	pThreadReady.tPriority=0;
	pThreadReady.tID=0;
	pThreadReady.pfoxTCB=NULL;
	pThreadReady.pNextTCB=NULL;
	pThreadReady.tStatus=THREAD_ZOMBIE;
	//��������
	pThreadRun.tPriority=0;
	pThreadRun.tID=0;
	pThreadRun.pfoxTCB=NULL;
	pThreadRun.pNextTCB=NULL;
	pThreadRun.tStatus=THREAD_ZOMBIE;
	
	//�����б�
	pThreadBlock.tPriority=0;
	pThreadBlock.tID=0;
	pThreadBlock.pfoxTCB=NULL;
	pThreadBlock.pNextTCB=NULL;
	pThreadBlock.tStatus=THREAD_ZOMBIE;
	//�����б�
	//pThreadPend=NULL;
	//��ʬ�б�
	//pThreadZombie=NULL;
	//��ʱ���б�
	pThreadTimer.count=0;
	pThreadTimer.pNextTimer=NULL;
	pThreadTimer.pThread=NULL;
	//������������
	fCreateThread(VoidThread,0,512);
	return 0;
}



/*
ʵ�ֵ�����
*��鵱ǰ����״̬
*����ǰ������ص������б�
*ѡȡ������ȼ���������
*�л�����״̬
*��������Ҫ����Ȩģʽ������
*/
extern uint32_t *volatile pCurrentTCB,*volatile pNextTCB;
void FoxScheduler(void)
{
	
	fThreadTCB* pThread;
	//pThread=pThreadRun.pNextTCB;
	pThread=pThreadRun.pNextTCB;
	//��鵱ǰ����״̬��ֻ����������̬��������ص������б�
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
	//�Ӿ����б���ժȡ������ȼ������б�ĵ�һ���ڵ���Զ�����ȼ���ߵ�����
	pCurrentTCB=pThread->pfoxTCB;
	//printf("curr id:%d\n",(*pThread).tPriority);
	pThread=pThreadReady.pNextTCB;
	//printf("next id:%d\n",(*pThread).tPriority);
	MoveThread(&pThreadReady,&pThreadRun,pThread,THREAD_RUNNING);
	pNextTCB=pThread->pfoxTCB;
	//���������л� 
	TRIGSWITCH;
}

/*
����ϵͳ
*/
uint8_t threadrun=0;
void FoxStart(void)
{
	fThreadTCB* pThread;
	threadrun=1;
	//���õ�һ������
	pThread=pThreadReady.pNextTCB;
	MoveThread(&pThreadReady,&pThreadRun,pThread,THREAD_RUNNING);
	pCurrentTCB=pThread->pfoxTCB;
	 //������һ������
	foxPortStartFirstTask();
}



/*
����һ����ʱ���ڵ�
�ڵ����ǲ�������ͷ
*/
void InsertTimer(fTimer* tim,fTimer* head)
{
	tim->pNextTimer=head->pNextTimer;
	head->pNextTimer=tim;
}

/*
�Ƴ�һ����ʱ���ڵ�
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
*����ǰ������ص������б�
*������ǰ���񣬵ȴ����������е���
*/
void FoxSleep(uint32_t time)
{
	fTimer* pTimer;
	fThreadTCB* pThread;
	pThread=pThreadRun.pNextTCB;
	(*pThread).tStatus=THREAD_BLOCKED;
	//������ʱ���ڵ㣬����ʱ���ڵ���ص���ʱ���б�
	pTimer=(fTimer*)memfox_malloc(sizeof(fTimer));
	pTimer->count=time+TIMBASE;
	pTimer->pThread=pThread;
	InsertTimer(pTimer,&pThreadTimer);
	//�������񣬵ȴ���һ�������л�
	while((pTimer->count)>TIMBASE);
	//�˳���ʱ��ɾ����ʱ���ڵ㣬�ͷ���Դ
	RemoveTimer(pTimer,&pThreadTimer);
	memfox_free(pTimer);
}

/*
sleep tick tock
ÿ�ε��ü��ٶ�ʱ�������е�countֵ��
��count=0ʱ������������б���ժ�������ص������б��ı�����״̬
*/
void SleepTickTock(void)
{
	#if 1
	int i=0;
	fTimer* pWork;
	fThreadTCB *pThread;
	//������ʱ���б�
	pWork=pThreadTimer.pNextTimer;
	while(pWork!=NULL)
	{
		(pWork->count)--;
		if((pWork->count)<=TIMBASE)
		{
			pThread=pWork->pThread;
			//�������б�ժ��
			//���ص������б�
			 MoveThread(&pThreadBlock,&pThreadReady,pThread,THREAD_READY);
		}
		pWork=pWork->pNextTimer;
	}
	#endif
}
