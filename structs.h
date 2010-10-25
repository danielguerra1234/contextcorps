/* ========================================================================== */
/*                                                                            */
/*   queue.c                                                                  */
/*   (c) 2001 Jeremy Keczan : Context Corps                                   */
/*                                                                            */
/*   Description                                                              */
/*   Contains all functions, defines, prototypes, and structs to implement    */
/*   queues                                                                   */
/* ========================================================================== */

#ifndef STRUCTS
#define STRUCTS
#include <stdio.h>
#include "Help.h"
#include "Errors.h"

#define SIZE 10000
#define SYSTEM 1
#define PROCESS 2
#define APPLICATION 3
#define RUNNING ru
#define READY rd
#define BLOCKED b
#define SUSPENDED_READY sr
#define SUSPENDED_BLOCKED sb

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
    char *process_name;
    int priority;

    enum processclass {
	one,
	two
    }process_class;

    enum state {
	ru,
	rdm,
	bm,
	sr,
	sb
    }state;

    stack_area process_stack_info;

    unsigned char* next_one;

    struct pcb *next;
    struct pcb *prev;

    unsigned char* stack_top;
    unsigned char* stack_base;

    memory process_memory_info;

}pcb;

typedef struct{
    unsigned char* queue_element;
    unsigned char* next_queue_descriptor;
    unsigned char* previous_queue_descriptor;
}queue_descriptor;

typedef struct context { 
    unsigned int BP, DI, SI, DS, ES; 
    unsigned int DX, CX, BX, AX; 
    unsigned int IP, CS, FLAGS; 
    } context; 


typedef struct{
    int nodes;
    pcb *head;
    pcb *tail;
    pcb *index;
}queue;

typedef struct params {
    int op_code;
    int device_id;
    unsigned char* buf_addr;
    int* count_addr;
    } params;

//PCB PROTOTYPES
pcb *allocatePcb();
void blocked_add(pcb *node);
void Free_PCB(pcb *ptr);
pcb* Setup_PCB(char *name, char *priorityc, char *classc);
pcb* Find_PCB(char *name);
void Insert_PCB(pcb*);
pcb* Remove_PCB(pcb*);
void Set_Priority(char*, int);
queue* initQueue(queue*);

//Show Functions
void Show_PCB(char*);
void Show_All();
void Show_Ready();
void Show_Blocked();

#endif