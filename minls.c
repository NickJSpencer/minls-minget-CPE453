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
      
   printf("p flag: %hu | prim_part = %d\n", p_flag, prim_part);
   printf("s flag: %hu | sub_part  = %d\n", s_flag, sub_part);
   printf("h flag: %hu\n", h_flag);
   printf("v flag: %hu\n\n", v_flag);
   
   if (image_file != NULL)
   {
      printf("image_file: %s\n", image_file);
   }
   if (src_path != NULL)
   {
      printf("src_path: ");
      for (i = 0; i < src_path_count; i++)
      {
         printf("/%s", src_path[i]);
      }
      printf("\n");
   }
   if (dst_path != NULL)
   {
      printf("dst_path: ");
      for (i = 0; i < dst_path_count; i++)
      {
         printf("/%s", dst_path[i]);
      }
      printf("\n");
   }   
   if ((image_file_fd = open(image_file, O_RDONLY)) == -1)
   {
      perror("open");
      exit(ERROR);
   }
   
   printf("\nfd: %d\n", (int) image_file_fd);
   
/*    if ((fread(&i, sizeof(int), 1, image_file_fd)) != 1)
   {
      perror("fread");
      exit(ERROR);
   } */
   
   get_super_block(image_file_fd);

   print_super_block(sb);

   fill_bitmaps(image_file_fd);

   fill_inodes(image_file_fd);
   print_inode(inodes[0]);
   print_inode(inodes[1]);
   print_inode(inodes[2]);

   return SUCCESS;
}


