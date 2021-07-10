/*
 * traffic_timer.h
 *
 *  Created on: 21Sep.,2020
 *      Author: yong
 */

#ifndef TRAFFIC_TIMER_H_
#define TRAFFIC_TIMER_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <stdint.h>
#include <pthread.h>

#define MY_PULSE_CODE    1  //_PULSE_CODE_MINAVAIL

 struct sigevent         event;
 struct itimerspec       itime;
 timer_t                 traffic_timer_id;
 int                     channel_id;
 int                     rcvid;

 struct _pulse          traffic_pulse;

 int timer_connect(int* chid, struct sigevent* event);

 int create_timer(clockid_t id, struct sigevent *event,timer_t *timer_id);

 void timer_start(struct itimerspec *time,timer_t timer_id, int timer_flag,uint32_t ini_delay_sec, uint32_t ini_delay_nsec, uint32_t interval_delay_sec, uint32_t interval_delay_nsec);

 int timer_delete(timer_t id);

#endif /* TRAFFIC_TIMER_H_ */
