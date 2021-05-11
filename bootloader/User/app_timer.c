#include "app_timer.h"

uint8_t timer_list[TIMER_MAX];

static Timer *timer_head=NULL;
TimerInfo timer_info={0};


void timerInfoInit(void)
{
    timer_info.sys_tick=0;
}


static uint8_t Create_Timer( uint32_t time,uint8_t timer_id,void (*fun)(void),uint8_t repeat)
{
    Timer * next;
    if(timer_head==NULL)
    {
        timer_head=malloc(sizeof(Timer));
        timer_head->timer_id=timer_id;
        timer_head->start_tick=timer_info.sys_tick;
        timer_head->stop_tick=timer_info.sys_tick+time;
        timer_head->repeat=repeat;
        timer_head->repeattime=time;
        timer_head->fun=fun;
        timer_head->next=NULL;
        LogPrintf("Create Head id=%d\r\n",timer_id);
        return 0;
    }
    next=timer_head;
    do
    {
        if(next->next==NULL)
        {
            next->next=malloc(sizeof(Timer));
            if(next->next==NULL)
                break;
            next=next->next;
            next->fun=fun;
            next->next=NULL;
            next->start_tick=timer_info.sys_tick;
            next->stop_tick=timer_info.sys_tick+time;
            next->repeat=repeat;
            next->repeattime=time;
            next->timer_id=timer_id;
            LogPrintf("Create node id=%d\n",timer_id);
            break;
        }
        else
        {
            next=next->next;
        }
    }
    while(next!=NULL);

    return 1;
}

int8_t startTimer(uint32_t time,void (*fun)(void),uint8_t repeat)
{
    int i=0;
    for(i=0; i<TIMER_MAX; i++)
    {
        if(timer_list[i]==0)
        {
            timer_list[i]=1;
            Create_Timer(time,i,fun,repeat);
            return i;
        }

    }
    return -1;

}

void stopTimer(uint8_t timer_id)
{
    Timer * next,*pre;
    next=timer_head;
    pre=next;
    while(next!=NULL)
    {
        if(next->timer_id==timer_id)
        {
            LogPrintf("Stop Timer %d\n",next->timer_id);
            if(next==timer_head)
            {
                LogMessage("stop head node\n");
                next=next->next;
                LogPrintf("next = %d\n",next->timer_id);
                timer_list[timer_head->timer_id]=0;
                free(timer_head);
                timer_head=next;
                continue;
            }
            pre->next=next->next;
			timer_list[next->timer_id]=0;
            free(next);
            next=pre;
        }
        pre=next;
        next=next->next;
    }
}

void timerRun(void)
{
    Timer * next,*pre,*cur;
    next=timer_head;
    pre=next;
    timer_info.sys_tick=HAL_GetTick();
    while(next!=NULL)
    {
        if(next->stop_tick<=timer_info.sys_tick)
        {
            next->fun();
            if(next->repeat==0)
            {
                if(next==timer_head)
                {
                    LogPrintf("Delete Head:%d\n",next->timer_id);
                    next=next->next;
                    free(timer_head);
                    timer_list[timer_head->timer_id]=0;
                    timer_head=next;
                    continue;
                }
                cur=next;
                timer_list[cur->timer_id]=0;
                LogPrintf("Delete:%d\n",cur->timer_id);
                pre->next=cur->next;
                next=pre;
                free(cur);
            }
            else
            {
                next->stop_tick+=next->repeattime;
            }
        }
        pre=next;
        next=next->next;
    }
}


