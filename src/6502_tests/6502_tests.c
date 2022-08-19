////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////

#include "../6502_emulator/emulator.h"
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


void test_LD_IM(emulator_t * em, u8_t rs) {
  u8_t ins;
  u8_t * reg;
  switch(rs){
    case REGISTER_A:
      printf("LDA Imidiate test:\n");
      ins = INS_LDA_IM;
      reg = &em->cpu.A;
      break;
    case REGISTER_X:
      printf("LDX Imidiate test:\n");
      ins = INS_LDX_IM;
      reg = &em->cpu.X;
      break;
    case REGISTER_Y:
      printf("LDY Imidiate test:\n");
      ins = INS_LDY_IM;
      reg = &em->cpu.Y;
      break;
  }

  reset(em);

  write_byte(em, ins, em->cpu.PC);
  write_byte(em, 0x42, em->cpu.PC+1);

  test(2 == execute(em), "correct cycles");

  test(*reg == 0x42, "loaded correct value");
  test(!em->cpu.Z, "zero flag");
  test(!em->cpu.N, "negative flag");
}
void test_LD_ZP(emulator_t * em, u8_t rs) {
  u8_t ins;
  u8_t * reg;

  switch(rs){
    case REGISTER_A:
      printf("LDA ZeroPage test:\n");
      ins = INS_LDA_ZP;
      reg = &em->cpu.A;
      break;
    case REGISTER_X:
      printf("LDX ZeroPage test:\n");
      ins = INS_LDX_ZP;
      reg = &em->cpu.X;
      break;
    case REGISTER_Y:
      printf("LDY ZeroPage test:\n");
      ins = INS_LDY_ZP;
      reg = &em->cpu.Y;
      break;
  }

  reset(em);

  write_byte(em, ins, em->cpu.PC);
  write_byte(em, 0xA6, em->cpu.PC+1);
  write_byte(em, 0x42, 0xA6);

  test(3 == execute(em), "correct cycles");

  test(*reg == 0x42, "loaded correct value");
  test(!em->cpu.Z, "zero flag");
  test(!em->cpu.N, "negative flag");
}

void test_LDA_ZPX(emulator_t * em) {
  printf("LDA ZeroPageX test:\n");

  reset(em);

  em->cpu.X = 0x02;
  write_byte(em, INS_LDA_ZPX, em->cpu.PC);
  write_byte(em, 0xA6, em->cpu.PC + 1);
  write_byte(em, 0x42, 0xA6 + 0x02);

  test(4 == execute(em), "correct cycles");

  test(em->cpu.A == 0x42, "loaded correct value");
  test(!em->cpu.Z, "zero flag");
  test(!em->cpu.N, "negative flag");

}
void test_LDX_ZPY(emulator_t * em) {
  printf("LDX ZeroPageY test:\n");

  reset(em);

  em->cpu.Y = 0x02;
  write_byte(em, INS_LDX_ZPY, em->cpu.PC);
  write_byte(em, 0xA6, em->cpu.PC + 1);
  write_byte(em, 0x42, 0xA6 + 0x02);

  test(4 == execute(em), "correct cycles");

  test(em->cpu.X == 0x42, "loaded correct value");
  test(!em->cpu.Z, "zero flag");
  test(!em->cpu.N, "negative flag");

}
void test_LDY_ZPX(emulator_t * em) {
  printf("LDY ZeroPageX test:\n");

  reset(em);

  em->cpu.X = 0x02;
  write_byte(em, INS_LDY_ZPX, em->cpu.PC);
  write_byte(em, 0xA6, em->cpu.PC + 1);
  write_byte(em, 0x42, 0xA6 + 0x02);

  test(4 == execute(em), "correct cycles");

  test(em->cpu.Y == 0x42, "loaded correct value");
  test(!em->cpu.Z, "zero flag");
  test(!em->cpu.N, "negative flag");

}
void test_LD_ABS(emulator_t * em, u8_t rs) {
  u8_t ins;
  u8_t * reg;

  switch(rs){
    case REGISTER_A:
      printf("LDA Absolute test:\n");
      ins = INS_LDA_ABS;
      reg = &em->cpu.A;
      break;
    case REGISTER_X:
      printf("LDX Absolute test:\n");
      ins = INS_LDX_ABS;
      reg = &em->cpu.X;
      break;
    case REGISTER_Y:
      printf("LDY Absolute test:\n");
      ins = INS_LDY_ABS;
      reg = &em->cpu.Y;
      break;
  }

  reset(em);

  write_byte(em, ins, em->cpu.PC);
  write_word(em, 0xA6FF, em->cpu.PC + 1);
  write_word(em, 0x42, 0xA6FF);

  test(4 == execute(em), "correct cycles");

  test(*reg == 0x42, "loaded correct value");
  test(!em->cpu.Z, "zero flag");
  test(!em->cpu.N, "negative flag");

}

void test_LDA_ABSX(emulator_t * em) {
  printf("LDA AbsoluteX test:\n");

  reset(em);

  em->cpu.X = 0x02;

  write_byte(em, INS_LDA_ABSX, em->cpu.PC);
  write_word(em, 0xA6FF, em->cpu.PC + 1);
  write_word(em, 0x42, 0xA6FF + 0x02);

  test(5 == execute(em), "correct cycles crossed page boundry");

  test(em->cpu.A == 0x42, "loaded correct value");
  test(!em->cpu.Z, "zero flag");
  test(!em->cpu.N, "negative flag");

  reset(em);

  em->cpu.X = 0x02;
  write_byte(em, INS_LDA_ABSX, em->cpu.PC);
  write_word(em, 0xA600, em->cpu.PC + 1);
  write_word(em, 0x42, 0xA600 + 0x02);

  test(4 == execute(em), "correct cycles not crossed page boundry");
}

void test_LDA_ABSY(emulator_t * em) {
  printf("LDA AbsoluteY test:\n");

  reset(em);

  em->cpu.Y = 0x02;

  write_byte(em, INS_LDA_ABSY, em->cpu.PC);
  write_word(em, 0xA6FF, em->cpu.PC + 1);
  write_word(em, 0x42, 0xA6FF + 0x02);

  test(5 == execute(em), "correct cycles crossed page boundry");

  test(em->cpu.A == 0x42, "loaded correct value");
  test(!em->cpu.Z, "zero flag");
  test(!em->cpu.N, "negative flag");

  reset(em);

  em->cpu.Y = 0x02;
  write_byte(em, INS_LDA_ABSY, em->cpu.PC);
  write_word(em, 0xA600, em->cpu.PC + 1);
  write_word(em, 0x42, 0xA600 + 0x02);

  test(4 == execute(em), "correct cycles not crossed page boundry");
}

void test_LDX_ABSY(emulator_t * em) {
  printf("LDX AbsoluteY test:\n");

  reset(em);

  em->cpu.Y = 0x02;

  write_byte(em, INS_LDX_ABSY, em->cpu.PC);
  write_word(em, 0xA6FF, em->cpu.PC + 1);
  write_word(em, 0x42, 0xA6FF + 0x02);

  test(5 == execute(em), "correct cycles crossed page boundry");

  test(em->cpu.X == 0x42, "loaded correct value");
  test(!em->cpu.Z, "zero flag");
  test(!em->cpu.N, "negative flag");

  reset(em);

  em->cpu.Y = 0x02;
  write_byte(em, INS_LDX_ABSY, em->cpu.PC);
  write_word(em, 0xA600, em->cpu.PC + 1);
  write_word(em, 0x42, 0xA600 + 0x02);

  test(4 == execute(em), "correct cycles not crossed page boundry");
}

void test_LDY_ABSX(emulator_t * em) {
  printf("LDY AbsoluteX test:\n");

  reset(em);

  em->cpu.X = 0x02;

  write_byte(em, INS_LDY_ABSX, em->cpu.PC);
  write_word(em, 0xA6FF, em->cpu.PC + 1);
  write_word(em, 0x42, 0xA6FF + 0x02);

  test(5 == execute(em), "correct cycles crossed page boundry");

  test(em->cpu.Y == 0x42, "loaded correct value");
  test(!em->cpu.Z, "zero flag");
  test(!em->cpu.N, "negative flag");

  reset(em);

  em->cpu.X = 0x02;
  write_byte(em, INS_LDY_ABSX, em->cpu.PC);
  write_word(em, 0xA600, em->cpu.PC + 1);
  write_word(em, 0x42, 0xA600 + 0x02);

  test(4 == execute(em), "correct cycles not crossed page boundry");
}

void test_LDA_INDX(emulator_t * em) {
  printf("LDA InderectX test:\n");

  reset(em);

  em->cpu.X = 0x02;

  write_byte(em, INS_LDA_INDX, em->cpu.PC);
  write_byte(em, 0xA6, em->cpu.PC + 1);
  write_word(em, 0xABAB, 0xA6 + 0x02);
  write_byte(em, 0x42, 0xABAB);

  test(6 == execute(em), "correct cycles");

  test(em->cpu.A == 0x42, "loaded correct value");
  test(!em->cpu.Z, "zero flag");
  test(!em->cpu.N, "negative flag");
}

void test_LDA_INDY(emulator_t * em) {
  printf("LDA InderectY test:\n");

  reset(em);

  em->cpu.Y = 0x02;

  write_byte(em, INS_LDA_INDY, em->cpu.PC);
  write_byte(em, 0xA6, em->cpu.PC + 1);
  write_word(em, 0xA6FF, 0xA6);
  write_byte(em, 0x42, 0xA6FF + 0x02);

  test(6 == execute(em), "correct cycles crossed page boundry");

  test(em->cpu.A == 0x42, "loaded correct value");
  test(!em->cpu.Z, "zero flag");
  test(!em->cpu.N, "negative flag");

  reset(em);

  em->cpu.Y = 0x02;

  write_byte(em, INS_LDA_INDY, em->cpu.PC);
  write_byte(em, 0xA6, em->cpu.PC + 1);
  write_word(em, 0xA600, 0xA6);
  write_byte(em, 0x42, 0xA600 + 0x02);

  test(5 == execute(em), "correct cycles not crossed page boundry");
}

void test_ST_ZP(emulator_t * em, u8_t rs) {
  u8_t ins;
  u8_t * reg;

  switch(rs){
    case REGISTER_A:
      printf("STA ZeroPage test:\n");
      ins = INS_STA_ZP;
      reg = &em->cpu.A;
      break;
    case REGISTER_X:
      printf("STX ZeroPage test:\n");
      ins = INS_STX_ZP;
      reg = &em->cpu.X;
      break;
    case REGISTER_Y:
      printf("STY ZeroPage test:\n");
      ins = INS_STY_ZP;
      reg = &em->cpu.Y;
      break;
  }

  reset(em);

  *reg = 0x42;
  write_byte(em, ins, em->cpu.PC);
  write_byte(em, 0xA6, em->cpu.PC+1);

  test(3 == execute(em), "correct cycles");
  test(read_byte(em, 0x00A6) == 0x42, "stored correct value");
}

void test_STA_ZPX(emulator_t * em) {
  printf("STA ZeroPageX test:\n");
  reset(em);

  em->cpu.A = 0x42;
  em->cpu.X = 0x01;
  write_byte(em, INS_STA_ZPX, em->cpu.PC);
  write_byte(em, 0xA6, em->cpu.PC+1);

  test(4 == execute(em), "correct cycles");
  test(read_byte(em, 0x00A6 + 0x01) == 0x42, "stored correct value");
}

void test_STY_ZPX(emulator_t * em) {
  printf("STY ZeroPageX test:\n");
  reset(em);

  em->cpu.Y = 0x42;
  em->cpu.X = 0x01;
  write_byte(em, INS_STY_ZPX, em->cpu.PC);
  write_byte(em, 0xA6, em->cpu.PC+1);

  test(4 == execute(em), "correct cycles");
  test(read_byte(em, 0x00A6 + 0x01) == 0x42, "stored correct value");
}

void test_STX_ZPY(emulator_t * em) {
  printf("STY ZeroPageX test:\n");
  reset(em);

  em->cpu.X = 0x42;
  em->cpu.Y = 0x01;
  write_byte(em, INS_STX_ZPY, em->cpu.PC);
  write_byte(em, 0xA6, em->cpu.PC+1);

  test(4 == execute(em), "correct cycles");
  test(read_byte(em, 0x00A6 + 0x01) == 0x42, "stored correct value");
}
void test_ST_ABS(emulator_t * em, u8_t rs) {
  u8_t ins;
  u8_t * reg;

  switch(rs){
    case REGISTER_A:
      printf("STA Absolute test:\n");
      ins = INS_STA_ABS;
      reg = &em->cpu.A;
      break;
    case REGISTER_X:
      printf("STX Absolute test:\n");
      ins = INS_STX_ABS;
      reg = &em->cpu.X;
      break;
    case REGISTER_Y:
      printf("STY Absolute test:\n");
      ins = INS_STY_ABS;
      reg = &em->cpu.Y;
      break;
  }

  reset(em);

  *reg = 0x42;
  write_byte(em, ins, em->cpu.PC);
  write_word(em, 0xA6FF, em->cpu.PC+1);

  test(4 == execute(em), "correct cycles");
  test(read_byte(em, 0xA6FF) == 0x42, "stored correct value");
}
void test_STA_ABSX(emulator_t * em) {
  printf("STA AbsoluteX test:\n");
  reset(em);

  em->cpu.A = 0x42;
  em->cpu.X = 0x01;
  write_byte(em, INS_STA_ABSX, em->cpu.PC);
  write_word(em, 0xA6FF, em->cpu.PC+1);

  test(5 == execute(em), "correct cycles");
  test(read_byte(em, 0xA6FF + 0x01) == 0x42, "stored correct value");
}
void test_STA_ABSY(emulator_t * em) {
  printf("STA AbsoluteY test:\n");
  reset(em);

  em->cpu.A = 0x42;
  em->cpu.Y = 0x01;
  write_byte(em, INS_STA_ABSY, em->cpu.PC);
  write_word(em, 0xA6FF, em->cpu.PC+1);

  test(5 == execute(em), "correct cycles");
  test(read_byte(em, 0xA6FF + 0x01) == 0x42, "stored correct value");
}
void test_STA_INDX(emulator_t * em) {
  printf("STA InderectX test:\n");
  reset(em);

  em->cpu.A = 0x42;
  em->cpu.X = 0x01;
  write_byte(em, INS_STA_INDX, em->cpu.PC);
  write_byte(em, 0xA6, em->cpu.PC+1);
  write_word(em, 0xABAB, 0xA6 + 0x01);


  test(6 == execute(em), "correct cycles");
  test(read_byte(em, 0xABAB) == 0x42, "stored correct value");
}

void test_STA_INDY(emulator_t * em) {
  printf("STA InderectY test:\n");
  reset(em);

  em->cpu.A = 0x42;
  em->cpu.Y = 0x01;
  write_byte(em, INS_STA_INDY, em->cpu.PC);
  write_byte(em, 0xA6, em->cpu.PC+1);
  write_word(em, 0xABAB, 0xA6);

  test(6 == execute(em), "correct cycles");
  test(read_byte(em, 0xABAB + 0x01) == 0x42, "stored correct value");
}

void test_JMP_ABS(emulator_t * em) {
  printf("JMP Absolute test:\n");

  reset(em);

  write_byte(em, INS_JMP_ABS, em->cpu.PC);
  write_word(em, 0xA6FF, em->cpu.PC + 1);
  write_word(em, 0x42, 0xA6FF);

  test(3 == execute(em), "correct cycles");
  test(read_byte(em, em->cpu.PC) == 0x42, "correct jump location");
}

void test_JMP_IND(emulator_t * em) {
  printf("JMP Inderect test:\n");

  reset(em);

  write_byte(em, INS_JMP_IND, em->cpu.PC);
  write_word(em, 0xA6FF, em->cpu.PC + 1);
  write_word(em, 0xABAB, 0xA6FF);
  write_word(em, 0x42, 0xABAB);

  test(5 == execute(em), "correct cycles");
  test(read_byte(em, em->cpu.PC) == 0x42, "correct jump location");
}

void test_LDA(emulator_t * em) {
  test_LD_IM(em, REGISTER_A);
  test_LD_ZP(em, REGISTER_A);
  test_LDA_ZPX(em);
  test_LD_ABS(em, REGISTER_A);
  test_LDA_ABSX(em);
  test_LDA_ABSY(em);
  test_LDA_INDX(em);
  test_LDA_INDY(em);
}
void test_LDX(emulator_t * em) {
  test_LD_IM(em, REGISTER_X);
  test_LD_ZP(em, REGISTER_X);
  test_LDX_ZPY(em);
  test_LD_ABS(em, REGISTER_A);
  test_LDX_ABSY(em);
}
void test_LDY(emulator_t * em) {
  test_LD_IM(em, REGISTER_Y);
  test_LD_ZP(em, REGISTER_Y);
  test_LDX_ZPY(em);
  test_LD_ABS(em, REGISTER_Y);
  test_LDY_ABSX(em);
}
void test_STA(emulator_t * em) {
  test_ST_ZP(em, REGISTER_A);
  test_STA_ZPX(em);
  test_ST_ABS(em, REGISTER_A);
  test_STA_ABSX(em);
  test_STA_ABSY(em);
  test_STA_INDX(em);
  test_STA_INDY(em);
}
void test_STX(emulator_t * em) {
  test_ST_ZP(em, REGISTER_X);
  test_STX_ZPY(em);
  test_ST_ABS(em, REGISTER_X);
}
void test_STY(emulator_t * em) {
  test_ST_ZP(em, REGISTER_Y);
  test_STY_ZPX(em);
  test_ST_ABS(em, REGISTER_Y);
}
void test_JSR(emulator_t * em) {
  printf("JSR test:\n");

  reset(em);

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
void test_RTS(emulator_t * em) {
  printf("RTS test:\n");

  reset(em);

  write_byte(em, INS_JSR, em->cpu.PC);
  write_word(em, 0xA6FF, em->cpu.PC + 1);
  write_word(em, 0x42, em->cpu.PC + 3);
  write_byte(em, INS_RTS, 0xA6FF);

  execute(em);

  test(6 == execute(em), "correct cycles");
  test(read_byte(em, em->cpu.PC) == 0x42, "correct return location");
  test(em->cpu.SP == 0xFF, "stack pointert returned correctly");
}
void test_JMP(emulator_t * em) {
  test_JMP_ABS(em);
  test_JMP_IND(em);
}
