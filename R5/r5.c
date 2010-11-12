/* ========================================================================== */
/*                                                                            */
/*   Filename.c                                                               */
/*   (c) 2001 Author                                                          */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */

#include r5.h
#include dos.h


void main()
{
    init();
}

void init() {
    clear_screen();
    //fputs("Welcome to R5 Module.\n\n");
    return 0;
}



void clear_screen() {
    clrsrn();
    return 0;
}

int com_open(int eflag_p, int baud_rate) {
    dcb* newDcb;
    int error;
    
    
    if (eflag == NULL) {
        errorTrans(INV_EVT_FLAG); 
    }
    if (eflag != OPEN) {
        errorTrans(INV_PORT_CLOSED);
    }
    if (baud_rate < 0) {
        error = (INV_BAUD_RATE);
    }
    
    newDcb            = Allocate_DCB();
    newDcb->flag      = OPEN;
    newDcb->evt_flag  = eflag_p;
    newDcb->status    = IDLE;
    
    //TODO: Add in ring buffer stuff
    
    
    setvect(COM_INT_ID, &sys_call);
    
    
}

dcb* Allocate_DCB() {
    int size = sizeof(dcb);
  	int *address;
  	dcb *dcb_ptr;
  	
  	dcb_ptr = (dcb*)sys_alloc_mem(size);//use sys_alloc_mem
  	return (dcb_ptr);
}
