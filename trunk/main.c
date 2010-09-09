#include <stdio.h>
#include <stdlib.h>
#include "mpx_supt.h"
#include <string.h>
int main(void) {
   char tester[50] = "Testing 1 2 3\0";
   int *testbug = strlen(tester);
   sys_init(MODULE_R1);
   sys_req(WRITE, TERMINAL, tester, testbug);
  

return 1;
}
