////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "util.h"

#define MEMORY_SIZE (0x10000)
#define ROM_SIZE (0x8000)
#define RAM_SIZE (MEMORY_SIZE-ROM_SIZE)


typedef struct memory {
  u8_t data[MEMORY_SIZE];
} memory_t;


void init_mem(memory_t * mem);

int read_mem_file(memory_t * mem, const char * filename);

void print_mem(memory_t * mem);

u8_t mem_read(memory_t * mem, u16_t addr);

void mem_write(memory_t * mem, u8_t data, u16_t addr);


#endif /* _MEMORY_H_ */
