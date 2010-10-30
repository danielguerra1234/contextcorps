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



int main(void) {
	int inputLength = 100;
	int *lengthPtr = &inputLength;
	char input[100];
	int exitCode = 0;
	init();
	do {
		printf("%s ", prompt);
		//sys_call();
		fgets(input,*lengthPtr,stdin);
		
//		sys_req(READ,TERMINAL,input,inputLength); //I'll eventually figure out how this works...
    printf("INput cont: %s\n", input);
		removeNL(input);
		printf("INput cont: %s\n", input);
		exitCode = parseCommand(input);
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
	sys_init(MODULE_R2);
	readyQ = initQueue(readyQ);
	blockQ = initQueue(blockQ);
	//initStruct(suspendreadyQ);
	//initStruct(suspendblockQ);
}

//Structure functions for PCB and QUEUE
queue* initQueue(queue* newQ) {
  
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
}

//############PCB Functions####################
//PCB error codes start in the 400
pcb *allocatePcb(){
	int size = sizeof(pcb);
	int *address;
	pcb *pcb_ptr;
	
	pcb_ptr = (pcb*)sys_alloc_mem(size);//use sys_alloc_mem
	address = sys_alloc_mem(1024);
	pcb_ptr->stack_base = address;
	pcb_ptr->stack_top = pcb_ptr->stack_base + size;
	return (pcb_ptr);
}

void Free_PCB(pcb *ptr) { //pcb pointer
 sys_free_mem(ptr->stack_base);
 sys_free_mem(ptr);

}
   
pcb* Setup_PCB(char *name, int priorityc, int classc) {
	pcb* pcb1;
	int class = classc;
	
	printf("SetupPCB: Name: %s, Prior: %d, Class: %d\n", name, priorityc, classc);
	
	if (name == NULL) {
		errorCodeTranslator(ERR_PCB_NONAME);
		return;
	}
	if (strlen(name) > 15) {
		errorCodeTranslator(ERR_PCB_NMETOLONG);
		return;
	}
/*	if (Find_PCB(name) != NULL) {
		errorCodeTranslator(ERR_PCB_NMEEXISTS);
		return;
	}  */
	if(priorityc >127 || priorityc<-128) {
		errorCodeTranslator(ERR_PCB_INVPRIORITY);
		return;
	}
	if (class!= 1 && class!= 2) {
		errorCodeTranslator(ERR_PCB_INVCLASS);
		return;
	}
	pcb1 = allocatePcb();
	pcb1->process_name= name;
	sprintf("%s, %s", name, pcb1->process_name);
	pcb1->priority = priorityc;
	pcb1->process_class = classc;
	pcb1->state = READY;
	pcb1->exe_addr = NULL;
	printf("PCB succesfully created\n\n");
	Insert_PCB(pcb1);
	Show_PCB(name);
	return pcb1;
}

pcb* Find_PCB(char *name){
	 pcb* walk = readyQ->head;
	 printf("Find_PCB Function executing with name: %s\n",name);
	 printf("Find_PCB walk var: %s\n", walk->process_name);
	 
	/* if (walk == NULL) {
    printf("walk is null\n");
    return NULL;
    }
    
	 while(walk != NULL) {
	  printf("Ready Queue Loop Start.\n");
		if (strcmp(walk->process_name,name) == 0) {
      return walk;
      }
      
		if (walk->next != NULL) {
		printf("Ready Queue Next walk.\n");
    walk = walk->next;
    }
	}
	
	 walk = blockQ->head;
	 while(walk != NULL) {
		if (strcmp(walk->process_name,name) == 0) return walk;
		if (walk->next != NULL) walk = walk->next;
	}  */
	
	printf("FIND_PCB finished.\n");
	return NULL;
}

void priority_insert(queue* q, pcb *ptr){
  pcb* prev;
  //pcb * next;
  pcb* walk; 
  int priority;
  priority = ptr->priority;
  walk = q->head;
  
  if (sizeof(q)>=buffer)
    printf("You cannot insert in this queue, it is already full");
  else
    while(walk != NULL)
    {
    if(priority > walk->priority) {
      prev= walk->prev;
      prev->next = ptr;
      ptr->prev = prev;
      walk->prev = ptr;
      ptr->next = walk;
    }
  else 
    walk=walk->next;
  }
}

void FIFO_insert(queue* q, pcb *ptr){
  if (sizeof(q)>=buffer) printf("You cannot insert in this queue, it is already full");
  else {
    (q->tail)->next= ptr;
    ptr->prev=q->tail;
    q->tail= ptr;
  }

}


void Insert_PCB(pcb* pcb1){
  int state;
  state = pcb1->state;
  if (state == RUNNING || state == READY)
    priority_insert(readyQ, pcb1);
    
  if (state == BLOCKED)
    priority_insert(blockQ, pcb1);
    
  if(state == SUSPENDED_READY)
    FIFO_insert(suspendreadyQ, pcb1);
    
  if(state == SUSPENDED_BLOCKED)
    FIFO_insert(suspendblockQ, pcb1);
}


pcb* Remove_PCB(pcb *pcb1){
  pcb * prev;
  pcb * next;
  prev= pcb1->prev;
  pcb1->prev= NULL;
  next=pcb1->next;
  pcb1->next=NULL;
  next->prev= prev;
  prev->next= next;
  return pcb1;
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
  if (pcbPtr == NULL) {
    printf("PCB: %s does not exist.\n",name);
    return;
  } else {
    pcbPtr2 = Find_PCB(name);
    if (pcbPtr2->state == RUNNING) {
      temp = "RUNNING";  
    } 
    else if (pcbPtr2->state == READY) {
      temp = "READY";
    }
    else if (pcbPtr2->state == BLOCKED) {
      temp = "BLOCKED";
    }
    else if (pcbPtr2->state == SUSPENDED_BLOCKED) {
      temp = "SUSPENDED BLOCKED";
    }
    else if (pcbPtr2->state == SUSPENDED_READY) {
      temp = "SUSPENDED READY";
    }
    else {
      printf("Invalid State.\n");
      return;
    }
    
    
    printf("Name: %s\n" 
            "Priority: %d\n" 
            "Process_Class: %d\n" 
            "State: %s\n",pcbPtr2->process_name, pcbPtr2->priority, pcbPtr2->process_class, temp);
    return;
  }
}




//NOTE: a return value other than 0 will result in program exit
int parseCommand(char *commandString) {
	//TODO: look to see what else I can do wtih strtok
	//TODO: turn these into an actual struct...preferably an array I can just loop over, if that's possible
	char *command = strtok(commandString, " ");
	char *arg1 = strtok(NULL, " ");
	char *arg2 = strtok(NULL, " ");
	char *arg3 = strtok(NULL, " ");
	char *arg4 = strtok(NULL, " ");

  printf("commandString: %sarg1: %s, arg2: %s, arg3: %s, arg4: %s\n", commandString, arg1, arg2, arg3, arg4);

//	printf("command: %s\narg1: %s\narg2: %s\narg3: %s\narg4: %s\n", command,arg1,arg2,arg3,arg4); //XXX: DEBUG
	if (command == NULL) return 0; //if no command given, simply return...since it's not really an issue
	if (strcmp(command,"help") == 0) {
		help(arg1);
		return 0;
	}
	if (strcmp(command,"version") == 0) {
		version();
		return 0;
	}
	if (strcmp(command,"clear") == 0) {
		clearScreen();
		return 0;
	}
	if (strcmp(command,"cd") == 0) {
		if (arg1 == NULL) printf("cd requires a path\n");
		else changeDir(arg1);
		return 0;
	}
	if (strcmp(command, "setprompt") == 0) {
		if (arg1 == NULL) puts("Prompt argument cannot be blank");
		else if (strlen(arg1) > 20) puts("Prompt argument must be 20 characters or less");
		else setPrompt(arg1);
		return 0;
	}
	if (strcmp(command,"date") == 0) {
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
	if (strcmp(command,"dir") == 0) {
		listDir();
		return 0;
	}
	
	if (strcmp(command, "pcb") == 0) {
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
				return 0;
			}
		}
		if (strcmp(arg1, "-d") == 0) {
		  if (arg2 == NULL) {
			puts("Deleting a pcb requires a name");
			return 0;
			} else {
				printf("Deleting pcb '%s'\n",arg2);
				Free_PCB(Find_PCB(arg2));
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
  
  if (strcmp(command, "block") == 0) {
    if (arg1 == NULL) {
      printf("block command requires name argument\n");
      return 0;
    }
    else {
      //add function call when function is ready
      printf("blocking %s\n",arg1);
      return 0;
    }
  }
  
  if (strcmp(command, "unblock") == 0) {
    if (arg1 == NULL) {
      printf("Unblock command requires a name\n");
      return 0;
    }
    else {
      //add function call when function is ready
      printf("Unblocking %s\n",arg1);
      return 0;
    }
  }
  
  if (strcmp(command, "suspend") == 0) {
    if (arg1 == NULL) {
      printf("Suspend requires a name\n");
      return 0;
    }
    else {
      //add function when function is ready
      printf("Suspending %s\n",arg1);
    }
  }
  
  if (strcmp(command, "resume") == 0) {
    if (arg1 == NULL) {
      printf("Resume requires a name\n");
      return 0;
    }
    else {
      printf("Resuming %s\n",arg1);
      return 0;
    }
  }
  
  if (strcmp(command, "priority") == 0) {
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
  
  if (strcmp(command, "show") == 0) {
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
        printf("calling show pcb function\n");
        Show_PCB(arg2);
        return 0;
      }
    }
    if (strcmp(arg1, "-a") == 0) {
      if (arg2 != NULL || arg3 != NULL || arg4 != NULL) {
        printf("show command argument -a takes no arguments\n");
        return 0;
      }
      else {
        //add function call
        printf("calling show with -a argument\n");
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
        return 0;
      }
    }
    else{
      printf("Argument '%s' is an invalid argument\n",arg1);
      return 0;
    }  
  }
  
	
	if (strcmp(command,"exit") == 0) {
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


pcb* move_pcb(pcb* ptr){

	pcb* head;
	head= Remove_PCB(ptr);
	head->state= RUNNING; //if it doesn't work use integer: actual number
	return (head);

}
 /*
void interrupt sys_call() {
    params* param_p;
    pcb* cop;
    //param_p = ((MK_FP(_SS,_SP) + sizeof(context)));
    cop-> stack_top = (unsigned char *)MK_FP(_SS, _SP);
    param_p = (params*)(cop->stack_top + sizeof(context));

    if (param_p->op_code == IDLE) {
        printf("Call routine to set process into ready queue");
	return 0;
    } else if (param_p->op_code == EXIT) {
	printf("call routine to set process to exit");
	return 0;
    } else {
        printf("Call dispatcher to send process not available in this module");
        return -1;
    }
    
    //dispatcher();
}       

void testn_R3(){

	pcb* test1;
	pcb* test2;
	pcb* test3;
	pcb* test4;
	pcb* test5;
	
	test1 = Setup_PCB("test1", 1, one);
	test1->exe_addr = &test1_R3;
	
	test2 = Setup_PCB("test2", 1, one);
	test2->exe_addr = &test2_R3;
	
	test3 = Setup_PCB("test3", 1, one);
	test3->exe_addr = &test3_R3;
	
	test4 = Setup_PCB("test4", 1, one);
	test4->exe_addr = &test4_R3;
	
	test5 = Setup_PCB("test5", 1, one);
	test5->exe_addr = &test5_R3;


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

void dispatch(){
 	pcb* cop;
 	//stack_p* stack;
 	unsigned int sp_save, ss_save;
 	if (sp_save== NULL){
 	  pcb* head;
	 	ss_save = _SS;
		sp_save = _SP;
		
		//ask for the first element in ready queue
		head= readyQ->head;
		cop= move_pcb(head);
				if(cop != NULL) { //remove the element located at the head of the queue
				cop = head;
				_SS = (int)cop->stack_base;
				_SP = (int)cop->stack_top;	
			} else {
				cop= NULL;
				_SS=ss_save;
				_SP=sp_save;
			}
	 }
}
*/


void version () {
	printf("This is the version #1.1.33 of GeckOs\n");
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
