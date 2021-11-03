#ifndef MY_EXT2_HPP
#define MY_EXT2_HPP

#include <stdio.h>
#include <string.h>
#include <math.h>

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

int Open_ext2fs(char *name, struct ext2_super_block *super_block);

int Get_inode   (int fd, struct ext2_inode *inode, const struct ext2_super_block *super_block, unsigned int inode_num);
int Dir_out     (int fd, unsigned int block_size, const struct ext2_inode *inode);
int File_out    (int fd, unsigned int block_size, const struct ext2_inode *inode);
int File_read   (int fd, char *buffer, unsigned int max_size, unsigned int block_size, const struct ext2_inode *inode);
int Block_read  (int fd, char *buffer, unsigned int max_size, unsigned int block_size, unsigned int block_num);
int I_block_read(int fd, char *buffer, unsigned int max_size, unsigned int block_size, unsigned int block_num, unsigned int level);
int Inode_read  (int fd, struct ext2_inode *inode, unsigned int block_size, unsigned int i_table_pos, unsigned int group_start, unsigned int inode_index);

unsigned int  Get_file_size(const struct ext2_inode *inode);
unsigned char Get_bit(const char *bitmap, unsigned int num);
void          Print_type(unsigned int type);
unsigned int  Get_type(unsigned int mode);

#endif //MY_EXT2_HPP