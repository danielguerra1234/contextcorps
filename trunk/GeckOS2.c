/*
 ============================================================================
 Name        : GeckOS2.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Turbo C-style
 ============================================================================
 */
            
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

unsigned int sp_save=NULL, ss_save=NULL;
unsigned int ssnew=NULL, spnew=NULL;
unsigned char sysStack[2048];

pcb* cop;
params* param_p;
context* context_p;

int main(void) {
  pcb* fake;
	int inputLength = 100;
	int *lengthPtr = &inputLength;
	char input[100];
	int exitCode = 0;
	int loopbreaker = 0;
	init();
	//fake->process_name = "Fake";
	//readyQ->head = fake;
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
		    strcpy(commands[id_com], input);
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
  char greeting[20] = "Welcome to GeckOS!\0";
	clearScreen();
	puts(greeting);
	dp = opendir ("./");
	sys_init(MODULE_R3);
	readyQ = initQueue(readyQ, "Ready\0");
	blockQ = initQueue(blockQ, "Blocked\0");
	suspendreadyQ = initQueue(suspendreadyQ, "Suspended Ready\0");
	suspendblockQ = initQueue(suspendblockQ, "Suspended Blocked\0");
	sys_set_vec(sys_call);  
}

//Structure functions for PCB and QUEUE
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
//#############Queue Functions#################
//Queue function error codes start in the 300

void blocked_add(pcb *node) {
    if (blockQ->nodes == 0) {
		blockQ->head = node;
		blockQ->tail = node;
		node->next = NULL;
        node->prev = NULL;
        blockQ->nodes = 1;
    }
    if (readyQ->nodes != 0) {
        pcb *current;
		current = blockQ->head;
          
        if (current->priority > node->priority) {
			current = current->next;
		}
		else if (current->priority < node->priority) {
			puts("start here");
	}
    }
}
/*
pcb* getNext(queue *q) {
	if (q->head == NULL) return NULL; //if no head, then no pcb in queue
	if (q->index == NULL) q->index = q->head; //set initial position
	else if (q->index->next == NULL) q->index = q->head; //if at end of queue, start at beginning
	else q->index = q->index->next; //change to next element
	return q->index;
}

pcb* getPrevious(queue *q) {
	if (q->head == NULL) return NULL; //if no head, then no pcb in queue
	if (q->index == NULL) q->index = q->head; //set initial position
	else if (q->index->prev == NULL) q->index = q->tail; //if at beginning of queue, start at end
	else q->index = q->index->prev; //change to previous element
	return q->index;
} */

//############PCB Functions####################
//PCB error codes start in the 400
pcb *allocatePcb(){
	int size = sizeof(pcb);
	int *address;
	pcb *pcb_ptr;
	
	pcb_ptr = (pcb*)sys_alloc_mem(size);//use sys_alloc_mem

	pcb_ptr->stack_base = sys_alloc_mem(1024);
	pcb_ptr->stack_top = pcb_ptr->stack_base + 1024 - sizeof(context);
	return (pcb_ptr);
}

void Free_PCB(pcb *ptr) { //pcb pointer
pcb* temp;
  temp = Find_PCB(ptr->process_name);
  if (temp == NULL){
      printf("PCB not found.\n");
      return 0;
  } else {
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
		errorCodeTranslator(ERR_PCB_NMETOLONG);
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
	//pcb1->process_name = strdup(name);
	pcb1->priority = priorityc;
	pcb1->process_class = classc;
	pcb1->state = 101;
	pcb1->next = NULL;
	pcb1->prev = NULL;
	printf("PCB succesfully created.\n\n\n");
	//Insert_PCB(pcb1);
	//Show_PCB(name);
	return pcb1;
}

pcb* Find_PCB_Ready(char* name) {
    pcb* walk;
    int check;
    
    walk = readyQ->head;
      //while(walk != NULL) {
            check = check + 1;
        	  //printf("Find_PCB Function executing with name: %s, %s\n",name, walk->process_name); 
            if (check > 50)
                //break; 
               // printf("Print break");
                      	  
        		if (strcmp(walk->process_name,name) == 0) {
              return walk;
            } else
                walk = walk->next;
      //}
}

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

pcb* Find_PCB(char *name){
	pcb* ptr;
	
	ptr = Find_PCB_Ready(name);
	/*
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
      printf("PCB %s not available", name);
      return NULL;
  } */
  return ptr;
	
}

void priority_insert(queue* q, pcb *ptr){
  pcb* walk; 
  pcb* prev;
  pcb* temp;
  int c = 0;
  int priority;
  int check= 0;
  
  priority = ptr->priority;
  walk = q->head;
  
  if (sizeof(q)>=buffer) {
    printf("You cannot insert in this queue, it is already full\n\n");
    return;
  }
    
  if (walk == NULL) {
    //printf("Walk is Null.\n");
    ptr->next == NULL;
    ptr->prev == NULL;
    q->head = ptr;
    q->index++;
    return;
  }
  
  while (walk != NULL) {
   // printf("Priority: %i\tWalk: %i\n",priority, walk->priority);
    if (check == 25){
      //printf("Broke by check");
      break;
    }
    
    if (priority <= walk->priority) {
     // printf("p < w\n");
      
        if (walk->prev == NULL && c == 0){
              //printf("Walk == NULL test.\n");
	     walk->prev       = ptr;
             walk->next       = NULL;
             
	     ptr->next        = walk;
             ptr->prev        = NULL;
             q->head          = ptr;
            // printf("walk name: %s\n\n", walk->process_name);
             q->index++;
             //c++;
             break;
        } else {
	    temp = walk->prev;
	    ptr->next = walk;
            ptr->prev = walk->prev;
	    temp->next = ptr;
	    walk->prev = ptr;
             q->index++;
            return;
            
        }  
    q->index++;
    return;
      
    } else if (priority > walk->priority) {
           if (walk->next == NULL) {
	      walk->next = ptr;
	      ptr->prev = walk;
              ptr->next = NULL;
              q->index++;
              return;
           }
           if (walk->next != NULL) {
	      walk = walk->next;
           }
   
    }
    
  }
}

void FIFO_insert(queue* q, pcb *ptr){
  if (sizeof(q)>=buffer) {
    printf("You cannot insert in this queue, it is already full\n");
  }
  else {
      if (q->head == NULL) {                              //No PCBs in queue
          printf("Queue head in Fifo check.\n\n");
          ptr->next   = NULL;
          ptr->prev   = NULL;
          
          q->head     = ptr;
          q->tail     = ptr;
      } else if ((q->head)->next == NULL) {               //first pcb in queue
          printf("Queue head is 1 in Fifo check.\n\n");
          ptr->next        = NULL;
          ptr->prev        = q->head;
          
          (q->head)->next  = ptr;
          q->tail          = ptr;
      }else {                                             //more than 1 pcb
          printf("Queue head many in Fifo check.\n\n");
          (q->tail)->next = ptr;
          ptr->prev       = q->tail;
          q->tail         = ptr;
    }
  }
}

void Insert_PCB(pcb* pcb1){
  int state;
  state = pcb1->state;
  //printf("Testing state before if statements in insert. state: %d\n\n", pcb1->state);
  
  if (state == READY) {
    priority_insert(readyQ, pcb1);
    //printf("ReadyQ name: %s Head: %s\n\n", readyQ->name, (readyQ->head)->process_name);
    return;
  }  
  
  if (state == 102) {
    //printf("Testing Blocked State on insert.\n\n");
    FIFO_insert(blockQ, pcb1);
    return;
  }  
  
  if(state == SUSPENDED_READY) {
    FIFO_insert(suspendreadyQ, pcb1);
    return;
  }  
  
  if(state == SUSPENDED_BLOCKED) {
    FIFO_insert(suspendblockQ, pcb1);
    return;
  }
}


pcb* Remove_PCB(pcb *pcb1){
  queue* q;
  pcb* prev;
  pcb* next;
  int state;
  
  state = pcb1->state;
  
  if (state == READY) {
      q = readyQ;  
  } else if (state == BLOCKED) {
      q = blockQ;
  }
  
  if (strcmp(q->head->process_name,pcb1->process_name) == 0) {//pcb1 is head of queue
      next = pcb1->next;
      
      if (next == NULL){ //only one queue
          next->prev = NULL;
          q->head = NULL;
          pcb1->next = NULL;
          pcb1->prev = NULL;
          return pcb1;
      }
  } else {
      if (pcb1->next = NULL && pcb1->prev != NULL){
          prev = pcb1->prev;
                
          prev->next = NULL;
          pcb1->next = NULL;
          pcb1->prev = NULL;
          return pcb1;  
      } else if (pcb1->next != NULL && pcb1->prev != NULL) {
          next = pcb1->next;
          prev = pcb1->prev;
          
          prev->next = next;
          next->prev = prev;
          
          pcb1->next = NULL;
          pcb1->prev = NULL;
          
          return pcb1;
      }    
  }
  return 0; 
}


void Set_Priority(char* name, int p) {
  pcb* procB;
  procB = Find_PCB(name);
  procB->priority = p;
  return;  
}

void Show_PCB(char* name) {
  
  pcb* pcbPtr;
  pcb* pcbPtr2;
  char* temp;
  //pcbPtr = Find_PCB(name);
  if (FALSE) {
    printf("PCB: %s does not exist.\n\n",name);
    return;
  } else {
    //pcbPtr2 = Find_PCB(name);
    pcbPtr2 = Find_PCB(name);
    if (pcbPtr2 == NULL) {
      //printf("PCB not found.\n");
      return;
    }
    if (pcbPtr2->state == RUNNING) {
      temp = "RUNNING";  
    } 
    else if (pcbPtr2->state == READY) {
      temp = "READY";
    }
    else {
      temp = "INVALID STATE";
    }

    printf("Name: %s\n" 
            "Priority: %d\n" 
            "Process_Class: %d\n" 
            "State: %s\n\n",pcbPtr2->process_name, pcbPtr2->priority, pcbPtr2->process_class, temp);
    return;
  }
}

void show_ready(){
	 pcb* walk;
	 int check;
	 int i = 0;
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
	i = 0;
	return NULL;
}

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

void show_all(){
	 pcb* walk;
	 int check;
	 
   printf("All PCB's:\n\n");
   show_ready();
   show_blocked();
   
	return NULL;
}

void block(char* pcb_name){
	pcb* ptr;
	ptr= Find_PCB(pcb_name);
	if(ptr != NULL)
	{
		Remove_PCB(ptr);      //Remove from whatever queue it is in
		ptr->state= BLOCKED;  //change state to blocked
		Insert_PCB(ptr);      //Insert into new queue
		printf("PCB is now blocked\n");
	}
	return;
}


void unblock(char* pcb_name){
	pcb* ptr;
	ptr = Find_PCB_Blocked(pcb_name);
	if (ptr == NULL) return;
	
	if(ptr != NULL)
	{
		Remove_PCB(ptr);
		ptr->state= 101;
		Insert_PCB(ptr);
		printf("PCB is now unblocked");
	}
	return;
}

void suspend(char* pcb_name){
	pcb* ptr;
	ptr= Find_PCB(pcb_name);
	
	if(ptr != NULL) {	
		if(ptr->state= READY || ptr->state == RUNNING) {
    		Remove_PCB(ptr);
    		ptr->state= 103;
    		Insert_PCB(ptr);
    		printf("PCB is now suspended");
		}
		if(ptr->state= BLOCKED) {
    		Remove_PCB(ptr);
    		ptr->state= 104;
    		Insert_PCB(ptr);
    		printf("PCB is now suspended");
		}
	}
	return;
}

void resume(char* pcb_name){
	pcb* ptr;
	ptr= Find_PCB(pcb_name);
	if(ptr != NULL) {
		if (ptr->state= 103) {
			ptr->state= 101;
			Remove_PCB(ptr);
			Insert_PCB(ptr);
			printf("PCB has now resumed");
		}
		if(ptr->state=104) {
			ptr->state= 102;
			Remove_PCB(ptr);
			Insert_PCB(ptr);
			printf("PCB has now resumed");
		}
	}
	return;
}


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
		testn_R3();
		return 0;
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
     
     if (command_check(arg2) == 0) {
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
			puts("You must define whether you are creating(-c) or deleting(-d) a pcb");
			return 0;
		}
		if (strcmp(arg1,"-c") == 0) {
			if(arg2 == NULL || arg3 == NULL || arg4 == NULL) {
				printf("Creating a pcb requires 3 arguments:  name, class, priority\n");
				return 0;
			} else {
				//add function call once functions are ready
				//printf("Creating a pcb\nName: %s\nClass: %s\nPriority: %d\n",arg2,atoi(arg3),arg4);
				Setup_PCB(arg2,atoi(arg3),atoi(arg4));
				//printf("parse command check: %s\n",readyQ->head->process_name);
				return 0;
			}
		}
		if (strcmp(arg1, "-d") == 0) {
		  if (arg2 == NULL) {
			puts("Deleting a pcb requires a pcb name.\n");
			return 0;
			} else {
				printf("Deleting pcb '%s'\n",arg2);

				Remove_PCB(Find_PCB(arg2));
				Free_PCB(arg2);
				
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
      printf("block command requires name argument\n");
      return 0;
    }
    else {
      //add function call when function is ready
      printf("blocking %s\n\n",arg1);
      block(arg1);
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
      printf("Unblocking %s\n",arg1);
      unblock(arg1);
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
      printf("Suspending %s\n",arg1);
      suspend(arg1);
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

void changeDir(DIR *arg) {
      dp = opendir(arg);
      if (dp == NULL) printf("Could not open %s", arg);
}

void setPrompt(char *s) {
	prompt = s;
}

void displayDate() {
	sys_get_date(date_p);
	printf("%d-",date_p->year);
	printf("%d-",date_p->month);
	printf("%d\n",date_p->day);
}

void clearScreen() {
 clrscr();
 return;
}

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
    		Free_PCB(Remove_PCB(cop)); //not sure this is really the function to call but seems so
    		result = -1;
    } 
    
    
    context_p->AX= result; //resetting the AX to the value returned, used later by sys_req no idea what for=    
    dispatcher();
}       

void testn_R3(){
  
	pcb* test1;
	pcb* test2;
	pcb* test3;
	pcb* test4;
	pcb* test5;
	
	//Context for each function
	context* test1con;
	context* test2con;
	context* test3con;
	context* test4con;
	context* test5con;
	
	//Test 1 function
	test1 = Setup_PCB("test1\0", 1, 1);
	//printf("\n\nTest1 name: %s, p: %d, c:d\n\n", test1->process_name,test1->priority,test1->process_class);
	test1con = (context *) test1->stack_top;
	
	test1con->DS = _DS;
	test1con->ES = _ES;
	test1con->CS = FP_SEG(&test1_R3);
	test1con->IP = FP_OFF(&test1_R3);
	test1con->FLAGS = 0x200;
	
	Insert_PCB(test1);
//show_ready();
	//Test 2 function  
	
	test2 = Setup_PCB("test2\0", 2, 1);
	test2con = (context *) test2->stack_top;
	
	test2con->DS = _DS;
	test2con->ES = _ES;
	test2con->CS = FP_SEG(&test2_R3);
	test2con->IP = FP_OFF(&test2_R3);
	test2con->FLAGS = 0x200;
	
	Insert_PCB(test1);
	//show_ready();
	/*
	test3 = Setup_PCB("test3", 1, one);
	test3->exe_addr = &test3_R3;
	
	test4 = Setup_PCB("test4", 1, one);
	test4->exe_addr = &test4_R3;
	
	test5 = Setup_PCB("test5", 1, one);
	test5->exe_addr = &test5_R3;
  */
  dispatcher();
  
}

void save_context(int n){ //takes an integer for what test number it is

	pcb pcb_a[5]; //array of pcbs
	context *context_p;
	pcb_a[n].stack_p = (int)pcb_a[n].stack_base + (int)pcb_a[n].stack_size - (int)sizeof(context);
	context_p =(context*) pcb_a[n].stack_p;
	context_p->DS = _DS;
	context_p->ES = _ES;
	context_p->CS = FP_SEG(&testn_R3);
	context_p->IP = FP_OFF(&testn_R3);
	context_p->FLAGS = 0x200;
}



void interrupt dispatcher(){
 	
 	static pcb* head; //HEAD of the ready queue
 	
 	
 	if (sp_save == NULL)
 	{
 	  ss_save = _SS;
		sp_save = _SP;
		//ask for the first element in ready queue
		head = readyQ->head;
		cop = Remove_PCB(head);
				if (cop != NULL) { //remove the element located at the head of the queue
				cop->state  = RUNNING;
				_SS         = FP_SEG(cop->stack_top);
				_SP         = FP_OFF(cop->stack_top);	
				
			  } 
  } else {
			cop= NULL;
			_SS         = ss_save;
			_SP         = sp_save;
			sp_save     = NULL;
			ss_save     = NULL;
		} 
}


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
        check = 0;
     }
     
     if (strcmp(name,show_c) == 0) {
        check = 0;
     }
     
     if (strcmp(name,block_c) == 0) {
        check = 0;
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
        check = 0;
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
     else {
        printf("\nCommand available.\n\n");
        return check;
     }
}

void ver () {
	printf("This is the version #2.4.65 of GeckOs\n");
	printf("Module #R2\n");
	printf("Last Modified: 10/08/2010\n");
}

void removeNL(char *s) {
	s[strcspn(s, "\n")] = '\0';
}

void terminate() {
	printf("Goodbye.\n");
	sys_exit();
}
