#ifndef GeckOS2
#define GeckOS2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <conio.h>
#include <dirent.h>
#include <dos.h>
#include <dir.h>
#include <errno.h>
#include <alloc.h>
#include "COMHAN.h"

#include "MPX_SUPT.H"
#include "Errors.h"
#include "Help.h"
#include "procsr3.h"
#include "r5.h"
//Structure:  QUEUE and PCB PROTOTYPES AND 

#define SIZE 10000
#define SYSTEM 1
#define PROCESS 2
#define APPLICATION 3
#define RUNNING 100
#define READY 101
#define BLOCKED 102
#define SUSPENDED_READY 103
#define SUSPENDED_BLOCKED 104

typedef struct{ 
    unsigned int BP, DI, SI, DS, ES; 
    unsigned int DX, CX, BX, AX; 
    unsigned int IP, CS, FLAGS; 
    } context;
    
typedef struct {
    char* buf_name;
    long size;
} paging;

typedef struct{
    char process_name[50];
    int priority;
    int state;
    int process_class;

    int index;

    struct pcb *next;
    struct pcb *prev;

    unsigned char* stack_top;
    unsigned char* stack_base;
	  unsigned char* stack_size;
	  unsigned int* stack_p;

    int memory;
    unsigned char* load;
    unsigned char* execution;

}pcb;

typedef struct{
    char* name;
    int nodes;
    pcb *head;
    pcb *tail;
    int index;
}queue;

typedef struct {
    int op_code;
    int device_id;
    unsigned char* buf_addr;
    int* count_addr;
    } params;

typedef struct {
    char name[50];
    pcb* requestor;
    int type;
    char* location;
    int* counter;
    struct IOD* next;
    }IOD;

typedef struct {
    int event_flag;
    int count;
    struct IOD* head;
    struct IOD* tail;
    } IOCB;
 
// R1 Prototypes
void init();

void displayDate();
void changeDate(char *year, char *month, char *day);
//int errorCodeTranslator(int code);
void ver();
void removeNL(char *s);
void terminate();
void clearScreen();
void listDir();
void dmd();
void changeDir(DIR *arg);


//R2 Prototypes
pcb *allocatePcb();
void blocked_add(pcb *node);
void Free_PCB(pcb *ptr);
pcb* Setup_PCB(char name[], int priorityc, int classc);
pcb* Find_PCB(char *name);
pcb* Find_PCB_Blocked(char* name);
pcb* Find_PCB_Ready(char* name);
pcb* Find_PCB_Suspended_Ready(char* name);
pcb* Find_PCB_Suspended_Blocked(char* name);
void Insert_PCB(pcb*);
pcb* Remove_PCB(pcb*);
void Set_Priority(char*, int);
queue* initQueue(queue*, char*);
void Show_PCB(char*);
void Show_All();
void Show_Ready();
void Show_Blocked();
void block(char* name);
void unblock(char* name);
pcb* move_pcb(pcb* ptr);
void show_all();
void show_ready();
void show_blocked();

// R3 Prototypes
void suspend(char* name);
void resume(char* name);
void interrupt sys_call(void);
pcb* save_context(pcb*);
void interrupt dispatcher();
void testn_R3();
void terminate_process(char* name);

//R4 Prototypes
pcb* Load_Program(char*, char*, int, char*);

//R6 Prototypes
void FIFO_insert_iod(IOCB* q, IOD* ptr);
void init_R6();
void init_iocb();
void process_io(IOD*,int);
void io_scheduler();
void io_cleanup();
void q_cleanup(queue* q);

//Extra Credit



#endif
