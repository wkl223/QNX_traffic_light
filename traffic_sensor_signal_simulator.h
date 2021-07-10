/*
 * traffic_sensor_signal_simulator.h
 *
 *  Created on: 23Sep.,2020
 *      Author: yong
 *
 *  Modified by Kaile Wang
 *  03/10/2020
 */

#ifndef TRAFFIC_SENSOR_SIGNAL_SIMULATOR_H_
#define TRAFFIC_SENSOR_SIGNAL_SIMULATOR_H_

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/syspage.h>
#include <pthread.h>
#include <process.h>
#include <stdint.h>


typedef struct
 {
  volatile uint8_t north_south_pedestrian_signal;
  //volatile uint8_t east_west_pedestrian_signal;
  volatile uint8_t tram_depot_gate_signal;
  //volatile uint8_t north_south_car_signal;
  //volatile uint8_t east_west_car_signal;
  volatile uint8_t train_1_coming;
  volatile uint8_t train_1_leaving;
  volatile uint8_t train_2_coming;
  volatile uint8_t train_2_leaving;

  pthread_mutex_t mutex_sensor_signal;

 }traffic_sensor_data;


 void *sensor_signal_simulator(void *data);


#endif /* TRAFFIC_SENSOR_SIGNAL_SIMULATOR_H_ */
