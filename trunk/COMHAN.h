#ifndef COMHAN
#define COMHAN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GeckOS2.h"
#include <sys/types.h>

int parseCommand(char *commandString);
int com_han();
int command_check(char*);
void setPrompt(char *s);

#endif