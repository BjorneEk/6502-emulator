////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////

#ifndef _M6502_H_
#define _M6502_H_

#define NO_DECIMAL_SUPORT

#include "cpu.h"
#include "memory.h"
#include "util.h"


typedef struct _6502_emulator {

  cpu_t cpu;
  memory_t mem;

  bool debug;
} m6502_t;

void reset(m6502_t * em);
void start_program(m6502_t * em);

void enable_debug(m6502_t * em);
void disable_debug(m6502_t * em);


m6502_t copy_state(m6502_t * em);

void debug(m6502_t * em);
void memset_word(m6502_t * em, u16_t data, u16_t addr); // endianess is not changed

u8_t read_byte(m6502_t * em, u16_t addr);
u16_t read_word(m6502_t * em, u16_t addr);

void write_byte(m6502_t * em, u8_t data, u16_t addr);
void write_word(m6502_t * em, u16_t data, u16_t addr);

u8_t fetch_byte(m6502_t * em);
i8_t fetch_s_byte(m6502_t * em);
u16_t fetch_word(m6502_t * em);

void push_byte(m6502_t * em, u8_t data);
void push_word(m6502_t * em, u16_t data);
u8_t pull_byte(m6502_t * em);
u16_t pull_word(m6502_t * em);

i32_t execute(m6502_t * em);

#endif /* _M6502_H_ */
