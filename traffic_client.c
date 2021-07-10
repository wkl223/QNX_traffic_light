/*
 * traffic_client.c
 *
 *  Created on: 23Sep.,2020
 *      Author: yong
 */

#include "traffic_client.h"


volatile uint8_t traffic_state_change=0;
volatile uint32_t traffic_encode_data=0; // 0-2 bit: NSR,NSG,NSY, 3-5bit :NSTR,NSTG,NSTY, 6-8bit: NSPCR,NSPCG,NSPCY, 9-11bit: EWR,EWG,EWY, 12-14bit:EWTR,EWTG,EWTY, 15-17bit:EWPCR,EWPCG,EWPCY

pthread_mutex_t mutex_traffic_state = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_traffic_state = PTHREAD_COND_INITIALIZER;


void *traffic_client(void *data)
{
    my_int_data msg;

    my_reply reply; // replymsg structure for sending back to client

    msg.ClientID = 600;      // unique number for this client
    msg.hdr.type = 0x22;     // We would have pre-defined data to stuff here

   // msg.ClientID = 600; 	 // unique number for this client

    int server_coid;
   // int index = 0;

    printf("  ---> Trying to connect to server named: %s\n", server_connect_point);
  //  if ((server_coid = name_open(sname, 0)) == -1)
  //  {
  //      printf("\n    ERROR, could not connect to server!\n\n");
  //      return EXIT_FAILURE;
  //  }
    while ((server_coid = name_open(server_connect_point, 0)) == -1){

    	printf("ERROR, could not connect to server!\n");

    	sleep(1);
    }

    printf("Connection established to: %s\n", server_connect_point);

    // Do whatever work you wanted with server connection

    while(1)
    {

    	       pthread_mutex_lock(&mutex_traffic_state);

    			// test the condition and wait until it is true
    	        while(!traffic_state_change)
    			{
    	            pthread_cond_wait(&cond_traffic_state, &mutex_traffic_state);
    	        }
                msg.data = traffic_encode_data;
    	        printf("Client (ID:%d), sending encode traffic data: %d \n", msg.ClientID, msg.data);
    	           	        fflush(stdout);

    	        if (MsgSend(server_coid, &msg, sizeof(msg), &reply, sizeof(reply)) == -1)
    	          {
    	           	printf(" Error data '%d' NOT sent to server\n", msg.data);
    	           	       // maybe we did not get a reply from the server
    	           	if ((server_coid = name_open(server_connect_point, 0)) == -1)printf("ERROR, could not connect to server!\n");


    	           	    }
    	          else
    	         { // now process the reply
    	           	printf("   -->Reply is: '%s'\n", reply.buf);
    	           	 }

    	        traffic_state_change = 0;
    	        pthread_cond_signal(&cond_traffic_state);
    	        pthread_mutex_unlock(&mutex_traffic_state);


    }


    // Close the connection
    printf("\n Sending message to server to tell it to close the connection\n");
    name_close(server_coid);

    return EXIT_SUCCESS;
}



