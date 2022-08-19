
////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include "../6502_emulator/emulator.h"
#include "6502_tests.h"

int main(int argc, char const *argv[]) {
  emulator_t em;

  test_LDA(&em);
  test_LDX(&em);
  test_LDY(&em);
  test_JSR(&em);
  test_RTS(&em);
  test_STA(&em);
  test_STX(&em);
  test_STY(&em);
  test_JMP(&em);
  return 0;
}
