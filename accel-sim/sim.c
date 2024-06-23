/*
 * Copyright 2023 Max Planck Institute for Software Systems, and
 * National University of Singapore
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <simbricks/pcie/if.h>

#include "sim.h"
#include "../common/common.h"
#include "../common/reg_defs.h"

// uncomment to enable debug prints
//#define DEBUG

// these are initialized from the main function based on command line parameters
uint64_t clock_period = 10000;
static uint64_t next_edge = 0;

uint64_t op_latency;
uint64_t matrix_size=100;
uint64_t device_status=0;
uint64_t task_status=0;
uint64_t dma_status=0;
uint64_t dma_len=0;
uint64_t dma_addr=0;
uint64_t dma_off=0;

uint64_t start_time;

char * buffer_in;
char * buffer_out;

int InitState(void) {
  // FILL ME IN
  	buffer_in = malloc(1024*1024);
  	buffer_out = malloc(1024*1024);
 	dprintf("~~~~~~~~~~ initstate \n");
 	return 0;
}

static void request_dma (bool iswrite,uint64_t request_id,uint64_t addr,uint64_t len,void * write_data){
    volatile union SimbricksProtoPcieD2H *msg = AllocPcieOut();
    if (iswrite) {
	//dprintf("write");
      volatile struct SimbricksProtoPcieD2HWrite *w = &msg->write;
      w->req_id = request_id;
      w->offset = addr;
      w->len = len;
      memcpy((void *) w->data, write_data, len);

      SendPcieOut(msg, SIMBRICKS_PROTO_PCIE_D2H_MSG_WRITE);
    } else {
	//dprintf("read");
      volatile struct SimbricksProtoPcieD2HRead *r = &msg->read;
      r->req_id = request_id;
      r->offset = addr;
      r->len = len;

      SendPcieOut(msg, SIMBRICKS_PROTO_PCIE_D2H_MSG_READ);
    }
}

void dma_trans_complete(volatile union SimbricksProtoPcieH2D *msg,uint8_t type){
	dprintf("DMA trans complete");
	if (type == SIMBRICKS_PROTO_PCIE_H2D_MSG_READCOMP) {
		memcpy((uint8_t *) buffer_in, (void *) msg->readcomp.data,dma_len);
		dprintf("read data:%s",buffer_in);
	} else if (type == SIMBRICKS_PROTO_PCIE_H2D_MSG_WRITECOMP) {

	} else {
		eprintf("DMAEvent: unexpected event (%u)", type);
	}
	dma_status=0;
}


void MMIORead(volatile struct SimbricksProtoPcieH2DRead *read)
{
  // prepare read completion
  volatile union SimbricksProtoPcieD2H *msg = AllocPcieOut();
  volatile struct SimbricksProtoPcieD2HReadcomp *rc = &msg->readcomp;
  rc->req_id = read->req_id; // set req id so host can match resp to a req
  // zero it out in case of bad register
  memset((void *) rc->data, 0, read->len);

  uint64_t val = 0;
  void *src = NULL;

  // YOU WILL NEED TO CHANGE THIS SUBSTANTIALLY, THIS IS JUST AN EXAMPLE
  if (read->offset < 128) {
    // design choice: All our actual registers need to be accessed with 64-bit
    // aligned reads
    assert(read->len <= 8);
	assert(read->offset % read->len == 0);

    switch (read->offset) {
      //case REG_SIZE: val = 88; break;
		case REG_SIZE:
			val = matrix_size;
			dprintf("~~~~~~~~~~ MMIO Read REG_SIZE:%ld", matrix_size);
			break;
		case REG_CTRL:
			val = device_status;
			dprintf("~~~~~~~~~~ MMIO Read REG_CTRL:%ld", device_status);
			break;
		case REG_CTRL_RUN:
			val = task_status;
			break;
		case REG_OFF_IN:
			val = buffer_in;
			break;
		case REG_OFF_OUT:
			val = buffer_out;
			break;
		case REG_DMA_CTRL:
			val = dma_status;
			break;
		case REG_DMA_LEN:
			val = dma_len;
			break;
		case REG_DMA_ADDR:
			val = dma_addr;
			break;
		case REG_DMA_OFF:
			val = dma_off;
			break;
		default:
			fprintf(stderr, "MMIO Read: warning invalid MMIO read 0x%lx\n",
          read->offset);
			break;
    }
    src = &val;
  } else {
    fprintf(stderr, "MMIO Read: warning invalid MMIO read 0x%lx\n",
          read->offset);
  }

  // copy data into response message
  if (src)
    memcpy((void *) rc->data, src, read->len);

  // send response
  SendPcieOut(msg, SIMBRICKS_PROTO_PCIE_D2H_MSG_READCOMP);
  //if(read->offset!=REG_DMA_CTRL){
#ifdef DEBUG_MMIO
  dprintf("~~~~~~~~~~ MMIO Read: BAR %d offset 0x%lx len %d %ld time:%ld own_type:%d val:%ld", read->bar,
    read->offset, read->len, read->offset % read->len, read->timestamp,read->own_type,val);
#endif
  //}
}

void MMIOWrite(volatile struct SimbricksProtoPcieH2DWrite *write)
{

#ifdef DEBUG_MMIO
  dprintf("~~~~~~~~~~ MMIO Write: BAR %d offset 0x%lx len %d time:%ld own_type:%d\n", write->bar,
    write->offset, write->len, write->timestamp,write->own_type);
#endif
  if (write->offset < 128) {
    assert(write->len <= 8);
    assert(write->offset % write->len == 0);
    uint64_t val = 0;
    memcpy(&val, (const void *) write->data, write->len);
    switch (write->offset) {
		case REG_CTRL:
			if(device_status!=0){
				printf("device is runing:%ld \n",device_status);
				return;
			}
			device_status=1;
			dprintf("~~~~~~~~~~ set device_status 0x%lx = 0x%lx\n",write->offset,val);
			dprintf("device_status:%ld \n",device_status);
			start_time = main_time;
			break;
		case REG_CTRL_RUN:
			//task_status=val;
			dprintf("~~~~~~~~~~ REG_CTRL_RUN only can read! 0x%lx = 0x%lx\n",write->offset,val);
			break;
		case REG_OFF_IN:
			printf("~~~~~~~~~~ REG_OFF_IN only can read! 0x%lx = 0x%lx\n",write->offset,val);
			break;
		case REG_OFF_OUT:
			printf("~~~~~~~~~~ REG_OFF_OUT only can read! 0x%lx = 0x%lx\n",write->offset,val);
			break;
		case REG_DMA_LEN:
			dma_len = val;
			break;
		case REG_DMA_ADDR:
			dma_addr = val;
			break;
		case REG_DMA_OFF:
			dma_off = val;
			break;
		case REG_DMA_CTRL:
			//copy dma
			dma_status=val;
			//void request_dma (bool iswrite,uint64_t request_id,uint64_t addr,uint64_t len,void * write_data);
			if(val & REG_DMA_CTRL_W){
				//memcpy(buffer_in,dma_addr,dma_len);
			//dprintf("request_dma read.");
			request_dma(false,0,dma_addr,dma_len,NULL);
				//dprintf("bufferin:%s",buffer_in);
			}else if(val & REG_DMA_CTRL_R){
			
			}
			//dma_status=0;
			break;
  } 
  }else {
    fprintf(stderr, "1MMIO Write: warning invalid MMIO write 0x%lx\n",
          write->offset);
  }

}


void PollEvent(void) {
if (main_time < next_edge)
    return;
next_edge += clock_period / 2;
}

uint64_t NextEvent(void) {
  return next_edge;
}

void DMACompleteEvent(uint64_t opaque) {
  dprintf("DMA Completed %ld\n",opaque);
}

