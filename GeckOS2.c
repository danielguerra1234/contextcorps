/*
 ============================================================================
 Name        : GeckOS2.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

//#include "Errors.h"
#include "MPX_SUPT.H"
#include "Errors.h"
#define SIZE 10000

#define SYSTEM 1
#define PROCESS 2
#define RUNNING ru
#define READY rd
#define BLOCKED b
#define SUSPENDED_READY sr
#define SUSPENDED_BLOCKED sb

typedef struct{
    char stack[1024];
}stack_area;


typedef struct{
    int mem_size;
    unsigned char* load_address;
    unsigned char* exec_address;
}memory;

typedef struct{
    char process_name[10];
    int priority;
    
    enum processclass {
        one, 
        two
    }processclass;
    
    enum state {
        ru, 
        rdm, 
        bm, 
        sr, 
        sb
    }state;

    stack_area process_stack_info;
    unsigned char* stack_top;
    unsigned char* stack_base;
    memory process_memory_info;
    
}pcb;

typedef struct{
    unsigned char* queue_element;
    unsigned char* next_queue_descriptor;
    unsigned char* previous_queue_descriptor;
}queue_descriptor;

typedef struct{
    int nodes;
    unsigned char* head_queue;
    unsigned char* tail_queue;
}queue;



void init();
int parseCommand(char *commandString);
void displayDate();
void changeDate(char *year, char *month, char *day);
void help(char *command);
int errorCodeTranslator(int code);
void version();
void removeNL(char *s);
void terminate();
void clearScreen();
void listDir();
void dmd();
void changeDir(DIR *arg);
void setPrompt(char *s);

long buffer_length = 100;
unsigned char buffer[SIZE];

DIR *dp;

struct dirent *ep;

date_rec *date_p;
char *prompt = "~> ";

int main(void) {
	int inputLength = 100;
	int *lengthPtr = &inputLength;
	char input[100];
	int exitCode = 0;
	init();
	do {
		printf("%s ", prompt);
		fgets(input,*lengthPtr,stdin);
//		sys_req(READ,TERMINAL,input,inputLength); //I'll eventually figure out how this works...
		removeNL(input);
		exitCode = parseCommand(input);
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
	sys_init(MODULE_R1);
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
    if (arg1 == NULL || arg2 == NULL || arg3 == NULL || arg4 == NULL) {
      printf("createpcb requires 4 arguments name, class, priority\n");
      return 0;
    }
    if (strcmp(arg1,"-c") == 0) {
      if (arg2 != NULL && arg3 != NULL) {
        //add function call once functions are ready
        printf("Calling createpcb\n");
        return 0;
      }
    }
    if (strcmp(arg1, "-d") == 0) {
      if (arg2 != NULL && arg3 != NULL) {
        //add function call once functions are ready
        printf("Calling deletepcb\n");
        return 0;
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
      printf("calling block function\n");
      return 0;
    }
  }
  
  if (strcmp(command, "unblock") == 0) {
    if (arg1 == NULL) {
      printf("unblock command requires name argument\n");
      return 0;
    }
    else {
      //add function call when function is ready
      printf("calling unblock function\n");
      return 0;
    }
  }
  
  if (strcmp(command, "suspend") == 0) {
    if (arg1 == NULL) {
      printf("suspend command requires name argument\n");
      return 0;
    }
    if (arg2 != NULL || arg3 != NULL || arg4 != NULL) {
      printf("suspend command takes only 1 argument\n");
      return 0;
    }
    else {
      //add function when function is ready
      printf("calling suspend command\n");
    }
  }
  
  if (strcmp(command, "resume") == 0) {
    if (arg1 == NULL) {
      printf("resume command takes name argument\n");
      return 0;
    }
    if (arg2 != NULL || arg3 != NULL || arg4 || NULL) {
      printf("resume command takes only 1 argument\n");
      return 0;
    }
    else {
      printf("calling resume command\n");
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

void help(char *command){
	if (command == NULL) {
		printf("Current list of commands:\n"
				"version : display GeckOS version\n"
				"date : display or set date. Enter 'help date' to see more info'\n"
				"dir : displays the current director of GeckOS\n"
				"clear : clears screen\n"
				"cd : changes directory -- Does not require -arguement\n"
				"exit : closes the system\n"
				"setprompt : sets the prompt with the given argument\n"
        "pcb: commands pertaining to the pcb. Enter 'help pcb' to see more info\n"
        "block: blocks the desired pcb\n"
        "unblock: unblocks the desired pcb\n"
        "suspend: suspends the desired pcb\n"
        "priority: commands pertaining to pcb priority. Enter 'help priority' to see more info\n"
        "resume: resumes the desire pcb\n"
        "show: commands pertaining to display pcb information. Enter 'help show' for more information\n");
		return;
	}
	if (strcmp(command,"help") == 0) {
		printf("Congrats, you've figured out how this command works! Now use it on something useful.\n");
		return;
	}
	if (strcmp(command,"date") == 0) {
		printf("This command displays the current date. Use with argument 'set year month day' to change the date instead, where year, month, and day are all integers.\n");
		return;
	}
	if (strcmp(command,"cd") == 0) {
		printf("Changes the directory\n");
		return;
	}
	if (strcmp(command,"clear") == 0) {
		printf("This command simply clears the screen\n");
		return;
	}
	if (strcmp(command,"version") == 0) {
		printf("This command displays the current version of GeckOS.\n");
		return;
	}
	if (strcmp(command,"dir") == 0) {
		printf("This command displays information about all the files: typically the name and size in bytes\n");
		return;
	}
	if (strcmp(command,"exit") == 0) {
		printf("This command closes the program\n");
		return;
	}
	if (strcmp(command,"setprompt") == 0) {
		printf("This command changes the prompt to the provided argument. The argument must be 20 characters or less\n");
		return;
	}
	if (strcmp(command,"pcb") == 0) {
    printf("This command will allow you to create or delete a pcb.  Use '-c name' for creation and '-d name' for deletion where name is the name of the pcb.\n");
    return;
  }
  if (strcmp(command,"block") == 0) {
    printf("The block command allows the user to block a pcb.  This command accepts a name for an argument.\n");
    return 0;
  }
  if (strcmp(command,"unblock") == 0) {
    printf("The unblock command allows the user to unblock a pcb.  This command accepts a name for an argument.\n");
    return;
  }
  if (strcmp(command, "suspend") == 0) {
    printf("The suspend command suspends the pcb the user specifies.  This command accepts a name for an argument.\n");
    return;
  }
  if (strcmp(command, "resume") == 0) {
    printf("The resume command resumes the pcb the user specifies. This command accepts a name for an argument.\n");
    return;
  }
  if (strcmp(command, "priority") == 0) {
    printf("This command controls the priority of a pcb.  It can be used with the '-s name priority' option.\n");
    return;
  }
  if (strcmp(command, "show") == 0) {
    printf("The show command displays information of the pcb(s).  The different options are -p(pcb) -a(all) -r(ready) -b(blocked).\n");
    return;
  }
	printf("Could not find help for command: %s", command);
}
void version () {
	printf("This is the version #1.0.23 of GeckOs\n");
	printf("Module #R1\n");
	printf("Last Modified: 09/17/2010\n");
}

void removeNL(char *s) {
	s[strcspn(s, "\n")] = '\0';
}

void terminate() {
	printf("Goodbye.\n");
	sys_exit();
}
