////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////

#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include "cpu.h"
#include "memory.h"
#include "util.h"


typedef struct _6502_emulator {

  cpu_t cpu;
  memory_t mem;

  bool debug;
} emulator_t;

void reset(emulator_t * em);
void start_program(emulator_t * em);

void enable_debug(emulator_t * em);
void disable_debug(emulator_t * em);


emulator_t copy_state(emulator_t * em);

void debug(emulator_t * em);
void memset_word(emulator_t * em, u16_t data, u16_t addr); // endianess is not changed
u8_t read_byte(emulator_t * em, u16_t addr);

u16_t read_word(emulator_t * em, u16_t addr);

void write_byte(emulator_t * em, u8_t data, u16_t addr);

void write_word(emulator_t * em, u16_t data, u16_t addr);
u8_t fetch(emulator_t * em);
i32_t execute(emulator_t * em);

#endif /* _EMULATOR_H_ */
