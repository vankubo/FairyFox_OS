#ifndef TASK__H
#define TASK__H
#include <stdint.h>
#include "foxport.h"
#define TIMBASE 0
#ifndef NULL
#define NULL ((void*)0)
#endif
//����������ƿ�
typedef struct threadTCB{
	uint8_t tID;//����id
	uint8_t tPriority;//�������ȼ�
	uint8_t tStatus;//����״̬
	struct threadTCB* pNextTCB;
	foxTCB* pfoxTCB;

}fThreadTCB;
//���嶨ʱ���ڵ�
typedef struct tTim{
	uint32_t count;//��ʱ��
	fThreadTCB* pThread;
	struct tTim* pNextTimer;
}fTimer;

//��������״̬
enum ThreadStatus{
	THREAD_RUNNING=0,
	THREAD_READY,
	THREAD_BLOCKED,
	THREAD_PENDING,
	THREAD_ZOMBIE
};

//function
uint8_t fThreadInit(void);
void FoxSleep(uint32_t time);
void FoxScheduler(void);
void SleepTickTock(void);
uint8_t fCreateThread(void (*pFunction)(void),uint8_t Pori,uint32_t SatckSize);
void FoxStart(void);
#endif 