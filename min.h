#ifndef MIN_H
   #define MIN_H
   
   #include <stdint.h>
   
   #define SUCCESS 0
   #define ERROR 1
   #define TRUE 1
   #define FALSE 0
   #define DIRECT_ZONES 7
   
   struct __attribute__ ((__packed__)) superblock { /* Minix Version 3 Superblock
                        * this structure found in fs/super.h
                        * in minix 3.1.1
                        */
      /* on disk. These fields and orientation are non–negotiable */
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

   struct superblock sb;
   struct inode **inodes;   

   short p_flag;
   short s_flag;
   short h_flag;
   short v_flag;
   
   int prim_part;
   int sub_part;
   
   char *image_file;
   char **src_path;
   char **dst_path;
  
   int src_path_count;
   int dst_path_count;

   char *inode_bitmap;
   char *zone_bitmap;

   void print_usage(char *argv[]);
   int parse_cmd_line(int argc, char *argv[]);
   char **parse_path(char *string, int *path_count);
   void get_super_block(FILE *fd);
   void print_super_block(struct superblock sb);
   void fill_inodes(FILE *fd);
   void print_inode(struct inode * node);
   void fill_bitmaps(FILE *fd);
#endif
