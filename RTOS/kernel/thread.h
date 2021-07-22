#ifndef TASK__H
#define TASK__H
#include <stdint.h>
#include "foxport.h"
#define TIMBASE 0
#ifndef NULL
#define NULL ((void*)0)
#endif
//定义任务控制块
typedef struct threadTCB{
	uint8_t tID;//任务id
	uint8_t tPriority;//任务优先级
	uint8_t tStatus;//任务状态
	struct threadTCB* pNextTCB;
	foxTCB* pfoxTCB;

}fThreadTCB;
//定义定时器节点
typedef struct tTim{
	uint32_t count;//延时量
	fThreadTCB* pThread;
	struct tTim* pNextTimer;
}fTimer;

//定义任务状态
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