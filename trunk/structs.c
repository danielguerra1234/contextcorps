#include "structs.h"


//Global Variables
queue *readyQ;
queue *blockQ;
queue *suspendreadyQ;
queue *suspendblockQ;    

queue* initQueue(queue*  newQ) {
  
	newQ->nodes = 0;
	newQ->head = NULL;
	newQ->tail = NULL;
	newQ->index = NULL;
	
	return newQ;
}

long buffer_length = 100;
unsigned char buffer[SIZE];
//#############Queue Functions#################
//Queue function error codes start in the 300

void initStruct(queue* q) {
  q->nodes = 0;
	q->head = NULL;
	q->tail = NULL;
	q->index = NULL;
}

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
	
	pcb_ptr = sys_alloc_mem(size);//use sys_alloc_mem
	address = sys_alloc_mem(1024);
	pcb_ptr->stack_base = address;
	pcb_ptr->stack_top = pcb_ptr->stack_base + size;
	return (&pcb_ptr);
}

void Free_PCB(pcb *ptr) { //pcb pointer
 sys_free_mem(ptr->stack_base);
 sys_free_mem(ptr);

}
   
pcb* Setup_PCB(char *name, char *priorityc, char *classc) {
	pcb * pcb1;
	int priority = atoi(priorityc);
	int class = atoi(classc);
	if (name == NULL) {
		errorCodeTranslator(ERR_PCB_NONAME);
		return;
	}
	if (strlen(name) > 15) {
		errorCodeTranslator(ERR_PCB_NMETOLONG);
		return;
	}
	if (Find_PCB(name) != NULL) {
		errorCodeTranslator(ERR_PCB_NMEEXISTS);
		return;
	}
	if(priority >127 || priority<-128) {
		errorCodeTranslator(ERR_PCB_INVPRIORITY);
		return;
	}
	if (class!= 1 && class!= 2) {
		errorCodeTranslator(ERR_PCB_INVCLASS);
		return;
	}
	pcb1= allocatePcb();
	pcb1->process_name= name;
	pcb1->priority= priority;
	pcb1->process_class= classc;
	pcb1->state= 1;
	printf("PCB succesfully created\n");
	readyQ->head = pcb1;
	Show_PCB(name);
	return pcb1;
}

pcb* Find_PCB(char *name){
	 pcb *walk = readyQ->head;
	 if (walk == NULL) {
    printf("walk is null\n");
    return;
    }
	 while(walk != NULL) {
		if (strcmp(walk->process_name,name) == 0) return walk;
		if (walk->next != NULL) walk = walk->next;
	}
	walk = blockQ->head;
	 while(walk != NULL) {
		if (strcmp(walk->process_name,name) == 0) return walk;
		if (walk->next != NULL) walk = walk->next;
	}
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
  char state= pcb1->state;
  if (state == 'ru' || state == 'rd')
    priority_insert(readyQ, pcb1);
  if (state == 'b')
    priority_insert(blockQ, pcb1);
  if(state == 'sr')
    FIFO_insert(suspendreadyQ, pcb1);
  if(state == 'sb')
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
  //pcbPtr = Find_PCB(name);
  if (pcbPtr == NULL) {
    printf("PCB: %s does not exist.\n",name);
    return;
  } else {
    printf("Name: %s\n" 
            "Priority: %d\n" 
            "Process_Class: %s\n" 
            "State: %s\n",pcbPtr->process_name, pcbPtr->priority, pcbPtr->process_class, pcbPtr->state);
    return;
  }
}