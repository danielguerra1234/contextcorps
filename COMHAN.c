/* ========================================================================== */
/*                                                                            */
/*   COMHAN.c                                                               */
/*   (c) 2010 Jeremy Keczan                                                          */
/*                                                                            */
/*   Description  Command Handler File                                                            */
/*                                                                            */
/* ========================================================================== */

#include "COMHAN.h"

char* commands[50];
char* cc; 
int id_com = 0;
int id_h = 0;
char *prompt = "~> ";
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

/******************************
 *Name: COMHAN
 *Parameters: none
 *Calls:    parseCommand
 *          removeNL
 *          strdup
 *          errorCodeTranslator   
 *Return:   nothing   
 *Desc:     This function accepts and parse the user input from the terminal.
 *          It also calls parseCommand which error checks the input and calls 
 *          the correct function.        
 */ 

int com_han() {
  int error;
  int inputLength = strlen(prompt);
	int lengthPtr;
	char input[80];
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
        //printf("%s ", prompt);
        error = sys_req(WRITE, TERMINAL, prompt, &inputLength);
		    fgets(input,80,stdin); 
		    //printf("error: %d", error);
		    if (error < 0) {
		        errorCodeTranslator(error);
		        return;
		    }
		    removeNL(input);  
		    //strcpy(commands[id_com], input);
		    commands[id_com] = strdup(input);
        id_com++;
        loopbreaker++;   
		    exitCode = parseCommand(input);
		}
		
		if (exitCode == 1) {
			puts("Are you sure you want to exit? (y/n)");
			//fgets(input,*lengthPtr,stdin);
			//sys_req(READ, TERMINAL, input, &lengthPtr);
			scanf("%s", &input);
			removeNL(input);
			if (strcmp(input,"N") == 0 || strcmp(input,"n") == 0 || strcmp(input,"No") == 0 || strcmp(input,"no") == 0 || strcmp(input,"NO") == 0) exitCode = 0;
		}
	} while (exitCode == 0);
	if (exitCode != 1) errorCodeTranslator(exitCode);
	terminate();	
	return 0;
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
	char* full;
	int error;
	int len; 
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
        return 0;
     } else {
        Load_Program(arg1, arg2, atoi(arg3), ".\\");
        return 0;
     }
  }
  
  if (strcmp(command, "terminate") == 0) {
      if (arg1 == NULL) {
          printf("requires argument.\n");
          return 0;
      }
      else {
          terminate_process(arg1);
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
       char* temp; 
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
              temp = strdup(commands[choice-1]);
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
		return 0;
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
        //printf("calling show with -a argument\n");
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
        //printf("calling show command with -r argument\n");
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
        //printf("calling show command with -b argument\n");
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
        //printf("calling show command with -sr argument\n");
        show_suspended_ready();
        return 0;
      }
    }
     if (strcmp(arg1, "-sb") == 0) {
      if (arg2 != NULL || arg3 != NULL || arg4 != NULL) {
        printf("show command argument -sr takes no arguments\n");
        return 0;
      }
      else {
        //printf("calling show command with -sr argument\n");
        show_suspended_blocked();
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
 *Name:       setPrompt
 *Parameters: char*
 *Calls:      none  
 *Returns:    void
 *Desc:       This functions changes the global variable prompt, XTRA CRED  
 */

void setPrompt(char *s) {
  strcat(s, " \0");
	prompt = s;
}