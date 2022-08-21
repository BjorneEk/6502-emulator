
////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include "../6502_emulator/m6502.h"
#include "6502_tests.h"

int main(int argc, char const *argv[]) {
  m6502_t em;

  test_LDA(&em);
  test_LDX(&em);
  test_LDY(&em);
  test_JSR(&em);
  test_RTS(&em);
  test_STA(&em);
  test_STX(&em);
  test_STY(&em);
  test_JMP(&em);
  reset(&em);
  read_mem_file(&em.mem,"program.bin");
  start_program(&em);
  enable_debug(&em);
  debug(&em);
  for(int i = 0; i < 1290; i++) execute(&em);
  execute(&em);
  execute(&em);
  debug(&em);
  execute(&em);
  debug(&em);
  execute(&em);
  debug(&em);
  execute(&em);
  debug(&em);
  execute(&em);
  debug(&em);
  execute(&em);
  debug(&em);
  execute(&em);
  debug(&em);
  execute(&em);
  debug(&em);
  execute(&em);

  return 0;
}
