/* This files contains one suggestion for a simple register-interface to the
   matrix multiply accelerator. The numbers are the offsets (or in one case, bit
   in the register) for the corresponding registers.

   YOU ARE WELCOME TO CHANGE THIS HOWEVER YOU LIKE!
*/

/** Size register: contains supported matrix size (width/height). read-only */
#define REG_SIZE 0x00

/** Control register: used to start the accelerator/wait for completion.
    read/write */
#define REG_CTRL 0x08
/** Run bit in the control register, set to start, then wait for device to clear
    this back to 0.*/
#define REG_CTRL_RUN 0x01

//通过寄存器小数据块写入
#define REG_OFF_IN 0x10
#define REG_OFF_OUT 0x18

//通过dma大数据块写入
#define REG_DMA_CTRL 0x30
	#define REG_DMA_CTRL_RUN 0x1
	#define REG_DMA_CTRL_W 0x2
	#define REG_DMA_CTRL_R 0x3

#define REG_DMA_LEN 0x38

#define REG_DMA_ADDR 0x40

#define REG_DMA_OFF 0x48
