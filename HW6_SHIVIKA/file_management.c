#include "file_system.h"

const int default_dir_inode_block_num = 0;

void create_sys_call(char file_name[])
{
    
    bool free_row = free_row_exists(default_dir_inode_block_num);
    if(! free_row)
        assign_new_data_block(default_dir_inode_block_num);
    
    inode this_inode = read_inode(default_dir_inode_block_num);
    
    //print(LOG, cat("FREE ROW? = ", itoa(this_inode.file_size)));
    print(LOG, cat("iNODE file size = ", itoa(this_inode.file_size)));
    print(LOG, cat("Next Block = ", itoa(this_inode.next_block)));
    
    bool success = write_dir_record(this_inode, file_name);
    if (success)
    {
        //update directory inode file size
        this_inode.file_size = this_inode.file_size + 32;
        write_inode(default_dir_inode_block_num, this_inode);
    }
}



void delete_sys_call(char file_name[])
{
    char bytes_read[9];
    int z = 0, x = 0, block_offset = 0, offset = 0;
    int block_number = 0, assignedBlocks = 0, totalBlocks = 0;
    int first_block = 0, first_inode_num = 0 ;
    
    inode dir_inode, inodeFile;
    FILE *fp = fopen("disk.txt", "r+");           //Open directory
    if (fp == NULL)
    {
        print(ERROR, "Error Opening The File!\n");
    }
    dir_inode = read_inode(default_dir_inode_block_num);       //finds the starting directory inode
    
    first_inode_num = get_file_name_inode(dir_inode, file_name);
    totalBlocks = dir_inode.blocks_assigned;
    
    inodeFile = read_inode(first_inode_num);      //Gets the inode
    
    while(block_number < totalBlocks)                //to get the last block for directory
    {
        if (block_number != 0 && block_number % 8 == 0)
            dir_inode = read_inode(dir_inode.next_block);
        
        block_offset = 32 + getBlockByteOffset(dir_inode.block[block_number % 8]);
        
        while(x <= 14)
        {
            offset = block_offset + (x * 32);
            fseek(fp, offset, SEEK_SET);
            while(z <= 7)                              //Gets the characters and the size of the file is 8 bytes
            {
                bytes_read[z] = fgetc(fp);
                z++;
            }
            if (0 == strcmp(fp, bytes_read))
            {
                fseek(fp, offset, SEEK_SET);
                fputs("  \n", fp);
            }
            x++;
        }
        block_number++;
    }
    
    assignedBlocks = inodeFile.blocks_assigned;
    free_block(inodeFile.block[0] - 1);
    
    while(z < assignedBlocks)
    {
        if (z != 0 && (z % 8) == 0)           //move to the next inode and deleting the initial inode block
        {
            first_block = inodeFile.next_block;
            inodeFile = read_inode(inodeFile.next_block);
            free_block(first_block);
        }
        
        free_block(inodeFile.block[z % 8]);         //Frees the blocks
        z++;
    }
}


void read_sys_call(char file_name[])
{
    
    int i=0, index = 0, file_inode_num = 0, Inodes = 0, assignedBlocks = 0, leftout_blocks = 0;
    char *datafile = "", *blockData = "";
    inode dir_inode, inodeFile;
    
    dir_inode = read_inode(default_dir_inode_block_num);
    file_inode_num = get_file_name_inode(dir_inode, file_name);
    inodeFile = read_inode(file_inode_num);
    
    assignedBlocks = inodeFile.blocks_assigned;
    Inodes = 1 + (assignedBlocks / 8);
    
    while (i < Inodes)                              //Traverses through the Inodes
    {
        leftout_blocks = assignedBlocks - (i*8);
        if (leftout_blocks >= 9)
        {
            leftout_blocks = 8;
        }
        for (index=0; index<leftout_blocks; index++)
        {
            blockData = read_data_block( inodeFile.block[index] );          //Reads the data to the blocks
            datafile = cat(datafile, blockData);
        }
        inodeFile = read_inode(inodeFile.next_block);
        i++;
    }
    print(NOTIFY, cat("", datafile));           //Output - Contents of the file
    
}


void copy_sys_call(char src_name[], char dest_name[])
{
    
    inode dir_inode = read_inode(default_dir_inode_block_num);
    int root_inode_num = get_file_name_inode(dir_inode, dest_name);
    int current_inode_num = root_inode_num;
    
    int fd = open(cat("files/",src_name) , O_RDONLY);
    if (fd == -1)
        print(ERROR, "ERROR COPY SRC FILE!\n");
    
    int bytes;
    do
    {
        char data[465];
        bytes = read(fd, data, 465);
        if (bytes > 0)
        {
            
            inode root_inode = read_inode(root_inode_num);
            
            print(LOG, cat("BLOCKS ASSIGNED: ",itoa(root_inode.blocks_assigned)));
            if(root_inode.blocks_assigned % 8 == 0 && root_inode.blocks_assigned != 0)
            {
                int inode_num = find_free_block();
                init_new_inode(inode_num);
                inode prev_inode = read_inode(current_inode_num);
                prev_inode.next_block = inode_num;
                write_inode(current_inode_num, prev_inode);
                if(root_inode_num == current_inode_num)
                    root_inode.next_block = inode_num;
                current_inode_num = inode_num;
            }
            
            int new_data_block = assign_new_data_block(current_inode_num);
            write_data_block(new_data_block, data, bytes);
            
            //update root inode....
            root_inode.file_size =     root_inode.file_size + bytes;
            root_inode.blocks_assigned++;
            write_inode(root_inode_num, root_inode);
        }
        else if (bytes == -1)
            print(ERROR, "ERROR - reading from file!");
        
    } while(bytes > 0);
    
    close(fd);
}

