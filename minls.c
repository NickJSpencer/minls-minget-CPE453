#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "min.h"

int main(int argc, char *argv[])
{
   FILE *image_file_fd;
   
   if (argc < 2)
   {
      print_usage(argv);
      return SUCCESS;
   }
   
   parse_cmd_line(argc, argv);

   if ((image_file_fd = fopen(image_file, "r")) == NULL)
   {
      perror("1open");
      exit(ERROR);
   }

   /* Load partition table */
   get_partition(image_file_fd);
   if (v_flag)
   {
      printf("Partition %d:\n", prim_part);
      print_partition(part);
   }

   /* Load and print super block */
   get_super_block(image_file_fd);
   if (v_flag) { 
      print_super_block(sb);
   }

   /* Fill i-node and zone bitmaps */
   get_bitmaps(image_file_fd);

   /* Load inodes into local inode list */
   get_inodes(image_file_fd);
   /* Print root's inode and direct zones */
   if (v_flag) {
      print_inode(&inodes[0]);
   }

   get_directory(image_file_fd, &inodes[0], 0);

   return SUCCESS;
}


