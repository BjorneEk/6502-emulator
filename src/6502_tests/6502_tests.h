////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////

#ifndef _6502_TESTS_H_
#define _6502_TESTS_H_

#include <stdbool.h>
#include "../6502_emulator/emulator.h"

void test(bool success, const char * msg);


void test_LDA(emulator_t * em);
void test_LDX(emulator_t * em);
void test_LDY(emulator_t * em);
void test_STA(emulator_t * em);
void test_STX(emulator_t * em);
void test_STY(emulator_t * em);
void test_JSR(emulator_t * em);
void test_RTS(emulator_t * em);
void test_JMP(emulator_t * em);


#endif /* _6502_TESTS_H_ */
