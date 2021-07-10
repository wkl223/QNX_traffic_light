/*
 * traffic_timer.c
 *
 *  Created on: 21Sep.,2020
 *      Author: yong
 */

#include "traffic_timer.h"

int timer_connect(int *chid, struct sigevent *event)  // create a channel and connect to itself
{

	*chid = ChannelCreate(0); // Create a communications channel
	event->sigev_notify = SIGEV_PULSE;

	 struct sched_param th_param;
	 pthread_getschedparam(pthread_self(), NULL, &th_param);
	 event->sigev_priority = th_param.sched_curpriority;    // old QNX660 version getprio(0);

	 event->sigev_code = MY_PULSE_CODE;
	// create a connection back to ourselves for the timer to send the pulse on
	event->sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, (int)*chid, _NTO_SIDE_CHANNEL, 0);
	if (event->sigev_coid == -1)
	{
	    return 0;
	   }


	return 1;
}

int create_timer(clockid_t id, struct sigevent *event,timer_t *timer_id )  // create a timer
{
	if (timer_create(id, event, timer_id) == -1)
	    		{
	    		   return 0;
	    		}

	return 1;

}

int timer_delete(timer_t id)  // delete the timer
{
	if (timer_delete(id)==-1)
	{
		return 0;
	}

	return 1;

}

// setting the timer delay value and start the timer
void timer_start(struct itimerspec *time,timer_t timer_id, int timer_flag,uint32_t ini_delay_sec, uint32_t ini_delay_nsec, uint32_t interval_delay_sec, uint32_t interval_delay_nsec)
{
	            time->it_value.tv_sec = ini_delay_sec;			  // 1 second
	    		time->it_value.tv_nsec = ini_delay_nsec;    // 100 million nsecs = .1 secs
	    		time->it_interval.tv_sec = interval_delay_sec;          // 1 second
	    		time->it_interval.tv_nsec = interval_delay_nsec; // 100 million nsecs = .1 secs
	    		timer_settime(timer_id, timer_flag, time, NULL);

}


