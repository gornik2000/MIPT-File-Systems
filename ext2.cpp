#include "ext2.hpp"

//=======================================================================================
int Open_ext2fs(char *name, struct ext2_super_block *super_block)
{
	int ext2fs_fd = open(name, O_RDONLY);
	if (ext2fs_fd == -1)
	{
		int err_num = errno;
		ERRNO_MSG(err_num);
		return -err_num;
	}

	off_t lseek_result = lseek(ext2fs_fd, BOOT_BLOCK_SIZE, SEEK_SET); // skip boot block
	if (lseek_result == -1)
	{
		int err_num = errno;
		ERRNO_MSG(err_num);
		close(ext2fs_fd);
		return -err_num;
	}

	ssize_t read_result = read(ext2fs_fd, super_block, sizeof(*super_block)); // get SB
	if (read_result == -1)
	{
		int err_num = errno;
		ERRNO_MSG(err_num);
		close(ext2fs_fd);
		return -err_num;
	}

	if (super_block->s_magic != EXT2_SUPER_MAGIC) // if not a ext2 retry
	{
		int err_num = ENEXT2;
		ERRNO_MSG(err_num)
		close(ext2fs_fd);
		return -err_num;
	}
	return ext2fs_fd;
}
//=======================================================================================
int Get_inode(int fd, struct ext2_inode *inode, const struct ext2_super_block *super_block, unsigned int inode_num)
{
	unsigned int inode_count = super_block->s_inodes_count;
	if (inode_num > inode_count || inode_num <= 0)
	{
		int err_num = EBADINODENUM;
		ERRNO_MSG(err_num);
		return -err_num;
	}

	unsigned int block_size       = BASE_BLOCK_SIZE << super_block->s_log_block_size;
	unsigned int blocks_per_group = super_block->s_blocks_per_group;

	unsigned int inodes_per_group = super_block->s_inodes_per_group;
	unsigned int inode_group_num  = (inode_num - 1) / inodes_per_group;
	unsigned int inode_group_ind  = (inode_num - 1) % inodes_per_group + 1;
		
	// get ext2 group desc
	char group_block[block_size]{0};
	int read_result = Block_read(fd, group_block, block_size, block_size, inode_group_num * blocks_per_group + 2);
	if (read_result < 0)
	{
		ERRNO_MSG(-read_result);
		return read_result;
	}
	ext2_group_desc *bg_headers = (ext2_group_desc *)group_block;

	// get inode bitmap 
	char inode_bitmap_block[block_size]{0};
	read_result = Block_read(fd, inode_bitmap_block, block_size, block_size, bg_headers->bg_inode_bitmap);
	if (read_result < 0)
	{
		ERRNO_MSG(-read_result);
		return read_result;
	}

	unsigned int group_start = inode_group_num * blocks_per_group;
		
	read_result = Inode_read(fd, inode, block_size, bg_headers->bg_inode_table, group_start, inode_group_ind);
	if (read_result < 0)
	{
		ERRNO_MSG(-read_result);
		return read_result;
	}

	return Get_bit(inode_bitmap_block, inode_group_ind);	
}

int Dir_out(int fd, unsigned int block_size, const struct ext2_inode *inode)
{
	unsigned int size = Get_file_size(inode);
	if (size == 0)
		return 0;

	char *dir_data = (char *)xzmalloc(size * block_size);
	int read_result = File_read(fd, dir_data, size * block_size, block_size, inode);
	if (read_result < 0)
	{
		ERRNO_MSG(-read_result); 
		free(dir_data);
		return read_result;
	}

	char *entry_pos = dir_data;
	struct ext2_dir_entry_2 *entry = (struct ext2_dir_entry_2 *)entry_pos;
	while ((size < read_result) && (entry->inode))
	{
		printf(" # \tInode num %5d name '", entry->inode);
		
		// name out, using %c because there is no guarantee of \0 at the end
		int name_len = entry->name_len;
		char *name   = entry->name;
		for (int i = 0; i < name_len; ++i, ++name)
			printf("%c", *name);

		printf("' type ");
		Print_type(entry->file_type);
		printf("\n");

		entry_pos += entry->rec_len;
		size      += entry->rec_len;
		entry = (struct ext2_dir_entry_2 *)entry_pos;
	}

	free(dir_data);
	return read_result;
}

int File_out(int fd, unsigned int block_size, const struct ext2_inode *inode)
{
	int size = Get_file_size(inode);
	char *buffer = (char *)xzmalloc(size);
	int read_result = File_read(fd, buffer, size, block_size, inode);
	if (read_result < 0)
	{
		ERRNO_MSG(-read_result);
		free(buffer); 
		return read_result;
	}
	ssize_t write_result = write(STDOUT_FILENO, buffer, read_result);
	if (write_result == -1)
	{
		int err_num = errno;
		ERRNO_MSG(err_num);
		free(buffer);
		return -err_num;
	}
	free(buffer);
	return write_result;
}

int File_read(int fd, char *buffer, unsigned int max_size, unsigned int block_size, const struct ext2_inode *inode)
{
	if (inode->i_blocks == 0)
		return 0; //empty file

	// read data
	unsigned int size = 0;
	for (int i = 0; i < 15; ++i)
	{
		if (max_size == 0) 
			return size; // buffer is full

		unsigned int block_num = inode->i_block[i];
		int read_result = 0;
		if (i < 12)
			read_result = Block_read(fd, buffer, fmin(block_size, max_size), block_size, block_num); // not indirrect read
		else
			read_result = I_block_read(fd, buffer, max_size, block_size, block_num, i - 11); // indirrect read

		if (read_result < 0)
		{
			ERRNO_MSG(-read_result); 
			return read_result; // return error num
		}

		buffer   += read_result;
		size     += read_result;
		max_size -= read_result;
	}

	return size;
}

int I_block_read(int fd, char *buffer, unsigned int max_size, unsigned int block_size, unsigned int block_num, unsigned int level)
{
	if (block_num == 0)
		return 0;

	// read indirrect block
	char i_block[block_size];
	int read_result = Block_read(fd, i_block, block_size, block_size, block_num);
	if (read_result < 0)
	{
		ERRNO_MSG(-read_result);
		return read_result;
	}

	unsigned int size = 0;
	unsigned int block_count = block_size / 4;
	unsigned int *block_num_ptr = (unsigned int *)i_block;
	for (int i = 0; i < block_count; ++i, ++block_num_ptr)
	{
		if (max_size == 0)
			return size; // buffer is full

		unsigned int new_block_num = *block_num_ptr;
		if (new_block_num == 0) 
			return size;

		int read_result = 0;
		if (level > 1)
			read_result = I_block_read(fd, buffer, max_size, block_size, new_block_num, level - 1);
		else if (level == 1)
			read_result = Block_read(fd, buffer, fmin(block_size, max_size), block_size, new_block_num);

		if (read_result == -1)
		{
			ERRNO_MSG(-read_result);
			return read_result; // return error num
		}

		buffer   += read_result;
		size     += read_result;
		max_size -= read_result;
	}

	return 0;
}

int Block_read(int fd, char *buffer, unsigned int max_size, unsigned int block_size, unsigned int block_num)
{
	off_t lseek_result = lseek(fd, BOOT_BLOCK_SIZE + (block_num - 1) * block_size, SEEK_SET);
	if (lseek_result == -1)
	{
		int err_num = errno;
		ERRNO_MSG(err_num);
		return -err_num;
	}

	ssize_t read_result = read(fd, buffer, max_size);
	if (read_result == -1)
	{
		int err_num = errno;
		ERRNO_MSG(err_num);
		return -err_num;
	}

	return read_result;
}

int Inode_read(int fd, struct ext2_inode *inode, unsigned int block_size, unsigned int i_table_pos, unsigned int group_start, unsigned int inode_index)
{
	off_t lseek_result = lseek(fd, BOOT_BLOCK_SIZE + (i_table_pos + group_start - 1) * block_size   \
	                           + (inode_index - 1) * sizeof(struct ext2_inode), SEEK_SET);
	if (lseek_result == -1)
	{
		int err_num = errno;
		ERRNO_MSG(err_num);
		return -err_num;
	}

	ssize_t read_result = read(fd, inode, sizeof(struct ext2_inode));
	if (read_result == -1)
	{
		int err_num = errno;
		ERRNO_MSG(err_num);
		return -err_num;
	}

	return read_result;
}
//=======================================================================================
unsigned int Get_file_size(const struct ext2_inode *inode)
{
	return inode->i_blocks * 512; // i_blocks is in 512 byte blocks
}

unsigned char Get_bit(const char *bitmap, unsigned int num)
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