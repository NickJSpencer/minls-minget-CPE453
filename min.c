#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "min.h"

void get_partition(FILE *image)
{
   /* Default offset for primary partition */
   part_start = 0;

   if(!p_flag)
   {
      return;
   }
   
   /* Ensure partition table is valid */
   validate_partition_table(image);

   /* Seek to the primary partition entry in partition table*/
   if (fseek(image, PARTITION_TABLE_LOCATION + sizeof(struct partition) *
            prim_part, SEEK_SET) != 0)
   {
      perror("fseek");
      exit(ERROR);
   }

   /* Load partition information from partition table */
   if (!fread(&part, sizeof(part), 1, image))
   {
      perror("1fread");
      exit(ERROR);
   }

   /* Update partition start location offset */
   part_start = part.lFirst * SECTOR_SIZE;

   /* Validate partition */
   validate_partition();

   /* If no subpartition was provided we can return here */
   if (!s_flag)
   {
      return;
   }

   /* Validate subpartition table */
   validate_partition_table(image);

   /* Seek to desired subpartition entry in the subpartition table */
   if (fseek(image, part_start + PARTITION_TABLE_LOCATION + 
            sizeof(struct partition) * sub_part, SEEK_SET) != 0)
   {
      perror("fseek");
      exit(ERROR);
   }
   
   /* Load subpartition information from subpartition table */
   if (!fread(&part, sizeof(struct partition), 1, image))
   {
      perror("2fread");
      exit(ERROR);
   }

   /* Update offset of partition to subpartition's start location */
   part_start = part.lFirst * SECTOR_SIZE;

   /* Validate subpartition */
   validate_partition();
}

void validate_partition() 
{
   if (part.type != MINIX_TYPE) {
      print_partition(part);
      fprintf(stderr, "Partition at %d is not a minix partition\n", 
         part_start);
      exit(ERROR);
   }
}

/* Validate a partition table based on an image and a partition offset
 * When used for the primary partition, the offset prim_start is 0
 * When used for a subpartition table, the offset prim_start is set to the 
 * parent partition's start */
void validate_partition_table(FILE *image)
{
   uint8_t byte510;
   uint8_t byte511;

   /* Read the data at byte 510 in partition table */
   if (fseek(image, 510 + part_start, SEEK_SET) != 0)
   {
      perror("fseek");
      exit(ERROR);
   }
   if (!fread(&byte510, sizeof(uint8_t), 1, image))
   {
      perror("3fread");
      exit(ERROR);
   }
   /* Validate data at byte 510 */
   if (byte510 != VALID_510) {
      fprintf(stderr, "Byte 510 in partition table is not valid: %d\n", 
         byte510);
      exit(ERROR);
   }
   
   /* Read the data at byte 511 in partition table */
   if (!fread(&byte511, sizeof(uint8_t), 1, image))
   {
      perror("4fread");
      exit(ERROR);
   }
   /* Validate data at byte 511 */
   if (byte511 != VALID_511) {
      fprintf(stderr, "Byte 511 in partition table is not valid\n");
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
      perror("5fread");
      exit(ERROR);
   }

   if (v_flag) {
      print_super_block(sb);
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
      perror("6fread");
      exit(ERROR);
   }
   /* Load zone bitmap */
   if (!fread(zone_bitmap, sb.z_blocks * sb.blocksize, 1, image)) {
      perror("7fread");
      exit(ERROR);
   }
}

void get_inodes(FILE *image)
{
   /* Allocate memory for local inode list */
   inodes = (struct inode *) malloc(sizeof(struct inode) * sb.ninodes);

   /* Seek to i-node block */
   if (fseek(image, (part.lFirst * SECTOR_SIZE) + (2 + sb.i_blocks + 
         sb.z_blocks) * sb.blocksize, SEEK_SET) != 0) {
      perror("get_inodes() fseek");
      exit(ERROR);
   }

   /* Read in inodes into local inode list 
    * No need to fseek -- the file pointer is already pointing to
    * inode block at this point */
   if (!fread(inodes, sizeof(struct inode), sb.ninodes, image)) {
      perror("get_inodes() fread");
      exit(ERROR);
   }
}

struct directory *get_inodes_in_dir(FILE *image, struct inode *node) {
   /* Allocate enough directory objects for all directories in
    * this inode's data zone */
   struct directory *dir = (struct directory *) malloc(sizeof(struct directory)
         * node->size / 64);
         
   /* Seek to location of the data zone specified by inode */
   if (fseek(image, part_start + node->zone[0] * sb.blocksize, SEEK_SET) != 0)
   {
      perror("fseek");
      exit(ERROR);
   }

   if (!fread(dir, sizeof(struct directory), node->size / 64, image))
   {
      perror("8fread");
      exit(ERROR);
   }
   return dir;
}

struct inode* get_directory_inode(FILE *image, struct inode *node, int arg)
{
   int i;
 
   if ((node->mode & REGULAR_FILE) == REGULAR_FILE) {
      return node;
   }

   struct directory *dir = get_inodes_in_dir(image, node);

   for (i = 0; i < node->size / 64; i++)
   {
      if (arg < src_path_count && !strcmp(src_path[arg], (char *) dir[i].name)) 
      {
         if (arg >= src_path_count)
         {
            return node;
         }
         arg++;
         struct inode *ret = get_directory_inode(image, 
            &inodes[dir[i].inode - 1], arg);
         return ret;
      }
   }

   return node;
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
   fprintf(stderr, "Partition Contents:\n");
   fprintf(stderr, "bootind      0x%x\n", part.bootind);
   fprintf(stderr, "start_head   %d\n", part.start_head);
   fprintf(stderr, "start_sec    %d\n", part.start_sec);
   fprintf(stderr, "start_cyl    %d\n", part.start_cyl);
   fprintf(stderr, "type         0x%x\n", part.type);
   fprintf(stderr, "end_head     %d\n", part.end_head);
   fprintf(stderr, "end_sec      %d\n", part.end_sec);
   fprintf(stderr, "end_cyl      %d\n", part.end_cyl);
   fprintf(stderr, "lFirst       %lu\n", part.lFirst);
   fprintf(stderr, "size         %d\n", part.size);
}

void print_super_block(struct superblock sb)
{
   fprintf(stderr, "Superblock Contents:\n");
   fprintf(stderr, "Stored Fields:\n");
   fprintf(stderr, "  ninodes        %d\n", sb.ninodes);
   fprintf(stderr, "  i_blocks       %d\n", sb.i_blocks);
   fprintf(stderr, "  z_blocks       %d\n", sb.z_blocks);
   fprintf(stderr, "  firstdata      %d\n", sb.firstdata);
   fprintf(stderr, "  log_zone_size  %d (zone size: %d)\n",
         sb.log_zone_size, sb.blocksize);
   fprintf(stderr, "  max_file       %lu\n", (long unsigned int) sb.max_file);
   fprintf(stderr, "  magic          0x%04x\n", sb.magic);
   fprintf(stderr, "  zones          %d\n", sb.zones);
   fprintf(stderr, "  blocksize      %d\n", sb.blocksize);
   fprintf(stderr, "  subversion     %d\n\n", sb.subversion);
}

void print_inode(struct inode * node)
{
   int i;
   fprintf(stderr, "File inode:\n");
   fprintf(stderr, "  uint16_t mode       0x%04x (%s)\n", node->mode, get_mode(
          node->mode));
   fprintf(stderr, "  uint16_t links      %d\n", node->links);
   fprintf(stderr, "  uint16_t uid        %d\n", node->uid);
   fprintf(stderr, "  uint16_t gid        %d\n", node->gid);
   fprintf(stderr, "  uint16_t size       %d\n", node->size);
   fprintf(stderr, "  uint32_t atime      %d --- %s", node->atime, get_time(
          node->atime));
   fprintf(stderr, "  uint32_t mtime      %d --- %s", node->mtime, get_time(
          node->mtime));
   fprintf(stderr, "  uint32_t ctime      %d --- %s", node->ctime, get_time(
            node->ctime));


   fprintf(stderr, "\nDirect zones:\n");
   for (i = 0; i < DIRECT_ZONES; i++) {
      fprintf(stderr, "%17s%d] = %5d\n", "zone[", i, node->zone[i]);
   }
   fprintf(stderr, "uint32_t %11s %6d\n", "indirect", node->indirect);
   fprintf(stderr, "uint32_t %9s %8d\n", "double", node->two_indirect);
}

void print_file(struct inode *node, char *name) {
   printf("%s%8d %s", get_mode(node->mode), node->size, name);
}

char *get_time(uint32_t time) 
{
   time_t t = time;
   return ctime(&t);
}

char *get_mode(uint16_t mode) 
{
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

/* Sets the flags, image_file, src_path, and dst_path 
 * based on the cmd line args. */
int parse_cmd_line(int argc, char *argv[])
{
   int isNumber;
   int opt;
   int flagCount;
   int imageLoc;
   char *s_path;
   char *d_path;
   char temp[256];
   int tempidx;

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
   flagCount = 0;
   while ((opt = getopt(argc, argv, "vp:s:h")) != -1)
   {
      switch (opt) 
      {
         case 'p':
            p_flag = TRUE;
            prim_part = atoi(optarg);
            flagCount++;
            break;
         case 's':
            s_flag = TRUE;
            sub_part = atoi(optarg);
            flagCount++;
            break;
         case 'h':
            h_flag = TRUE;
            print_usage(argv);
            exit(ERROR);
         case 'v':
            v_flag = TRUE;
            flagCount++;
            break;
         default:
            print_usage(argv);
            exit(ERROR);
      }
   }

   /* Walk back through the arguments to find where the image path is */
   imageLoc = 1;
   while(flagCount) {
      if (argv[imageLoc][0] == '-') {
         flagCount--;
      }
      imageLoc++;
   }

   /* Check if current argument is a number */
   strcpy(temp, argv[imageLoc]);
   tempidx = 0;
   isNumber = 1;
   while(temp[tempidx]) {
      if(temp[tempidx] < '0' || temp[tempidx] > '9') {
         isNumber = 0;
         break;
      }
      tempidx++;
   }
   if (isNumber) {
      imageLoc++;
   }

   image_file = argv[imageLoc++];

   if (imageLoc < argc) {
      s_path = argv[imageLoc++];
      src_path = parse_path(s_path, &src_path_count);
   }
   if (imageLoc < argc) {
      d_path = argv[imageLoc++];
      dst_path = parse_path(d_path, &dst_path_count);
   }

   /* If no src was provided, default to root */
   if (src_path == NULL) {
      src_path = (char **) malloc(sizeof(char *));
      *src_path = "";
      src_path_count = 0;
   }

   return SUCCESS;
}

char **parse_path(char *string, int *path_count)
{
   char **path_ptr = (char **) malloc(sizeof(char *));
   int count = 0;

   path_ptr[0] = strtok(string, "/");
   count++;
   while (path_ptr[count - 1] != NULL)
   {
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

   return path_ptr;
}
