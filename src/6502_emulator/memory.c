////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"
#include "util.h"


void init_mem(memory_t * mem) {
  u32_t i;
  for (i = 0; i < MEMORY_SIZE; i++) mem->data[i] = 0;
}

void read_mem_file(memory_t * mem, const char * filename) {
  FILE  *  file;
  u32_t   mem_len;

  init_mem(mem);
  /* open the file */
  file = fopen(filename, "rb");
  if (file == NULL) {
    log_error_str("could not open file", filename);
    return;
  }

  /* get size of file */
  fseek(file, 0, SEEK_END);
  mem_len = ftell(file);

  fseek(file, 0, SEEK_SET);
  if(mem_len != ROM_SIZE) {
    log_error_int("error in read memory size", mem_len);
    return;
  }
  /* read file into a buffer and close it*/
  fread(mem->data, mem_len, 1, file);
  fclose(file);
}

void print_mem(memory_t * mem) {
  hexdump(mem->data, MEMORY_SIZE);
}

u8_t mem_read(memory_t * mem, u16_t addr) {
  return mem->data[addr];
}
void mem_write(memory_t * mem, u8_t data, u16_t addr) {
  mem->data[addr] = data;
}
