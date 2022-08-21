////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////

#ifndef _6502_TESTS_H_
#define _6502_TESTS_H_

#include <stdbool.h>
#include "../6502_emulator/m6502.h"

void test(bool success, const char * msg);


void test_LDA(m6502_t * em);
void test_LDX(m6502_t * em);
void test_LDY(m6502_t * em);
void test_STA(m6502_t * em);
void test_STX(m6502_t * em);
void test_STY(m6502_t * em);
void test_JSR(m6502_t * em);
void test_RTS(m6502_t * em);
void test_JMP(m6502_t * em);


#endif /* _6502_TESTS_H_ */
