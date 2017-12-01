#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "min.h"

void print_path() {
   if (src_path_count == 0) {
      printf("/");
      return;
   }
   int i;

   printf("%s", src_path_string);
   
   /*for (i = 0; i < src_path_count; i++) {
      printf("/%s", src_path[i]);
   }*/
}

int main(int argc, char *argv[])
{
   FILE *image_file_fd;
   int i;
   
   if (argc < 2)
   {
      print_usage(argv);
      return SUCCESS;
   }
   
   parse_cmd_line(argc, argv);

   if ((image_file_fd = fopen(image_file, "r")) == NULL)
   {
      perror("open");
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

   /* Fill i-node and zone bitmaps */
   get_bitmaps(image_file_fd);

   /* Load inodes into local inode list */
   get_inodes(image_file_fd);
   /* Print root's inode and direct zones */
   if (v_flag) {
      print_inode(&inodes[0]);
   }

   struct inode *node = get_directory_inode(image_file_fd, &inodes[0], 0);

   /* Dir */
   if ((node->mode & MASK_DIR) == MASK_DIR) {
      print_path();
      printf(":\n");
      struct directory *dir = get_inodes_in_dir(image_file_fd, node);
      //printf("size of dir: %d\n", sizeof(*dir));
      for (i = 0; i < node->size / 64; i++) {
         //printf("dir value: %d\n", dir[i].inode - 1);
         if (dir[i].inode != 0)
         {
            print_file(&inodes[dir[i].inode - 1], (char *)dir[i].name);
            printf("\n");
         }
      }
   }
   /* File */
   else {
      print_single_file_contents(node);

      printf("%s", src_path_string); 
      /*for (i = 0; i < src_path_count; i++) {
         if (src_path_count > 1)
         {
            printf("/%s", src_path[i]);
         }
         else
         {
            printf("%s", src_path[i]);
         }
      }*/
      printf("\n");
  }

   return SUCCESS;
}


