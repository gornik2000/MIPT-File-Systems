#include <stdio.h>
#include <string.h>
#include <dirent.h>

void Fill_path(char *path, const char *dir_name)
{
  strncpy(path, "/proc/", 6);
  strncat(path, dir_name, PATH_MAX);
  strncat(path, "/stat", 5);
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
  fscanf(file, "%d %[^)]%[^ ] %c %d", &pid, name, test, &state, &ppid);
  fclose(file);

  printf("%4d %4d %5c %s\n", pid, ppid, state, name + 1);
  return;
}

void My_ps()
{
  printf(" PID PPID State Name\n");
  DIR *proc_dir = opendir("/proc");
  struct dirent *dir = readdir(proc_dir);
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
    dir = readdir(proc_dir);
  }
  closedir(proc_dir); 
}


int main()
{
  My_ps();
  return 0;
}