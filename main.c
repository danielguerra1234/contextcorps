#include <stdio.h>
#include <stdlib.h>
#include "mpx_supt.h"
#include <string.h>

date_rec *date_p; 
void displayDate();
void changeDate(int year, int month, int day);

int main(void) {
   char tester[50] = "Testing 1 2 3\n\0";
   int *testbug = strlen(tester);
   sys_init(MODULE_R1);
   //displayDate();
   //displayDate();
   //errorCodeTranslator(ERR_SUP_INVDEV);
   clrscr();
   changeDate(3009, 9, 9);
return 1;
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
}

void displayDate (){
    
    
    
    sys_get_date(date_p);
    printf("The year is : %d\n",date_p->year);
    printf("The month is : %d\n",date_p->month);
    printf("The day is : %d\n",date_p->day);
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
        if((month== 1 || month==3 || month==5 || month==7 || month== 8 || month==10 || month==12) || day>31 ) {
                printf("Error Encountered: 31 day error");
                return;
                }
        if((month== 4 || month==6 || month==9 || month==11) || day>30)  {
                printf("Error Encountered: 30 day error");
                return;
                }
        if (month == 2) {
                if (year%4 == 0 && day > 30) {
                  printf("Error Encounterd: Leap Year Error");
                  return;
                  }
                else if(month== 2 && day>29)    {
                  printf("Error Encountered: February Error");
                  return;
                  }// End else if
              } //End inside if
          } //End Outer if
     else {
      printf("Invalid Year");  
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
/* 
int help(int i){
    printf("The commands:\n");
    printf("1- Version \n");
    printf("2- Display the date \n");
    printf("3- Change the date \n")
    printf("4- Display MPX Directory \n");
    printf("5- Exit");
    switch(i){
        case 1:
        printf("This command is 'Version', it displays the version and module of GeckOS");
        case 2:
        printf("This command is 'displayDate', it prints to the screen the current date of the system ");
        case 3:
        printf("This command is 'changeDate', it takes integers year, month and date; verifies they are correct, and change the date of the system");
        case 4:
        printf("This command is 'dmd', it displays information about all the files: typically the name and size in bytes");
        case 5:
        printf("This command is 'exit', it stops GeckOS from running");
    }
         
    return  i;
}   
 
void version (){
    printf("This is the version #1.0 of GeckOs");
    printf("Module #R1")
    printf("Last Modified: 09/15/2010");
    return 0;
}
*/