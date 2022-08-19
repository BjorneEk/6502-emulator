
////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include "../6502_emulator/memory.h"
int main(int argc, char const *argv[]) {
  printf("new project!!!\n");
  memory_t mem;
  read_mem_file(&mem, "program.bin");
  print_mem(&mem);
  return 0;
}
