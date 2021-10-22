#include <stdio.h>
#include <sys/prctl.h>

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("ERROR! Try more arguements!\n");
		return -1;
	}

	prctl(PR_SET_MM, PR_SET_MM_ARG_START, argv[1], 0, 0);
	FILE* cmd = fopen("/proc/self/cmdline", "r");
  char test[10]{0};
  fscanf(cmd, "%10s", test);
  fclose(cmd);

  printf("In argv    = %s\n", argv[0]);
  printf("In cmdline = %s\n", test);

	return 0;
}