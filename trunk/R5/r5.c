/* ========================================================================== */
/*                                                                            */
/*   Filename.c                                                               */
/*   (c) 2001 Author                                                          */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */

#include "r5.h"

dcb* dcb1;
void interrupt (*old_int) ();
void main()
{
    printf("");
}

int com_open(int *eflag_p, int baud_rate) {
	int brd;
	int mask;
	dcb1 = (dcb*)sys_alloc_mem(sizeof(dcb));
	if (eflag_p != NULL) {
		if (baud_rate >= 0) {
			if (dcb1->open == DCB_CLOSED) {
				dcb1->open = DCB_OPEN;
				dcb1->event_flag = eflag_p;
				dcb1->status = DCB_IDLE;
				dcb1->ring_buffer_in = 0;
				dcb1->ring_buffer_out = 0;
				dcb1->ring_buffer_count = 0;
				old_int = getvect(COM1_INT_ID); //save old interrupt handler
				setvect(COM1_INT_ID, &int_handler);//set new one
				outportb(COM1_LC, 0x80); // Store Line Control Register
				brd = 115200 / (long) baud_rate;//set baud rate
				outportb(COM1_BRD_LSB, brd & 0xFF);
				outportb(COM1_BRD_MSB,(brd >> 8) & 0xFF);
				outportb(COM1_LC, 0x03); // Store Line Control Register
				
        disable();
				mask = inportb(PIC_MASK);
				mask = mask & ~0x10;// level for com1 is 4 so bit 4 must be changed to 0
				outportb(PIC_MASK,mask);
				enable();

				outportb(COM1_MC, 0x08);
				outportb(COM1_INT_EN, 0x01); //stores 0x01 to the comm interrupt enable reg.  

			} else {
				return ERR_PORT_OPEN;
			}
		} else {
			return ERR_INV_BAUD_RATE;
		}
	} else {
		return ERR_INV_FLAG_PTR;
	}
	return 0;
}

int com_close(){
	int mask;
	if(dcb1->open == DCB_OPEN){
		dcb1->open = DCB_CLOSED;

		disable(); //disable interrupt
		mask = inportb(PIC_MASK); //get mask value.
		mask = mask|0x10; //check it
		outportb(PIC_MASK, mask); //send to comm port to shut down
		enable(); //enable interrupts

		outportb(COM1_MS,0x00); //send shut down command.
		outportb(COM1_INT_EN, 0x00);//send shut down command.
		setvect(COM1_INT_ID,old_int); //restore old interrupts.
		sys_free_mem(dcb1);
	}else{
		return ERR_PORT_OPEN ;
	}
	return 0;
}

int com_write(char *buf_p, int *count_p){
	int mask;
	if(dcb1->open != DCB_OPEN)
		return ERR_WRITE_PORT_NOT_OPEN;
	if(dcb1->status != DCB_IDLE)
		return ERR_WRITE_BUSY;
	if(buf_p == NULL)
		return ERR_WRITE_INV_BUFF_ADDR;
	if(count_p ==NULL)
		return ERR_WRITE_INV_COUNT;

	dcb1->out_buff = buf_p;
	dcb1->out_count = count_p;

	dcb1->out_done =0; //sets to zero since we haven't written anything.
	dcb1->status = DCB_WRITING; //status is writting.
	dcb1->event_flag = EVENT_FLAG_NOT_DONE; //event not done so flag = 0.
	outportb(COM1_BASE, *dcb1->out_buff); //send data to write the first character.

	dcb1->out_buff++; 
	dcb1->out_done++;
	
	disable();
	mask = inportb(COM1_INT_EN); //enable the write interrupts
	mask = mask|0x02;       
	outportb(COM1_INT_EN,mask);
	enable();
	return 0;

}



int com_read(char *buf_p, int *count_p){
	int mask;

	if(dcb1->open == DCB_CLOSED)
		return ERR_READ_PORT_NOT_OPEN;
	if(dcb1->status != DCB_IDLE)
		return ERR_READ_BUSY;
	if(buf_p == NULL)
		return ERR_READ_INV_BUFF_ADDR;
	if(count_p ==NULL)
		return ERR_READ_INV_COUNT;

	

	//copy the parameters to the DCB structure.

	dcb1->in_buff = buf_p;
	dcb1->in_count = count_p;
	dcb1->in_done = 0;

	*dcb1->event_flag = EVENT_FLAG_NOT_DONE;

	disable();
	dcb1->status= DCB_READING;
	while((dcb1->ring_buffer_count > 0) && (dcb1->in_done < *dcb1->in_count) && (dcb1->ring_buffer[dcb1->ring_buffer_in]) != '\r'){
		dcb1->in_buff[dcb1->in_done] = dcb1->ring_buffer[dcb1->ring_buffer_in];
		dcb1->in_done++;

		dcb1->ring_buffer_in++;
		dcb1->ring_buffer_count--;                
	} 
	enable();
	if(dcb1->in_done < *dcb1->in_count)
		return 0;

  dcb1->status = DCB_IDLE;		
	*dcb1->event_flag = EVENT_FLAG_DONE;
	dcb1->in_buff[dcb1->in_done] = '\0';
	*dcb1->in_count = dcb1->in_done;
	return dcb1->in_done;

}

void interrupt int_handler() {
	int int_cause;
	if (dcb1->open == DCB_CLOSED) {
		outportb(PIC_CMD, EOI);
	} else {
		int_cause = inportb(COM1_INT_ID_REG);
		int_cause = int_cause & 0x07;
		if(int_cause == 0x02)
			write_int();
		else if(int_cause == 0x04)
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

