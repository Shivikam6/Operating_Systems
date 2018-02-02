

#include "file_system.h"

int main(void)
{
   print(NOTIFY, "STARTING PROGRAM");

   init_disk();
   init_new_inode(0); //initialized the directory inode

   FILE *file = stdin;
   if ( file != NULL )
   {
      char line [ 128 ]; /* or other suitable maximum line size */
      while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
      {

			print(LOG, "***********************************");
			print(NOTIFY, cat("System Call: ", line));
			char fileName[9];
			memcpy( fileName, &line[5], 8 );	
			fileName[8] = '\0';
			print(LOG, cat("Reading in FileName: ",fileName));

			if (strncmp(line, "CRET", 4) == 0)
			{
				print(LOG, "CREATE SYS CALL");
				create_sys_call(fileName);
			}
			else if (strncmp(line, "DELT", 4) == 0)
			{
				//frees block locations & clears file name in directory data block(s)
				print(LOG, "DELETE SYS CALL");
				delete_sys_call(fileName);
			}
			else if (strncmp(line, "READ", 4) == 0)
			{
				print(LOG, "READ SYS CALL");
				read_sys_call(fileName);
				//find in directory inode
				//traverse blocks
			}			
			else if (strncmp(line, "COPY", 4) == 0)
			{
				print(LOG, "COPY SYS CALL");
				char dest_file[9];
				memcpy(dest_file, &line[14], 8 );
				dest_file[8] = '\0';
				print(LOG,cat("src file: ",  fileName));
				print(LOG,cat("dest_file: ", dest_file));

				copy_sys_call(fileName,dest_file);
			}

      }
      fclose ( file );
   }
   else
   {
	  print(ERROR, "ERROR: Could not open input file");
	  exit(EXIT_FAILURE);
   }

     print(NOTIFY, "***********************************");
	 print(NOTIFY, "ENDED PROGRAM - Open disk.txt to see contents of the file system......");

     exit(EXIT_SUCCESS);
}




