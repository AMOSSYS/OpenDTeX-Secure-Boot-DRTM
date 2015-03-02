// -*- coding: utf-8 -*-

/*
 * Copyright (c) 2012-2014, AMOSSYS
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the organizations nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <uc/uc.h>
#include <uc/types.h>
#include <uvideo/uvideo.h>
#include <memory.h>
#include <e820.h>
#include <log.h>

#define E820ENTRYMAX 256
#define HEAPSIZE (16 * 1024 * 1024)

#pragma pack(push,1)
typedef struct _FREEBLOB FREEBLOB;
struct _FREEBLOB {
	size_t 	   size;
	FREEBLOB	*	next;
};

typedef struct {
	uint8_t *	pool;
	uint32_t		maxsize;
	FREEBLOB *	freelist;
} DESCRPOOL;

typedef struct {
	uint8_t *   address;
	size_t      size;
} INFO_FREEBLOB;

typedef struct {
	size_t   size;
	uint32_t integrity;
} ALLOCHEADER;
#pragma pack(pop)

#define MOTIF 0xaa55cc33
#define ALLOCALIGN 8

static void alloc_init(DESCRPOOL * descr_pool, uint8_t * pool, size_t size);

static struct e820_mmap e820tab[E820ENTRYMAX];
static uint32_t nbentries = 0;

static uint8_t * mem_relocmax = 0;
static uint8_t * mem_relocmin = 0;
static uint8_t * heap_base = 0;
static DESCRPOOL heapdescr;


void memory_init(multiboot_info_t * mbi) {
   uint32_t k;
   uint64_t sizemodules = 0;
   
   // Copy E820 entries
   if(mbi->flags & MULTIBOOT_INFO_MEM_MAP) {
		multiboot_memory_map_t * mmap;
   
		for(mmap = (multiboot_memory_map_t *) mbi->mmap_addr, nbentries = 0 ;
                    (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length ;
                    mmap = (multiboot_memory_map_t *) ((unsigned long) mmap + mmap->size + sizeof (mmap->size)), nbentries++) {
			if(nbentries == E820ENTRYMAX) {
			   LOGERROR("!! Too many E820 entries !!\n");
	         return;
			}
			
			if((nbentries > 0) &&
                           (e820tab[nbentries - 1].addr + e820tab[nbentries - 1].size == mmap->addr &&
                            e820tab[nbentries - 1].type == mmap->type)) {
			   e820tab[nbentries - 1].size += mmap->len;
			} else {
			   e820tab[nbentries].addr = mmap->addr;
			   e820tab[nbentries].size = mmap->len;
			   e820tab[nbentries].type = mmap->type;
			}
		}
	} else {
	   LOGERROR("!! Can't get memory information from multiboot !!\n");
	   return;
	}
	
	// Get size of modules to relocate
	if(mbi->flags & MULTIBOOT_INFO_MODS) {
		multiboot_module_t * mod;
		
		for(k = 0, mod = (multiboot_module_t *) mbi->mods_addr ; k < mbi->mods_count ; k++, mod++) {
		   sizemodules += ((mod->mod_end - mod->mod_start) + 0xfff) & -0x1000;
		}
	}
	
	// Found the upper max of 32 bits memory in order to put our relocation area
	// The max can't be greater than 0xfffff000 in order to prevent some arithmetic errors
	for(k = 0 ; k < nbentries ; k++) {
	   uint64_t temp;
	   
	   // Is RAM ?
	   if(e820tab[k].type != E820_RAM) {
	      continue;
	   }
      
       // Is greater than our max ?
      if(e820tab[k].addr >= 0xfffff000) {
         continue;
      }
	   
	   temp = e820tab[k].addr + e820tab[k].size;

      // Adjust if this area is overlapping  our max
	   if(temp >= 0xfffff000) {
	      temp = 0xfffff000;
	   }
	   
	   // Align
	   temp &= -0x1000;
	   
	   // Don't use this area if there is not enought memory
	   if(temp - e820tab[k].addr < sizemodules + HEAPSIZE) {
	      continue;
	   }
	    
	   if(mem_relocmax < (uint8_t *) (uintptr_t) temp) {
	      mem_relocmax = (uint8_t *) (uintptr_t) temp;
	   }
	}
	
	if(mem_relocmax == 0) {
	   LOGERROR("!! Can't find the upper max of 32 bits memory !!\n");
	   return;
	}
	
	mem_relocmin = mem_relocmax;
	
	// Relocate multiboot module in the upper heap
	if(mbi->flags & MULTIBOOT_INFO_MODS) {
		multiboot_module_t * mod;
		
		for(k = 0, mod = (multiboot_module_t *) mbi->mods_addr ; k < mbi->mods_count ; k++, mod++) {
		   uint64_t start = mod->mod_start;
		   uint64_t end = mod->mod_end;
		   
		   // Create a aligned area for this module
			mem_relocmin -= (uintptr_t) (end - start);
			mem_relocmin = (uint8_t *) ((uintptr_t) mem_relocmin & -0x1000);
			
			// Relocate the module
			memcpy(mem_relocmin, (uint8_t *) (uintptr_t) start, end - start);
			
			// Update multiboot structure
			mod->mod_start = (uint64_t) (uintptr_t) mem_relocmin;
			mod->mod_end = (uint64_t) (uintptr_t) mem_relocmin + end - start;
			
			// Wipe old area
			memset((uint8_t *) (uintptr_t) start, 0, end - start);
		}
	}
	
	// Initialize HEAP
	heap_base = mem_relocmin - HEAPSIZE;
	alloc_init(&heapdescr, heap_base, HEAPSIZE);
	
	LOGDEBUG("Memory initialization done !");
}

void memory_clear(void) {
   memset(heap_base, 0, HEAPSIZE);
   memset(mem_relocmin, 0, mem_relocmax - mem_relocmin);
   LOGDEBUG("Memory clearing done !");
}

uint8_t * memory_getmaxmem(void) {
   return heap_base - 1;
}

static void alloc_init(DESCRPOOL * descr_pool, uint8_t * pool, size_t size) {
	descr_pool->pool = pool;
	descr_pool->maxsize = size;
	memset(pool, 0, size);
	
	descr_pool->freelist = (FREEBLOB *) descr_pool->pool;
	descr_pool->freelist->size = size;
	descr_pool->freelist->next = 0;
}

static void * alloc_sub(DESCRPOOL * descr_pool, size_t size) {
	FREEBLOB	*  tmp = descr_pool->freelist;
	uint8_t *   ptr = 0;

	while(tmp) {
		if((tmp->size >= size) && (tmp->size - size >= sizeof(FREEBLOB))) {
			ptr = ((uint8_t *) tmp + tmp->size - size);
			tmp->size -= size;
			break;
		}

		tmp = tmp->next;
	}

	if(ptr == (uint8_t *) descr_pool->freelist)
		descr_pool->freelist = 0;

	return ptr;
}

static void free_sub(DESCRPOOL * descr_pool, uint8_t * ptr, size_t size) {
	FREEBLOB *cur = descr_pool->freelist;
	FREEBLOB *prec = 0;

	if(((uint32_t) ptr < (uint32_t) descr_pool->pool) || ((uint32_t) ptr + size > (uint32_t) descr_pool->pool + descr_pool->maxsize)) {
		return;
	}

	((FREEBLOB *) ptr)->size = size;
	((FREEBLOB *) ptr)->next = 0;

	if(descr_pool->freelist == 0) {
		descr_pool->freelist = (FREEBLOB *) ptr;
		return;
	}

	if((uint32_t) cur < (uint32_t) ptr) {
		((FREEBLOB *) ptr)->next = descr_pool->freelist;
		descr_pool->freelist = (FREEBLOB *) ptr;

		if((uint32_t) cur + cur->size == (uint32_t) ptr) {
			cur->size += ((FREEBLOB *) ptr)->size;
			descr_pool->freelist = cur;
		}
		
		return;
	}

	while(cur) {
		if((uint32_t) cur->next < (uint32_t) ptr) {
			((FREEBLOB *) ptr)->next = cur->next;
			cur->next = (FREEBLOB *) ptr;

			if((uint32_t) ptr + ((FREEBLOB *) ptr)->size == (uint32_t) cur) {
				((FREEBLOB *) ptr)->size += cur->size;
				if(prec) {
					prec->next = (FREEBLOB *) ptr;
					cur = prec;
				} else {
					descr_pool->freelist = (FREEBLOB *) ptr;
					cur = 0;
				}
			}

			if((((FREEBLOB *) ptr)->next)
			&& ((uint32_t) ((FREEBLOB *) ptr)->next + ((FREEBLOB *) ptr)->next->size == (uint32_t) ptr)) {
				((FREEBLOB *) ptr)->next->size += ((FREEBLOB *) ptr)->size;
				
				if(cur) {
					cur->next = ((FREEBLOB *) ptr)->next;
				} else {
					descr_pool->freelist = ((FREEBLOB *) ptr)->next;
				}
			}

			break;
		}

		prec = cur;
		cur = cur->next;
	}
}

void * malloc(size_t size) {
	ALLOCHEADER * ptr;

	if(size == 0)
		return 0;
		
   //Set alignement
   size = (size + ALLOCALIGN - 1) & -ALLOCALIGN;

	ptr = alloc_sub(&heapdescr, size + sizeof(ALLOCHEADER));
	if(ptr) {
		ptr->size = size + sizeof(ALLOCHEADER);
		ptr->integrity = ptr->size ^ (uint32_t) ptr ^ MOTIF;
		ptr = (ALLOCHEADER *) ((uint8_t *) ptr + sizeof(ALLOCHEADER));
	}

	return ptr;
}

void free(void * ptr) {
	if(ptr == 0)
		return;

	ptr = ((uint8_t *) ptr - sizeof(ALLOCHEADER));
	
	if(((ALLOCHEADER *) ptr)->integrity != (((ALLOCHEADER *) ptr)->size ^ (uint32_t) ptr ^ MOTIF)) {
      // ...
		return;
	}

	free_sub(&heapdescr, ptr, ((ALLOCHEADER *) ptr)->size);
}

