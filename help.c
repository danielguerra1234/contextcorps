#include <stdlib.h>
#include <stdio.h>

/***************************
 *Name:       help
 *Parameters: char*
 *Calls:      none  
 *Returns:    void
 *Desc:       This function displays a list and small description if the word
 *            "help" is passed in.  If help and string is passed in, it shows 
 *            detailed information about the function.     
 */

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
        "pcb: commands pertaining to the pcb. Enter 'help pcb' to see more info:DEPRECATED\n"
        "block: blocks the desired pcb: DEPRECATED\n"
        "unblock: unblocks the desired pcb: DEPRECATED\n"
        "suspend: suspends the desired pcb\n"
        "priority: commands pertaining to pcb priority. Enter 'help priority' to see more info\n"
        "resume: resumes the desire pcb\n"
        "show: commands pertaining to display pcb information. Enter 'help show' for more information\n"
        "dispatch: start round robin dispatching: DEPRECATED\n");
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
    return;
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
  if (strcmp(command, "dispatch") == 0) {
    printf("The dispatch command starts the round robin execution. \n");
    return;
  }
  if (strcmp(command, "load") == 0) {
    printf("The load command loads a program into a pcb.  Takes process_name, program_name(without extension), priority, and directory_name.\n");
    return;
  }
	printf("Could not find help for command: %s", command);
}
