/*
 * Errors.c
 *
 *  Created on: Sep 20, 2010
 *      Author: daniel
 */

#include "Errors.h"

/***************************
 *Name:       errorCodeTranslator
 *Parameters: int
 *Calls:      none  
 *Returns:    int
 *Desc:       This function accepts an error code int and displays the proper 
 *            message   
 */

int errorCodeTranslator(int code) {
	printf("ERROR (%d): ", code);
	switch (code) {
	case (-101):
		printf("Invalid Device Code\n");
		return 0;

	case (-102):
		printf("Invalid Op Code\n");
		return 0;

	case (-103):
		printf("Invalid Character Position\n");
		return 0;

	case (-104):
		printf("Read Failed\n");
		return 0;

	case (-105):
		printf("Write Failed\n");
		return 0;

	case (-106):
		printf("Invalid Memory Pointer\n");
		return 0;

	case (-107):
		printf("Free Memory Failed\n");
		return 0;

	case (-108):
		printf("Invalid Date\n");
		return 0;

	case (-109):
		printf("Date Not Changed\n");
		return 0;

	case (-110):
		printf("Invalid Directory Name\n");
		return 0;

	case (-111):
		printf("Could Not Open Directory\n");
		return 0;

	case (-112):
		printf("No Directory is Open\n");
		return 0;

	case (-113):
		printf("No More Directory Entries\n");
		return 0;

	case (-114):
		printf("Name Too Long for the Buffer\n");
		return 0;

	case (-115):
		printf("Could Not Close Directory\n");
		return 0;

	case (-116):
		printf("Program Load Failed\n");
		return 0;

	case (-117):
		printf("File Not Found\n");
		return 0;

	case (-118):
		printf("File Invalid\n");
		return 0;

	case (-119):
		printf("Program Size Error\n");
		return 0;

	case (-120):
		printf("Invaliid Load Address\n");
		return 0;

	case (-121):
		printf("Unable to Allocate Memory\n");
		return 0;

	case (-122):
		printf("Unable to Free Memory\n");
		return 0;

	case (-123):
		printf("Invalid Handler Address\n");
		return 0;

	case (-124):
		printf("Invalid Month\n");
		return 0;

	case (-125):
		  printf("Invalid Year\n");
		  return 0;

	case (-126):
		  printf("Invalid Day\n");
		  return 0;

	case (-401):
		  printf("You must supply a name\n");
		  return 0;

	case (-402):
		  printf("Name already exists, choose a different one\n");
		  return 0;

	case (-403):
		  printf("Invalid priority: must be between -128 to 127\n");
		  return 0;

	case (-404):
		  printf("Must supply a class number: 1 or 2\n");
		  return 0;

	}
	printf("\nFATAL ERROR: error code supplied was not found!\n");
	return -1; //failed to find error code
}
