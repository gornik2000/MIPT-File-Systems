#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "error.hpp"

void Fill_path(char *path, const char *dir_name)
{  
  if (path == NULL)
  { 
    ERROR("got NULL path");
    return;
  }

  if (dir_name == NULL)
  { 
    ERROR("got NULL dir_name");
    return;
  }

  sprintf(path, "/proc/%s/stat", dir_name);
  return;
}

void Print_stat(const char *path)
{
  if (path == NULL)
  { 
    ERROR("got NULL path");
    return;
  }

  char name[PATH_MAX]{0};
  char test[PATH_MAX]{0};
  char state = 0;
  int  pid   = 0;
  int  ppid  = 0;

  ERROR_CHECKING(FILE *file = fopen(path, "r"));
  int args = fscanf(file, "%d %[^)]%[^ ] %c %d", &pid, name, test, &state, &ppid);
  if (args != 5)
  {
    ERROR("Read not enough arguements");
  }
  ERROR_CHECKING(fclose(file));

  printf("%5d %5d %5c %s\n", pid, ppid, state, name + 1);
  return;
}

void My_ps()
{
  printf(" PID PPID State Name\n");
  ERROR_CHECKING(DIR *proc_dir = opendir("/proc"));
  ERROR_CHECKING(struct dirent *dir = readdir(proc_dir));
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
    ERROR_CHECKING(dir = readdir(proc_dir));
  }
  ERROR_CHECKING(closedir(proc_dir)); 
}


int main()
{
  My_ps();
  return 0;
}