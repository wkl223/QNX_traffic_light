#include <stdio.h>
#include <stdlib.h>
#include "traffic_light.h"

int main(void) {


  printf("Traffic light program starts\n");

  thread_create(&traffic_server,&server_traffic_data,traffic_server_th1,traffic_server_th1_attr,traffic_server_th1_param,SCHED_RR,10,8000);
  thread_create(&traffic_client,NULL,traffic_client_th2,traffic_client_th2_attr,traffic_client_th2_param,SCHED_RR,10,8000);
  thread_create(&sensor_signal_simulator,&sensor_data,traffic_sensor_th3,traffic_sensor_th3_attr,traffic_sensor_th3_param,SCHED_RR,10,8000);

  while (timer_connect(&channel_id,&event)!=1){
	  printf("fail to connect timer channel,please restart\n");
	  sleep(1);
  }

  printf("timer channel %d connected\n", channel_id);

  while (create_timer(CLOCK_REALTIME,&event,&traffic_timer_id)!=1){
	  printf("fail to create timer channel,please restart\n");
      sleep(1);
  }

  printf("timer created\n");


  timer_start(&itime,traffic_timer_id,0,0,100000000,0,100000000);



  while (1)
  	{
	  traffic_intersection_I3_process();
  	}

  thread_join(traffic_sensor_th3,traffic_sensor_retval_3);
  thread_join(traffic_client_th2,traffic_client_retval_2);
  thread_join(traffic_server_th1,traffic_server_retval_1);

	return EXIT_SUCCESS;
}
