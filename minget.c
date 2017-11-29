#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "min.h"

int main(int argc, char *argv[])
{
   
   if (argc < 2)
   {
      print_usage(argv);
      return SUCCESS;
   }
   
   parse_cmd_line(argc, argv);
      
   printf("p flag: %hu | prim_part = %d\n", p_flag, prim_part);
   printf("s flag: %hu | sub_part  = %d\n", s_flag, sub_part);
   printf("h flag: %hu\n", h_flag);
   printf("v flag: %hu\n", v_flag);
   
   printf("image_file: %s\n", image_file);
   printf("src_path: %s\n", src_path);
   printf("dst_path: %s\n", dst_path);
      
   return SUCCESS;
}




