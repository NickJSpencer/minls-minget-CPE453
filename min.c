#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "min.h"

/* Print the usage statement.
 * minls and minget have the same usage statement, 
 * except for the first line. */
void print_usage(char *argv[])
{
   if (!strcmp(argv[0], "./minls"))
   {
      printf("usage: minls [ -v ] [ -p num [ -s num ] ] imagefile [path]\n");
   }
   else if (!strcmp(argv[0], "./minget"))
   {
      printf("usage: minget [ -v ] [ -p part [ -s subpart ] ] imagefile ");
      printf("srcpath [ dstpath ]");
   }
   printf("Options:\n");
   printf("-p part    --- select partition for filesystem (default: none)\n");
   printf("-s sub     --- select subpartition for filesystem (default: none)");
   printf("\n");
   printf("-h help    --- print usage information and exit\n");
   printf("-v verbose --- increase verbosity level\n");
}

/* Sets the flags, image_file, src_path, and dst_path 
 * based on the cmd line args. */
int parse_cmd_line(int argc, char *argv[])
{
   int opt;
   int count = 0;
   int flags;
   char *image;
   char *s_path;
   char *d_path;

   /* Set all the flags to false to start */
   p_flag = FALSE;
   s_flag = FALSE;
   h_flag = FALSE;
   v_flag = FALSE;
   
   /* Primary and sub partitions initialize to 0 and remain 0 if the
    * following arg of '-p' or '-s' is invalid or does not exist */
   prim_part = 0;
   sub_part = 0;

   image_file = NULL;
   src_path = NULL;
   dst_path = NULL;

   src_path_count = 0;
   dst_path_count = 0;
   
   /* Set all the specified flags from the cmd line.
    * Also set prim_part and sub_part if '-p' or '-s' is set */
   while ((opt = getopt(argc, argv, "p:s:hv")) != -1)
   {
      count++;
      switch (opt) 
      {
         case 'p':
            count++;
            p_flag = TRUE;
            prim_part = atoi(optarg);
            break;
         case 's':
            count++;
            s_flag = TRUE;
            sub_part = atoi(optarg);
            break;
         case 'h':
            h_flag = TRUE;
            print_usage(argv);
            exit(ERROR);
         case 'v':
            v_flag = TRUE;
            break;
         default:
            print_usage(argv);
            exit(ERROR);
      }
   }

   /* getopt orders the remaining args at the back of argv. 
    * The remaining args will be used for image_file, src_path, and dst_path */
   flags = count;
   count++;
   printf("count in parse_cmd_line: %d flags: %d argc: %d\n", count, flags, argc);
   while (count < argc)
   {
      int arg = count - flags;
      if (arg == 1)
      {
         image_file = argv[count];
         printf("arg 1: %s\n", image_file);
      }
      else if (arg == 2)
      {
         s_path = argv[count];
         src_path = parse_path(s_path, &src_path_count);
         printf("arg 2: %s\n", s_path);
      }
      else if (arg == 3)
      {
         d_path = argv[count];
         dst_path = parse_path(d_path, &dst_path_count);
         printf("arg 3: %s\n", d_path);
      }
      count++;
   }
   
   return SUCCESS;
}

char **parse_path(char *string, int *path_count)
{
   int i;
   char *temp;
   char **path_ptr = (char **) malloc(sizeof(char *));
   int count = 0;

   path_ptr[0] = strtok(string, "/");
   count++;
   while (path_ptr[count - 1] != NULL)
   {
      printf("%d: %s\n", count - 1, path_ptr[count - 1]);
      count++;
      *path_count = *path_count + 1;
      if ((path_ptr = (char**) realloc(path_ptr, sizeof(char *) * count)) == NULL)
      {
         perror("realloc");
         exit(ERROR);
      }
      if ((path_ptr[count - 1] = (char *) malloc(sizeof(strlen(path_ptr[count - 1])))) == NULL)
      {
         perror("malloc");
         exit(ERROR);
      }

      path_ptr[count - 1] = strtok(NULL, "/");
      //temp = strtok(NULL, "/");
      //printf("%s\n", temp);
   }
   printf("count: %d\n", count);
   for (i = 0; i < count; i++)
   {
      printf("/%s", path_ptr[i]);
   }
   printf("\n");

   return path_ptr;
}

void get_super_block(FILE *fd)
{
   char buffer[1024];

   if (read(fd, buffer, 1024) == -1)
   {
      perror("read");
      exit(ERROR);
   }
         
   if (read(fd, &sb, sizeof(struct superblock)) == -1)
   {
      perror("read");
      exit(ERROR);
   }
   
   if (read(fd, buffer, 1024 - sizeof(struct superblock)) == -1)
   {
      perror("read");
      exit(ERROR);
   }
}

void fill_bitmaps(FILE *fd)
{
   inode_bitmap = (char *) malloc(sb.blocksize);
   zone_bitmap = (char *) malloc(sb.blocksize);

   if (read(fd, inode_bitmap, sb.blocksize) == -1)
   {
      perror("read");
      exit(ERROR);
   }
   if (read(fd, zone_bitmap, sb.blocksize) == -1)
   {
      perror("read");
      exit(ERROR);
   }

   printf("inode_bitmap: %02x\n", inode_bitmap);
   printf("zone_bitmap: %02x\n\n", zone_bitmap);
}

void fill_inodes(FILE *fd)
{
   int i;
   char buffer[sb.blocksize];
   inodes = (struct inode **) malloc(sizeof(struct inode *) * sb.ninodes);
   
   //printf("size of inode: %d", sizeof(struct inode));

   for (i = 0; i < sb.ninodes; i++)
   {
      inodes[i] = (struct inode *) malloc(sizeof(struct inode));
      if (read(fd, inodes[i], sizeof(struct inode)) == -1)
      {
         perror("read");
         exit(ERROR);
      }
   }

   return 0;
}

void print_super_block(struct superblock sb)
{
   printf("Superblock Contents:\n");
   printf("Stored Fields:\n");
   printf("  ninodes        %d\n", sb.ninodes);
   printf("  i_blocks       %d\n", sb.i_blocks);
   printf("  z_blocks       %d\n", sb.z_blocks);
   printf("  firstdata      %d\n", sb.firstdata);
   printf("  log_zone_size  %d (zone size: %d)\n", sb.log_zone_size, sb.blocksize);
   printf("  max_file       %lu\n", sb.max_file);
   printf("  magic          0x%04x\n", sb.magic);
   printf("  zones          %d\n", sb.zones);
   printf("  blocksize      %d\n", sb.blocksize);
   printf("  subversion     %d\n", sb.subversion);
}

void print_inode(struct inode * node)
{
   printf("File inode:\n");
   printf("mode       0x%04x\n", node->mode);
   printf("links      %d\n", node->links);
   printf("uid        %d\n", node->uid);
   printf("gid        %d\n", node->gid);
   printf("size       %d\n", node->size);
}
