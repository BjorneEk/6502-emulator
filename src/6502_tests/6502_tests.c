////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////

#include "../6502_emulator/m6502.h"
#include "6502_tests.h"
#include <stdio.h>

#define REGISTER_A 0
#define REGISTER_X 1
#define REGISTER_Y 2

void test(bool success, const char * msg) {
  if (!success)
  printf("[\033[31;1;4m Test failed \033[0m] %s\n", msg);
  else
    printf("[\033[32;1;4mTest succeded\033[0m] %s\n", msg);
}
void write_IM(m6502_t * em, u8_t ins, u8_t val, u16_t * addr) {
  write_byte(em, ins, em->cpu.PC);
  write_byte(em, val, em->cpu.PC+1);
  *addr = 0;
}

void write_ZP(m6502_t * em, u8_t ins, u8_t val, u16_t * addr) {
  write_byte(em, ins, em->cpu.PC);
  write_byte(em, 0xA6, em->cpu.PC+1);
  write_byte(em, val, 0xA6);
  *addr = 0xA6;
}
void write_ZPX(m6502_t * em, u8_t ins, u8_t val, u16_t * addr) {
  em->cpu.X = 0x02;
  write_byte(em, ins, em->cpu.PC);
  write_byte(em, 0xA6, em->cpu.PC + 1);
  write_byte(em, val, 0xA6 + 0x02);
  *addr = 0xA6 + 0x02;
}
void write_ZPY(m6502_t * em, u8_t ins, u8_t val, u16_t * addr) {
  em->cpu.Y = 0x02;
  write_byte(em, ins, em->cpu.PC);
  write_byte(em, 0xA6, em->cpu.PC + 1);
  write_byte(em, val, 0xA6 + 0x02);
  *addr = 0xA6 + 0x02;
}
void write_ABS(m6502_t * em, u8_t ins, u8_t val, u16_t * addr) {
  write_byte(em, ins, em->cpu.PC);
  write_word(em, 0xA6FF, em->cpu.PC + 1);
  write_word(em, val, 0xA6FF);
  *addr = 0xA6FF;
}
void write_ABSX(m6502_t * em, u8_t ins, u8_t val, u16_t * addr) {
  em->cpu.X = 0x02;
  write_byte(em, ins, em->cpu.PC);
  write_word(em, 0xA600, em->cpu.PC + 1);
  write_word(em, val, 0xA600 + 0x02);
  *addr = 0xA600 + 0x02;
}
void write_ABSX_5(m6502_t * em, u8_t ins, u8_t val, u16_t * addr) {
  em->cpu.X = 0x02;
  write_byte(em, ins, em->cpu.PC);
  write_word(em, 0xA6FF, em->cpu.PC + 1);
  write_word(em, val, 0xA6FF + 0x02);
  *addr = 0xA6FF + 0x02;
}
void write_ABSY(m6502_t * em, u8_t ins, u8_t val, u16_t * addr) {
  em->cpu.Y = 0x02;
  write_byte(em, ins, em->cpu.PC);
  write_word(em, 0xA600, em->cpu.PC + 1);
  write_word(em, val, 0xA600 + 0x02);
  *addr = 0xA600 + 0x02;
}
void write_ABSY_5(m6502_t * em, u8_t ins, u8_t val, u16_t * addr) {
  em->cpu.Y = 0x02;
  write_byte(em, ins, em->cpu.PC);
  write_word(em, 0xA6FF, em->cpu.PC + 1);
  write_word(em, val, 0xA6FF + 0x02);
  *addr = 0xA6FF + 0x02;
}
void write_INDX(m6502_t * em, u8_t ins, u8_t val, u16_t * addr) {
  em->cpu.X = 0x02;

  write_byte(em, ins, em->cpu.PC);
  write_byte(em, 0xA6, em->cpu.PC + 1);
  write_word(em, 0xABAB, 0xA6 + 0x02);
  write_byte(em, val, 0xABAB);
  *addr = 0xABAB;
}
void write_INDY_6(m6502_t * em, u8_t ins, u8_t val, u16_t * addr) {
  em->cpu.Y = 0x02;

  write_byte(em, ins, em->cpu.PC);
  write_byte(em, 0xA6, em->cpu.PC + 1);
  write_word(em, 0xA6FF, 0xA6);
  write_byte(em, val, 0xA6FF + 0x02);
  *addr = 0xA6FF + 0x02;
}
void write_INDY(m6502_t * em, u8_t ins, u8_t val, u16_t * addr) {
  em->cpu.Y = 0x02;

  write_byte(em, ins, em->cpu.PC);
  write_byte(em, 0xA6, em->cpu.PC + 1);
  write_word(em, 0xA600, 0xA6);
  write_byte(em, val, 0xA600 + 0x02);
  *addr = 0xA600 + 0x02;
}

void prep_LD_IM(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  write_IM(em, ins, val, addr);
}
void prep_LD_ZP(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  write_ZP(em, ins, val, addr);
}
void prep_LD_ZPX(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);

  enable_debug(em);
  write_ZPX(em, ins, val, addr);
}
void prep_LD_ZPY(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  write_ZPY(em, ins, val, addr);
}
void prep_LD_ABS(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  write_ABS(em, ins, val, addr);
}
void prep_LD_ABSX(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  write_ABSX(em, ins, val, addr);
}
void prep_LD_ABSX_5(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  write_ABSX_5(em, ins, val, addr);
}
void prep_LD_ABSY(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  write_ABSY(em, ins, val, addr);
}
void prep_LD_ABSY_5(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  write_ABSY_5(em, ins, val, addr);
}
void prep_LD_INDX(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  write_INDX(em, ins, val, addr);
}
void prep_LD_INDY_6(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  write_INDY_6(em, ins, val, addr);
}
void prep_LD_INDY(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  write_INDY(em, ins, val, addr);
}
void prep_ST_ZP(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  *reg = val;
  write_ZP(em, ins, 0x00, addr);
}
void prep_ST_ZPX(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  *reg = val;
  write_ZPX(em, ins, 0x00, addr);
}
void prep_ST_ZPY(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  *reg = val;
  write_ZPY(em, ins, 0x00, addr);
}
void prep_ST_ABS(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  *reg = val;
  write_ABS(em, ins, 0x00, addr);
}
void prep_ST_ABSX(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  *reg = val;
  write_ABSX(em, ins, 0x00, addr);
}
void prep_ST_ABSX_5(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  *reg = val;
  write_ABSX_5(em, ins, 0x00, addr);
}
void prep_ST_ABSY(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  *reg = val;
  write_ABSY(em, ins, 0x00, addr);
}
void prep_ST_INDX(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  *reg = val;
  write_INDX(em, ins, 0x00, addr);
}
void prep_ST_INDY(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr) {
  reset(em);
  enable_debug(em);
  *reg = val;
  write_INDY(em, ins, 0x00, addr);
}
void test_LD(m6502_t * em, u8_t *reg, u8_t operand,
  m6502_t pre_state, u16_t addr){
  test(*reg == operand, "loaded correct value");
  test(!em->cpu.Z, "zero flag");
  test(!em->cpu.N, "negative flag");
}
void test_ST(m6502_t * em, u8_t *reg, u8_t operand,
  m6502_t pre_state, u16_t addr){
  test(read_byte(em, addr) == operand, "stored correct value");
}
void test_ADC(m6502_t * em, u8_t *reg, u8_t operand,
  m6502_t pre_state, u16_t addr){
  const bool same_sign = !((em->cpu.A ^ operand) & 0b10000000);
  u16_t res;
  res = 0;
  res += pre_state.cpu.A;
  res += operand;
  test(em->cpu.A == (res & 0xFF), "correct addition result");
}

void test_INS(
  m6502_t * em, u8_t ins, u8_t * reg,  i32_t cycles,
  void (prep_func)(m6502_t * em, u8_t ins, u8_t * reg, u8_t val, u16_t * addr),
  void (test_func)(m6502_t * em, u8_t *reg, u8_t operand,m6502_t pre_state, u16_t addr)) {
  u16_t addr;
  prep_func(em, ins, reg, 0x42, &addr);
  m6502_t pre_state = copy_state(em);
  test(cycles == execute(em), "correct cycles");
  test_func(em, reg, 0x42, pre_state, addr);
}

void test_JMP_ABS(m6502_t * em) {
  printf("JMP Absolute test:\n");

  reset(em);
  enable_debug(em);

  write_byte(em, INS_JMP_ABS, em->cpu.PC);
  write_word(em, 0xA6FF, em->cpu.PC + 1);
  write_word(em, 0x42, 0xA6FF);

  test(3 == execute(em), "correct cycles");
  test(read_byte(em, em->cpu.PC) == 0x42, "correct jump location");
}

void test_JMP_IND(m6502_t * em) {
  printf("JMP Inderect test:\n");

  reset(em);
  enable_debug(em);

  write_byte(em, INS_JMP_IND, em->cpu.PC);
  write_word(em, 0xA6FF, em->cpu.PC + 1);
  write_word(em, 0xABAB, 0xA6FF);
  write_word(em, 0x42, 0xABAB);

  test(5 == execute(em), "correct cycles");
  test(read_byte(em, em->cpu.PC) == 0x42, "correct jump location");
}

void test_LDA(m6502_t * em) {
  printf("LDA Imidiate test:\n");
  test_INS(em, INS_LDA_IM, &em->cpu.A, 2, prep_LD_IM, test_LD);
  printf("LDA ZeroPage test:\n");
  test_INS(em, INS_LDA_ZP, &em->cpu.A, 3, prep_LD_ZP, test_LD);
  printf("LDA ZeroPageX test:\n");
  test_INS(em, INS_LDA_ZPX, &em->cpu.A, 4, prep_LD_ZPX, test_LD);
  printf("LDA Absolute test:\n");
  test_INS(em, INS_LDA_ABS, &em->cpu.A, 4, prep_LD_ABS, test_LD);
  printf("LDA AbsoluteX 5 cycle test:\n");
  test_INS(em, INS_LDA_ABSX, &em->cpu.A, 5, prep_LD_ABSX_5, test_LD);
  printf("LDA AbsoluteX 4 cycle test:\n");
  test_INS(em, INS_LDA_ABSX, &em->cpu.A, 4, prep_LD_ABSX, test_LD);
  printf("LDA AbsoluteY 5 cycle test:\n");
  test_INS(em, INS_LDA_ABSY, &em->cpu.A, 5, prep_LD_ABSY_5, test_LD);
  printf("LDA AbsoluteY 4 cycle test:\n");
  test_INS(em, INS_LDA_ABSY, &em->cpu.A, 4, prep_LD_ABSY, test_LD);
  printf("LDA InderectX test:\n");
  test_INS(em, INS_LDA_INDX, &em->cpu.A, 6, prep_LD_INDX, test_LD);
  printf("LDA InderectY 6 cycle test test:\n");
  test_INS(em, INS_LDA_INDY, &em->cpu.A, 6, prep_LD_INDY_6, test_LD);
  printf("LDA InderectY 5 cycle test test:\n");
  test_INS(em, INS_LDA_INDY, &em->cpu.A, 5, prep_LD_INDY, test_LD);
}
void test_LDX(m6502_t * em) {
  printf("LDX Imidiate test:\n");
  test_INS(em, INS_LDX_IM, &em->cpu.X, 2, prep_LD_IM, test_LD);
  printf("LDX ZeroPage test:\n");
  test_INS(em, INS_LDX_ZP, &em->cpu.X, 3, prep_LD_ZP, test_LD);
  printf("LDX ZeroPageY test:\n");
  test_INS(em, INS_LDX_ZPY, &em->cpu.X, 4, prep_LD_ZPY, test_LD);
  printf("LDX Absolute test:\n");
  test_INS(em, INS_LDX_ABS, &em->cpu.X, 4, prep_LD_ABS, test_LD);
  printf("LDX AbsoluteY 5 cycle test:\n");
  test_INS(em, INS_LDX_ABSY, &em->cpu.X, 5, prep_LD_ABSY_5, test_LD);
  printf("LDX AbsoluteY 4 cycle test:\n");
  test_INS(em, INS_LDX_ABSY, &em->cpu.X, 4, prep_LD_ABSY, test_LD);
}
void test_LDY(m6502_t * em) {
  printf("LDY Imidiate test:\n");
  test_INS(em, INS_LDY_IM, &em->cpu.Y, 2, prep_LD_IM, test_LD);
  printf("LDY ZeroPage test:\n");
  test_INS(em, INS_LDY_ZP, &em->cpu.Y, 3, prep_LD_ZP, test_LD);
  printf("LDY ZeroPageX test:\n");
  test_INS(em, INS_LDY_ZPX, &em->cpu.Y, 4, prep_LD_ZPX, test_LD);
  printf("LDY Absolute test:\n");
  test_INS(em, INS_LDY_ABS, &em->cpu.Y, 4, prep_LD_ABS, test_LD);
  printf("LDY AbsoluteY 5 cycle test:\n");
  test_INS(em, INS_LDY_ABSX, &em->cpu.Y, 5, prep_LD_ABSX_5, test_LD);
  printf("LDY AbsoluteY 4 cycle test:\n");
  test_INS(em, INS_LDY_ABSX, &em->cpu.Y, 4, prep_LD_ABSX, test_LD);
}
void test_STA(m6502_t * em) {
  printf("STA ZeroPage test:\n");
  test_INS(em, INS_STA_ZP, &em->cpu.A, 3, prep_ST_ZP, test_ST);
  printf("STA ZeroPageX test:\n");
  test_INS(em, INS_STA_ZPX, &em->cpu.A, 4, prep_ST_ZPX, test_ST);
  printf("STA Absolute test:\n");
  test_INS(em, INS_STA_ABS, &em->cpu.A, 4, prep_ST_ABS, test_ST);
  printf("STA AbsoluteX test:\n");
  test_INS(em, INS_STA_ABSX, &em->cpu.A, 5, prep_ST_ABSX, test_ST);
  printf("STA AbsoluteY test:\n");
  test_INS(em, INS_STA_ABSY, &em->cpu.A, 5, prep_ST_ABSY, test_ST);
  printf("STA InderectX test:\n");
  test_INS(em, INS_STA_INDX, &em->cpu.A, 6, prep_ST_INDX, test_ST);
  printf("STA InderectY test:\n");
  test_INS(em, INS_STA_INDY, &em->cpu.A, 6, prep_ST_INDY, test_ST);
}
void test_STX(m6502_t * em) {
  printf("STX ZeroPage test:\n");
  test_INS(em, INS_STX_ZP, &em->cpu.X, 3, prep_ST_ZP, test_ST);
  printf("STX ZeroPageY test:\n");
  test_INS(em, INS_STX_ZPY, &em->cpu.X, 4, prep_ST_ZPY, test_ST);
  printf("STX Absolute test:\n");
  test_INS(em, INS_STX_ABS, &em->cpu.X, 4, prep_ST_ABS, test_ST);
}
void test_STY(m6502_t * em) {
  printf("STY ZeroPage test:\n");
  test_INS(em, INS_STY_ZP, &em->cpu.Y, 3, prep_ST_ZP, test_ST);
  printf("STY ZeroPageX test:\n");
  test_INS(em, INS_STY_ZPX, &em->cpu.Y, 4, prep_ST_ZPX, test_ST);
  printf("STY Absolute test:\n");
  test_INS(em, INS_STY_ABS, &em->cpu.Y, 4, prep_ST_ABS, test_ST);
}
void test_JSR(m6502_t * em) {
  printf("JSR test:\n");

  reset(em);
  enable_debug(em);

  write_byte(em, INS_JSR, em->cpu.PC);
  write_word(em, 0xA6FF, em->cpu.PC + 1);
  write_byte(em, 0x42, 0xA6FF);
  u16_t prev_pc = em->cpu.PC + 3;

  test(6 == execute(em), "correct cycles");
  test(read_byte(em, em->cpu.PC) == 0x42, "correct jump location");
  test(em->cpu.SP == 0xFF-2, "stack pointert decremented");
  test(prev_pc == read_word(em, 0x0100 | em->cpu.SP+1),
  "stack contains previous program counter");
}
void test_RTS(m6502_t * em) {
  printf("RTS test:\n");

  reset(em);
  enable_debug(em);

  write_byte(em, INS_JSR, em->cpu.PC);
  write_word(em, 0xA6FF, em->cpu.PC + 1);
  write_word(em, 0x42, em->cpu.PC + 3);
  write_byte(em, INS_RTS, 0xA6FF);

  execute(em);

  test(6 == execute(em), "correct cycles");
  test(read_byte(em, em->cpu.PC) == 0x42, "correct return location");
  test(em->cpu.SP == 0xFF, "stack pointert returned correctly");
}
void test_JMP(m6502_t * em) {
  test_JMP_ABS(em);
  test_JMP_IND(em);
}
