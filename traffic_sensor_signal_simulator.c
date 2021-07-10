/*
 * traffic_sensor_signal_simulator.c
 *
 *  Created on: 23Sep.,2020
 *      Author: yong
 */

#include "traffic_sensor_signal_simulator.h"

void *sensor_signal_simulator(void *data) {
	traffic_sensor_data *SD = (traffic_sensor_data*) data;

	while (1) {
		printf("please input sensor signal "
				"'a'for tram depot site T1 signal, "
				"'b'for east west pedestrian,"
				"'e'for train 1 come, "
				"'f'for train 1 leave, "
				"'g'for train 2 come, "
				"'h'for train 2 leaving \n");
		char c_signal = 0;
		scanf(" %c", &c_signal);

		if (c_signal == 'a') {
			pthread_mutex_lock(&SD->mutex_sensor_signal);
			if (SD->tram_depot_gate_signal) {
				SD->tram_depot_gate_signal = 0;
				printf("sensor_signal_simulator: DEPOT GATE OFF");
			} else {
				SD->tram_depot_gate_signal = 1;
				printf("sensor_signal_simulator: DEPOT GATE ON");
			}
			pthread_mutex_unlock(&SD->mutex_sensor_signal);
		} else if (c_signal == 'b') {
			pthread_mutex_lock(&SD->mutex_sensor_signal);
			SD->north_south_pedestrian_signal = 1;
			pthread_mutex_unlock(&SD->mutex_sensor_signal);
		} else if (c_signal == 'e') {
			pthread_mutex_lock(&SD->mutex_sensor_signal);
			SD->train_1_coming = 1;
			pthread_mutex_unlock(&SD->mutex_sensor_signal);
		} else if (c_signal == 'f') {
			pthread_mutex_lock(&SD->mutex_sensor_signal);
			if (SD->train_1_coming)
				SD->train_1_leaving = 1;
			pthread_mutex_unlock(&SD->mutex_sensor_signal);
		} else if (c_signal == 'g') {
			pthread_mutex_lock(&SD->mutex_sensor_signal);
			SD->train_2_coming = 1;
			pthread_mutex_unlock(&SD->mutex_sensor_signal);
		} else if (c_signal == 'h') {
			pthread_mutex_lock(&SD->mutex_sensor_signal);
			if (SD->train_2_coming)
				SD->train_2_leaving = 1;
			pthread_mutex_unlock(&SD->mutex_sensor_signal);
		}
	}

	return EXIT_SUCCESS;
}
