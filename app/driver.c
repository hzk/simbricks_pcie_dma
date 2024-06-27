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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vfio-pci.h>

#include "../common/common.h"
#include "../common/reg_defs.h"
#include "driver.h"
#include "dma-alloc.h"

/** Use this macro to safely access a register at a specific offset */

static struct vfio_region_info *regs;
//static void *regs;
static struct vfio_dev dev;
static int bufferA;

static uint64_t data_size;
static uint64_t reg_in;
static uint64_t reg_out;

static size_t dma_mem_size = 16 * 1024 * 1024;
static uint8_t *dma_mem_r;
static uintptr_t dma_mem_phys_r;
static uint8_t *dma_mem_w;
static uintptr_t dma_mem_phys_w;

#define MAX_LINE_LENGTH 2048

int accelerator_init(void) {
  //struct vfio_dev dev;
  size_t reg_len;

  if (vfio_dev_open(&dev, "/dev/vfio/noiommu-0", "0000:00:00.0") != 0) {
    fprintf(stderr, "open device failed\n");
    return -1;
  }

  if(vfio_region_map(&dev, 0, &regs, &reg_len)) {
    fprintf(stderr, "mapping registers failed\n");
    return -1;
  }
  dprintf("~~~~~~~~~~vfio_region_map regs: %p reg_len:%ld reg->offset%lld\n", regs,reg_len,regs->offset);

  if (dma_alloc_init()) {
    fprintf(stderr, "DMA INIT failed\n");
    return -1;
  }

  if (!(dma_mem_w = dma_alloc_alloc(dma_mem_size, &dma_mem_phys_w))) {
    fprintf(stderr, "Allocating DMA memory failed\n");
    return -1;
  }

  dma_mem_r = (uint8_t *) dma_mem_w + (dma_mem_size / 2);
  dma_mem_phys_r = dma_mem_phys_w + (dma_mem_size / 2);

  if (vfio_busmaster_enable(&dev)) {
    fprintf(stderr, "Enabling busmastering failed\n");
    return -1;
  }

  data_size = ACCESS_REG(REG_SIZE);
  reg_in = ACCESS_REG(REG_OFF_IN);
  reg_out = ACCESS_REG(REG_OFF_OUT);
  dprintf("data_size:%ld",data_size);

  return 0;
}

void test_read(void) {
	dprintf("size:%ld",ACCESS_REG(REG_SIZE));
}

static inline void dma_write(const uint8_t *data_src,size_t len) {
  //dprintf("dma_wirte start %p %s %ld",dma_mem_w,data_src,len);
  memcpy(dma_mem_w,data_src,len);
  ACCESS_REG(REG_DMA_LEN) = len;
  ACCESS_REG(REG_DMA_ADDR) = dma_mem_phys_w;

  ACCESS_REG(REG_DMA_CTRL) = REG_DMA_CTRL_RUN | REG_DMA_CTRL_W;
  while ((ACCESS_REG(REG_DMA_CTRL) & REG_DMA_CTRL_RUN));
  //dprintf("dma_wirte end");
}

static inline void dma_read(uint8_t *data_dst,
                                 size_t out_rowlen) {
  dprintf("dma_read start");
  size_t len = ACCESS_REG(REG_DMA_LEN);
  ACCESS_REG(REG_DMA_ADDR) = dma_mem_phys_r;

  ACCESS_REG(REG_DMA_CTRL) = REG_DMA_CTRL_RUN | REG_DMA_CTRL_R;
  while ((ACCESS_REG(REG_DMA_CTRL) & REG_DMA_CTRL_RUN));

  memcpy(data_dst,dma_mem_r,len);
  dprintf("dma_read end");
}

int is_empty_or_comment_line(const char *line) {
    // 遍历字符串，检查每个字符是否为空白字符
    for (int i = 0; line[i] != '\0'; i++) {
        if (!isspace((unsigned char)line[i])) {
			if(line[i]=='#'){
				return 1;
			}
            return 0; // 发现非空白字符
        }
    }
    return 1; // 全部都是空白字符
}

int accelerator_run(size_t loop){

    FILE *file;
    char line[MAX_LINE_LENGTH];
	dprintf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~read eqasm start~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~:");
    // 打开文件
    file = fopen("echo.eqasm", "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // 读取并打印文件内容
    while (fgets(line, sizeof(line), file)) {
		size_t len = strlen(line);
		if (len > 0 && line[len-1] == '\n') {
			line[len-1] = '\0'; 
		}
		//if(!is_empty_or_comment_line(line)){
			//dprintf("new command:%s,len:%ld", line,strlen(line));
			//dma_write(line,sizeof(line));
			dma_write(line,len);
		//}
    }

    // 关闭文件
    fclose(file);
	dprintf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~read eqasm end~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~:");

	//char buf[]="123ABCDEFGH456 TEST HAHA 2024 06 20";
	//dma_write(buf,sizeof(buf));
	return 0;
}

