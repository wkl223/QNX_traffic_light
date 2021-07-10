/*
 * traffic_server.c
 *
 *  Created on: 22Sep.,2020
 *      Author: yong
 */

#include "traffic_server.h"

void *traffic_server(void *data) {

	traffic_data *TD = (traffic_data*) data;

	name_attach_t *attach;

	my_string_data msg;
	my_reply replymsg; // replymsg structure for sending back to client

	replymsg.hdr.type = 0x01;    // some number to help client process reply msg
	replymsg.hdr.subtype = 0x00; // some number to help client process reply msg

	// Create a global name (/dev/name/local/...)
	while ((attach = name_attach(NULL, msg_attach_point, 0)) == NULL)
	// Create a global name (/dev/name/global/...)
	//if ((attach = name_attach(NULL, ATTACH_POINT, NAME_FLAG_ATTACH_GLOBAL)) == NULL)
	{
		printf("\nFailed to name_attach on ATTACH_POINT: %s \n",
				msg_attach_point);
		printf(
				"\n Possibly another server with the same name is already running or you need to start the gns service!\n");

	}

	printf("Traffic Server Listening for Command on ATTACH_POINT: %s \n",
			msg_attach_point);

	/*
	 *  Server Loop
	 */
	int rcvid = 0, msgnum = 0;  		// no message received yet
	int Stay_alive = 1, living = 0;	// server stays running (ignores _PULSE_CODE_DISCONNECT request)
	living = 1;
	while (living) {
		// Do your MsgReceive's here now with the chid
		rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);

		if (rcvid == -1)  // Error condition, exit
				{
			printf("\nFailed to MsgReceive\n");
			break;
		}

		// did we receive a Pulse or message?
		// for Pulses:
		if (rcvid == 0)  //  Pulse received, work out what type
				{
			printf("\nServer received a pulse from ClientID:%d ...\n",
					msg.ClientID);
			printf("Pulse received:%d \n", msg.hdr.code);

			switch (msg.hdr.code) {
			case _PULSE_CODE_DISCONNECT:
				printf("Pulse case:    %d \n", _PULSE_CODE_DISCONNECT);
				// A client disconnected all its connections by running
				// name_close() for each name_open()  or terminated
				if (Stay_alive == 0) {
					ConnectDetach(msg.hdr.scoid);
					printf("\nServer was told to Detach from ClientID:%d ...\n",
							msg.ClientID);
					living = 0; // kill while loop
					continue;
				} else {
					printf(
							"\nServer received Detach pulse from ClientID:%d but rejected it ...\n",
							msg.ClientID);
				}
				break;

			case _PULSE_CODE_UNBLOCK:
				// REPLY blocked client wants to unblock (was hit by a signal
				// or timed out).  It's up to you if you reply now or later.
				printf("\nServer got _PULSE_CODE_UNBLOCK after %d, msgnum\n",
						msgnum);
				break;

			case _PULSE_CODE_COIDDEATH:  // from the kernel
				printf("\nServer got _PULSE_CODE_COIDDEATH after %d, msgnum\n",
						msgnum);
				break;

			case _PULSE_CODE_THREADDEATH: // from the kernel
				printf(
						"\nServer got _PULSE_CODE_THREADDEATH after %d, msgnum\n",
						msgnum);
				break;

			default:
				// Some other pulse sent by one of your processes or the kernel
				printf("\nServer got some other pulse after %d, msgnum\n",
						msgnum);
				break;

			}
			continue; // go back to top of while loop
		}

		// for messages:
		if (rcvid > 0) // if true then A message was received
				{
			msgnum++;

			// If the Global Name Service (gns) is running, name_open() sends a connect message. The server must EOK it.
			if (msg.hdr.type == _IO_CONNECT) {
				MsgReply(rcvid, EOK, NULL, 0);
				printf(
						"\nClient messaged indicating that GNS service is running....");
				printf("\n    -----> replying with: EOK\n");
				msgnum--;
				continue;	// go back to top of while loop
			}

			// Some other I/O message was received; reject it
			if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX) {
				MsgError(rcvid, ENOSYS);
				printf("\n Server received and IO message and rejected it....");
				continue;	// go back to top of while loop
			}

			// A message (presumably ours) received

			if (strncmp(msg.data, "MODE:T", 6) == 0)  // fix time mode
					{
				pthread_mutex_lock(&TD->mutex_mode);
				TD->traffic_mode = 0;
				pthread_mutex_unlock(&TD->mutex_mode);

			} else if (strncmp(msg.data, "MODE:S", 6) == 0) // sensor mode
					{
				pthread_mutex_lock(&TD->mutex_mode);
				TD->traffic_mode = 1;
				pthread_mutex_unlock(&TD->mutex_mode);
			} else if (strncmp(msg.data, "MODE:A", 6) == 0) // advanced R3 clear mode
					{
				pthread_mutex_lock(&TD->mutex_mode);
				TD->traffic_mode = 2;
				pthread_mutex_unlock(&TD->mutex_mode);
			} else if (strncmp(msg.data, "MODE:B", 6) == 0) // advanced R2 R1 clear mode
					{
				pthread_mutex_lock(&TD->mutex_mode);
				TD->traffic_mode = 3;
				pthread_mutex_unlock(&TD->mutex_mode);
			} else if (strncmp(msg.data, "TIME:", 5) == 0) {
				uint32_t time_buf = parse_timecommand(msg.data);
				pthread_mutex_lock(&TD->mutex_time_cnt);
				TD->traffic_time_cnt_ms = time_buf;
				pthread_mutex_unlock(&TD->mutex_time_cnt);
			} else if (strncmp(msg.data, "SP", 2) == 0){
				pthread_mutex_lock(&TD->mutex_mode);
				TD->traffic_mode = 4;
				pthread_mutex_unlock(&TD->mutex_mode);
				pthread_mutex_lock(&TD->mutex_seq);
				strncpy(TD->sp_sequence,msg.data+2,5);
				pthread_mutex_unlock(&TD->mutex_seq);
			}
			// put your message handling code here and assemble a reply message
			sprintf(replymsg.buf, "processed OK");
			printf(
					"Server received data packet with value of '%s' from client (ID:%d), ",
					msg.data, msg.ClientID);
			fflush(stdout);

			// sleep(1); // Delay the reply by a second (just for demonstration purposes)

			printf("\n    -----> replying with: '%s'\n", replymsg.buf);
			MsgReply(rcvid, EOK, &replymsg, sizeof(replymsg));
		} else {
			printf(
					"\nERROR: Server received something, but could not handle it correctly\n");
		}

	}

	// Remove the attach point name from the file system (i.e. /dev/name/local/<myname>)
	name_detach(attach, 0);

	return EXIT_SUCCESS;
}


uint32_t parse_timecommand(unsigned char* string) {
	unsigned char *ret, *ret_1;
	unsigned char parse_data[5] = { 0 };

	ret = strstr(string, "TIME:");
	//Uart1_write_string(ret,strlen(ret));

	ret_1 = ret;
	while (*ret_1 < '0' || *ret_1 > '9')
		ret_1++;
	//Uart1_write_string(ret_1,strlen(ret_1));
	for (uint8_t i = 0; i < 4; i++) {
		if (*ret_1 >= '0' && *ret_1 <= '9')
			parse_data[i] = *ret_1++;
		else
			parse_data[i] = 0;
	}

	//Uart1_write_string(parse_data,strlen(parse_data));

	uint32_t val = atoi(parse_data);

	return val;

}

