#include <stdio.h>
#include <string.h>
#include "ext2.hpp"

int Processing_ext2fs_fd_repeatable(int fd, const struct ext2_super_block *super_block);

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf(" # Try to launch again with 1 argument: ext2fs name\n");
		return -1;
	}
	
	struct ext2_super_block super_block;
	int ext2fs_fd = Open_ext2fs(argv[1], &super_block);
	while (ext2fs_fd < 0)
	{
		char new_name[255]{0};
		switch (-ext2fs_fd)
		{
		case ENOENT:
			printf(" # No such file or directory, try again or write 'q' to terminate and exit\n");
			printf(" >> ");
			scanf("%s", new_name);
			if ((new_name[0] == 'q') && (strlen(new_name) == 1)) 
				return 0;
			ext2fs_fd = Open_ext2fs(new_name, &super_block);
			break;
		case ENEXT2:
			printf(" # File is not an ext2 filesystem, try again or write 'q' to terminate and exit\n");
			printf(" >> ");
			scanf("%s", new_name);
			if ((new_name[0] == 'q') && (strlen(new_name) == 1)) 
				return 0;
			ext2fs_fd = Open_ext2fs(new_name, &super_block);
			break;
		// other error codes if needed
		}
	}

	int result = Processing_ext2fs_fd_repeatable(ext2fs_fd, &super_block);
	if (result == -1)
	{
		ERRNO_MSG(result);
	}

	close(ext2fs_fd);
	return 0;
}

int Processing_ext2fs_fd_repeatable(int fd, const struct ext2_super_block *super_block)
{
	unsigned int block_size = BASE_BLOCK_SIZE << super_block->s_log_block_size;
	unsigned int inode_count = super_block->s_inodes_count;

	printf(" # This function prints file data accroding to it's inode num\n");
	printf(" # Write required inode num or write 0 or less to quit\n");
	int inode_num = 0;
	char buf_string[256]{0};

	while (true)
	{
		printf(" >> ");
		scanf("%s", buf_string);
		int scanned = sscanf(buf_string, "%d", &inode_num);

		if (scanned != 1)
		{
			printf(" # Inode num is not a number, try again\n");
			continue;
		}

		if (inode_num <= 0) 
			break;

		if (inode_num > inode_count)
		{
			printf(" # Inode num %d is bigger than inode count %d, try smaller number\n", inode_num, inode_count);
			continue;
		}

		// find inode through inode_num
		struct ext2_inode inode;
		int get_result = Get_inode(fd, &inode, super_block, inode_num);
		if (get_result < 0)
		{
			ERRNO_MSG(-get_result); 
			return get_result;
		}
		if (get_result == 0)
		{
			printf(" # Inode with num %d is free\n", inode_num);
			continue;
		}

		printf(" # ");
		Print_type(Get_type(inode.i_mode));
		printf("\n");

		// inode data out
		if (S_ISDIR(inode.i_mode))
		{
			int out_result = Dir_out(fd, block_size, &inode);
			if (out_result < 0)
			{
				ERRNO_MSG(-out_result); 
				return out_result;
			}
		}
		else
		{
			int out_result = File_out(fd, block_size, &inode);
			if (out_result < 0)
			{
				ERRNO_MSG(-out_result); 
				return out_result;
			}
		}
	}

	printf(" # Function finished\n");
	return 0;
}