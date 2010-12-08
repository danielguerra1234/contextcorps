/*
 ============================================================================
 Name        : GeckOS2.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : R5 Module, Turbo C-style
 ============================================================================
 */
        
#include "r5.h"

dcb* dcb1;
void interrupt (*old_int) ();

int com_open(int *eflag_p, int baud_rate) {
	int baud_rate1;
	int mk; 
	dcb1 = (dcb*)sys_alloc_mem(sizeof(dcb)); 

	//check the parameters
	if (eflag_p != NULL) {
		return ERR_INV_FLAG_PTR;
	}if (baud_rate < 0) { 
		return ERR_INV_BAUD_RATE;
	}if (dcb1->flag != DCB_CLOSED) {
		return ERR_PORT_OPEN;
	}else{
		dcb1->flag = DCB_OPEN;
		dcb1->event_flag = eflag_p;
			
		dcb1->status = DCB_IDLE;
		dcb1->ring_buffer_in = 0;
		dcb1->ring_buffer_out = 0;
		dcb1->ring_buffer_count = 0;
	
		old_int = getvect(COM1_INT_ID); 
		setvect(COM1_INT_ID, &int_handler);
				
		baud_rate1 = 115200 / (long) baud_rate;
		// Store Line Control Register
		outportb(COM1_LC, 0x80); 
		outportb(COM1_BRD_LSB, baud_rate1 & 0xFF);
		outportb(COM1_BRD_MSB,(baud_rate1 >> 8) & 0xFF);
		outportb(COM1_LC, 0x03); 
		//enable the level 4 for COM1 i PIC mask
       	disable(); 
       	mk = inportb(PIC_MASK);
		mk = mk & ~0x10;  
		outportb(PIC_MASK,mk);
		enable();
		//store 0x01 in the interrupt enable register
		outportb(COM1_MC, 0x08);
		outportb(COM1_INT_EN, 0x01); 
	} 
	return 0;
}

int com_close(){
	int mask;

	if(dcb1->flag != DCB_OPEN){
		return ERR_PORT_OPEN ;
	}
	/*
	if (dcb1-> flag =0)  //check if 0 means the port is closed, else make it 1
		return -201; 
		*/
	else{
		//close the DCB
		dcb1->flag = DCB_CLOSED;
		disable(); 
		mask = inportb(PIC_MASK); 
		mask = mask|0x10; 
		outportb(PIC_MASK, mask); 
		enable(); 
		outportb(COM1_MS,0x00); 
		outportb(COM1_INT_EN, 0x00);
		setvect(COM1_INT_ID,old_int); 
		sys_free_mem(dcb1);
	}
	return 0;
}

int com_write(char *buf_p, int *count_p){
	int mask;
	//check the parameters
	if(dcb1->flag != DCB_OPEN)
		return ERR_WRITE_PORT_NOT_OPEN;
	if(dcb1->status != DCB_IDLE)
		return ERR_WRITE_BUSY;
	if(buf_p == NULL)
		return ERR_WRITE_INV_BUFF_ADDR;
	if(count_p ==NULL)
		return ERR_WRITE_INV_COUNT;
	//install output buffer in DCB
	dcb1->out_buff = buf_p;
	dcb1->out_count = count_p;
	dcb1->out_done =0; 
	dcb1->status = DCB_WRITING; 
	*dcb1->event_flag = EVENT_FLAG_NOT_DONE;
	outportb(COM1_BASE, *dcb1->out_buff); 
	dcb1->out_buff++; 
	dcb1->out_done++;
	//eable the write interrupts
	disable();
	mask = inportb(COM1_INT_EN); 
	mask = mask|0x02;       
	outportb(COM1_INT_EN,mask);
	enable();
	return 0;

}



int com_read(char *buf_p, int *count_p){
	int mask;
	//check parameters
	if(dcb1->flag == DCB_CLOSED)
		return ERR_READ_PORT_NOT_OPEN;
	if(dcb1->status != DCB_IDLE)
		return ERR_READ_BUSY;
	if(buf_p == NULL)
		return ERR_READ_INV_BUFF_ADDR;
	if(count_p ==NULL)
		return ERR_READ_INV_COUNT;
	//initialize the input buffer
	dcb1->in_buff = buf_p;
	dcb1->in_count = count_p;
	dcb1->in_done = 0;
	*dcb1->event_flag = EVENT_FLAG_NOT_DONE;
	
	disable();
	dcb1->status= DCB_READING;
	//check for characters not being read
	
	while((dcb1->ring_buffer_count > 0) && (dcb1->in_done < *dcb1->in_count) && (dcb1->ring_buffer[dcb1->ring_buffer_in]) != '\r'){
		dcb1->in_buff[dcb1->in_done] = dcb1->ring_buffer[dcb1->ring_buffer_in];
		dcb1->in_done++;
		dcb1->ring_buffer_in++;
		dcb1->ring_buffer_count--;                
	} 

	enable();
	if(dcb1->in_done < *dcb1->in_count)
		return 0;

	*dcb1->event_flag = EVENT_FLAG_DONE;

	dcb1->in_buff[dcb1->in_done] = '\0';
 	dcb1->status = DCB_IDLE;	
 	
 	/*
 		strcat(ring_buffer, '\0');
 	dcb1->status =DCB_IDLE;
 	*/
	*dcb1->in_count = dcb1->in_done;
	return dcb1->in_done; 
}

void interrupt int_handler() {
	int cause;
	if (dcb1->flag == DCB_CLOSED) {
		outportb(PIC_CMD, EOI);
	} else {
		cause = inportb(COM1_INT_ID_REG);
		cause = cause & 0x07;
		if(cause == 0x02)
			write_int();
		else if(cause == 0x04)
			read_int();
  	outportb(PIC_CMD,EOI);
  }
}


void write_int() {
	int mask;
	if (dcb1->status != DCB_WRITING) {
		return;
	} else if (dcb1->out_done < *dcb1->out_count){
		outportb(COM1_BASE, *dcb1->out_buff);
		dcb1->out_done++;
		dcb1->out_buff++;
	} else {
		dcb1->status = DCB_IDLE;
		*dcb1->event_flag = EVENT_FLAG_DONE;
				
		disable();
		mask = inportb(COM1_INT_EN);
		mask = mask&~0x02;
		outportb(COM1_INT_EN, mask);
		enable();
	}
}

void  read_int() {
	char input_register;
	input_register = inportb(COM1_BASE);

	if (dcb1->status != DCB_READING) {
		if(dcb1->ring_buffer_count < RING_BUFF_SIZE){
			dcb1->ring_buffer[dcb1->ring_buffer_in] = input_register;
			dcb1->ring_buffer_in++;
			dcb1->ring_buffer_count++;
	   	return;
		}
	}

	if ( dcb1->in_done < *dcb1->in_count && input_register != '\r' ) {
		dcb1->in_buff[dcb1->in_done] = input_register;
		dcb1->in_done++;
		return;
	}

	*dcb1->in_count = dcb1->in_done;
	dcb1->in_buff[dcb1->in_done] = '\0';
	dcb1->status = DCB_IDLE;
	*dcb1->event_flag = EVENT_FLAG_DONE;

}

