#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "min.h"

int main(int argc, char *argv[])
{
   int i;
   FILE *image_file_fd;
   
   if (argc < 2)
   {
      print_usage(argv);
      return SUCCESS;
   }
   
   parse_cmd_line(argc, argv);

   if (v_flag) {      
      printf("p flag: %hu | prim_part = %d\n", p_flag, prim_part);
      printf("s flag: %hu | sub_part  = %d\n", s_flag, sub_part);
      printf("h flag: %hu\n", h_flag);
      printf("v flag: %hu\n\n", v_flag);
   }

   if (image_file != NULL && v_flag)
   {
      printf("image_file: %s\n", image_file);
   }
   if (src_path != NULL && v_flag)
   {
      printf("src_path: ");
      for (i = 0; i < src_path_count; i++)
      {
         printf("/%s", src_path[i]);
      }
      printf("\n");
   }
   if (dst_path != NULL && v_flag)
   {
      printf("dst_path: ");
      for (i = 0; i < dst_path_count; i++)
      {
         printf("/%s", dst_path[i]);
      }
      printf("\n");
   }   
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
      print_inode_zones(&inodes[0]);
   }

   if (v_flag)
   {
      print_inode(&inodes[0]);
   }
   return SUCCESS;
}


