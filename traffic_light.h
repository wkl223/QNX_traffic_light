/*
 * traffic_light.h
 *
 *  Created on: 21Sep.,2020
 *      Author: yong
 *
 *  Modified by Kaile Wang
 *  03/10/2020
 */

#ifndef TRAFFIC_LIGHT_H_
#define TRAFFIC_LIGHT_H_

#include "traffic_client.h"
#include "traffic_server.h"
#include "traffic_light.h"
#include "traffic_sensor_signal_simulator.h"
#include "traffic_timer.h"

#define train_cross_time_cnt  50  //5 sec to change state
#define fix_time_cnt      100
#define fix_time_mode     0
#define sensor_mode       1
#define advanced_mode_1   2  // R3 clear green light
#define advanced_mode_2   3  // R1 clear green light
#define sp_mode           4
#define advanced_mode_reset_timer 100


enum states {State0,State1,State2,State3,State4};

int execute_seq[5];

extern enum states CurrentState;

extern traffic_sensor_data sensor_data;

extern traffic_data server_traffic_data;

pthread_t  traffic_server_th1, traffic_client_th2, traffic_sensor_th3;

void *traffic_server_retval_1, *traffic_client_retval_2, *traffic_sensor_retval_3;

pthread_attr_t traffic_server_th1_attr, traffic_client_th2_attr, traffic_sensor_th3_attr;

struct sched_param traffic_server_th1_param, traffic_client_th2_param, traffic_sensor_th3_param;


void traffic_state_machine(enum states *CurrentState,traffic_sensor_data *data, traffic_data *server_data);

void thread_create(void* (*thread_name)(void* ),void* thread_data,pthread_t thread,pthread_attr_t thread_attr,struct sched_param thread_param,int sched_policy,int priority,int stacksize);

void thread_join(pthread_t thread, void* retval);

void traffic_mode_change(traffic_data *data,uint8_t mode);

void traffic_intersection_I3_process(void);

void check_server_msg(traffic_data *server_data);

void traffic_state_move(traffic_data *server_data);

void load_sequence(traffic_data *server_data);











#endif /* TRAFFIC_LIGHT_H_ */
