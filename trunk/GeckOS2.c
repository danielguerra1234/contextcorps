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

#include "MPX_SUPT.H"

#define SIZE 10000


void init();
int parseCommand(char *commandString);
void displayDate();
void changeDate(int year, int month, int day);
int help(char *command);
int errorCodeTranslator(int code);
void version();
void removeNL(char *s);
void terminate();
void clearScreen();
void listDir();
void dmd();
void changeDir(DIR *arg);

long buffer_length = 100;
unsigned char buffer[SIZE];

DIR *dp;

struct dirent *ep;

date_rec *date_p;
char prompt[20] = "~> ";

int main(void) {
	int inputLength = 100;
	int *lengthPtr = &inputLength;
	char input[100];
	int exitCode = 0;
	init();
	do {
		printf("%s ", prompt);
		fgets(input,*lengthPtr,stdin);
//		sys_req(READ,TERMINAL,input,inputLength);
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
	//	sys_init(MODULE_R1);
}

int parseCommand(char *commandString) {
	//TODO: actually look to see what this parses on...I'm just using it as-is because for now it works
	//TODO: turn these into an actual struct...preferably an array I can just loop over, if that's possible
	char *command = strtok(commandString, " -");
	char *arg1 = strtok(NULL, " -");
	char *arg2 = strtok(NULL, " -");
	char *arg3 = strtok(NULL, " -");
	char *arg4 = strtok(NULL, " -");

//	printf("command: %s\narg1: %s\narg2: %s\narg3: %s\narg4: %s\n", command,arg1,arg2,arg3,arg4); //XXX: DEBUG
	if (command == NULL) return 0; //if no command given, simply return...since it's not really an issue
	if (strcmp(command,"help") == 0) {
		int returnCode = help(arg1);
		if (returnCode < 0) printf("Could not find command '%s'\n",arg1);
	}
	else if (strcmp(command,"version") == 0) {
		version();
	}
	else if (strcmp(command,"clear") == 0) {
    clrscr();
  }
  else if (strcmp(command,"cd") == 0) {
    if (arg1 == NULL) {
      printf("cd required a path\n");
      return 0;
    }
    else if(arg1 != NULL) {
      changeDir(arg1);
      return 0;
    }
  }
  
	else if (strcmp(command,"date") == 0) {
		puts("calling date function");
		if (arg1 == NULL) displayDate(); //if the user wants to display the date
		else if (strcmp(arg1,"set") == 0) { //if the user wants to set the date
			if (arg2 != NULL && arg3 != NULL && arg4 != NULL) {
				changeDate(atoi(arg2),atoi(arg3),atoi(arg4));
			}
		} else { //invalid argument
			printf("Argument '%s' is not defined for %s", arg1, command);
		}
	} else if (strcmp(command,"dir") == 0) {
		puts("Calling dir function");
		listDir();
	} else if (strcmp(command,"exit") == 0) {
		return 1;
	} else printf("%s is not a valid command. For a list of valid commands, type 'help'\n", command);
	return 0; //by getting this far, a valid command was passed and run, so let's get another one
}

void listDir() {
       
       if (dp != NULL)
         {
           while (ep = readdir (dp))
             puts (ep->d_name);
           (void) closedir (dp);
         }
       else
         perror ("Couldn't open the directory");
     
       return 0;
}

void changeDir(DIR *arg) {
      dp = opendir(arg);
      return 0;
}

int errorCodeTranslator(int code) {

	switch (code) {
	case (-100):
	printf("Invalid Device Code\n");
	  return 0;

	case(-101):
	  printf("Invalid Op Code\n");
	  return 0;

	case(-102):
	  printf("Invalid Character Position\n");
	  return 0;

	case (-103):
	  printf("Read has failed\n");
	  return 0;

	case (-104):
	  printf("Write has failed\n");
	  return 0;

	case (-105):
	  printf("Invalid Memory Pointer\n");
	  return 0;

	case (-106):
	  printf("Free Memory Failed\n");
	  return 0;

	case (-107):
	  printf("Invalid Date\n");
	  return 0;

	case (-108):
	  printf("Date Not Changed\n");
	  return 0;

	case (-109):
	  printf("Invalid Directory\n");
	  return 0;

	case (-110):
	  printf("Directoy Open\n");
	  return 0;

	case (-111):
	  printf("No Directory is Open\n");
	  return 0;

	case (-112):
	  printf("No More Directory Entries\n");
	  return 0;

	case (-113):
	  printf("Name Buffer was Too Long\n");
	  return 0;

	case (-114):
	  printf("The Directory is Closed\n");
	  return 0;

	case (-115):
	  printf("Loading Failed\n");
	  return 0;

	case (-116):
	  printf("File Not Found\n");
	  return 0;

	case (-117):
	  printf("File Invalid\n");
	  return 0;

	case (-118):
	  printf("Program Size Error\n");
	  return 0;

	case (-119):
	  printf("Invaliid Load Address\n");
	  return 0;

	case (-120):
	  printf("Memory allociation Error\n");
	  return 0;

	case (-121):
	  printf("Free Memory Error\n");
	  return 0;

	case (-122):
	  printf("Invalid Handler Address\n");
	  return 0;
	}
	return -1; //failed to find error code
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

void changeDate(int year, int month, int day) {
	int i;
	date_rec reset;
	date_rec *save_date;
	printf("Original date:\n");
	displayDate();

	save_date->year = year;
	save_date->month = month;
	save_date->day = day;

	if (year > 1000 || year < 2013) {
		if((month== 1 || month==3 || month==5 || month==7 || month== 8 || month==10 || month==12) && day>31 ) {
				printf("Error Encountered: %d is greater than 31\n",save_date->day);
				return;
				}
		if((month== 4 || month==6 || month==9 || month==11) || day>30)  {
				printf("Error Encountered: %d is greater than 30\n",save_date->day);
				return;
			}
		if (month == 2) {
				if (year%4 == 0 && day > 30) {
				  printf("Error Encounterd: %D is an invalid day for month %d\n",save_date->day, save_date->month);
			  return;
				  }
				else if(month== 2 && day>29)    {
				  printf("Error Encountered: %d is greater than 28 for month %d\n",save_date->day, save_date->month);
				  return;
				  }// End else if
			  } //End inside if
		  } //End Outer if
	 else {
	  printf("Invalid Year: %d",save_date->year);
	}
	i = sys_set_date(save_date);
	if (save_date->year  == date_p->year && save_date->month  == date_p->month && save_date->day  == date_p->day)
		 i= ERR_SUP_DATNCH;
	if ( i == ERR_SUP_INVDAT)
		  printf("Error Code: %d\n Invalid Date\n",ERR_SUP_INVDAT);
	if (i == OK)   {
		  printf("Date Successfully Set\n");
		  printf("Date after setting:\n");
		  displayDate();
		}
}

int help(char *command){
	if (command == NULL) {
		printf("Current list of commands:\n"
				"version : display GeckOS version\n"
				"date : display or set date. Enter 'help date' to see more info'\n"
				"dir : displays the current director of GeckOS\n"
				"clear : clears screen\n"
				"cd : changes directory -- Does not require -arguement\n"
				"exit : closes the system\n");
		return 0;
	}
	if (strcmp(command,"help") == 0) {
		printf("Congrats, you've figured out how this command works! Now use it on something useful.\n");
	}
	if (strcmp(command,"date") == 0) {
		printf("This command displays the current date. Use with argument '-set year month day' to change the date instead, where year, month, and day are all integers.\n");
		return 0;
	}
	if (strcmp(command,"cd") == 0) {
    printf("Changes the directory\n");
    return 0;
  }
	if (strcmp(command,"clear") == 0) {
    printf("This command simply clears the screen\n");
    return 0;
  }
	if (strcmp(command,"version") == 0) {
		printf("This command displays the current version of GeckOS.\n");
		return 0;
	}
	if (strcmp(command,"dir") == 0) {
		printf("This command displays information about all the files: typically the name and size in bytes\n");
		return 0;
	}
	if (strcmp(command,"exit") == 0) {
		printf("This command closes the program\n");
		return 0;
	}
	return -1; //if it got this far, the command was not found
}
void version () {
	printf("This is the version #1.0 of GeckOs\n");
	printf("Module #R1\n");
	printf("Last Modified: 09/15/2010\n");
}

void removeNL(char *s) {
	s[strcspn(s, "\n")] = '\0';
}

void terminate() {
	printf("Goodbye.\n");
//	sys_exit();
}
