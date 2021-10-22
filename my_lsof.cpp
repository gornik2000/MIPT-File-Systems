#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

void Get_path(char *path, const char *pid)
{
	strncpy(path, "/proc/", 6);
	strncat(path, pid, 9);
	strncat(path, "/fd", 3);
	return;
}

void Print_data(const char *path)
{
	int dir_fd = open(path, O_RDONLY);
	DIR *proc_dir = fdopendir(dir_fd);
	struct dirent *dir = readdir(proc_dir);
	printf("Size   IID   UID Name\n");

	while(dir != NULL)
	{
		if (dir->d_type == DT_LNK)
		{
			char buf[PATH_MAX]{0};
			readlinkat(dir_fd, dir->d_name, buf, PATH_MAX);

			struct stat node_stat;
			fstatat(dir_fd, dir->d_name, &node_stat, 0);
			printf("%4ld %5ld %5d %s\n", node_stat.st_size, node_stat.st_ino, node_stat.st_uid, buf);
		}
		dir = readdir(proc_dir);
	}

	closedir(proc_dir);
	close(dir_fd);
	return;	
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("ERROR! Try with arguement: pid\n");
		return -1;
	}

	char path[PATH_MAX]{0};
	char *pid = argv[1];
	Get_path(path, pid);
	Print_data(path);

	return 0;
}