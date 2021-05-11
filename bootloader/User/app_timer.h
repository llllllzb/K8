#ifndef TIMER_H
#define TIMER_H

#include "app_common.h"
#define TIMER_MAX	16
typedef struct
{
	uint32_t sys_tick;
} TimerInfo;

typedef struct System_Timer
{
	uint8_t  timer_id;
	uint8_t  repeat;
	uint32_t repeattime;
	uint32_t start_tick;
	uint32_t stop_tick;
	void (*fun)(void);
	struct System_Timer *next;
} Timer;

void timerInfoInit(void);
int8_t startTimer(uint32_t time,void (*fun)(void),uint8_t repeat);
void stopTimer(uint8_t timer_id);
void timerRun(void);
#endif
