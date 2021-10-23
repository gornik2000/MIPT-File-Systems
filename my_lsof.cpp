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
  if (path == NULL)
  { 
    ERROR("got NULL path");
    return;
  }

  if (pid == NULL)
  { 
    ERROR("got NULL pid");
    return;
  }

	sprintf(path, "/proc/%s/fd", pid);
	return;
}

void Print_data(const char *path)
{
  if (path == NULL)
  { 
    ERROR("got NULL path");
    return;
  }

	ERROR_CHECKING(int dir_fd = open(path, O_RDONLY));
	ERROR_CHECKING(DIR *proc_dir = fdopendir(dir_fd));
	ERROR_CHECKING(struct dirent *dir = readdir(proc_dir));
	
	printf("Size   IID   UID Name\n");
	while(dir != NULL)
	{
		if (dir->d_type == DT_LNK)
		{
			char buf[PATH_MAX]{0};
			ERROR_CHECKING(readlinkat(dir_fd, dir->d_name, buf, PATH_MAX));

			struct stat node_stat;
			ERROR_CHECKING(fstatat(dir_fd, dir->d_name, &node_stat, 0));
			printf("%6ld %6ld %6d %s\n", node_stat.st_size, node_stat.st_ino, node_stat.st_uid, buf);
		}
		dir = readdir(proc_dir);
	}

	ERROR_CHECKING(closedir(proc_dir));
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