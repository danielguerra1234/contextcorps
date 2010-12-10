/*
 ============================================================================
 Name        : R6.c
 Author      : Jeremy Keczan
 Version     :
 Copyright   : Your copyright notice
 Description : R6, Turbo C-style
 ============================================================================
 */
                 
#include "GeckOS2.h"


//Global Variables:
date_rec *date_p;



//4 Queues
queue* readyQ;
queue* blockQ;
queue* suspendreadyQ;
queue* suspendblockQ; 


unsigned int sp_save=NULL, ss_save=NULL;
unsigned int ssnew=NULL, spnew=NULL;
unsigned int temp_ss = NULL, temp_sp = NULL;
unsigned char sysStack[2048];

pcb* cop;
params* param_p;
context* context_p;

IOCB* wait_term;
IOCB* wait_com;


IOD* temp_iod;
IOD* temp_head;
//dcb* com_port; 
//dcb* term_port;
//void interrupt (*old_int) ();

/******************************
 *Name: main
 *Parameters: int
 *Calls:    init
 *          init_iocb
 *          com_open
 *          trm_open
 *          init_R6
 *          dispatcher
 *          com_close
 *          trm_close
 *          io_cleanup
 *          q_cleanup x 4
 *          sys_exit     
 *Return:   nothing   
 *Desc:     This function calls sys_free_mem to release the memory for the ptr*
 *          along with the stack base       
 */ 

int main(void) {
	int e_flag;
	init();
	init_iocb();
	com_open(&wait_com->event_flag, 1200);   //
	trm_open(&wait_term->event_flag);         //
	init_R6();
	dispatcher();
	com_close();
	trm_close();
	io_cleanup();
	q_cleanup(readyQ) ;
	q_cleanup(blockQ);
	q_cleanup(suspendreadyQ);
	q_cleanup(suspendblockQ);
	//cleanup pcbs
	//cleanup iods and iocbs
	sys_exit();
	return 0;
}

/******************************
 *Name: io_scheduler
 *Parameters: none
 *Calls:    sys_alloc_mem
 *          FIFO_insert_iod 
 *Return:   nothing   
 *Desc:     This function the proper function based on device to schedule the 
 *          new iod request into the proper waiting queue.       
 */ 

void io_scheduler(){
	int dev;
	IOD* new_iod;
	IOCB* wait_q;
	pcb* new_pcb;
	dev= param_p->device_id;
	
	
	new_iod= (IOD*) sys_alloc_mem(sizeof(IOD));
	new_iod->requestor = cop;
	strcpy(new_iod->name, cop->process_name);
	new_iod->location= param_p->buf_addr; //check if this is we set addresses
	new_iod->counter= param_p->count_addr; //same here
	new_iod->type=param_p->op_code;
	new_pcb = new_iod->requestor;
	
	if(dev == TERMINAL)
		wait_q= wait_term;
	if(dev == COM_PORT)
		wait_q= wait_com;
	
	if(wait_q->count==0){
		wait_q->head = new_iod;
		wait_q->tail = new_iod;
		wait_q->count= 1;
		process_io(new_iod, dev);
	}else {
		FIFO_insert_iod(wait_q, new_iod);
	}//block the process
	new_pcb->state= BLOCKED;	 
	Insert_PCB(new_pcb);
	//show_blocked();
	return;
}

/******************************
 *Name: process_io
 *Parameters: IOD*, int
 *Calls:    trm_read
 *          com_read
 *          term_write
 *          com_write
 *          trm_clear
 *          trm_gotoxy     
 *Return:   nothing   
 *Desc:     This function process the io request based on the type of request: 
 *          READ, WRITE, CLEAR, GOTOXY by calling the proper driver routines 
 *          which depend on which device the request is for: COM or TERMINAL         
 */ 

void process_io(IOD* new_iod, int device){
	int request;
	request = new_iod->type;
	
	switch (request) {
			case READ:
				  if(device == TERMINAL)
					   trm_read(new_iod->location, new_iod->counter );
				  else if (device == COM_PORT)
					   com_read(new_iod->location, new_iod->counter);
			    break;
			case WRITE:
				  if(device == TERMINAL)
					   trm_write(new_iod->location, new_iod->counter);
				  else if (device == COM_PORT)
					   com_write(new_iod->location, new_iod->counter);
				  break;
			case CLEAR:
				  if(device == TERMINAL)
					   trm_clear();
				  break;
			case GOTOXY:
				  if(device == TERMINAL)
					   trm_gotoxy(0,0);
				  break;
			default:
				  break;
		}
		return;
}

/******************************
 *Name: init_iocb
 *Parameters: none
 *Calls:    sys_alloc_mem
 *Return:   nothing   
 *Desc:     This function initializes all iocbs to the proper default values.       
 */ 

void init_iocb(){

	wait_term= (IOCB*) sys_alloc_mem(sizeof(IOCB));
	wait_term->event_flag= 0;
	wait_term->count= 0;
	wait_term->head= (IOD*)sys_alloc_mem(sizeof(IOD));
	wait_term->tail= (IOD*)sys_alloc_mem(sizeof(IOD));
	
	wait_com= (IOCB*) sys_alloc_mem(sizeof(IOCB));
	wait_com->event_flag= 0;
	wait_com->count= 0;
	wait_com->head= (IOD*)sys_alloc_mem(sizeof(IOD));
	wait_com->tail= (IOD*)sys_alloc_mem(sizeof(IOD));
	return;
}

/******************************
 *Name: init_R6
 *Parameters: none
 *Calls:    Setup_PCB
 *          FP_SEG
 *          FP_OFF
 *          Insert_PCB
 *          Load_Program    
 *Return:   nothing   
 *Desc:     This function calls sys_free_mem to release the memory for the ptr*
 *          along with the stack base       
 */ 	

void init_R6() {

  pcb* comhan;
  context* comhan_con;

  // Set up COMHAN PROCESS
	comhan = Setup_PCB("COMHAN\0", 1, 1);
	comhan_con = (context *) comhan->stack_top;
	
	comhan_con->DS = _DS;
	comhan_con->ES = _ES;
	comhan_con->CS = FP_SEG(&com_han);
	comhan_con->IP = FP_OFF(&com_han);
	comhan_con->FLAGS = 0x200;
	
	Insert_PCB(comhan);
	
	Load_Program("IDLE", "IDLE", 123, "\0");
	resume("IDLE");
	
	show_all();
	
}

/******************************
 *Name: io_cleanup
 *Parameters: none
 *Calls:    sys_free_mem         
 *Return:   nothing   
 *Desc:     This function releases the memory for all io related structures 
 *          including all IODs and IOCBs       
 */ 

void io_cleanup(){
	IOD* temp= wait_com->head;
	IOD* temp2= wait_term->head;
	while(temp != NULL){
		sys_free_mem(temp);
		temp= temp->next;
	}
	while(temp2!= NULL){
		sys_free_mem(temp2);
		temp2= temp2->next;
	}
	sys_free_mem(wait_com);
	sys_free_mem(wait_term);
}

/******************************
 *Name: q_cleanup
 *Parameters: queue*
 *Calls:    sys_free_mem
 *Return:   nothing   
 *Desc:     This function calls sys_free_mem to release the memory for all pcbs
 *          and queue.        
 */ 

void q_cleanup(queue* q){
	pcb* temp;
	temp= q->head;
	while(temp!= NULL){
		sys_free_mem(temp);
		temp= temp->next;
	}
	sys_free_mem(q);
}



/******************************
 *Name: init
 *Parameters: none
 *Calls:    ClearScreen
 *          sys_int(Module_F)
 *          initQueue
 *          sys_set_vec   
 *Return:   nothing   
 *Desc:     This function initializes the necessary data structures and support
 *          software for the program to execute effectively       
 */ 
                                                                       
void init() {
  int error;
  char greeting[20] = "Welcome to GeckOS!\0";
	clearScreen();
	puts(greeting);
	sys_init(MODULE_F);
	readyQ = initQueue(readyQ, "Ready\0");
	blockQ = initQueue(blockQ, "Blocked\0");
	suspendreadyQ = initQueue(suspendreadyQ, "Suspended Ready\0");
	suspendblockQ = initQueue(suspendblockQ, "Suspended Blocked\0");
	error = sys_set_vec(sys_call); 
  
  if (error != 0) {
	   errorCodeTranslator(error);
	   return;
	} 
}

/***************************
 *Name: initQueue
 *Parameters: queue*, char*
 *Calls:      sys_alloc_mem
 *Returns:    pcb
 *Desc:       This function inits the queue that is passed in with the name also
 *            passed in.  
 */
 
queue* initQueue(queue* newQ, char* name) {
  
  newQ = (queue*)sys_alloc_mem(sizeof(queue));
  
  newQ->name = name;
	newQ->nodes = 0;
	newQ->head = NULL;
	newQ->tail = NULL;
	newQ->index = 0;
	
	return newQ;
}



long buffer_length = 100;
unsigned char buffer[SIZE];

//############PCB Functions####################
//PCB error codes start in the 400

/***************************
 *Name: Allocate_PCB
 *Parameters: none
 *Calls:      sizeof
 *            sys_alloc_mem
 *Returns:    pcb
 *Desc:       This function allocates the memory for a pcb by settings its 
 *            stack_base and stack_top  
 ***************************/ 

pcb *allocatePcb(){
	int size = sizeof(pcb);
	int *address;
	pcb *pcb_ptr;
	
	pcb_ptr = (pcb*)sys_alloc_mem(size);//use sys_alloc_mem

	pcb_ptr->stack_base = sys_alloc_mem(1024);
	pcb_ptr->stack_top = pcb_ptr->stack_base + 1024 - sizeof(context);
	return (pcb_ptr);
}

/******************************
 *Name: Free_PCB
 *Parameters: pcb
 *Calls:    3_mem
 *Return:   nothing   
 *Desc:     This function calls sys_free_mem to release the memory for the ptr*
 *          along with the stack base       
 */    

void Free_PCB(pcb *ptr) { 
  if (ptr == NULL){
      printf("PCB not found, can not be releaseed.\n");
      return;
  } else {
        if (ptr->load != NULL) {
          sys_free_mem(ptr->load);
          }
       sys_free_mem(ptr->stack_base);
       
       sys_free_mem(ptr);
  }
}



pcb* Setup_PCB(char name[], int priorityc, int classc) {
	pcb* pcb1;
	int class = classc;
	
	//printf("SetupPCB: Name: %s, Prior: %d, Class: %d\n\n", name, priorityc, classc);
	
	if (name == NULL) {
		errorCodeTranslator(ERR_PCB_NONAME);
		return NULL;
	}
	if (strlen(name) > 15) {
		errorCodeTranslator(ERR_SUP_NAMLNG);
		return NULL;
	}
	/*if (Find_PCB(name) != NULL) {
		errorCodeTranslator(ERR_PCB_NMEEXISTS);
		return;
	}*/  
	if(priorityc > 127 || priorityc < (-128)) {
		errorCodeTranslator(ERR_PCB_INVPRIORITY);
		return NULL;
	}
	if (class!= 1 && class!= 2) {
		errorCodeTranslator(ERR_PCB_INVCLASS);
		return NULL;
	}
	
	pcb1 = allocatePcb();
	strcpy(pcb1->process_name, name);
	pcb1->priority = priorityc;
	pcb1->process_class = classc;
	pcb1->state = READY;
	pcb1->next = NULL;
	pcb1->prev = NULL;
	return pcb1;
}

/*********************************************
 *Function: Load Program
 *Calls:    sys_alloc_mem
 *          allocatePCB
 *          Find_PCB
 *          sys_load_program
 *          sys_check_program
 *          Insert_PCB 
 *Returns:  PCB
 *Desc:     The function takes the parameters and created a pcb with a process
 *          attached by address. The context stack is kept so it may be restored 
 */
   
pcb* Load_Program(char name[], char prog_name[], int priorityc, char dir_name[]) {
	pcb* pcb1;
	int error;
	                          
	int prog_len_p;
	int size;
	unsigned char* load_address;
	unsigned char* exec_address;
	int start_offset_p;
	context* con;
	
	if (name == NULL) {
		errorCodeTranslator(ERR_PCB_NONAME);
		return NULL;
	}
	if (strlen(name) > 15) {
		errorCodeTranslator(ERR_SUP_NAMLNG);
		return NULL;
	}
	if (Find_PCB(name) != NULL) {
		errorCodeTranslator(ERR_PCB_NMEEXISTS);
		return NULL;
	}  
	if(priorityc > 127 || priorityc < (-128)) {
		errorCodeTranslator(ERR_PCB_INVPRIORITY);
		return NULL;
	}
	//returns size of memory needed to hold program
	error = sys_check_program(dir_name, prog_name, &prog_len_p, &start_offset_p);
	
   if (error != 0) {
      errorCodeTranslator(error);
      return NULL;
   }
	       
	pcb1 = allocatePcb();
	strcpy(pcb1->process_name, name);

	pcb1->priority = priorityc;
	if (strcmp(name, "IDLE") == 0 ) {
      pcb1->process_class = SYSTEM;
  } else {
	   pcb1->process_class = APPLICATION;
	}
	pcb1->state = SUSPENDED_READY;

  //sys_alloc_mem inits the memory for the program
	load_address = (unsigned char*) sys_alloc_mem(prog_len_p);
	
	
	error = sys_load_program (load_address, prog_len_p, "\0", prog_name);
	if (error != 0) {
      errorCodeTranslator(error);
      Free_PCB(pcb1);
      return NULL;
  }
	
	pcb1->load = load_address;
	pcb1->execution = pcb1->load + start_offset_p;
	pcb1->memory = prog_len_p;
	
	  

    con = (context*)pcb1->stack_top;
  	con->CS = FP_SEG(pcb1->execution);
  	con->IP = FP_OFF(pcb1->execution);
  	con->FLAGS = 0x200;
  	con->DS = _DS;
  	con->ES = _ES;
  	
	  Insert_PCB(pcb1); // should go to the suspended ready queue

	return pcb1;
}

/**************************
 *Name: Find_PCB_Ready
 *Parameters:     char*
 *calls:          none
 *returns:        pcb
 *
 *Desc:           Searchs Ready Q only
 *   
 */    

pcb* Find_PCB_Ready(char* name) {
   pcb* walk;
	 int check;
	 
   walk = readyQ->head; 
	 
	 if (walk == NULL) {
    		//printf("Queue not available or is empty.\n");
    		return NULL;
    }
     
	 while(walk != NULL) {
	     check = check + 1;
	  //printf("Find_PCB Function executing with name: %s, %s\n",name, walk->process_name); 
	     if (check == 30) {
          break;
        }
    
       if (strcmp(name, walk->process_name) == 0 ) 
          return walk;
      
	      walk = walk->next;
		 }
	return NULL;
}

/**************************
 *Name: Find_PCB_Blocked
 *Parameters:     name
 *calls:          none
 *returns:        pcb
 *
 *Desc:           Searchs Block Q only
 *   
 */ 

pcb* Find_PCB_Blocked(char* name) {
    pcb* walk;
    int check;
    walk = blockQ->head;
    check = 0;
    while(walk != NULL) {
      	  check = check + 1; 
      	  if (check == 25) {
      	    //printf("Infinite Looping Error. Aborted.\n");
            break;
          }
      		if (strcmp(walk->process_name,name) == 0) {
            return walk;
          }
            
          walk = walk->next;
    }
    walk = NULL;  
    return NULL;
}

/**************************
 *Name: Find_PCB_Suspended_Ready
 *Parameters:     name
 *calls:          none
 *returns:        pcb
 *
 *Desc:           Searchs Suspended Ready Q only
 *   
 */ 

pcb* Find_PCB_Suspended_Ready(char* name) {
    pcb* walk;
    int check;
    walk = suspendreadyQ->head;
      check = 0;
      while(walk != NULL) {
        	  check = check + 1;
        	  //printf("Find_PCB Function executing with name: %s, %s\n",name, walk->process_name); 
        	  if (check == 25) {
        	    //printf("Infinite Looping Error. Aborted.\n");
              break;
            }
        		if (strcmp(walk->process_name,name) == 0) {
              return walk;
            }
              
            walk = walk->next;
      }
    return NULL;
}

/**************************
 *Name: Find_PCB_Suspended_Blcoked
 *Parameters:     name
 *calls:          none
 *returns:        pcb
 *
 *Desc:           Searchs Suspended Blocked Q only
 *   
 */ 

pcb* Find_PCB_Suspended_Blocked(char* name) {
    pcb* walk;
    int check;
    walk = suspendblockQ->head;
      check = 0;
      while(walk != NULL) {
        	  check = check + 1;
        	  //printf("Find_PCB Function executing with name: %s, %s\n",name, walk->process_name); 
        	  if (check == 25) {
        	    //printf("Infinite Looping Error. Aborted.\n");
              break;
            }
        		if (strcmp(walk->process_name,name) == 0) {
              return walk;
            }
              
            walk = walk->next;
      }
      return NULL;
}

/**************************
 *Name: Find_PCB
 *Parameters: name
 *Calls:    Find_PCB_Ready
 *          Find_PCB_Blocked
 *          Find_PCB_Suspended_Ready
 *          Find_PCB_Suspended_Blocked  
 *Desc:     The function uses the 4 defined search functions already written 
 *          above to search through all queues  
 */

pcb* Find_PCB(char *name){
	pcb* ptr;
	
	ptr = Find_PCB_Ready(name);
	
	if (ptr == NULL) {
      ptr = Find_PCB_Blocked(name);
         
  }
  
  if (ptr == NULL) {
      ptr = Find_PCB_Suspended_Ready(name);
      
  }
  
  if (ptr == NULL) {
      ptr = Find_PCB_Suspended_Blocked(name);
      
  }
  
  if (ptr == NULL) {
      //printf("PCB %s not available.\n", name);
      return NULL;
  } else {
      return ptr;
  }  
	
}

/***************************
 *Name:         priority_insert
 *Parameters:   queue*, pcb*
 *Calls:        none
 *Returns:      Void
 *Desc:         The functions accepts which queue to insert into to and which 
 *              ptr to insert.  It checks for where to insert and keeps track by
 *              lowest first.   
 */   

void priority_insert(queue* q, pcb *ptr){
  pcb* walk; 
  pcb* prev;
  pcb* next;
  
  int c = 0;
  int priority;
  int check= 0;
  
  priority = ptr->priority;
  walk = q->head;
    
  if (walk == NULL) {           //Queue is empty, insert at head
    //printf("Walk is Null.\n");
    ptr->next = NULL;
    ptr->prev = NULL;
    q->head = ptr;
    //(q->index)++;
    return;
  }
   
  while (walk != NULL) {   //not empty, start searching
    next = walk->next;
    prev = walk->prev;
    
   // printf("Priority: %i\tWalk: %i\n",priority, walk->priority);
    if (check == 25){
      //printf("Broke by check");
      break;
    }
    
    if (priority <= walk->priority) {
     // printf("p < w\n");
      
        if (walk->prev == NULL && c == 0){  ////insert at head, move head to next
              //printf("Walk == NULL test.\n");
	           prev       = ptr;
	           ptr->next        = walk;
             ptr->prev        = NULL;
             q->head          = ptr;
             return; 
        } else {      
      	    ptr->next = walk;
            ptr->prev = prev;
      
      	    prev->next = ptr;
      	    walk->prev = ptr;
            return;
        }  
    } else if (priority > walk->priority) {    //check priority is greater than walk
           if (next == NULL) {           //insert at tail
      	      walk->next    = ptr;
      	      ptr->prev     = walk;
              ptr->next     = NULL;  
              return;
           }
           if (walk->next != NULL) {
	             walk = walk->next;
           }
    }    
  }
}

/***************************
 *Name: FIFO_insert
 *Parameters: queue*, iod*
 *Calls:      none
 *Returns:    void
 *Desc:       The functions simply inserts the iod at the end of the queue 
 */

void FIFO_insert_iod(IOCB* q, IOD* ptr){
      IOD* current;
      if (q->head == NULL) {                              //No IODs in queue
          //printf("Queue head in Fifo check.\n\n");
          ptr->next   = NULL;              
          q->head     = ptr;
          q->tail     = ptr;
          q->count = q->count+1;
      } else {
          current = q->tail;
          current->next = ptr;
          q->tail = ptr;
          q->count = q->count + 1;      
    }
}

/***************************
 *Name: FIFO_insert
 *Parameters: queue*, ptr*
 *Calls:      none
 *Returns:    void
 *Desc:       The functions simply inserts the pcb at the end of the queue 
 */

void FIFO_insert(queue* q, pcb *ptr){
      if (q->head == NULL) {                              //No PCBs in queue
          //printf("Queue head in Fifo check.\n\n");
          ptr->next   = NULL;
          ptr->prev   = NULL;
          
          q->head     = ptr;
          q->tail     = ptr;
      } else if ((q->head)->next == NULL) {               //first pcb in queue
          //printf("Queue head is 1 in Fifo check.\n\n");
          ptr->next        = NULL;
          ptr->prev        = q->head;
          
          (q->head)->next  = ptr;
          q->tail          = ptr;
      }else {                                             //more than 1 pcb
          //printf("Queue head many in Fifo check.\n\n");
          (q->tail)->next = ptr;
          ptr->prev       = q->tail;
          q->tail         = ptr;
    }
}

/***************************
 *Name: Insert_PCB
 *Parameters: pcb*
 *Calls:      FIFO_insert
 *            priority_insert 
 *Returns:    pcb
 *Desc:       Is simply a delegator of which insert algorithm to call based on
 *            the state  
 */

void Insert_PCB(pcb* pcb1){
  int state;
  state = pcb1->state;
  
  if (state == READY) {             // READY
    priority_insert(readyQ, pcb1);
    return;
  }  
  
  if (state == BLOCKED) {           // BLOCKED
    FIFO_insert(blockQ, pcb1);
    return;
  }  
  
  if(state == SUSPENDED_READY) {    // SUSPENDED_READY
    FIFO_insert(suspendreadyQ, pcb1);
    return;
  }  
  
  if(state == SUSPENDED_BLOCKED) {  // SUSPENDED_BLOCKED
    FIFO_insert(suspendblockQ, pcb1);
    return;
  }
}

/***************************
 *Name: Remove_PCB
 *Parameters: pcb*
 *Calls:      
 *Returns:    pcb
 *Desc:       This function removes the pcb from the queue it is currently 
 *            in. The queue is determined based on the state.  
 */

pcb* Remove_PCB(pcb *pcb1){
  queue* q;
  pcb* prev;
  pcb* next;
  int state;
  
  
  
   if (pcb1 == NULL) {
      return NULL;
   }
  
  state = pcb1->state;
 // printf("Pcb name: %s\n\n\0",pcb1->process_name);
  if (state == READY) {                    //choose q to remove from 
      q = readyQ;  
  } else if (state == BLOCKED) {
      q = blockQ;
  } else if (state == SUSPENDED_READY) {
      q = suspendreadyQ;
  } else if (state == SUSPENDED_BLOCKED) {
      q = suspendblockQ;
  } else {
      printf("Queue %s is not valid.\n",q->name);
  }
  
  next=	pcb1->next;   //pcb next
  prev= pcb1->prev;   //pcb prev both temp variables
  
  if (strcmp(q->head->process_name,pcb1->process_name) == 0) {//pcb1 is head of queue
      if (next == NULL){ //only one pcb is queue and it is the head
          q->head     = NULL;
          next        = NULL;
          
          pcb1->next  = NULL;
          pcb1->prev  = NULL;
          
          return pcb1;
      } else if (next != NULL) {    //at least two pcbs- set head = head's next
      	   q->head      = next;
  		     next->prev   = NULL;
  		     //next->next   = NULL;
  		     
  		     pcb1->next   = NULL;
  		     pcb1->prev   = NULL;
  		     return pcb1;
	      }
  //now, if the pcb is not the head    
  } else if (pcb1->next == NULL && pcb1->prev != NULL){ //pcb1 is the last element in the queue
          prev->next    = NULL;
          
          pcb1->next    = NULL;
          pcb1->prev    = NULL;
          
          return pcb1;  
          
  } else if (pcb1->next != NULL && pcb1->prev != NULL) { //pcb is in the middle of the queue
          prev->next  = next;
          next->prev  = prev;
          
          pcb1->next  = NULL;
          pcb1->prev  = NULL;  
          return pcb1;
      }    
  return NULL; 
}

/***************************
 *Name: Set_Priority
 *Parameters: char*, int
 *Calls:      Find_PCB
 *Returns:    void
 *Desc:       The function finds the pch using the Find function and changes
 *            the priority. 
 */

void Set_Priority(char* name, int p) {
  pcb* procB;
  procB = Find_PCB(name);
  procB->priority = p;
  return;  
}

/***************************
 *Name: Show_PCB
 *Parameters: char*
 *Calls:      Find_PCB
 *Returns:    void
 *Desc:       The function finds the pcb using the find function and prints the 
 *            details of the pcb  
 */

void Show_PCB(char* name) {
  
  pcb* pcbPtr;
  pcb* pcbPtr2;
  char* temp;
  char* temp2;
  if (FALSE) {
    printf("PCB: %s does not exist.\n\n",name);
    return;
  } else {
    pcbPtr2 = Find_PCB(name);
    if (pcbPtr2 == NULL) {
      //printf("PCB not found.\n");
      return;
    }
    if (pcbPtr2->state == RUNNING) {
      temp = "RUNNING";  
    } else if (pcbPtr2->state == READY) {
      temp = "READY";
    } else if (pcbPtr2->state == BLOCKED) {
      temp = "READY";
    } else if (pcbPtr2->state == SUSPENDED_READY) {
      temp = "READY";
    } else if (pcbPtr2->state == SUSPENDED_BLOCKED) {
      temp = "READY";
    } else {
      temp = "INVALID STATE";
    }
    
    if (pcbPtr2->process_class == APPLICATION) {
        temp2 = "APPLICATION"; 
    } else if (pcbPtr2->process_class == SYSTEM) {
        temp2 = "SYSTEM";    
    } else {
        printf("Something bad has happened. Advise quiting application with (exit). \n\n");
        return;
    }

    printf("Name: %s\n" 
            "Priority: %d\n" 
            "Process_Class: %s\n" 
            "State: %s\n"
            "Memory %i\n\n",pcbPtr2->process_name, pcbPtr2->priority, temp2, temp, pcbPtr2->memory);
    return;
  }
}

/***************************
 *Name: Show_ready
 *Parameters: none
 *Calls:      none
 *Returns:    void
 *Desc:       The function prints all the pcbs in the ready queue.  
 */

void show_ready(){
	 pcb* walk;
	 int check;
	 int i = 1;
   	 walk = readyQ->head; 
	 
	 if (walk == NULL) { 
    		printf("Queue not available or is empty.\n");
    		return;
    }
   printf("Ready Queue:\n\n"); 
	 while(walk != NULL) {
	     check = check + 1;
	  //printf("Find_PCB Function executing with name: %s, %s\n",name, walk->process_name); 
	     if (check == 30) {
          break;
        }
    
        printf("\t%d:\t%s\n",i++,walk->process_name); 
      
	walk = walk->next;
		 }
	i = 1;
	return;
}

/***************************
 *Name: show_suspended_ready
 *Parameters: none
 *Calls:      none
 *Returns:    void
 *Desc:       The function prints the name of all the pcbs in the suspended
 *            ready queue   
 */

void show_suspended_ready(){
	 pcb* walk;
	 int check;
	 int i = 1;
   	 walk = suspendreadyQ->head; 
	 
	 if (walk == NULL) {
    		printf("Queue not available or is empty.\n");
    		return;
    }
   printf("Ready Queue:\n\n"); 
	 while(walk != NULL) {
	     check = check + 1;
	  //printf("Find_PCB Function executing with name: %s, %s\n",name, walk->process_name); 
	     if (check == 30) {
          break;
        }
    
        printf("\t%d:\t%s\n",i++,walk->process_name); 
      
	walk = walk->next;
		 }
	i = 1;
	return;
}

/***************************
 *Name: show_blocked
 *Parameters: none
 *Calls:      none
 *Returns:    void
 *Desc:       The function prints the name of all the pcbs in the blocked queue   
 */

void show_blocked(){
	 pcb* walk;
	 int check;
	 int i = 1;
   	 walk = blockQ->head; 
	 
	 if (walk == NULL) {
    printf("Queue not available or is empty.\n");
    return;
    } 
    printf("Blocked Queue:\n\n");
	 while(walk != NULL) {
	     check = check + 1;
	  //printf("Find_PCB Function executing with name: %s, %s\n",name, walk->process_name); 
	     if (check == 30) {
	        printf("Check Break.\n");
          break;
        }
    
        printf("\t%d:\t%s\n",i++,walk->process_name); 
      
	walk = walk->next;
		 }
	i = 0;
	return;
}

void show_suspended_blocked(){
	 pcb* walk;
	 int check;
	 int i = 1;
   	 walk = suspendblockQ->head; 
	 
	 if (walk == NULL) {   
    printf("Queue not available or is empty.\n");
    return;
    } 
    printf("Suspended Blocked Queue:\n\n");
	 while(walk != NULL) {
	     check = check + 1;
	  //printf("Find_PCB Function executing with name: %s, %s\n",name, walk->process_name); 
	     if (check == 30) {
	        printf("Check Break.\n");
          break;
        }
    
        printf("\t%d:\t%s\n",i++,walk->process_name); 
      
	walk = walk->next;
		 }
	i = 0;
	return;
}

/***************************
 *Name: show_all
 *Parameters: none
 *Calls:      none
 *Returns:    void
 *Desc:       The function prints the name of all the pcbs in all the queues.
 */ 

void show_all(){
	 pcb* walk;
	 int check;
	 
   printf("All PCB's:\n\n");
   show_ready();
   show_blocked();
   show_suspended_ready();
   show_suspended_blocked();
   
	return;
}

/***************************
 *Name:       block
 *Parameters: pcb_name
 *Calls:      Find_PCB
 *            Remove_PCB
 *            Insert_PCB  
 *Returns:    void
 *Desc:       This functions changes the state of the pcb found to blocked   
 */

void block(char* pcb_name){
	pcb* ptr;
	//printf("ptr name: %s\n\n\0", pcb_name);
	ptr= Find_PCB(pcb_name);
	if(ptr != NULL)
	{
	//printf("ptr name inside if: %s\n\n\0", ptr->process_name);
		Remove_PCB(ptr);      //Remove from whatever queue it is in
		ptr->state= BLOCKED;
    //printf("after state change: %s\n\n\0",ptr->process_name);  //change state to blocked
		Insert_PCB(ptr);      //Insert into new queue
		printf("PCB is now blocked.\n\n\0");
	}
	return;
}

/***************************
 *Name:       unblock
 *Parameters: pcb_name
 *Calls:      Find_PCB
 *            Remove_PCB
 *            Insert_PCB  
 *Returns:    void
 *Desc:       This functions changes the state of the pcb found to unblocked and 
 *            insert into the ready queue   
 */

void unblock(char* pcb_name){
	pcb* ptr;
	ptr = Find_PCB(pcb_name);
	if (ptr == NULL) return;
	
	if(ptr != NULL)
	{
		Remove_PCB(ptr);
		ptr->state= 101;
		Insert_PCB(ptr);
		printf("PCB is now unblocked.\n\n\0");
	}
	return;
}

/***************************
 *Name:       suspend
 *Parameters: pcb_name
 *Calls:      Find_PCB
 *            Remove_PCB
 *            Insert_PCB  
 *Returns:    void
 *Desc:       This functions changes the state of the pcb found to suspended 
 *            ready or suspended blocked depending on which queue the pcb was 
 *            found.    
 */

void suspend(char* pcb_name){
	pcb* ptr;
	ptr= Find_PCB(pcb_name);
	
	if(ptr != NULL) {	
		if(ptr->state == READY || ptr->state == RUNNING) {
    		Remove_PCB(ptr);
    		ptr->state= SUSPENDED_READY;
    		Insert_PCB(ptr);
    		printf("PCB is now suspended.\n");
		}
		if(ptr->state == BLOCKED) {
    		Remove_PCB(ptr);
    		ptr->state= SUSPENDED_BLOCKED;
    		Insert_PCB(ptr);
    		printf("PCB is now suspended.\n");
		}
	}
	return;
}

/***************************
 *Name:       resume
 *Parameters: pcb_name
 *Calls:      Find_PCB
 *            Remove_PCB
 *            Insert_PCB  
 *Returns:    void
 *Desc:       This function changes the state to either ready or blocked 
 *            depending on which queue it was found.   
 */

void resume(char* pcb_name){
	pcb* ptr;
	ptr = Find_PCB(pcb_name);
	//printf("ptr: %s\n", ptr->process_name);
	if(ptr != NULL) {
		if (ptr->state == SUSPENDED_READY) {
			ptr = Remove_PCB(ptr);
			ptr->state= READY;
			Insert_PCB(ptr);
			//printf("PCB has now resumed.\n");
			return;
		}
		if(ptr->state=SUSPENDED_BLOCKED) {
			Remove_PCB(ptr);
			ptr->state= BLOCKED;
			Insert_PCB(ptr);
			//printf("PCB has now resumed.\n");
			return;
		}
	}
	return;
}

         

void listDir(char dir_name[]) {
      paging* current; 
      paging list[100];
      int index = 0;
      int check;
      int inputLength = 100;
    	int *lengthPtr = &inputLength;
    	char input[100];
      int i;
      int dir;
      long* filesize = 0;
      char name_buf[50] = {0};
      
      if (strlen(dir_name) > 50) {
              errorCodeTranslator(ERR_SUP_NAMLNG);
              return;
      }
      
      dir = sys_open_dir("\0");
      
      
      if (dir != 0) {
          errorCodeTranslator(dir);
          sys_close_dir();
          return;
      } else {
          printf("Name \t\t Size\n\n");
          while (i <= 10) {
               if (i == 10) {
                    printf("Press enter to continue...");
                    fgets(input,*lengthPtr,stdin);
                    i = 0;
                    continue;
                 } else {
                      while (check = sys_get_entry(name_buf,50,filesize) == 0) {                 
                          if (strlen(name_buf) < 7) {
                              printf("%s \t\t %d \n", name_buf, *filesize);
                              i++;
                              break;
                          } else {
                              printf("%s \t %d \n", name_buf, *filesize );
                              i++;
                              break;
                          }
                    }
                }
          if (check == 0) break;
          else if (check != 0) continue;
          } 
      }
         /* for (i=0; i < sizeof(list)/sizeof(paging); i++) {
              if (strlen(list[i]->buf_name) < 7) {
                  printf("%s \t\t %d \n", list[i]->buf_name, list[i]->size);
              } else {
              printf("%s \t %d \n", list[i]->buf_name, list[i]->size );
              }
          }*/
      sys_close_dir();
      return;
}

/***************************
 *Name:       displayDate
 *Parameters: none
 *Calls:      sys_get_date 
 *Returns:    void
 *Desc:       This functions prints the system date  
 */

void displayDate() {
	sys_get_date(date_p);
	printf("%d-",date_p->year);
	printf("%d-",date_p->month);
	printf("%d\n",date_p->day);
}

/***************************
 *Name:       clearScreen
 *Parameters: none
 *Calls:      clrscr  
 *Returns:    void
 *Desc:       Clears the screen  
 */

void clearScreen() {
 clrscr();
 return;
}

/***************************
 *Name:       changeDate
 *Parameters: char*, char*, char*
 *Calls:      atoi
 *            errorCodeTranslator
 *            displayDate    
 *Returns:    void
 *Desc:       This functions changes the state of the pcb found to blocked   
 */

void changeDate(char *yearc, char *monthc, char *dayc) {
	int i;
	int year;
	int month;
	int day;
	date_rec reset;
	date_rec *save_date;
	printf("Original date:\n");
	displayDate();

	year = atoi(yearc);
	month = atoi(monthc);
	day = atoi(dayc);

	save_date->year = year;
	save_date->month = month;
	save_date->day = day;

	if (year == NULL || month == NULL || day == NULL) {
		errorCodeTranslator(ERR_SUP_INVDAT);
		printf("Date arguments must all be Integers.\n");
		return;
	}

	if (year < 1000 || year > 2011) {
		errorCodeTranslator(ERR_INV_YEAR);
		printf("Valid years are between 1000 and 2011");
		return;
	}
	if (month < 1 || month > 12) {
		errorCodeTranslator(ERR_INV_MONTH);
		printf("Valid months are between 1 and 12");
		return;
	}
	if (day < 1 || day > 31) {
		errorCodeTranslator(ERR_INV_DAY);
		printf("Valid days for this month are between 1 and 31");
		return;
	}
	if((month== 4 || month==6 || month==9 || month==11) && day>30) {
		errorCodeTranslator(ERR_SUP_INVDAT);
		printf("Valid days for this month are between 1 and 30");
		return;
	}
	//TODO: get leap years right
	if (month == 2) {
		if (year%4 == 0 && day > 30) {
        errorCodeTranslator(ERR_SUP_INVDAT); 
        return;
        }
		else if(month == 2 && day>29) {
        errorCodeTranslator(ERR_SUP_INVDAT);
        return;
        }
		else if (year%100 != 0 || year%400 != 0) { 
        errorCodeTranslator(ERR_SUP_INVDAT);
        return;
        }
	}
	if (save_date->year == date_p->year && save_date->month == date_p->month && save_date->day == date_p->day) {
		errorCodeTranslator(ERR_SUP_DATNCH);
	}
	i = sys_set_date(save_date);
	if (i == OK) {
		  printf("Date successfully set to:\n");
		  displayDate();
	}
}
    	
/***************************
 *Name:       sys_call (interrupt)
 *Parameters: none
 *Calls:      FP_SEG
 *            FP_OFF
 *            sizeof
 *            Insert_PCB
 *            Free_PCB
 *            dispatcher       
 *Returns:    void
 *Desc:       This is an interrupt that saves the current program state then
 *            uses a temporary stack to keep the processes running correctly 
 *            without trashes the system stack.     
 */ 
 
void interrupt sys_call() {
    static int result;
    temp_ss = _SS;
    temp_sp = _SP;
    
    
    ssnew = FP_SEG(&sysStack);
    spnew = FP_OFF(&sysStack);
    spnew += 2048;
    _SS = ssnew;
    _SP = spnew;
    
    trm_getc();    //flush keyboard from dos to mpx   //new for R6   

    if (wait_term->event_flag == 1) {
         wait_term->event_flag = 0;         
         temp_iod = wait_term->head;
         wait_term->head = temp_iod->next;
         
        if (wait_term->head != NULL) {
      			switch (temp_iod->type) {
      				case READ:
      					 trm_read(temp_iod->location, temp_iod->counter);
      					 break;
      				case WRITE:
      					 trm_write(temp_iod->location, temp_iod->counter);
      					 break;
      				case CLEAR:
      					 trm_clear();
      					 break;
      				case GOTOXY:
      					 trm_gotoxy(0,0);
      					 break;
      				default:
      					 break;
      			}
        } 
      			unblock(temp_iod->requestor);
		        sys_free_mem(temp_iod);
            wait_term->count--;
		  
		}
		
		if (wait_com->event_flag == 1) {
         wait_com->event_flag = 0;
         
         temp_iod = wait_com->head;
         wait_com->head = temp_iod->next;
         
        if (wait_com->head != NULL) {
      			switch (temp_iod->type) {
      				case READ:
      					 trm_read(temp_iod->location, temp_iod->counter);
      					 break;
      				case WRITE:
      					 trm_write(temp_iod->location, temp_iod->counter);
      					 break;
      				default:
      					 break;
      			}
      			unblock(temp_iod->requestor);
		        sys_free_mem(temp_iod);
            wait_com->count--;
		    }
		}
    cop-> stack_top = (unsigned char *)MK_FP(temp_ss, temp_sp);
    param_p = (params*)(cop->stack_top + sizeof(context));
    
    switch(param_p->op_code) {
        case IDLE:
            cop->state = READY;
            Insert_PCB(cop);
            break;
        case EXIT:
            Free_PCB(cop);
            result = -1;
            break;
        case READ:
            io_scheduler();
            break;
        case WRITE:
            io_scheduler();
        case GOTOXY:
            io_scheduler();
        case CLEAR:
            io_scheduler();
        default:
            break;
    }
    
    temp_ss = NULL;
    temp_sp = NULL;   
    dispatcher();
    
}       
/*
void interrupt sys_call() {
    static int result;
    cop-> stack_top = (unsigned char *)MK_FP(_SS, _SP);
    
    
    ssnew = FP_SEG(&sysStack);
    spnew = FP_OFF(&sysStack);
    spnew += 2048;
    _SS = ssnew;
    _SP = spnew;
    
    //trm_getc();    //flush keyboard from dos to mpx   //new for R6
    
    param_p = (params*)(cop->stack_top + sizeof(context));

    if (param_p->op_code == IDLE) {
        cop->state = READY; //puts the process in the ready queue by changing its state
        Insert_PCB(cop);
        result = 0;
    } else if (param_p->op_code == EXIT) {
    		Free_PCB(cop); //not sure this is really the function to call but seems so
    		result = -1;
    } 
       
    //context_p->AX= result; //resetting the AX to the value returned, used later by sys_req no idea what for=    
    dispatcher();
    
}
 */
/***************************
 *Name:       dispatcher (interrupt)
 *Parameters: pcb_name
 *Calls:      Remove_PCB
 *            FP_SEG
 *            FP_OFF
 *                 
 *Returns:    void
 *Desc:       This interrupt sets the temp stack and then gets the next pcb.
 *            OR
 *            This will reset the system stack back to normal once the process 
 *            is finished running.       
 */

void interrupt dispatcher(){
 	
 	static pcb* head; //HEAD of the ready queue
 	static int temp_ss;
 	static int temp_sp;
 	if (sp_save == NULL)
 	{
 	  ss_save = _SS;
		sp_save = _SP;
	}
	
	head = readyQ->head;
	cop = Remove_PCB(head);
	
			if (cop != NULL) { //remove the element located at the head of the queue
    			cop->state  = RUNNING;
    			temp_ss = FP_SEG(cop->stack_top);
          temp_sp = FP_OFF(cop->stack_top);	 
    			_SS         = temp_ss;
    			_SP         = temp_sp;	
		  } else {
    			cop= NULL;
    			_SS         = ss_save;
    			_SP         = sp_save;
    			sp_save     = NULL;
    			ss_save     = NULL;
		  } 
}

/***************************
 *Name:       terminate_process
 *Parameters: pcb_name
 *Calls:      Find_PCB
 *            Remove_PCB
 *            Insert_PCB  
 *Returns:    void
 *Desc:       This function finds, removes, and frees the pcb, ending the 
 *            process.   
 */

void terminate_process(char* pcb_name){
  	pcb* pcb1;
  	pcb* pcb2;
  	pcb1= Find_PCB(pcb_name);
  	
  	if(pcb1== NULL) {
  		  printf("Process does not exist");
  		  return;
    } else {
      	pcb2= Remove_PCB(pcb1);
      	Free_PCB(pcb2);
      	return;
  	}
}



/***************************
 *Name:       ver
 *Parameters: none
 *Calls:      none  
 *Returns:    void
 *Desc:       This function simply shows the version of the project.  
 */

void ver () {
	printf("This is the version #6.1.5 of GeckOs\n");
	printf("Module #R6\n");
	printf("Last Modified: 12/4/2010\n");
}

/***************************
 *Name:       removeNL
 *Parameters: char*
 *Calls:      strcspn 
 *Returns:    void
 *Desc:       This functions removes the new line character from the end of a
 *            string.   
 */

void removeNL(char *s) {
	s[strcspn(s, "\n")] = '\0';
}

/***************************
 *Name:       terminate
 *Parameters: none
 *Calls:      sys_exit  
 *Returns:    void
 *Desc:       This function closes the mpx program.   
 */

void terminate() {
	printf("Goodbye.\n");
	sys_exit();
}
