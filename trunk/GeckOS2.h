
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

#include "MPX_SUPT.H"
#include "Errors.h"
#include "Help.h"
#include "procsr3.h"

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
    char stack[1024];
	unsigned char *stack_top;
	unsigned char *stack_base;
}stack_area;


typedef struct{
    int mem_size;
    unsigned char* load_address;
    unsigned char* exec_address;
}memory;

typedef struct{
    char process_name[30];
    int priority;
    int state;
    int process_class;

    int index;

    stack_area process_stack_info;

    unsigned char* next_one;

    struct pcb *next;
    struct pcb *prev;

    unsigned char* stack_top;
    unsigned char* stack_base;
	  unsigned char* stack_size;
	  unsigned int* stack_p;

    memory process_memory_info;

}pcb;

typedef struct {
    int id;
    char *command;
    struct history *next;
} history;

typedef struct {
    history* head;
    int index;
} hist_stack;

typedef struct{
    unsigned char* queue_element;
    unsigned char* next_queue_descriptor;
    unsigned char* previous_queue_descriptor;
}queue_descriptor;

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

typedef struct{ 
    unsigned int BP, DI, SI, DS, ES; 
    unsigned int DX, CX, BX, AX; 
    unsigned int IP, CS, FLAGS; 
    } context; 


void interrupt sys_call(void);
void save_context();
void interrupt dispatcher();

//PCB PROTOTYPES
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

//Show Functions
void Show_PCB(char*);
void Show_All();
void Show_Ready();
void Show_Blocked();
void block(char* name);
void unblock(char* name);
void suspend(char* name);
void resume(char* name);
void testn_R3();



void init();
int parseCommand(char *commandString);
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
void setPrompt(char *s);
pcb* move_pcb(pcb* ptr);
void show_all();
void show_ready();
void show_blocked();
int command_check(char*);

#endif
