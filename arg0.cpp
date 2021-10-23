#include <stdio.h>
#include <sys/prctl.h>
#include "error.hpp"

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    ERROR("Try more arguements!");
    return -1;
  }
 
  int result_prctl = prctl(PR_SET_MM, PR_SET_MM_ARG_START, argv[1], 0, 0);
  ERROR_CHECK(result_prctl == -1);
  
  FILE* cmd = fopen("/proc/self/cmdline", "r");
  ERROR_CHECK(cmd == NULL);

  char test[10]{0};
  fscanf(cmd, "%10s", test);

  int result_fclose = fclose(cmd);
  ERROR_CHECK(result_fclose != 0);

  printf("In argv    = %s\n", argv[0]);
  printf("In cmdline = %s\n", test);

  return 0;
}
