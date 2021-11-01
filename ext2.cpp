#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

//#include <linux/fs.h>
//#include <linux/ext2_fs.h>

#include "ext2_fs.h"

#include "error.hpp"
#include "malloc.hpp"
//=======================================================================================
const int BOOT_BLOCK_SIZE = 1024;
const int BASE_BLOCK_SIZE = 1024;

int Processing_ext2fs_fd_repeatable(int fd, const struct ext2_super_block *super_block);
int Open_ext2fs(char *name, struct ext2_super_block *super_block);

int           Inode_out(char **data, unsigned int block_size, unsigned int blocks_per_group, unsigned int group_num, const struct ext2_inode *inode);
int  Indirect_block_out(char **data, unsigned int block_size, unsigned int blocks_per_group, unsigned int block_num, unsigned int  level, unsigned int mode);
void          Dir_b_out(char **data, unsigned int block_size, unsigned int blocks_per_group, unsigned int block_num);
int           Block_out(char **data, unsigned int block_size, unsigned int blocks_per_group, unsigned int block_num);
struct ext2_inode *Inode_pos(const char  *data, unsigned int block_size, unsigned int inode_num, const struct ext2_group_desc *group);
const char        *Block_pos(const char  *data, unsigned int block_size, unsigned int block);

unsigned char Get_bit(const unsigned char *bitmap, unsigned int num);
void       Print_type(unsigned int type);
unsigned int Get_type(unsigned int mode);
//=======================================================================================
int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Try to launch again with 1 argument: ext2fs name\n");
		return -1;
	}
	
	struct ext2_super_block super_block;
	int ext2fs_fd = Open_ext2fs(argv[1], &super_block);
	if (ext2fs_fd == 0) 
	{
		printf("%s is not an ext2 filesystem\n", argv[1]);
		return 0;
	}
	if (ext2fs_fd == -1)
	{
		printf("Something got wrong in Open_ext2fs(), check stderr for info\n");
		return -1;
	}

	int result = Processing_ext2fs_fd_repeatable(ext2fs_fd, &super_block);
	if (result == -1)
	{
		printf("Something got wrong in Processing_ext2fs_fd_repeatable(), check stderr for info\n");
	}
	close(ext2fs_fd);
	return 0;
}
//=======================================================================================
int Processing_ext2fs_fd_repeatable(int fd, const struct ext2_super_block *super_block)
{
	unsigned int block_size       = BASE_BLOCK_SIZE << super_block->s_log_block_size;
	unsigned int block_count      = super_block->s_blocks_count;
	unsigned int blocks_per_group = super_block->s_blocks_per_group;

	unsigned int group_size       = block_size * blocks_per_group;
	unsigned int group_count      = 1 + (block_count - 1) / (blocks_per_group);

	unsigned int inode_count      = super_block->s_inodes_count;
	unsigned int inodes_per_group = super_block->s_inodes_per_group;

	char *data[group_count];
	for (int i = 0; i < group_count; ++i)
	{
		data[i] = (char *)xzmalloc(group_size);
	}

	off_t lseek_result = lseek(fd, BOOT_BLOCK_SIZE, SEEK_SET); // go to first SB
	if (lseek_result == -1)
	{
		ERRNO_MSG();
		return -1;
	}

	for (int i = 0; i < group_count; ++i)
	{
		ssize_t read_result = read(fd, data[i], group_size); // get each group data
		if (read_result == -1)
		{
			ERRNO_MSG();
			return -1;
		}
	}

	printf(" # This function prints file data accroding to it's inode num\n");
	printf(" # Write required inode num or write 0 or less to quit\n");
	int inode_num = 1;
	char buf_string[256]{0};

	//for (inode_num = 1; inode_num < inode_count / 3 * 2; ++inode_num)
	while (true)
	{
	  printf(" >> ");
	  scanf("%s", buf_string);
	  int scanned = sscanf(buf_string, "%d", &inode_num);

	  if (inode_num <= 0) break;

		if (scanned != 1)
		{
			printf(" # Inode num is not a number, try again\n");
			continue;
		}
		printf("%d\n", inode_num);
		if (inode_num > inode_count)
		{
			printf(" # Inode num %d is bigger than inode count %d, try smaller number\n", inode_num, inode_count);
			continue;
		}

		unsigned int inode_group = (inode_num - 1) / inodes_per_group;
		unsigned int inode_index = (inode_num - 1) % inodes_per_group + 1;
		ext2_group_desc *bg_headers = (ext2_group_desc *)Block_pos(data[inode_group], block_size, 2);

		unsigned char *inode_bitmap = (unsigned char *)Block_pos(data[inode_group], block_size, bg_headers->bg_inode_bitmap);
		if (!Get_bit(inode_bitmap, inode_index)) // if enode is free
		{
			printf(" # Inode with num %d is free\n", inode_num);
			continue;
		}

		struct ext2_inode *inode = Inode_pos(data[inode_group], block_size, inode_index, bg_headers);
		int inode_result = Inode_out(data, block_size, blocks_per_group, inode_group, inode);
		if (inode_result == -1)
		{
			ERRNO_MSG();
			return -1;
		}
	}

	for (int i = 0; i < group_count; ++i)
	{
		free(data[i]);
	}
	printf(" # Function finished\n");
	return 0;
}

int Open_ext2fs(char *name, struct ext2_super_block *super_block)
{
	int ext2fs_fd = open(name, O_RDONLY);
	if (ext2fs_fd == -1)
	{
		ERRNO_MSG();
		return -1;
	}

	off_t lseek_result = lseek(ext2fs_fd, BOOT_BLOCK_SIZE, SEEK_SET); // skip boot block
	if (lseek_result == -1)
	{
		ERRNO_MSG();
		return -1;
	}

	ssize_t read_result = read(ext2fs_fd, super_block, sizeof(*super_block)); // get SB
	if (read_result == -1)
	{
		ERRNO_MSG();
		return -1;
	}

	if (super_block->s_magic != EXT2_SUPER_MAGIC) // if not a ext2 retry
	{
		close(ext2fs_fd);
		return 0;
	}
	return ext2fs_fd;
}

int Inode_out(char **data, unsigned int block_size, unsigned int blocks_per_group, unsigned int group_num, const struct ext2_inode *inode)
{
	// type out
	printf(" # This is ");
	unsigned int mode = inode->i_mode;
	Print_type(Get_type(mode));
	printf("\n");

	if (S_ISDIR(mode))
	{
		// dir out
		for (int i = 0; i < 12; ++i)
		{
			unsigned int block_num = inode->i_block[i];
			Dir_b_out(data, block_size, blocks_per_group, block_num);
		}
	}
	else
	{
		// file out
		unsigned int size = inode->i_blocks * 512 / block_size; // because contain 512 byte blocks num
		printf(" # It contains %d blocks\n", size);
		if (size == 0)
			return 0;

		printf(" << \n");
		for (int i = 0; i < 12; ++i) // out with no indirent blocks
		{
			unsigned int block_num = inode->i_block[i];
			int result_out = Block_out(data, block_size, blocks_per_group, block_num);
			if (result_out == -1)
			{
				ERRNO_MSG();
				return -1;
			}
		}
	}

	unsigned int i_block_num = inode->i_block[12];
	int result_out = Indirect_block_out(data, block_size, blocks_per_group, i_block_num, 1, mode);
	if (result_out == -1)
	{
		ERRNO_MSG();
		return -1;
	}
	
	i_block_num = inode->i_block[13];
	result_out = Indirect_block_out(data, block_size, blocks_per_group, i_block_num, 2, mode);
	if (result_out == -1)
	{
		ERRNO_MSG();
		return -1;
	}

	i_block_num = inode->i_block[14];
	result_out = Indirect_block_out(data, block_size, blocks_per_group, i_block_num, 3, mode);
	if (result_out == -1)
	{
		ERRNO_MSG();
		return -1;
	}

	printf("\n");
	return 0;
}

int Indirect_block_out(char **data, unsigned int block_size, unsigned int blocks_per_group, unsigned int block_num, unsigned int level, unsigned int mode)
{
	if (block_num == 0) return 0;
	unsigned int block_count = block_size / 4; // 4 = sizeof(int)
	unsigned int group_num = (block_num - 1) / blocks_per_group;
	unsigned int block_ind = (block_num - 1) % blocks_per_group + 1;

	char is_dir = S_ISDIR(mode);

	unsigned int *block_num_ptr = (unsigned int *)Block_pos(data[group_num], block_size, block_ind);
	for (int i = 0; i < block_count; ++i, ++block_num_ptr)
	{
		unsigned int new_block_num = *block_num_ptr;
		if (new_block_num == 0) return 0;

		if (level > 1)
		{
			int result_out = Indirect_block_out(data, block_size, blocks_per_group, new_block_num, level - 1, mode);
			if (result_out == -1)
			{
				ERRNO_MSG();
				return -1;
			}
		}
		else if (level == 1)
		{
			if (is_dir)
			{
				Dir_b_out(data, block_size, blocks_per_group, new_block_num);
			}
			else
			{
				int result_out = Block_out(data, block_size, blocks_per_group, new_block_num);
				if (result_out == -1)
				{
					ERRNO_MSG();
					return -1;
				}
			}
		}
	}

	return 0;
}

void Dir_b_out(char **data, unsigned int block_size, unsigned int blocks_per_group, unsigned int block_num)
{
	if (block_num == 0) return;
	unsigned int group_num = (block_num - 1) / blocks_per_group;
	unsigned int block_ind = (block_num - 1) % blocks_per_group + 1;

	struct ext2_dir_entry_2 *entry = (struct ext2_dir_entry_2 *)Block_pos(data[group_num], block_size, block_ind);
	unsigned int size = 0;
	while ((size < block_size) && (entry->inode))
	{
		printf(" #	Inode num %d name '%s' type ", entry->inode, entry->name);
		Print_type(entry->file_type);
		printf("\n");

		entry += entry->rec_len;
		size  += entry->rec_len;
	}
	return;
}

int Block_out(char **data, unsigned int block_size, unsigned int blocks_per_group, unsigned int block_num)
{
	if (block_num == 0) return 0;
	unsigned int group_num = (block_num - 1) / blocks_per_group;
	unsigned int block_ind = (block_num - 1) % blocks_per_group + 1;
	int write_result = write(STDOUT_FILENO, Block_pos(data[group_num], block_size, block_ind), block_size); // block out
	if (write_result == -1) 
	{
		ERRNO_MSG();
		return -1;
	}
	return write_result;
}

struct ext2_inode *Inode_pos(const char *data, unsigned int block_size, unsigned int inode_num, const struct ext2_group_desc *group)
{
	return (ext2_inode *)(Block_pos(data, block_size, group->bg_inode_table) + (inode_num - 1) * sizeof(struct ext2_inode));
}

const char *Block_pos(const char *data, unsigned int block_size, unsigned int block)
{
	return data + (block - 1) * block_size;
}

unsigned char Get_bit(const unsigned char *bitmap, unsigned int num)
{
	unsigned int byte = num / 8;
	unsigned int bit  = num % 8;

	return (bitmap[byte] >> bit) & 1u;
}

void Print_type(unsigned int type)
{
	switch (type)
	{
	case 0:
		printf("Unknown file");
		break;
	case 1:
		printf("Regular File");
		break;
	case 2:
		printf("Directory");
		break;
	case 3:
		printf("Character Device");
		break;
	case 4:
		printf("Block Device");
		break;
	case 5:
		printf("Named pipe");
		break;
	case 6:
		printf("Socket");
		break;
	case 7:
		printf("Symbolic Link");
		break;
	default:
		printf("Somesing got wrong, unexpected type %d\n", type);
	}
}

unsigned int Get_type(unsigned int mode)
{
	if (S_ISREG(mode))
		return 1;
	if (S_ISDIR(mode))
		return 2;
	if (S_ISCHR(mode))
		return 3;
	if (S_ISBLK(mode))
		return 4;
	if (S_ISFIFO(mode))
		return 5;
	if (S_ISSOCK(mode))
		return 6;
	if (S_ISLNK(mode))
		return 7;
	return 0;
}