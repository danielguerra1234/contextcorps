/*
 ============================================================================
 Name        : R6.c
 Author      : Jeremy Keczan
 Version     :
 Copyright   : Your copyright notice
 Description : R6, Turbo C-style
 ============================================================================
 */
            
#include "R5/r5.h"            
#include "GeckOS2.h"


//Global Variables:
DIR *dp;
struct dirent *ep;
date_rec *date_p;
char *prompt = "~> ";


//4 Queues
queue* readyQ;
queue* blockQ;
queue* suspendreadyQ;
queue* suspendblockQ; 

char* commands[50];
char* cc;
int id_com = 0;
int id_h = 0;

//Command variables used for aliasing
char* version_c       = "version";
char* exit_c          = "exit";
char* show_c          = "show";
char* pcb_c           = "pcb";
char* help_c          = "help";
char* clear_c         = "clear";
char* setprompt_c     = "setprompt";
char* hist_c          = "hist";
char* date_c          = "date";
char* dir_c           = "dir";
char* block_c         = "block";
char* unblock_c       = "unblock";
char* suspend_c       = "suspend";
char* resume_c        = "resume";
char* priority_c      = "priority";
char* alias_c         = "alias";
char* dispatch_c      = "dispatch";
char* load_c          = "load";

unsigned int sp_save=NULL, ss_save=NULL;
unsigned int ssnew=NULL, spnew=NULL;
unsigned char sysStack[2048];

pcb* cop;
params* param_p;
context* context_p;



int main(void) {
  pcb* fake;
	init();
	init_R6();
	COMHAN();
	
}

void init_R6() {

  pcb* comhan;
  context* comhan_con;

  // Set up COMHAN PROCESS
	comhan = Setup_PCB("COMHAN\0", 1, 1);
	comhan_con = (context *) comhan->stack_top;
	
	comhan_con->DS = _DS;
	comhan_con->ES = _ES;
	comhan_con->CS = FP_SEG(&COMHAN);
	comhan_con->IP = FP_OFF(&COMHAN);
	comhan_con->FLAGS = 0x200;
	
	Insert_PCB(comhan);
	
	Load_Program("IDLE", "IDLE", 123, "\0");
}

int COMHAN() {

  int inputLength = 100;
	int *lengthPtr = &inputLength;
	char input[100];
	int exitCode = 0;
	int loopbreaker = 0;
	
    do { 
  
    if (loopbreaker >= 200) {
        printf("Infinite Loop Error. Aborting.\n");
    }
  
    if (cc) {
        printf("\nHist command returned: %s\n\n", cc);
        exitCode = parseCommand(cc);
        cc = NULL;
        loopbreaker = 0;
    }
       
    else {
        printf("%s ", prompt);
		    fgets(input,*lengthPtr,stdin);
		    removeNL(input);
		    //strcpy(commands[id_com], input);
		    commands[id_com] = strdup(input);
        id_com++;
        loopbreaker++;   
		    exitCode = parseCommand(input);
		}
		
		if (exitCode == 1) {
			puts("Are you sure you want to exit? (y/n)");
			fgets(input,*lengthPtr,stdin);
			removeNL(input);
			if (strcmp(input,"N") == 0 || strcmp(input,"n") == 0 || strcmp(input,"No") == 0 || strcmp(input,"no") == 0 || strcmp(input,"NO") == 0) exitCode = 0;
		}
	} while (exitCode == 0);
	if (exitCode != 1) errorCodeTranslator(exitCode);
	terminate();
	
	return 0;
}
                                                                       
void init() {
  int error;
  char greeting[20] = "Welcome to GeckOS!\0";
	clearScreen();
	puts(greeting);
	dp = opendir ("./");
	sys_init(MODULE_R4);
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
 *Calls:    sys_free_mem
 *Return:   nothing   
 *Desc:     This function calls sys_free_mem to release the memory for the ptr*
 *          along with the stack base       
 */    

void Free_PCB(pcb *ptr) { 
  if (ptr == NULL){
      printf("PCB not found, can not be releaseed.\n");
      return 0;
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
		return;
	}
	if (strlen(name) > 15) {
		errorCodeTranslator(ERR_SUP_NAMLNG);
		return;
	}
	/*if (Find_PCB(name) != NULL) {
		errorCodeTranslator(ERR_PCB_NMEEXISTS);
		return;
	}*/  
	if(priorityc > 127 || priorityc < (-128)) {
		errorCodeTranslator(ERR_PCB_INVPRIORITY);
		return;
	}
	if (class!= 1 && class!= 2) {
		errorCodeTranslator(ERR_PCB_INVCLASS);
		return;
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
		return;
	}
	if (strlen(name) > 15) {
		errorCodeTranslator(ERR_SUP_NAMLNG);
		return;
	}
	if (Find_PCB(name) != NULL) {
		errorCodeTranslator(ERR_PCB_NMEEXISTS);
		return;
	}  
	if(priorityc > 127 || priorityc < (-128)) {
		errorCodeTranslator(ERR_PCB_INVPRIORITY);
		return;
	}
	//returns size of memory needed to hold program
	error = sys_check_program("\0", prog_name, &prog_len_p, &start_offset_p);
	
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
	 int i = 0;
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
	i = 0;
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
    ptr->next == NULL;
    ptr->prev == NULL;
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
  } else if (pcb1->next = NULL && pcb1->prev != NULL){ //pcb1 is the last element in the queue
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
  //return 0; 
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
    		return NULL;
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
	return NULL;
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
    		return NULL;
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
	return NULL;
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
    return NULL;
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
	return NULL;
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
   
	return NULL;
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
	printf("ptr: %s\n", ptr->process_name);
	if(ptr != NULL) {
		if (ptr->state= SUSPENDED_READY) {
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

/***************************
 *Name:       parseCommand
 *Parameters: char*
 *Calls:      Load_Program
 *            ver
 *            dispatch
 *            clearScreen
 *            terminate_process
 *            changeDir
 *            displayDate
 *            changeDate
 *            block
 *            unblock
 *            resume
 *            suspend
 *            dir
 *            exit
 *            show_all
 *            show_ready
 *            show_blocked                   
 *Returns:    void
 *Desc:       This functions changes the state of the pcb found to blocked   
 */

//NOTE: a return value other than 0 will result in program exit
int parseCommand(char *commandString) {
	//TODO: look to see what else I can do wtih strtok
	//TODO: turn these into an actual struct...preferably an array I can just loop over, if that's possible
	char* command;
	char* arg1;       //option flag
	char* arg2;
	char* arg3;
	char* arg4;
	
	char* strtokaddr;
	command = strtok(commandString, " ");
	arg1 = strtok(NULL, " ");
	arg2 = strtok(NULL, " ");
	arg3 = strtok(NULL, " ");
	arg4 = strtok(NULL, " "); 
  //printf("\ncommandString: %s command: %s, arg1: %s, arg2: %s, arg3: %s, arg4: %s\n", commandString, command, arg1, arg2, arg3, arg4);

//	printf("command: %s\narg1: %s\narg2: %s\narg3: %s\narg4: %s\n", command,arg1,arg2,arg3,arg4); //XXX: DEBUG
	if (command == NULL) return 0; //if no command given, simply return...since it's not really an issue
	if (strcmp(command,help_c) == 0) {
		help(arg1);
		return 0;
	}
	if (strcmp(command,version_c) == 0) {
		ver();
		return 0;
	}
	if (strcmp(command,clear_c) == 0) {
		clearScreen();
		return 0;
	}
	if (strcmp(command,dispatch_c) == 0) {
		printf("The dispatch function has been deprecated for GeckOS v6. \n");
		return 0;
	} 
	if (strcmp(command,load_c) == 0) {
	   if (arg1 == NULL || arg2 == NULL || arg3 == NULL) {
        printf("Load function requires 3 arguments.\n");
     } else {
        Load_Program(arg1, arg2, atoi(arg3), NULL);
        return 0;
     }
  }
  
  if (strcmp(command, "terminate") == 0) {
      if (arg1 == NULL) {
          printf("requires argument.\n");
      }
      else {
          terminate_process((char*)arg1);
          return 0;
      }
  }
	
	if (strcmp(command,alias_c) == 0) {
	   if (arg1 == NULL || arg2 == NULL) {                   //Check args
        printf("Alias function requries two arguments.\n");
        return 0;
     }
     
     if (strcmp(arg1, alias_c) == 0) {
        printf("Alias function can not be aliased. Its just too confusing.\n");
        return 0;
     }
     
     if (command_check(arg2) != 1) {
        return 0;
     }
     
     if (strcmp(arg1, "dispatch") == 0) {
        printf("We do not allow the aliasing of the dispatch function.\n");
        return 0;
     }
     if (strcmp(arg1, "load") == 0) {
        printf("We do not allow the aliasing of the load function.\n");
        return 0;
     }
     
     if (strcmp(arg1,"version") == 0) {                   //Check version
        version_c = strdup(arg2);
        return 0;
     }
     
     if (strcmp(arg1,"exit") == 0) {
        exit_c = strdup(arg2);
        return 0;
     } 
     
     if (strcmp(arg1,"clear") == 0) {
        clear_c = strdup(arg2);
        return 0;
     }
     
     if (strcmp(arg1,"help") == 0) {
        help_c = strdup(arg2);
        return 0;
     }
     
     if (strcmp(arg1,"pcb") == 0) {
        pcb_c = strdup(arg2);
        return 0;
     }
     
     if (strcmp(arg1,"show") == 0) {
        show_c = strdup(arg2);
        return 0;
     }
     
     if (strcmp(arg1,"block") == 0) {
        block_c = strdup(arg2);
        return 0;
     }
     
     if (strcmp(arg1,"setprompt") == 0) {
        setprompt_c = strdup(arg2);
        return 0;
     }
     
     if (strcmp(arg1,"hist") == 0) {
        hist_c = strdup(arg2);
        return 0;
     }
     
     if (strcmp(arg1,"date") == 0) {
        date_c = strdup(arg2);
        return 0;
     }
     
     if (strcmp(arg1,"dir") == 0) {
        dir_c = strdup(arg2);
        return 0;
     }
     
     if (strcmp(arg1,"unblock") == 0) {
        unblock_c = strdup(arg2);
        return 0;
     }
     
     if (strcmp(arg1,"suspend") == 0) {
        suspend_c = strdup(arg2);
        return 0;
     }
     
     if (strcmp(arg1,"resume") == 0) {
        resume_c = strdup(arg2);
        return 0;
     }
     
     if (strcmp(arg1,"priority") == 0) {
        priority_c = strdup(arg2);
        return 0;
     }
     else 
      return 0;
  }
	if (strcmp(command,"cd") == 0) {
		if (arg1 == NULL) printf("cd requires a path\n");
		else changeDir(arg1);
		return 0;
	}
	if (strcmp(command, setprompt_c) == 0) {
		if (arg1 == NULL) puts("Prompt argument cannot be blank");
		else if (strlen(arg1) > 20) puts("Prompt argument must be 20 characters or less");
		else setPrompt(arg1);
		return 0;
	}
	
	if (strcmp(command, hist_c) == 0) {
	     int inputLength = 100;
	     int *lengthPtr = &inputLength;
	     char input[100]; 
       int choice;           
	     int exitcode = 0;
	     int i = 0;
	     
	     while (i < id_com) {
          printf("%d: %s\n",i+1,commands[i]);
          i++;
       }
	     
	     do {
          printf("\nhist> ");
          fgets(input,*lengthPtr,stdin);
          removeNL(input);
          if (strcmp(input, "q") == 0) {
              exitcode = 1;
              return 0;
          }
          else {
              choice = atoi(input);
              cc = strdup(commands[choice-1]);
              exitcode = 1;
          }
       } while (exitcode == 0);
       return 0;
  }
	
	if (strcmp(command,date_c) == 0) {
		if (arg1 == NULL) {
			displayDate(); //if the user wants to display the date
			return 0;
		}
		if (strcmp(arg1,"set") == 0) { //if the user wants to set the date
			if (arg2 == NULL && arg3 == NULL && arg4 == NULL) printf("There must be Year, Month, and Day arguments. Date not changed\n");
			else changeDate(arg2,arg3,arg4);
		} else { //invalid argument
			printf("Argument '%s' is not defined for %s", arg1, command);
		}
		return 0;
	}
	if (strcmp(command,dir_c) == 0) {
		listDir();
		return 0;
	}
	
	if (strcmp(command, pcb_c) == 0) {
		if (arg1 == NULL) {
			printf("This function has been deprecated for GeckOS v6. \n");
			return 0;
		}
		if (strcmp(arg1,"-c") == 0) {
			if(arg2 == NULL || arg3 == NULL || arg4 == NULL) {
				printf("Unblocking has been deprecated for GeckOS v6. \n");;
				return 0;
			} else {
				printf("PCB create has been deprecated for GeckOS v6. \n");
				return 0;
			}
		}
		if (strcmp(arg1, "-d") == 0) {
		  if (arg2 == NULL) {
			printf("Unblocking has been deprecated for GeckOS v6. \n");
			return 0;
			} else {
				printf("PCB delete has been deprecated for GeckOS v6. \n");
				
				return 0;
			}
		}
		if (strcmp(arg1, "-s") == 0) {
      		  if (arg2 == NULL || arg3 == NULL) {
       			puts("Set Priority function requires PCB name and new priority.\n");
      		  	} else {
        			printf("Setting new priority:%d for PCB:%s",arg2,arg3);
        			Set_Priority(arg2, atoi(arg3));
		return;
      		}
      
    }
    }
  
  if (strcmp(command, block_c) == 0) {
    if (arg1 == NULL) {
      printf("block command requires name argument.\n\0");
      return 0;
    }
    else {
      //add function call when function is ready
      printf("Blocking has been deprecated for GeckOS v6. \n");
      return 0;
    }
  }
  
  if (strcmp(command, unblock_c) == 0) {
    if (arg1 == NULL) {
      printf("Unblock command requires a name\n");
      return 0;
    }
    else {
      //add function call when function is ready
      printf("Unblocking has been deprecated for GeckOS v6. \n");
      return 0;
    }
  }
  
  if (strcmp(command, suspend_c) == 0) {
    if (arg1 == NULL) {
      printf("Suspend requires a name\n");
      return 0;
    }
    else {
      //add function when function is ready
      //printf("Suspending %s\n",arg1);
      suspend(arg1);
      return 0;
    }
  }
  
  if (strcmp(command, resume_c) == 0) {
    if (arg1 == NULL) {
      printf("Resume requires a name\n");
      return 0;
    }
    else {
      printf("Resuming %s\n",arg1);
      resume(arg1);
      return 0;
    }
  }
  
  if (strcmp(command, priority_c) == 0) {
    if (arg1 == NULL) {
      printf("priority command requires option operation -c\n");
      return 0;
    }
    if (arg2 == NULL) {
      printf("priority command requires name arugment second\n");
      return 0;
    }
    if (arg3 == NULL) {
      printf("priority command requires priority value arugment third\n");
      return 0;
    }
    if (arg4 != NULL) {
      printf("priority command takes only 3 arguments\n");
      return 0;
    }
    else {
      if (strcmp(arg1, "-s") == 0) {
         //add function call
         printf("calling set priority function\n");
         return 0;
      }
      else {
        printf("invalid argument\n");
        return 0;
      }
    }
  }
  
  if (strcmp(command, show_c) == 0) {
    if (arg1 == NULL) {
      printf("show command requires an argument: -p(show PCB), -a(show all), -r(show ready), -b(show blocked)\n");
      return 0;
    }
    if (strcmp(arg1,"-p") == 0) {
      if (arg2 == NULL) {
        printf("show command arguement -p requires name argument\n");
        return 0;
      }
      else {
        //add function call 
        //printf("calling show pcb function\n");
        Show_PCB(arg2);
        return 0;
      }
    }
    if (strcmp(arg1, "-a") == 0) {
      if (arg3 != NULL || arg4 != NULL) {
        printf("show argument -a requires no extra arguments ");
        return 0;
      }
      else {
        //add function call
        printf("calling show with -a argument\n");
        show_all();
        return 0;
      } 
    }
    if (strcmp(arg1, "-r") == 0) {
      if (arg2 != NULL || arg3 != NULL || arg4 != NULL) {
        printf("show command argument -r takes no arguments\n");
        return 0;
      }
      else {
        printf("calling show command with -r argument\n");
        show_ready();
        return 0;
      }
    }
    if (strcmp(arg1, "-b") == 0) {
      if (arg2 != NULL || arg3 != NULL || arg4 != NULL) {
        printf("show command argument -b takes no arguments\n");
        return 0;
      }
      else {
        printf("calling show command with -b argument\n");
        show_blocked();
        return 0;
      }
    }
    if (strcmp(arg1, "-sr") == 0) {
      if (arg2 != NULL || arg3 != NULL || arg4 != NULL) {
        printf("show command argument -sr takes no arguments\n");
        return 0;
      }
      else {
        printf("calling show command with -sr argument\n");
        show_suspended_ready();
        return 0;
      }
    }
    else{
      printf("Argument '%s' is an invalid argument\n",arg1);
      return 0;
    }  
  }
  
	
	if (strcmp(command,exit_c) == 0) {
		return 1;
	}
	printf("%s is not a valid command. For a list of valid commands, type 'help'\n", command);
	return 0;
}         

void listDir() {
       char *command;
       char *extension;
       if (dp != NULL) {
	   while (ep = readdir(dp)) {
        	   
		   command = strtok(ep->d_name, ".");
		   extension = strtok(NULL, ".");
		   puts(extension);
		   if (strcmp(extension, "mpx") == 0) {
			puts(ep->d_name);
		   }
		   (void) closedir (dp);  }
       }else
	   perror("Couldn't open the directory");

}

/***************************
 *Name:       changeDir
 *Parameters: DIR
 *Calls:      opendir  
 *Returns:    void
 *Desc:       changes directory to which is passed in.   
 */

void changeDir(DIR *arg) {
      dp = opendir(arg);
      if (dp == NULL) printf("Could not open %s", arg);
}

/***************************
 *Name:       setPrompt
 *Parameters: char*
 *Calls:      none  
 *Returns:    void
 *Desc:       This functions changes the global variable prompt, XTRA CRED  
 */

void setPrompt(char *s) {
	prompt = s;
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
		if (year%4 == 0 && day > 30) errorCodeTranslator(ERR_SUP_INVDAT);
		else if(month == 2 && day>29) errorCodeTranslator(ERR_SUP_INVDAT);
		return;
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
    cop-> stack_top = (unsigned char *)MK_FP(_SS, _SP);
    
    
    ssnew = FP_SEG(&sysStack);
    spnew = FP_OFF(&sysStack);
    spnew += 2048;
    _SS = ssnew;
    _SP = spnew;
    
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
 *Name:       command_check
 *Parameters: pcb_name
 *Calls:      none  
 *Returns:    int
 *Desc:       This functions checks all commands for the alias command.   
 */

int command_check(char* name) {
     int check = 1;
     if (strcmp(name,alias_c) == 0) {
        printf("Alias command may not be aliased.\n");
        return 0;
     }
     
     if (strcmp(name,version_c) == 0) {                   //Check version
        check = 0;
     }
     
     if (strcmp(name,exit_c) == 0) {
        check = 0;
     } 
     
     if (strcmp(name,clear_c) == 0) {
        check = 0;
     }
     
     if (strcmp(name,help_c) == 0) {
        check = 0;
     }
     
     if (strcmp(name,pcb_c) == 0) {
        check = 2;
     }
     
     if (strcmp(name,show_c) == 0) {
        check = 0;
     }
     
     if (strcmp(name,block_c) == 0) {
        check = 2;
     }
     
     if (strcmp(name,setprompt_c) == 0) {
        check = 0;
     }
     
     if (strcmp(name,hist_c) == 0) {
        check = 0;
     }
     
     if (strcmp(name,date_c) == 0) {
        check = 0;
     }
     
     if (strcmp(name,dir_c) == 0) {
        check = 0;
     }
     
     if (strcmp(name,unblock_c) == 0) {
        check = 2;
     }
     
     if (strcmp(name,suspend_c) == 0) {
        check = 0;
     }
     
     if (strcmp(name,resume_c) == 0) {
        check = 0;
     }
     
     if (strcmp(name,priority_c) == 0) {
        check = 0;
     }
     if (check == 0) {
        printf("Command: %s is already taken, please chose another.\n",name);
        return check;
     }
     if (check == 2) {
        printf("Command may not be aliased since it is deprecated. \n");
        return check;
     }
     else {
        printf("\nCommand available.\n\n");
        return check;
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
