#ifndef R5
#define R5

#include <dos.h>
#include <stdio.h>
#include <stddef.h>
#include "mpx_supt.h"

//Sybolic Constants
#define COM1_INT_ID 0x0C
#define COM1_BASE 0x3F8 
#define COM1_INT_EN COM1_BASE+1
#define COM1_BRD_LSB COM1_BASE
#define COM1_BRD_MSB COM1_BASE+1
#define COM1_INT_ID_REG COM1_BASE+2
#define COM1_LC COM1_BASE+3
#define COM1_MC  COM1_BASE+4
#define COM1_LS COM1_BASE+5
#define COM1_MS COM1_BASE+6
#define PIC_MASK 0x21
#define PIC_CMD 0x20
#define EOI 0x20

//Error Codes
#define OPEN 900
#define CLOSED 901
#define ERR_INV_FLAG_PTR -134
#define ERR_INV_BAUD_RATE -135
#define ERR_PORT_OPEN -136
#define ERR_PORT_NOT_OPEN -201
#define ERR_READ_PORT_NOT_OPEN -301
#define ERR_READ_INV_BUFF_ADDR -302
#define ERR_READ_INV_COUNT -303
#define ERR_READ_BUSY -304
#define ERR_WRITE_PORT_NOT_OPEN -401
#define ERR_WRITE_INV_BUFF_ADDR -402
#define ERR_WRITE_INV_COUNT -403
#define ERR_WRITE_BUSY -404

//Symbolics Constants
#define RING_BUFF_SIZE 1024
#define DCB_READING 1
#define DCB_WRITING 2
#define DCB_IDLE 3
#define DCB_OPEN 1

#define DCB_CLOSED 0
#define EVENT_FLAG_DONE 1
#define EVENT_FLAG_NOT_DONE 0

typedef struct{
	int flag; 
	int *event_flag; 
	int status;  
	//input counters and buffers
	char *in_buff; 
	int *in_count; 
	int in_done;  
	//output counters and buffers
	char *out_buff;  
	int *out_count; 
	int out_done;   
	//ring buffer
	char ring_buffer[RING_BUFF_SIZE];  
	int ring_buffer_in;   
	int ring_buffer_out; 
	int ring_buffer_count;  

}dcb;

/*****************************
 *Prototypes
 */
 
int com_open(int *eflag_p, int baud_rate);
int com_close(void);
int com_read(char* buf_p, int* count_p);
int com_write(char* buf_p, int* count_p);

//DCB Prototypes
int Allocate_DCB();
int Setup_DCB();


//Interrupts
void interrupt int_handler();      //Level 1
void read_int();                  //Level 2
void write_int();                 //Level 2
 
//Main functions 
void init();
void clear_screen();

//Error
void errorTrans(int);

#endif
