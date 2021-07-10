/*
 * traffic_server.h
 *
 *  Created on: 22Sep.,2020
 *      Author: yong
 */

#ifndef TRAFFIC_SERVER_H_
#define TRAFFIC_SERVER_H_

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
#include <string.h>

#define msg_attach_point "traffic_msg"
#define BUF_SIZE 50

typedef struct
 {
  volatile uint8_t traffic_mode; // 0: time mode, 1: sensor mode, 2: advanced mode R3 clear all green, 3: advanced mode R2 and R1 clear all green. 4: sp mode.
  volatile uint8_t traffic_time_cnt_ms;

  char sp_sequence[5];
  pthread_mutex_t mutex_mode;  // declare traffic mode mutex in the global struct
  pthread_mutex_t mutex_time_cnt; //declare timer mutex in the global struct
  pthread_mutex_t mutex_seq;
 } traffic_data;


typedef union {	  			// This replaced the standard:  union sigval
	union{
		_Uint32t sival_int;
		void *sival_ptr;	// This has a different size in 32-bit and 64-bit systems
	};
	_Uint32t dummy[4]; 		// Hence, we need this dummy variable to create space
}_mysigval;

typedef struct _Mypulse {   // This replaced the standard:  typedef struct _pulse msg_header_t;
   _Uint16t type;
   _Uint16t subtype;
   _Int8t code;
   _Uint8t zero[3];         // Same padding that is used in standard _pulse struct
   _mysigval value;
   _Uint8t zero2[2];		// Extra padding to ensure alignment access.
   _Int32t scoid;
} msg_header_t;

typedef struct {
   msg_header_t hdr;  // Custom header
   int ClientID;      // our data (unique id from client)
   char data[BUF_SIZE];          // our data <-- This is what we are here for
} my_string_data;

typedef struct {
   msg_header_t hdr;  // Custom header
   int ClientID;      // our data (unique id from client)
   uint32_t data;          // our data <-- This is what we are here for
} my_int_data;

typedef struct {
   msg_header_t hdr;   // Custom header
   char buf[BUF_SIZE]; // Message to send back to send back to other thread
} my_reply;


void *traffic_server(void *Data);
uint32_t parse_timecommand(unsigned char* string);

#endif /* TRAFFIC_SERVER_H_ */
