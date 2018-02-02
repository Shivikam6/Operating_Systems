#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "utils.h"
#include "file_management.h"


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>


extern int BLOCKS_ASSIGNED_ATTR; //number of block addresses is always the 1st row
extern int FILE_SIZE_ATTR; //file size is always the second row
extern int BLOCK_ADDR_0_ATTR; //the first block address is the 7th row
extern int NEXT_BLOCK_ATTR; //last item in the block pints to the next block


typedef struct inode
{
	int blocks_assigned;
	int file_size;

	int block[8];
	int next_block;
} inode;


/* Major File System Operations */
FILE *openDiskFile(char *permissions);
void init_disk();
int get_inode_attribute(int block_num, int attribute);
void write_inode_attribute(int inode_block_num, int attribute, char new_value[]);
int assign_new_data_block(int inode_block_num);
void free_block(int block_num);
bool free_row_exists(int block_num);

//Directory Operations
bool write_dir_record(inode, char[]);
int get_file_name_inode(inode dir_inode, char file_name[]);

/* File Handler Operations */
inode read_inode(int);
char *read_data_block(int);
void write_data_block(int block_num, char *data, int num_bytes);
void write_inode(int block_num, inode the_inode);


#endif

