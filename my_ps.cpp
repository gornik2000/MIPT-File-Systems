#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "error.hpp"

void Fill_path(char *path, const char *dir_name)
{ 
  sprintf(path, "/proc/%s/stat", dir_name);
  return;
}

void Print_stat(const char *path)
{
  char name[PATH_MAX]{0};
  char test[PATH_MAX]{0};
  char state = 0;
  int  pid   = 0;
  int  ppid  = 0;

  FILE *file = fopen(path, "r");
  ERROR_CHECK(file == NULL);
  int args = fscanf(file, "%d %[^)]%[^ ] %c %d", &pid, name, test, &state, &ppid);
  if (args != 5)
  {
    ERROR("Read not enough arguements");
  }

  int result_fclose = fclose(file);
  ERROR_CHECK(result_fclose != 0);

  printf("%5d %5d %5c %s\n", pid, ppid, state, name + 1);
  return;
}

void My_ps()
{
  int dir_fd = open("/proc", O_RDONLY);
  ERROR_CHECK(dir_fd == -1);

  DIR *proc_dir = fdopendir(dir_fd);
  ERROR_CHECK(proc_dir == NULL);

  errno = 0;
  struct dirent *dir = readdir(proc_dir);
  ERROR_CHECK(dir == NULL && errno != 0);

  printf("  PID  PPID State Name\n");
  while(dir != NULL)
  {
    if (dir->d_type == DT_DIR)
    {
      int num = 0;
      if (sscanf(dir->d_name, "%d", &num) == 1)
      {
        char path[PATH_MAX]{0};
        Fill_path(path, dir->d_name);
        Print_stat(path);
      }
    }
    errno = 0;
    dir = readdir(proc_dir);
    ERROR_CHECK(dir == NULL && errno != 0);
  }
  int result_close = close(dir_fd);
  ERROR_CHECK(result_close == -1);
}


int main()
{
  My_ps();
  return 0;
}