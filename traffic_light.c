/*
 * traffic_light.c
 *
 *  Created on: 21Sep.,2020
 *      Author: yong
 *
 *  Modified by Kaile Wang
 *  03/10/2020
 */

#include "traffic_light.h"

uint8_t ped_light = 0;
uint8_t gate_status = 0; //0 for close, 1 for open

traffic_sensor_data sensor_data = { 0, 0, 0, 0, 0, 0 };

enum states CurrentState = State0; // traffic states

traffic_data server_traffic_data = { 0, 100 };

uint32_t pulse_cnt = 0;
uint8_t traffic_mode_buff = 0;
uint8_t train_1_in_buff = 0;
uint8_t train_2_in_buff = 0;
uint8_t train_1_out_buff = 0;
uint8_t train_2_out_buff = 0;
uint8_t tram_depot_gate_buff = 0;
uint32_t state_change_cnt_buff = 0;
uint8_t north_south_state_change_signal = 0;
uint8_t advanced_mode_reset_cnt = 0;

int execute_seq[5] = { State0, State1, State2, State3, State4 };
int current_exet = 0;

void check_server_msg(traffic_data *server_data) {
	pthread_mutex_lock(&server_data->mutex_mode);
	if (server_data->traffic_mode == fix_time_mode)
		traffic_encode_data |= T_mode;
	else if (server_data->traffic_mode == sensor_mode)
		traffic_encode_data |= S_mode;
	else if (server_data->traffic_mode == advanced_mode_1)
		traffic_encode_data |= A1_mode;
	else if (server_data->traffic_mode == advanced_mode_2)
		traffic_encode_data |= A2_mode;
	else
		traffic_encode_data |= SP_MODE;
	pthread_mutex_unlock(&server_data->mutex_mode);
}
void traffic_mode_change(traffic_data *data, uint8_t mode) {
	pthread_mutex_lock(&data->mutex_mode);
	data->traffic_mode = mode;
	pthread_mutex_unlock(&data->mutex_mode);
}

void traffic_state_move(traffic_data *server_data) {
	int is_sp_mode = 0;
	pthread_mutex_lock(&server_data->mutex_mode);
	if (server_data->traffic_mode == sp_mode)
		is_sp_mode = 1;
	pthread_mutex_unlock(&server_data->mutex_mode);
	if (!is_sp_mode) {
		current_exet=0; //reset special mode counter.
		if (CurrentState < State3)
			CurrentState++;
		else if (tram_depot_gate_buff)
			CurrentState = State4;
		else
			CurrentState = State0;
	} else {
		current_exet++;
		if(current_exet < sizeof(execute_seq)/sizeof(*execute_seq)){
			CurrentState = execute_seq[current_exet];
		}
		else{
			current_exet=0;
			CurrentState = execute_seq[current_exet];
		}
	}
}

void traffic_state_machine(enum states *CurrentState, traffic_sensor_data *data,
		traffic_data *server_data) {
	switch (*CurrentState) {
	case State0:

		printf("state 0 east west CAR light RED\n");
		printf("state 0 east west TRAM RED\n");
		printf("state 0 north south pedestrain RED\n");
		printf("state 0 GATE OFF\n");

		pthread_mutex_lock(&mutex_traffic_state);

		if (ped_light == 1)
			ped_light = 0;
		traffic_encode_data = 0;
		check_server_msg(server_data);
		traffic_encode_data |= (EWR | NSPCR | TRAMR); //EW CAR:RED, NS PEDESTRAIN: RED, TRAM: RED

		traffic_state_change = 1;
		pthread_cond_signal(&cond_traffic_state);
		pthread_mutex_unlock(&mutex_traffic_state);
		break;

	case State1:
		printf("state 1 east west CAR light GREEN\n");
		printf("state 1 east west TRAM GREEN\n");
		printf("state 1 north south pedestrain RED\n");
		printf("state 1 GATE OFF\n");

		pthread_mutex_lock(&mutex_traffic_state);
		traffic_encode_data = 0;
		check_server_msg(server_data);
		traffic_encode_data |= (EWG | NSPCR | TRAMG); //NS CAR:GREEN, EW PEDESTRAIN: RED, TRAM: GREEN

		traffic_state_change = 1;
		pthread_cond_signal(&cond_traffic_state);
		pthread_mutex_unlock(&mutex_traffic_state);

		break;

	case State2:

		printf("state 2 east west CAR light YELLOW\n");
		printf("state 2 east west TRAM GREEN\n"); //assume tram light only has red and green
		printf("state 2 north south pedestrain RED\n");
		printf("state 2 GATE OFF\n");

		pthread_mutex_lock(&mutex_traffic_state);
		traffic_encode_data = 0;
		check_server_msg(server_data);
		traffic_encode_data |= (EWY | NSPCR | TRAMG);

		traffic_state_change = 1;
		pthread_cond_signal(&cond_traffic_state);
		pthread_mutex_unlock(&mutex_traffic_state);

		break;

	case State3:

		printf("state 3 east west CAR light RED\n");
		printf("state 3 east west TRAM RED\n");
		// check sensor signal.
		pthread_mutex_lock(&data->mutex_sensor_signal);
		// check pedestrain signal
		if (data->north_south_pedestrian_signal == 1) {
			// local var up
			ped_light = 1;
			// reset signal
			data->north_south_pedestrian_signal = 0;
			// encode current state
			traffic_encode_data |= (EWR | NSPCG | TRAMR);
			printf("state 3 north south pedestrain GREEN\n");
		} else {
			printf("state 3 north south pedestrain RED\n");
			traffic_encode_data |= (EWR | NSPCR | TRAMR);
		}
		printf("state 3 GATE OFF\n");
		pthread_mutex_unlock(&data->mutex_sensor_signal);

		pthread_mutex_lock(&mutex_traffic_state);
		traffic_encode_data = 0;
		check_server_msg(server_data);
		traffic_state_change = 1;
		pthread_cond_signal(&cond_traffic_state);
		pthread_mutex_unlock(&mutex_traffic_state);

		break;

	case State4:

		printf("state 4 east west CAR light RED\n");
		printf("state 4 east west TRAM GREEN\n");
		printf("state 4 north south pedestrain GREEN\n");
		printf("state 4 DEPOT GATE OPEN\n");
		//*CurrentState = State5;

		pthread_mutex_lock(&mutex_traffic_state);
		traffic_encode_data = 0;
		check_server_msg(server_data);
		traffic_encode_data |= (EWR | NSPCG | TRAMG | DEPOT_GATE);
		traffic_state_change = 1;
		pthread_cond_signal(&cond_traffic_state);
		pthread_mutex_unlock(&mutex_traffic_state);

		break;
	}
}

void thread_create(void* (*thread_name)(void*), void* thread_data,
		pthread_t thread, pthread_attr_t thread_attr,
		struct sched_param thread_param, int sched_policy, int priority,
		int stacksize) {
	// Initialise thread attribute object to the default values (required)
	pthread_attr_init(&thread_attr);
	// Explicitly set the scheduling policy to round robin
	pthread_attr_setschedpolicy(&thread_attr, sched_policy);
	// Set thread priority (can be from 1..63 in QNX, default is 10)
	thread_param.sched_priority = priority;
	pthread_attr_setschedparam(&thread_attr, &thread_param);
	// Now set attribute to use the explicit scheduling settings
	pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
	// Increase the thread stacksize
	pthread_attr_setstacksize(&thread_attr, stacksize);

	pthread_create(&thread, &thread_attr, thread_name, thread_data);

}

//question?
void thread_join(pthread_t thread, void* retval) {
	pthread_join(thread, &retval);
}

void traffic_intersection_I3_process(void) {
	rcvid = MsgReceive(channel_id, &traffic_pulse, sizeof(traffic_pulse), NULL);

	if (rcvid == 0) {   //if pulse received
		// printf("received pulse\n");
		if (traffic_pulse.code == MY_PULSE_CODE) // we got a pulse
		{
			pulse_cnt++;
			// set shared variables with mutex lock
			pthread_mutex_lock(&sensor_data.mutex_sensor_signal);
			train_1_in_buff = sensor_data.train_1_coming;
			train_2_in_buff = sensor_data.train_2_coming;
			train_1_out_buff = sensor_data.train_1_leaving;
			train_2_out_buff = sensor_data.train_2_leaving;
			tram_depot_gate_buff = sensor_data.tram_depot_gate_signal;
			pthread_mutex_unlock(&sensor_data.mutex_sensor_signal);
			// train coming
			if (train_1_in_buff || train_2_in_buff) {
				//printf("I3: received train signal.\n");
				if (pulse_cnt >= train_cross_time_cnt) {
					// stay state 0 or state 3
					if (CurrentState != State0 && CurrentState != State3) {
						traffic_state_machine(&CurrentState, &sensor_data,
								&server_traffic_data);
						traffic_state_move(&server_traffic_data);
					}
					// else, stay in current state but keep informing.
					else {
						traffic_state_machine(&CurrentState, &sensor_data,
								&server_traffic_data);
					}
					pulse_cnt = 0;

				}
				// train 1 crossed clear all the signal
				if (train_1_in_buff && train_1_out_buff) {
					train_1_in_buff = 0;
					train_1_out_buff = 0;
					pthread_mutex_lock(&sensor_data.mutex_sensor_signal);
					sensor_data.train_1_coming = 0;
					sensor_data.train_1_leaving = 0;
					pthread_mutex_unlock(&sensor_data.mutex_sensor_signal);
					printf("I3: train left");
				}
				// train 2 crossed clear all the signal
				if (train_2_in_buff && train_2_out_buff) {
					train_2_in_buff = 0;
					train_2_out_buff = 0;
					pthread_mutex_lock(&sensor_data.mutex_sensor_signal);
					sensor_data.train_2_coming = 0;
					sensor_data.train_2_leaving = 0;
					pthread_mutex_unlock(&sensor_data.mutex_sensor_signal);
					printf("I3: train left");
				}

			} else  // if no train coming, we check for traffic mode
			{
				pthread_mutex_lock(&server_traffic_data.mutex_mode);
				traffic_mode_buff = server_traffic_data.traffic_mode;
				pthread_mutex_unlock(&server_traffic_data.mutex_mode);

				pthread_mutex_lock(&server_traffic_data.mutex_time_cnt);
				state_change_cnt_buff = server_traffic_data.traffic_time_cnt_ms;
				pthread_mutex_unlock(&server_traffic_data.mutex_time_cnt);
				// fix_time_mode, always move to next state depending on the gap time.
				if (traffic_mode_buff == fix_time_mode) {
					advanced_mode_reset_cnt = 0;
					if (pulse_cnt >= state_change_cnt_buff) {
						traffic_state_machine(&CurrentState, &sensor_data,
								&server_traffic_data);
						traffic_state_move(&server_traffic_data);
						pulse_cnt = 0;
					}

				}
				// sensor_mode, stops at state 1, waiting for signal from EW Pedestrain.
				else if (traffic_mode_buff == sensor_mode) {
					pthread_mutex_lock(&sensor_data.mutex_sensor_signal);
					if (sensor_data.north_south_pedestrian_signal) {
						north_south_state_change_signal = 1;
					}
					pthread_mutex_unlock(&sensor_data.mutex_sensor_signal);
					advanced_mode_reset_cnt = 0;

					if (pulse_cnt >= state_change_cnt_buff) {
						pulse_cnt = 0;
						if (CurrentState == State1) // wait pedestrain button signal at state1
								{
							// if there is a signal
							if (north_south_state_change_signal) {
								traffic_state_machine(&CurrentState,
										&sensor_data, &server_traffic_data);
								traffic_state_move(&server_traffic_data);
								north_south_state_change_signal = 0;
								pthread_mutex_lock(
										&sensor_data.mutex_sensor_signal);
								sensor_data.north_south_pedestrian_signal = 0;
								pthread_mutex_unlock(
										&sensor_data.mutex_sensor_signal);
							}
							// if no, stay in current state
							else {
								traffic_state_machine(&CurrentState,
										&sensor_data, &server_traffic_data);
							}

						}
						// move state until meet state 1
						else {
							traffic_state_machine(&CurrentState, &sensor_data,
									&server_traffic_data);
							traffic_state_move(&server_traffic_data);
						}
					}
				}
				// Adv mode, R3 clear
				else if (traffic_mode_buff == advanced_mode_1) {
					if (pulse_cnt >= state_change_cnt_buff) {
						pulse_cnt = 0;
						advanced_mode_reset_cnt++;
						if (CurrentState != State3) {
							traffic_state_machine(&CurrentState, &sensor_data,
									&server_traffic_data);
							traffic_state_move(&server_traffic_data);
						} else {
							traffic_state_machine(&CurrentState, &sensor_data,
									&server_traffic_data);
						}
					}
					if (advanced_mode_reset_cnt >= advanced_mode_reset_timer) {
						traffic_mode_change(&server_traffic_data,
						fix_time_mode);

					}

				}  // // Adv mode, R2 and R1 clear, not supported by I3 site
				else if (traffic_mode_buff == advanced_mode_2) {
					traffic_mode_change(&server_traffic_data, fix_time_mode);
				} else if (traffic_mode_buff == sp_mode) {
					load_sequence(&server_traffic_data);
					if (pulse_cnt >= state_change_cnt_buff) {
						advanced_mode_reset_cnt++;
						pulse_cnt = 0;
						traffic_state_machine(&CurrentState, &sensor_data,
								&server_traffic_data);
						traffic_state_move(&server_traffic_data);
					}
					if (advanced_mode_reset_cnt >= advanced_mode_reset_timer) {
						traffic_mode_change(&server_traffic_data,
						fix_time_mode);
					}
				}
			} // advanced_mode_2

		} // no train coming
	} // my pulse code
} // received timer pulse

void load_sequence(traffic_data *server_data){
	char seq[5];
	char empty[5]={'0','0','0','0','0'};
	pthread_mutex_lock(&server_data->mutex_seq);
	char temp[5];
	strncpy(&temp, server_data->sp_sequence,5);
	if(strcmp(&temp,&empty)==0)
		;//do nothing, currently don't accept 00000
	else
		strncpy(&seq, &temp,5);
	pthread_mutex_unlock(&server_data->mutex_seq);

	for (int i =0; i <sizeof(seq)/sizeof(*seq); i ++){
		execute_seq[i] =(int)seq[i]- 48; //very very bad way of doing this but just do it for now.
	}
}

