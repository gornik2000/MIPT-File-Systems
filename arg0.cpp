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

  ERROR_CHECKING(prctl(PR_SET_MM, PR_SET_MM_ARG_START, argv[1], 0, 0));
  ERROR_CHECKING(FILE* cmd = fopen("/proc/self/cmdline", "r"));
  char test[10]{0};
  fscanf(cmd, "%10s", test);
  ERROR_CHECKING(fclose(cmd));

  printf("In argv    = %s\n", argv[0]);
  printf("In cmdline = %s\n", test);

  return 0;
}
