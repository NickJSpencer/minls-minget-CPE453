#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "min.h"

/* Sets the flags, image_file, src_path, and dst_path 
 * based on the cmd line args. */
int parse_cmd_line(int argc, char *argv[])
{
   int opt;
   int count = 0;
   int flags;
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
   while (count < argc)
   {
      int arg = count - flags;
      if (arg == 1)
      {
         image_file = argv[count];
      }
      else if (arg == 2)
      {
         s_path = argv[count];
         src_path = parse_path(s_path, &src_path_count);
      }
      else if (arg == 3)
      {
         d_path = argv[count];
         dst_path = parse_path(d_path, &dst_path_count);
      }
      count++;
   }

   /* If no src was provided, default to root */
   if (src_path == NULL) {
      src_path = (char **) malloc(sizeof(char *));
      *src_path = "";
      src_path_count = 1;
   }

   return SUCCESS;
}

char **parse_path(char *string, int *path_count)
{
   int i;
   char **path_ptr = (char **) malloc(sizeof(char *));
   int count = 0;

   path_ptr[0] = strtok(string, "/");
   count++;
   while (path_ptr[count - 1] != NULL)
   {
      if(v_flag)
      {
         printf("%d: %s\n", count - 1, path_ptr[count - 1]);
      }
      count++;
      *path_count = *path_count + 1;
      if ((path_ptr = (char**) realloc(path_ptr, sizeof(char *) * count)) 
            == NULL)
      {
         perror("realloc");
         exit(ERROR);
      }
      if ((path_ptr[count - 1] = (char *) malloc(sizeof(strlen(
            path_ptr[count - 1])))) == NULL)
      {
         perror("malloc");
         exit(ERROR);
      }

      path_ptr[count - 1] = strtok(NULL, "/");
   }
   if (v_flag) {
      printf("count: %d\n", count);
      for (i = 0; i < count; i++)
      {
         printf("/%s", path_ptr[i]);
      }
      printf("\n");
   }

   return path_ptr;
}

void get_partition(FILE *image)
{
   if(!p_flag)
   {
      return;
   }

   validate_partition(image, 0);

   if (fseek(image, PARTITION_TABLE_LOCATION + sizeof(struct partition) *
            prim_part, SEEK_SET) != 0)
   {
      perror("fseek");
      exit(ERROR);
   }

   if (!fread(&part, sizeof(part), 1, image))
   {
      perror("fread");
      exit(ERROR);
   }

   if (!s_flag)
   {
      return;
   }

   validate_partition(image, part.lFirst * SECTOR_SIZE);

   if (fseek(image, part.lFirst * SECTOR_SIZE + PARTITION_TABLE_LOCATION + 
            sizeof(struct partition) * sub_part, SEEK_SET) != 0)
   {
      perror("fseek");
      exit(ERROR);
   }

   if (!fread(&part, sizeof(struct partition), 1, image))
   {
      perror("fread");
      exit(ERROR);
   }
}

void validate_partition(FILE *image, unsigned int part_start)
{
   char byte510;
   char byte511;

   if (fseek(image, 510 + part_start, SEEK_SET) != 0)
   {
      perror("fseek");
      exit(ERROR);
   }

   if (!fread(&byte510, 1, 1, image))
   {
      perror("fread");
      exit(ERROR);
   }

   if (!fread(&byte511, 1, 1, image))
   {
      perror("fread");
      exit(ERROR);
   }

   if (byte510 != (char) 85 || byte511 != (char) 170)
   {
      fprintf(stderr, "Error: Partition is not valid\n");
      fprintf(stderr, "510: %d\n", byte510);
      fprintf(stderr, "511: %d\n", byte511);
      exit(ERROR);
   }
}

void get_super_block(FILE *image)
{
   int seek_val = 1024;

   if (p_flag)
   {
      seek_val += part.lFirst * SECTOR_SIZE;
   }

   /* Seek past boot block */
   if (fseek(image, seek_val, SEEK_SET) != 0) {
      perror("fseek");
      exit(ERROR);
   }
    
   /* Load super block */
   if (!fread(&sb, sizeof(sb), 1, image))
   {
      perror("fread");
      exit(ERROR);
   }

   validate_superblock();
}

void validate_superblock()
{
   if (sb.magic != MAGIC)
   {
      fprintf(stderr, "Error: filesystem is not a minix filesystem\n");
      exit(ERROR);
   }
}

void get_bitmaps(FILE *image)
{
   inode_bitmap = (char *) malloc(sb.blocksize);
   zone_bitmap = (char *) malloc(sb.blocksize);

   /* Seek past boot block and super block */
   if (fseek(image, 2 * sb.blocksize, SEEK_SET) != 0) {
      perror("fseek");
      exit(ERROR);
   }
   
   /* Load i-node bitmap */
   if (!fread(inode_bitmap, sb.i_blocks * sb.blocksize, 1, image)) {
      perror("1fread");
      exit(ERROR);
   }
   /* Load zone bitmap */
   if (!fread(zone_bitmap, sb.z_blocks * sb.blocksize, 1, image)) {
      perror("2fread");
      exit(ERROR);
   }
}

void get_inodes(FILE *image)
{
   /* Allocate memory for local inode list */
   inodes = (struct inode *) malloc(sizeof(struct inode) * sb.ninodes);

   /* Read in inodes into local inode list 
    * No need to fseek -- the file pointer is already pointing to
    * inode block at this point */
   if (!fread(inodes, sizeof(struct inode), sb.ninodes, image)) {
      perror("fread");
      exit(ERROR);
   }
}



/* Print the usage statement.
 * minls and minget have the same usage statement, 
 * except for the first line. */
void print_usage(char *argv[])
{
   if (!strcmp(argv[0], "./minls"))
   {
      fprintf(stderr, "usage: minls [ -v ] [ -p num [ -s num ] ] imagefile");
      fprintf(stderr, "[path]\n");
   }
   else if (!strcmp(argv[0], "./minget"))
   {
      fprintf(stderr, "usage: minget [ -v ] [ -p part [ -s subpart ] ]");
      fprintf(stderr, " imagefile srcpath [ dstpath ]");
   }
   fprintf(stderr, "Options:\n");
   fprintf(stderr, "-p part    --- select partition for filesystem ");
   fprintf(stderr, "(default: none)\n");
   fprintf(stderr, "-s sub     --- select subpartition for filesystem ");
   fprintf(stderr, "(default: none)\n");
   fprintf(stderr, "-h help    --- print usage information and exit\n");
   fprintf(stderr, "-v verbose --- increase verbosity level\n");
}

void print_partition(struct partition part)
{
   printf("Partition Contents:\n");
   printf("bootind      0x%x\n", part.bootind);
   printf("start_head   %d\n", part.start_head);
   printf("start_sec    %d\n", part.start_sec);
   printf("start_cyl    %d\n", part.start_cyl);
   printf("type         0x%x\n", part.type);
   printf("end_head     %d\n", part.end_head);
   printf("end_sec      %d\n", part.end_sec);
   printf("end_cyl      %d\n", part.end_cyl);
   printf("lFirst       %lu\n", part.lFirst);
   printf("size         %d\n", part.size);
}

void print_super_block(struct superblock sb)
{
   printf("Superblock Contents:\n");
   printf("Stored Fields:\n");
   printf("  ninodes        %d\n", sb.ninodes);
   printf("  i_blocks       %d\n", sb.i_blocks);
   printf("  z_blocks       %d\n", sb.z_blocks);
   printf("  firstdata      %d\n", sb.firstdata);
   printf("  log_zone_size  %d (zone size: %d)\n",
         sb.log_zone_size, sb.blocksize);
   printf("  max_file       %lu\n", (long unsigned int) sb.max_file);
   printf("  magic          0x%04x\n", sb.magic);
   printf("  zones          %d\n", sb.zones);
   printf("  blocksize      %d\n", sb.blocksize);
   printf("  subversion     %d\n\n", sb.subversion);
}

void print_inode(struct inode * node)
{
   int i;
   printf("File inode:\n");
   printf("  uint16_t mode       0x%04x (%s)\n", node->mode, get_mode(
            node->mode));
   printf("  uint16_t links      %d\n", node->links);
   printf("  uint16_t uid        %d\n", node->uid);
   printf("  uint16_t gid        %d\n", node->gid);
   printf("  uint16_t size       %d\n", node->size);
   printf("  uint32_t atime      %d --- %s", node->atime, get_time(
            node->atime));
   printf("  uint32_t mtime      %d --- %s", node->mtime, get_time(
            node->mtime));
   printf("  uint32_t ctime      %d --- %s", node->ctime, get_time(
            node->ctime));


   printf("\nDirect zones:\n");
   for (i = 0; i < DIRECT_ZONES; i++) {
      printf("%17s%d] = %5d\n", "zone[", i, node->zone[i]);
   }
   printf("uint32_t %11s %6d\n", "indirect", node->indirect);
   printf("uint32_t %9s %8d\n", "double", node->two_indirect);
}

char *get_time(uint32_t time) {
   time_t t = time;
   return ctime(&t);
}

char *get_mode(uint16_t mode) {
   char* permissions = (char *) malloc(sizeof(char) * 11);
   permissions[0] = GET_PERM(mode, MASK_DIR, 'd');
   permissions[1] = GET_PERM(mode, MASK_O_R, 'r');
   permissions[2] = GET_PERM(mode, MASK_O_W, 'w');
   permissions[3] = GET_PERM(mode, MASK_O_X, 'x');
   permissions[4] = GET_PERM(mode, MASK_G_R, 'r');
   permissions[5] = GET_PERM(mode, MASK_G_W, 'w');
   permissions[6] = GET_PERM(mode, MASK_G_X, 'x');
   permissions[7] = GET_PERM(mode, MASK_OT_R, 'r');
   permissions[8] = GET_PERM(mode, MASK_OT_W, 'w');
   permissions[9] = GET_PERM(mode, MASK_OT_X, 'x');
   permissions[10] = '\0';

   return permissions;
}
