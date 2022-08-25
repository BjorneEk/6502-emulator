////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////
#include "cpu.h"

void reset_cpu(cpu_t * cpu) {
  cpu->PC = CPU_RESET;
  cpu->SP = RESET_STACK;
  cpu->irq = false;
  cpu->nmi = false;
  cpu->status = 0;
  cpu->A = 0;
  cpu->X = 0;
  cpu->Y = 0;
}
