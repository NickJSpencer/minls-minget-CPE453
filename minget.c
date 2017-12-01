#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "min.h"

int main(int argc, char *argv[])
{
   FILE *image_file_fd;
   int i;

   if (argc < 2) {
      print_usage(argv);
      return SUCCESS;
   }

   parse_cmd_line(argc, argv);

   if ((image_file_fd = fopen(image_file, "r")) == NULL) {
      perror("open");
      exit(ERROR);
   }

   get_partition(image_file_fd);
   if (v_flag) {
      printf("Partition %d:\n", prim_part);
      print_partition(part);
   }

   get_super_block(image_file_fd);
   
   get_bitmaps(image_file_fd);

   get_inodes(image_file_fd);

   if (v_flag) {
      print_inode(&inodes[0]);
   }

   if (!src_path_count) {
      exit(ERROR);
   }

   struct inode *node = get_directory_inode(image_file_fd, &inodes[0], 0);
   if (!node->size) {
      return SUCCESS;
   }

   if((node->mode & MASK_DIR) == MASK_DIR) { 
      exit(ERROR);
   }
  
   FILE *output;
   if (dst_path_count && (output = fopen(dst_path_string, "w")) == NULL) {
      perror("open");
      exit(ERROR);
   }
   else {
      output = stdout;
   }
   set_file_data(image_file_fd, node, output);
  
   return SUCCESS;
}

