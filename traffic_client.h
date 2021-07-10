/*
 * traffic_client.h
 *
 *  Created on: 23Sep.,2020
 *      Author: yong
 *
 *  Modified by Kaile Wang
 *  03/10/2020
 */

#ifndef TRAFFIC_CLIENT_H_
#define TRAFFIC_CLIENT_H_

#include "traffic_server.h"

#define server_connect_point "/net/QNX_Central/dev/name/local/server_msg"

#define NSR 1         //north south red light
#define NSG 1<<1       // .......green light
#define NSY 1<<2        // ...... yellow light
#define NSTR 1<<3        // north south turn RED light
#define NSTG 1<<4        //..................Green light
#define NSTY 1<<5        //..................Yellow light
#define NSPCR 1<<6       // north south pedestrian crossing Red light
#define NSPCG 1<<7       // ...............................Green light
#define NSPCY 1<<8       //................................Yellow light
#define EWR 1<<9         // east west red light
#define EWG 1<<10        //...........green light
#define EWY 1<<11        //...........yellow light
#define EWTR 1<<12       //east west turn Red light
#define EWTG 1<<13       //...............Green light
#define EWTY 1<<14       //...............Yellow light
#define EWPCR 1<<15      //east west pedestrian crossing Red light
#define EWPCG 1<<16      //.............................Green light
#define EWPCY 1<<17      //.............................Yellow light
#define T_mode 1<<18
#define S_mode 1<<19
#define A1_mode 1<<20
#define A2_mode 1<<21

// I3 ONLY BITS
#define DEPOT_GATE 1<<22
#define TRAMG 1<<23 //Tram green light
#define TRAMR 1<<24 //Tram red light

//update: special sequence bits
#define SP_MODE 1<<25

extern volatile uint8_t traffic_state_change;
extern volatile uint32_t traffic_encode_data; // 0-2 bit: NSR,NSG,NSY, 3-5bit :NSTR,NSTG,NSTY, 6-8bit: NSPCR,NSPCG,NSPCY, 9-11bit: EWR,EWG,EWY, 12-14bit:EWTR,EWTG,EWTY, 15-17bit:EWPCR,EWPCG,EWPCY

extern pthread_mutex_t mutex_traffic_state;
extern pthread_cond_t cond_traffic_state;


void *traffic_client(void *data);

#endif /* TRAFFIC_CLIENT_H_ */
