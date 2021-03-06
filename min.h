#ifndef MIN_H
   #define MIN_H
   
   #include <stdint.h>
   
   #define SUCCESS 0
   #define ERROR 1
   #define TRUE 1
   #define FALSE 0

   #define DIRECT_ZONES 7
   
   #define PARTITION_TABLE_LOCATION 0x1BE
   #define SECTOR_SIZE 512  
   #define BOOT_SIZE 1024
   #define MAGIC 19802
   #define BOOTABLE 0x80
   #define MINIX_TYPE 0x81
   #define VALID_510 0x55
   #define VALID_511 0xAA

   #define FILE_TYPE 0170000
   #define SYM_LINK_TYPE 0120000
   #define REGULAR_FILE 0100000

   #define MASK_DIR  0040000
   #define MASK_O_R  0000400
   #define MASK_O_W  0000200
   #define MASK_O_X  0000100
   #define MASK_G_R  0000040
   #define MASK_G_W  0000020
   #define MASK_G_X  0000010
   #define MASK_OT_R 0000004
   #define MASK_OT_W 0000002
   #define MASK_OT_X 0000001
 
   #define GET_PERM(mode, mask, c) ( (((mode)&(mask)) == mask) ? c : '-' )
   #define MIN(a,b) (((a)<(b))?(a):(b))
   #define MAX(a,b) (((a)>(b))?(a):(b))

   /* Minix Version 3 Superblock
    * this structure found in fs/super.h
    * in minix 3.1.1
    * on disk. These fields and orientation are non–negotiable */
   struct __attribute__ ((__packed__)) superblock { 
      uint32_t ninodes;         /* number of inodes in this filesystem */
      uint16_t pad1;            /* make things line up properly */
      int16_t i_blocks;         /* # of blocks used by inode bit map */
      int16_t z_blocks;         /* # of blocks used by zone bit map */
      uint16_t firstdata;       /* number of first data zone */
      int16_t log_zone_size;    /* log2 of blocks per zone */
      int16_t pad2;             /* make things line up again */
      uint32_t max_file;        /* maximum file size */
      uint32_t zones;           /* number of zones on disk */
      int16_t magic;            /* magic number */
      int16_t pad3;             /* make things line up again */
      uint16_t blocksize;       /* block size in bytes */
      uint8_t subversion;       /* filesystem sub–version */
   };

   struct __attribute__ ((__packed__)) inode {
      uint16_t mode;            /* mode */
      uint16_t links;           /* number or links */
      uint16_t uid;
      uint16_t gid;
      uint32_t size;
      int32_t atime;
      int32_t mtime;
      int32_t ctime;
      uint32_t zone[DIRECT_ZONES];
      uint32_t indirect;
      uint32_t two_indirect;
      uint32_t unused;
   };

   struct __attribute__ ((__packed__)) partition {
      uint8_t bootind;
      uint8_t start_head;
      uint8_t start_sec;
      uint8_t start_cyl;
      uint8_t type;
      uint8_t end_head;
      uint8_t end_sec;
      uint8_t end_cyl;
      uint32_t lFirst;
      uint32_t size;
   };

   struct __attribute__ ((__packed__)) directory {
      uint32_t inode;
      unsigned char name[60];
   };

   struct partition part;  /* Partition */
   struct superblock sb;   /* Superblock of partition */
   struct inode *inodes;   /* Reference to all inodes */

   unsigned int zonesize;  /* Size of zone */

   short p_flag;           /* Flag for if -p was provided */
   short s_flag;           /* Flag for if source was provided */
   short h_flag;           
   short v_flag;
      
   int prim_part;
   int sub_part;
   
   uint32_t part_start;

   char *image_file;
   char **src_path;
   char *src_path_string;
   char **dst_path;
   char *dst_path_string;
  
   int src_path_count;
   int dst_path_count;

   uint8_t *inode_bitmap;
   uint8_t *zone_bitmap;

   int parse_cmd_line(int argc, char *argv[]);
   char **parse_path(char *string, int *path_count);

   void get_partition(FILE *fd);
   void validate_partition_table(FILE *fd);
   void validate_partition();

   void get_super_block(FILE *fd);
   void validate_superblock();
   
   void get_inodes(FILE *fd);
   void get_bitmaps(FILE *fd);

   struct inode *get_directory_inode(FILE *fd, struct inode *node, int);
   struct directory *get_inodes_in_dir(FILE *fd, struct inode *node);
   void fill_dir(FILE *image, struct directory *dir, int location, int size);
   void set_file_data(FILE *image, struct inode *node, uint8_t *ptr);

   void print_partition(struct partition part);
   void print_super_block(struct superblock sb);
   void print_usage(char *argv[]);
   void print_inode(struct inode * node);
   void print_file(struct inode *node, char *name);
   void print_single_file_contents(struct inode *node);
   char *get_time(uint32_t);
   char *get_mode(uint16_t);

#endif
