
#include "file_system.h"

//extern const int FILE_SIZE_ATTR;
#define FREE 0
#define USED 1

int BLOCKS_ASSIGNED_ATTR = 1; //number of block addresses is always the 1st row
int FILE_SIZE_ATTR = 2; //file size is always the second row
int BLOCK_ADDR_0_ATTR = 7; //the first block address is the 7th row
int NEXT_BLOCK_ATTR = 15; //last item in the block pints to the next block


//single level directory inode gets the first disk block
int single_level_dir_block = 0;

int file_system_disk_blocks[128];
void init_file_system_disk_blocks();


//Open the location (file) of the simulated disk.
FILE *openDiskFile(char *permissions)
{
	FILE *f = fopen("disk.txt", permissions);
	if (f == NULL)
	{
	    print(ERROR, "ERROR OPENING FILE!\n");
	    return NULL;
	}
	return f;
}



//initializes the disk (simulated using a disk file)
void init_disk()
{
	init_file_system_disk_blocks();

	FILE *f = openDiskFile("w");

	//We have 128 blocks....
	int i;
	for (i=0;i<128;i++)
	{
		char* block_header;
		if (i<10)
			block_header = "---------- BLOCK #  ";
		else if(i<100)
			block_header = "---------- BLOCK # ";
		else
			block_header = "---------- BLOCK #";

		fprintf(f, "%s\n", cat(cat(block_header, itoa(i)), " ---------" ));

		//Print 31 white space characters (plus \n) over 15 lines
		int n,k;
		for (n=0;n<15;n++)
		{
			for(k=0;k<31;k++)
				fprintf(f, " ");
			fprintf(f, "\n");
		}
	}

	fclose(f);
}


//initially set all the blocks to FREE except block 0 (used for the directory inode)
void init_file_system_disk_blocks()
{
	int i;
	file_system_disk_blocks[0] = USED;
 	for(i=1;i<128;i++)
	{
		file_system_disk_blocks[i] = FREE;
	}
}


//initializes a new inode block on disk
void init_new_inode(int block_number)
{

	FILE *f = openDiskFile("r+");

	int rows = 16;
	int bytes_per_row = 32;

	int byte_offset = (block_number * rows * bytes_per_row) + 32;

	fseek(f, byte_offset, SEEK_SET);

	fputs("Blocks Assigned: 0             \n",f); //must always be 32 bytes
	fputs("      File Size: 0             \n",f); //must always be 32 bytes

	int i;
	for(i=2;i<6;i++)
		fputs("File Attributes                \n",f); //must always be 32 bytes
	for(i=0;i<8;i++)
	{
 		char *tmp_str = cat( cat("Block Address ",itoa(i)) ,":               \n");
		fputs( tmp_str ,f); //must always be 32 bytes

	}
	fputs("Next iNode Addr:               \n",f);

  fclose(f);

}

//finds a block in memory that is Free, sets it to being USED and returns the block number
int find_free_block()
{
	int i;
	for(i=0;i<128;i++)
	{
		if (file_system_disk_blocks[i] == FREE)
		{
			file_system_disk_blocks[i] = USED;
			return i;
		}
	}
	print(ERROR, "OUT OF DISK SPACE! NO MORE FREE BLOCKS!");
	return -1; //no more free blocks! Out of disk space
}


//wipes the contents of a disk block.
//Most real systems just free the location without wasting cycles overwriting the contents
void free_block(int block_num)
{
	file_system_disk_blocks[block_num] = FREE;

	FILE *f = openDiskFile("r+");

	int byte_offset = getBlockByteOffset(block_num) + 32;
        fseek(f, byte_offset, SEEK_SET);

	int row;
	for(row=0;row<15;row++)
	{
		char *blank_row = "                               \n";
		fputs(blank_row,f);
	}

	fclose(f);
}



//Determines the location on disk based on the block number
int getBlockByteOffset(int block_number)
{
	int rows = 16;
	int bytes_per_row = 32;
	return (block_number * rows * bytes_per_row);
}




//return TRUE if there is still room in assigned data blocks.  Otherwise FALSE
bool free_row_exists(int inode_block_num)
{
	int blocksAssigned = get_inode_attribute(inode_block_num, BLOCKS_ASSIGNED_ATTR);
	int fileSize = get_inode_attribute(inode_block_num, FILE_SIZE_ATTR);

	int rows_per_block = 15;
	int bytes_per_row = 32;

	int bytes_available = blocksAssigned * rows_per_block * bytes_per_row;

	if ( bytes_available == fileSize )  //should be equal when we have maxed out data blocks...
		return false;
	else
		return true;
}



void write_inode_attribute(int inode_block_num, int attribute, char new_value[32])
{

	FILE *f = openDiskFile("r+");

	int byte_offset = getBlockByteOffset(inode_block_num) + (attribute*32);
        fseek(f, byte_offset, SEEK_SET);

	print(LOG,cat("WRITING VALUE: ", new_value));
	print(LOG,cat("BYTE OFFSET: ", itoa(byte_offset) ));

	fputs(new_value, f);

    fclose(f);
}



int assign_new_data_block(int inode_block_num)
{
	//STEP #1 - assign next free block
	int next_block = find_free_block();

	//STEP #2 - update inode (Data Block Address)
	int blocksAssigned = get_inode_attribute(inode_block_num, BLOCKS_ASSIGNED_ATTR);

	char *tmp1 = cat("Block Address ", itoa(blocksAssigned));
	char *tmp2 = cat(": ", itoa(next_block) );
	char *new_value = cat(tmp1,tmp2);
	char *padded_value = pad(new_value);
	print(LOG, cat("AFTER PAD: ", padded_value));

	int attribute_row = BLOCK_ADDR_0_ATTR + blocksAssigned;
	write_inode_attribute(inode_block_num, attribute_row, padded_value);


	//STEP #3 - update inode (Number of Assigned Blocks)
	blocksAssigned++;
	new_value = "Blocks Assigned: ";
	new_value = cat(new_value, itoa(blocksAssigned));
	new_value = pad(new_value);
	write_inode_attribute(inode_block_num, BLOCKS_ASSIGNED_ATTR, new_value);

	return next_block;
}




/* --- DIRECTORY OPERATIONS --- */

//finds a free row in a data block and writes the directory name
bool write_dir_record(inode dir_inode, char file_name[])
{

		int row, b;
		for(b=0;b<8;b++)
		{
			int data_block = dir_inode.block[b];
			if (data_block == -1)
				return false;
			char *data = read_data_block(data_block);    // READ THE BLOCK......

			for (row=0;row<15;row++)
			{

				char rowbuff[32];
				rowbuff[31] = ' ';
				memcpy(rowbuff, &data[row*31], 31 );

				print(LOG, cat("rowbuff: ",rowbuff));

				//check if row is blank
				int i;
				bool space = true;
				for(i=0;i<32;i++)
				{
					if (!isspace(rowbuff[i]))
						space = false;
				}

				print(LOG, cat("IS SPACE: ",itoa( (int) space) ));
				if (space)
				{
					int next_block = find_free_block();
				//	file_system_disk_blocks[next_block] = USED;
					write_inode_attribute(data_block, row+1, cat (cat(file_name," : "), itoa(next_block) ) );
					init_new_inode(next_block);
					return true;
				}
			}
		}
		return false;
}



//get inode block num using the file name
int get_file_name_inode(inode dir_inode, char file_name[])
{
		int row, b;
		for(b=0;b<8;b++)
		{
			int data_block = dir_inode.block[b];
			if (data_block == -1)
				return false;
			char *data = read_data_block(data_block);    // READ THE BLOCK......

			for (row=0;row<15;row++)
			{

				char rowbuff[9];
				rowbuff[8] = '\0';
				memcpy(rowbuff, &data[row*31], 8 );

				print(LOG, cat("rowbuff: ",rowbuff));
				print(LOG, cat("file_name: ",file_name));

				//check if file name matches...
				if (strcmp(file_name,rowbuff) == 0)
				{
					char block_num_buff[4];
					block_num_buff[3] = '\0';
					memcpy(block_num_buff, &data[(row*31)+11], 3);
					int block_num = atoi(block_num_buff);
					print(LOG, cat("BLOCK NUMBER!! = ", itoa(block_num)));
					return block_num;
				}
			}
		}

	print(ERROR, cat("ERROR - File Name does not exist in File System! name: ",file_name) );
	return -1;
}
