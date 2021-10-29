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
//=======================================================================================
const int BOOT_BLOCK_SIZE = 1024;
const int BASE_BLOCK_SIZE = 1024;

void Processing_ext2fs_fd_repeatable(int fd, const struct ext2_super_block *super_block);
int  Open_ext2fs_repeatable(char *name, struct ext2_super_block *super_block);
int  Open_r_repeatable(char *name);

void          Inode_out(char **data, unsigned int block_size, unsigned int blocks_per_group, unsigned int group_num, const struct ext2_inode *inode);
void Indirect_block_out(char **data, unsigned int block_size, unsigned int blocks_per_group, unsigned int block_num, unsigned int  level);
void          Block_out(char **data, unsigned int block_size, unsigned int blocks_per_group, unsigned int block_num);

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
	int ext2fs_fd = Open_ext2fs_repeatable(argv[1], &super_block);
	if (ext2fs_fd == 0) 
		return 0;   // the user changed his/her mind and exit

	Processing_ext2fs_fd_repeatable(ext2fs_fd, &super_block);
	close(ext2fs_fd);
	return 0;
}
//=======================================================================================
void Processing_ext2fs_fd_repeatable(int fd, const struct ext2_super_block *super_block)
{
	unsigned int block_size       = BASE_BLOCK_SIZE << super_block->s_log_block_size;
	unsigned int block_count      = super_block->s_blocks_count;
	unsigned int blocks_per_group = super_block->s_blocks_per_group;

	unsigned int group_size       = block_size * blocks_per_group;
	unsigned int group_count      = 1 + (block_count - 1) / (blocks_per_group);

	unsigned int inode_count      = super_block->s_inodes_count;
	unsigned int inodes_per_group = super_block->s_inodes_per_group;

	printf("%d blocks with %d size in %d groups BPG %d\n", block_count, block_size, group_count, blocks_per_group);

	char *data[group_count];
	for (int i = 0; i < group_count; ++i)
	{
		data[i] = (char *)calloc(1, group_size);
		for (int i = 0; (i < 5) && (data == 0); ++i)
		{
			sleep(1);
			data[i] = (char *)calloc(1, group_size); // try to allocate again
		}
		if (data == 0) // if still couldn't allocate
		{
			printf("ERROR! Couldn't allocate data for 5 times with size %d, process terminated\n", group_size);
			exit(EXIT_FAILURE);
		}
	}

	off_t lseek_result = lseek(fd, BOOT_BLOCK_SIZE, SEEK_SET); // go to first SB
	ERROR_CHECK(lseek_result == -1);
	for (int i = 0; i < group_count; ++i)
	{
		ssize_t read_result = read(fd, data[i], group_size); // get each group data
		ERROR_CHECK(read_result == -1);
	}

	printf("This function prints file data accroding to it's inode num\n");
	printf("Write required inode num, write 0 or less to quit\n");
	int inode_num = 1;
	char buf_string[256]{0};

	printf(" >> ");
	scanf("%s", buf_string);
	int scanned = sscanf(buf_string, "%d", &inode_num);
	for (; inode_num > 0; 	printf(" >> "), scanf("%s", buf_string), scanned = sscanf(buf_string, "%d", &inode_num))
	{
		if (scanned != 1)
		{
			printf("ERROR! Inode num is not a number, try again\n");
			continue;
		}

		if (inode_num > inode_count)
		{
			printf("ERROR! Inode num %d is bigger than inode count %d, try smaller number\n", inode_num, inode_count);
			continue;
		}

		unsigned int inode_group = (inode_num - 1) / inodes_per_group;
		unsigned int inode_index = (inode_num - 1) % inodes_per_group + 1;

		ext2_group_desc *bg_headers = (ext2_group_desc *)Block_pos(data[inode_group], block_size, 2);

		unsigned char *block_bitmap = (unsigned char *)Block_pos(data[inode_group], block_size, bg_headers->bg_block_bitmap);
		unsigned char *inode_bitmap = (unsigned char *)Block_pos(data[inode_group], block_size, bg_headers->bg_inode_bitmap);

		if (!Get_bit(inode_bitmap, inode_index)) // if enode is free
		{
			printf("Inode with num %d is free\n", inode_num);
			continue;
		}

		struct ext2_inode *inode = Inode_pos(data[inode_group], block_size, inode_index, bg_headers);
		Inode_out(data, block_size, blocks_per_group, inode_group, inode);
	}

	for (int i = 0; i < group_count; ++i)
	{
		free(data[i]);
	}
	printf("Function finished\n");
	return;
}

int Open_ext2fs_repeatable(char *name, struct ext2_super_block *super_block)
{
	int ext2fs_fd = -1;
	do
	{
		ext2fs_fd = Open_r_repeatable(name); // get ext2fs fd 
		if (ext2fs_fd == 0) return 0;

		off_t lseek_result = lseek(ext2fs_fd, BOOT_BLOCK_SIZE, SEEK_SET); // skip boot block
		ERROR_CHECK(lseek_result == -1);
		ssize_t read_result = read(ext2fs_fd, super_block, sizeof(*super_block)); // get SB
		ERROR_CHECK(read_result == -1);

		if (super_block->s_magic != EXT2_SUPER_MAGIC) // if not a ext2 retry
		{
			printf("ERROR! File is not an EXT2 filesystem, try again or write 'q' to terminate and exit\n");
			printf(" >> ");
			scanf("%s", name);
			close(ext2fs_fd);
		}
	}
	while (super_block->s_magic != EXT2_SUPER_MAGIC);
	return ext2fs_fd;
}

int Open_r_repeatable(char *name)
{
	if ((name[0] == 'q') && (strlen(name) == 1)) return 0;
	int fd = open(name, O_RDONLY);

	while (fd == -1)
	{
		char new_name[255]{0};
		switch(errno)
		{
		case ENOENT:
			printf("ERROR! No such file or directory, try again or write 'q' to terminate and exit\n");
			printf(" >> ");
			scanf("%s", new_name);
			if ((new_name[0] == 'q') && (strlen(new_name) == 1)) return 0;
			fd = open(new_name, O_RDONLY);
			break;
		case EINVAL:
			printf("ERROR! File name contains invalid symbols, try again or write 'q' to terminate and exit\n");
			printf(" >> ");
			scanf("%s", new_name);
			if ((new_name[0] == 'q') && (strlen(new_name) == 1)) return 0;
			fd = open(new_name, O_RDONLY);
			break;
		case ETXTBSY:
			printf("ERROR! File is currently busy, auto retry in 1 sec\n");
			sleep(1);
			fd = open(new_name, O_RDONLY);
			break;
		default:
			return -1;
		}
	}

	return fd;
}

void Inode_out(char **data, unsigned int block_size, unsigned int blocks_per_group, unsigned int group_num, const struct ext2_inode *inode)
{
	// type out
	printf(" # This is ");
	Print_type(Get_type(inode->i_mode));
	printf("\n");

	// dir out
	if (S_ISDIR(inode->i_mode))
	{
		struct ext2_dir_entry_2 *entry = (struct ext2_dir_entry_2 *)Block_pos(data[group_num], block_size, inode->i_block[0]);
		unsigned int size = 0;
		while ((size < inode->i_size) && entry->inode)
		{
			printf(" #	Inode num %d name '%s' type ", entry->inode, entry->name);
			Print_type(entry->file_type);
			printf("\n");

			entry += entry->rec_len;
			size  += entry->rec_len;
		}
		return;
	}
	
	// file out
	unsigned int size = inode->i_blocks * 512 / block_size; // because contain 512 byte blocks num
	printf(" # It contains %d blocks\n", size);
	if (size == 0)
		return;

	printf(" << \n");
	for (int i = 0; i < 12; ++i) // out with no indirent blocks
	{
		unsigned int block_num = inode->i_block[i];
		Block_out(data, block_size, blocks_per_group, block_num);
	}

	unsigned int i_block_num = inode->i_block[12];
	Indirect_block_out(data, block_size, blocks_per_group, i_block_num, 1);
	
	i_block_num = inode->i_block[13];
	Indirect_block_out(data, block_size, blocks_per_group, i_block_num, 2);
	
	i_block_num = inode->i_block[14];
	Indirect_block_out(data, block_size, blocks_per_group, i_block_num, 3);
	return;
}

void Indirect_block_out(char **data, unsigned int block_size, unsigned int blocks_per_group, unsigned int block_num, unsigned int  level)
{
	if (block_num == 0) return;
	unsigned int block_count = block_size / 4;
	unsigned int group_num = (block_num - 1) / blocks_per_group;
	unsigned int block_ind = (block_num - 1) % blocks_per_group + 1;

	unsigned int *block_num_ptr = (unsigned int *)Block_pos(data[group_num], block_size, block_num);
	for (int i = 0; i < block_count; ++i, ++block_num_ptr)
	{
		unsigned int new_block_num = *block_num_ptr;
		if (new_block_num == 0) return;

		if (level > 1)
			Indirect_block_out(data, block_size, blocks_per_group, new_block_num, level - 1);
		else if (level == 1);
			Block_out(data, block_size, blocks_per_group, new_block_num);
	}

	return;
}

void Block_out(char **data, unsigned int block_size, unsigned int blocks_per_group, unsigned int block_num)
{
	if (block_num == 0) return;
	unsigned int group_num = (block_num - 1) / blocks_per_group;
	unsigned int block_ind = (block_num - 1) % blocks_per_group + 1;
	int write_result = write(STDOUT_FILENO, Block_pos(data[group_num], block_size, block_num), block_size); // block out
	ERROR_CHECK(write_result == -1);
	printf("\n");
	return;
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
		printf("ERROR! Somesing got wrong, unexpected type %d\n", type);
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