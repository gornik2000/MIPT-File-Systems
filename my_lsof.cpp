#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "error.hpp"

void Get_path(char *path, const char *pid)
{
  sprintf(path, "/proc/%s/fd", pid);
  return;
}

void Print_data(const char *path)
{
  int dir_fd = open(path, O_RDONLY);
  ERROR_CHECK(dir_fd == -1);

  DIR *proc_dir = fdopendir(dir_fd);
  ERROR_CHECK(proc_dir == NULL);

  errno = 0;
  struct dirent *dir = readdir(proc_dir);
  ERROR_CHECK(dir == NULL && errno != 0);

  printf("  Size    IID    UID Name\n");
  while(dir != NULL)
  {
    if (dir->d_type == DT_LNK)
    {
      char buf[PATH_MAX]{0};

      ssize_t result_readlinkat = readlinkat(dir_fd, dir->d_name, buf, PATH_MAX);
      ERROR_CHECK(result_readlinkat == -1);

      struct stat node_stat;
      int result_fstatat = fstatat(dir_fd, dir->d_name, &node_stat, 0);
      ERROR_CHECK(result_fstatat == -1)
      
      printf("%6ld %6ld %6d %s\n", node_stat.st_size, node_stat.st_ino, node_stat.st_uid, buf);
    }
    errno = 0;
    dir = readdir(proc_dir);
    ERROR_CHECK(dir == NULL && errno != 0);
  }
  int result_close = close(dir_fd);
  ERROR_CHECK(result_close == -1);
  return;	
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    ERROR("Try with arguement: pid");
    return -1;
  }

  char path[PATH_MAX]{0};
  char *pid = argv[1];

  Get_path(path, pid);
  Print_data(path);

  return 0;
}