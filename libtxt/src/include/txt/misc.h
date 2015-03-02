#pragma once 

#ifndef SHA1_LENGTH
#define SHA1_LENGTH        20
#endif
typedef uint8_t sha1_hash_t[SHA1_LENGTH];

typedef struct __packed {
  uint32_t    data1;
  uint16_t    data2;
  uint16_t    data3;
  uint16_t    data4;
  uint8_t     data5[6];
} uuid_t;

static inline bool txt_are_uuids_equal(const uuid_t * uuid1, const uuid_t * uuid2) {
   int i;
   
   if(uuid1->data1 != uuid2->data1) {
      return false;
   }

   if(uuid1->data2 != uuid2->data2) {
      return false;
   }
   
   if(uuid1->data3 != uuid2->data3) {
      return false;
   }
   
   if(uuid1->data4 != uuid2->data4) {
      return false;
   }
   
   for(i = 0 ; i < 6 ; i++) {
      if(uuid1->data5[i] != uuid2->data5[i]) {
         return false;
      }
   }
   
   return true;
}

