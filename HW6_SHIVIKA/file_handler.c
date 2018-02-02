#include "file_system.h"


void write_inode(int block_num, inode the_inode)
{
	write_inode_attribute(block_num, BLOCKS_ASSIGNED_ATTR, cat("Blocks Assigned: ", itoa(the_inode.blocks_assigned) ));
	write_inode_attribute(block_num, FILE_SIZE_ATTR, cat("      File Size: ", itoa(the_inode.file_size) ));
	int i;
	for (i=1;i<=4;i++)
		write_inode_attribute(block_num,FILE_SIZE_ATTR +i ,"File Attributes");
	for (i=0;i<8;i++)
	{
		char *tmp1 = cat("Block Address ",itoa(i));
		char *tmp2;
		if (the_inode.block[i] > -1)
			tmp2 = cat(": ",itoa(the_inode.block[i]));
		else
			tmp2 = ": ";
		write_inode_attribute(block_num, BLOCK_ADDR_0_ATTR + i , cat(tmp1,tmp2));
	}
//	print(NOTIFY, cat("block num: ", itoa(block_num)) );
//	print(NOTIFY, cat("in write inode, next block: ", itoa(the_inode.next_block)) );
	write_inode_attribute(block_num, NEXT_BLOCK_ATTR, cat("Next iNode Addr: ", itoa(the_inode.next_block)) );

}



int get_inode_attribute(int inode_block_num, int attribute)
{

	FILE *f = openDiskFile("r+");

	int byte_offset = getBlockByteOffset(inode_block_num) + (attribute*32);
    fseek(f, byte_offset, SEEK_SET);

    char row_buff[32];
    fgets(row_buff, 32, f);
    print(LOG, cat("Attribute ROW: ", row_buff));

	char subbuff[16];
	memcpy(subbuff, &row_buff[16], 16 );
	print(LOG, cat("Attitribute SubString: ", subbuff));

	int attribute_value = atoi(subbuff);
	print(LOG, cat("attibute convert to int: ", itoa(attribute_value)));

    fclose(f);

    return attribute_value;
}


char *read_data_block(int block_num)
{
	FILE *f = openDiskFile("r+");

	int byte_offset = getBlockByteOffset(block_num) + 32;
        fseek(f, byte_offset, SEEK_SET);

	char *the_data = "";

	char buffer[32];

	int k;
	for (k=0;k<15;k++)
	{
		fseek(f, byte_offset + (32*k), SEEK_SET);
		fgets(buffer, 32, f);
		the_data = cat(the_data, buffer);
	}

    print(LOG, cat( cat("THE DATA: ",the_data), "||\n" ));

    fclose(f);
	return the_data;
}


void write_data_block(int block_num, char *data, int num_bytes)
{
	FILE *f = openDiskFile("r+");

	int byte_offset = getBlockByteOffset(block_num) + 32;
	fseek(f, byte_offset, SEEK_SET);

	char byte[2];
	byte[1] = '\0';

	int b;
	for(b=0;b<num_bytes;b++)
	{
		//check if record belongs on next line
		if (b % 31 == 0 && b != 0)
		{
			byte[0] = '\n';
			fputs(&byte[0],f);
		}

		byte[0] = data[b];
		fputs(&byte[0],f);
	}

    fclose(f);

}


inode read_inode(int inode_block_num)
{
	inode current_inode;

	int next_block;

	current_inode.blocks_assigned = get_inode_attribute(inode_block_num, BLOCKS_ASSIGNED_ATTR);
	current_inode.file_size       = get_inode_attribute(inode_block_num, FILE_SIZE_ATTR);
	current_inode.next_block      = get_inode_attribute(inode_block_num, NEXT_BLOCK_ATTR);
//	print(NOTIFY, cat("READ inode: ", itoa(inode_block_num)));
//	print(NOTIFY, cat("READ next block: ", itoa(current_inode.next_block)));

	int i;
	for(i=0;i<8;i++)
	{
		int block_addr = -1;
		if (i < current_inode.blocks_assigned)
			block_addr = get_inode_attribute(inode_block_num, BLOCK_ADDR_0_ATTR + i);
		current_inode.block[i] = block_addr;
	}

	return current_inode;

}

