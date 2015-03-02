#include <stdio.h>

typedef union {
   unsigned long _raw;
   struct {
      unsigned long ACM_Type        :4;
      unsigned long ACM_Progress    :6;
      unsigned long ACM_ErrorCode   :5;
      unsigned long Origin          :1;
      unsigned long Other           :9;
      unsigned long Reserved        :5;
      unsigned long External        :1;
      unsigned long Valid           :1;
   } u;
} txt_error;

int main(int argc, char ** argv) {
   unsigned long error;
   txt_error data;
   char * endptr;

   if(argc != 2) {
      printf("Usage : txt-error <error code>\n");
      return 1;
   }

   error = strtoul(argv[1], &endptr, 0);
   if(endptr == argv[1]) {
      printf("Invalid error code\n");
      return 1;
   }
   
   data._raw = error;

   printf("0x%08lx\n", data._raw);
   printf("Valid         = %x\n", data.u.Valid);
   printf("External      = %x\n", data.u.External);
   printf("Other         = %x\n", data.u.Other);
   printf("Origin        = %x\n", data.u.Origin);
   printf("ACM_ErrorCode = %x\n", data.u.ACM_ErrorCode);
   printf("ACM_Progress  = %x\n", data.u.ACM_Progress);
   printf("ACM_Type      = %x\n", data.u.ACM_Type);

   return 0;
}

