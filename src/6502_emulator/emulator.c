////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////

#include "emulator.h"
#include <stdio.h>


void reset(emulator_t * em) {
  init_mem(&em->mem);
  reset_cpu(&em->cpu);
}

void memset_word(emulator_t * em, u16_t data, u16_t addr) {
  mem_write(&em->mem, (u8_t)(data >> 8), addr);
  mem_write(&em->mem, (u8_t)data, addr+1);
}

u8_t fetch_byte(emulator_t * em) {
  u8_t data;
  data = mem_read(&em->mem, em->cpu.PC);
  em->cpu.PC++;
  return data;
}
i8_t fetch_s_byte(emulator_t * em) {
  return fetch_byte(em);
}

u16_t fetch_word(emulator_t * em) {
  u8_t lb;
  u16_t hb;
  lb = fetch_byte(em);
  hb = fetch_byte(em);
  hb = hb << 8;
  hb |= lb;
  return hb;
}

u8_t read_byte(emulator_t * em, u16_t addr) {
  u8_t data;
  data = mem_read(&em->mem, addr);
  return data;
}

u16_t read_word(emulator_t * em, u16_t addr) {
  u8_t lb;
  u16_t hb;
  lb = read_byte(em, addr);
  hb = read_byte(em, addr + 1);
  hb = hb << 8;
  hb |= lb;
  return hb;
}

void write_byte(emulator_t * em, u8_t data, u16_t addr) {
  mem_write(&em->mem, data, addr);
}

void write_word(emulator_t * em, u16_t data, u16_t addr) {
  u8_t lb;
  u8_t hb;

  hb = data >> 8;
  lb = (u8_t)data;
  write_byte(em, lb, addr);
  write_byte(em, hb, addr + 1);
}
void push_byte(emulator_t * em, u8_t data) {
  write_word(em, data, 0x0100 | (em->cpu.SP--));
}
void push_word(emulator_t * em, u16_t data) {
  write_word(em, data, 0x0100 | (em->cpu.SP-1));
  em->cpu.SP -= 2;
}

u8_t pull_byte(emulator_t * em) {
  u8_t data;
  em->cpu.SP++;
  data = read_byte(em, 0x0100 | em->cpu.SP);
  write_byte(em, 0x00, 0x0100 | em->cpu.SP);
  return data;
}
u16_t pull_word(emulator_t * em) {
  u8_t lb;
  u16_t hb;
  lb = pull_byte(em);
  hb = pull_byte(em);
  hb = hb << 8;
  hb |= lb;
  return hb;
}

void set_flags(emulator_t * em, u8_t * reg){
  em->cpu.Z = (*reg == 0);
  em->cpu.N = (*reg & 0b10000000) != 0;
}

void load_register(emulator_t * em, u8_t * reg, u16_t addr) {
  *reg = read_byte(em, addr);
  set_flags(em, reg);
}

u16_t addr_abs_reg(emulator_t * em, u8_t * reg, i32_t * cycles){
  u16_t ABS_addr = fetch_word(em);
  u16_t ABS_addr_reg = ABS_addr + *reg;
  if((ABS_addr ^ ABS_addr_reg) >> 8) (*cycles)++; //page boundry is crossed
  return ABS_addr_reg;
}
u16_t addr_abs_reg_5(emulator_t * em, u8_t * reg){
  return fetch_word(em) + *reg;
}


void ADC(emulator_t * em, u8_t operand) {
  assert_w(em->cpu.D, "ADC: Decimal mode not yet suported");
  const bool same_sign = !((em->cpu.A ^ operand) & 0b10000000);
  u16_t sum = em->cpu.A;
  sum += operand;
  sum += em->cpu.C;
  em->cpu.A = (sum & 0xFF);
  set_flags(em, &em->cpu.A);
  em->cpu.C = sum > 0xFF;
  // set overflow if both operands are neggative
  em->cpu.V = same_sign && ((em->cpu.A ^ operand) & 0b10000000);
}
void SBC(emulator_t * em, u8_t operand) {
  ADC(em, ~operand);
}

u8_t ASL(emulator_t * em, u8_t operand) {
  em->cpu.C = (operand & 0b10000000) > 0;
  u8_t res = operand << 1;
  set_flags(em, &res);
  return res;
}

u8_t LSR(emulator_t * em, u8_t operand) {
  em->cpu.C = (operand & 0b00000001) > 0;
  u8_t res = operand >> 1;
  set_flags(em, &res);
  return res;
}

void compare(emulator_t * em, u8_t operand, u8_t reg) {
  u8_t tmp = reg - operand;
  em->cpu.N = (tmp & 0b10000000) > 0;
  em->cpu.Z = reg == operand;
  em->cpu.C = reg >= operand;
}

void branch_if(emulator_t * em, i32_t * cycles, bool test, bool expected){
  i8_t Offset = fetch_s_byte(em);
  if ( test == expected ) {
    cycles++;
    const u16_t PCOld = em->cpu.PC;
    em->cpu.PC += Offset;
    if((em->cpu.PC >> 8) != (PCOld >> 8))cycles++;
  }
}


i32_t execute(emulator_t * em) {
  u8_t ins, data;
  u8_t ZP_addr, addr_lb;
  u16_t ABS_addr, ABS_addrX, ABS_addrY, sub_addr;
  i32_t cycles;
  ins = fetch_byte(em);
  switch(ins){
    /**
     *  ADC instructions
     **/
    case INS_ADC_IM:
      cycles = 2;
      ADC(em, fetch_byte(em));
      break;

    case INS_ADC_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      ADC(em, read_byte(em, ZP_addr));
      break;

    case INS_ADC_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      ADC(em, read_byte(em, ZP_addr));
      break;

    case INS_ADC_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      ADC(em, read_byte(em, ABS_addr));
      break;

    case INS_ADC_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.X, &cycles);
      ADC(em, read_byte(em, ABS_addr));
      break;

    case INS_ADC_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.Y, &cycles);
      ADC(em, read_byte(em, ABS_addr));
      break;

    case INS_ADC_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      ADC(em, read_byte(em, read_word(em, ZP_addr)));
      break;

    case INS_ADC_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + em->cpu.Y;
      if((ABS_addr ^ ABS_addrY) >> 8) cycles++; //page boundry is crossed
      ADC(em, read_byte(em, ABS_addrY));
      break;
    /**
     *  SBC instructions
     **/
    case INS_SBC_IM:
      cycles = 2;
      SBC(em, fetch_byte(em));
      break;

    case INS_SBC_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      SBC(em, read_byte(em, ZP_addr));
      break;

    case INS_SBC_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      SBC(em, read_byte(em, ZP_addr));
      break;

    case INS_SBC_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      SBC(em, read_byte(em, ABS_addr));
      break;

    case INS_SBC_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.X, &cycles);
      SBC(em, read_byte(em, ABS_addr));
      break;

    case INS_SBC_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.Y, &cycles);
      SBC(em, read_byte(em, ABS_addr));
      break;

    case INS_SBC_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      SBC(em, read_byte(em, read_word(em, ZP_addr)));
      break;

    case INS_SBC_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + em->cpu.Y;
      if((ABS_addr ^ ABS_addrY) >> 8) cycles++; //page boundry is crossed
      SBC(em, read_byte(em, ABS_addrY));
      break;
    /**
     *  CMP instructions
     **/
    case INS_CMP_IM:
      cycles = 2;
      compare(em, fetch_byte(em), em->cpu.A);
      break;

    case INS_CMP_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      compare(em, read_byte(em, ZP_addr), em->cpu.A);
      break;

    case INS_CMP_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      compare(em, read_byte(em, ZP_addr), em->cpu.A);
      break;

    case INS_CMP_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      compare(em, read_byte(em, ABS_addr), em->cpu.A);
      break;

    case INS_CMP_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.X, &cycles);
      compare(em, read_byte(em, ABS_addr), em->cpu.A);
      break;

    case INS_CMP_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.Y, &cycles);
      compare(em, read_byte(em, ABS_addr), em->cpu.A);
      break;

    case INS_CMP_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      compare(em, read_byte(em, read_word(em, ZP_addr)), em->cpu.A);
      break;

    case INS_CMP_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + em->cpu.Y;
      if((ABS_addr ^ ABS_addrY) >> 8) cycles++; //page boundry is crossed
      compare(em, read_byte(em, ABS_addrY), em->cpu.A);
      break;
    /**
     *  CPX instructions
     **/
    case INS_CPX_IM:
      cycles = 2;
      compare(em, fetch_byte(em), em->cpu.X);
      break;

    case INS_CPX_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      compare(em, read_byte(em, ZP_addr), em->cpu.X);
      break;

    case INS_CPX_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      compare(em, read_byte(em, ABS_addr), em->cpu.X);
      break;
    /**
     *  CPY instructions
     **/
    case INS_CPY_IM:
      cycles = 2;
      compare(em, fetch_byte(em), em->cpu.Y);
      break;

    case INS_CPY_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      compare(em, read_byte(em, ZP_addr), em->cpu.Y);
      break;

    case INS_CPY_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      compare(em, read_byte(em, ABS_addr), em->cpu.Y);
      break;

    /**
     *  AND instructions
     **/
    case INS_AND_IM:
      cycles = 2;
      em->cpu.A &= fetch_byte(em);
      set_flags(em, &em->cpu.A);
      break;

    case INS_AND_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      em->cpu.A &= read_byte(em, ZP_addr);
      set_flags(em, &em->cpu.A);
      break;

    case INS_AND_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      em->cpu.A &= read_byte(em, ZP_addr);
      set_flags(em, &em->cpu.A);
      break;

    case INS_AND_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      em->cpu.A &= read_byte(em, ABS_addr);
      set_flags(em, &em->cpu.A);
      break;

    case INS_AND_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.X, &cycles);
      em->cpu.A &= read_byte(em, ABS_addr);
      set_flags(em, &em->cpu.A);
      break;

    case INS_AND_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.Y, &cycles);
      em->cpu.A &= read_byte(em, ABS_addr);
      set_flags(em, &em->cpu.A);
      break;

    case INS_AND_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      em->cpu.A &= read_byte(em, read_word(em, ZP_addr));
      set_flags(em, &em->cpu.A);
      break;

    case INS_AND_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + em->cpu.Y;
      if((ABS_addr ^ ABS_addrY) >> 8) cycles++; //page boundry is crossed
      em->cpu.A &= read_byte(em, ABS_addrY);
      set_flags(em, &em->cpu.A);
      break;
    /**
     *  ORA instructions
     **/
    case INS_ORA_IM:
      cycles = 2;
      em->cpu.A |= fetch_byte(em);
      set_flags(em, &em->cpu.A);
      break;

    case INS_ORA_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      em->cpu.A &= read_byte(em, ZP_addr);
      set_flags(em, &em->cpu.A);
      break;

    case INS_ORA_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      em->cpu.A |= read_byte(em, ZP_addr);
      set_flags(em, &em->cpu.A);
      break;

    case INS_ORA_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      em->cpu.A |= read_byte(em, ABS_addr);
      set_flags(em, &em->cpu.A);
      break;

    case INS_ORA_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.X, &cycles);
      em->cpu.A |= read_byte(em, ABS_addr);
      set_flags(em, &em->cpu.A);
      break;

    case INS_ORA_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.Y, &cycles);
      em->cpu.A |= read_byte(em, ABS_addr);
      set_flags(em, &em->cpu.A);
      break;

    case INS_ORA_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      em->cpu.A |= read_byte(em, read_word(em, ZP_addr));
      set_flags(em, &em->cpu.A);
      break;

    case INS_ORA_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + em->cpu.Y;
      if((ABS_addr ^ ABS_addrY) >> 8) cycles++; //page boundry is crossed
      em->cpu.A |= read_byte(em, ABS_addrY);
      set_flags(em, &em->cpu.A);
      break;
    /**
     *  EOR instructions
     **/
    case INS_EOR_IM:
      cycles = 2;
      em->cpu.A ^= fetch_byte(em);
      set_flags(em, &em->cpu.A);
      break;

    case INS_EOR_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      em->cpu.A |= read_byte(em, ZP_addr);
      set_flags(em, &em->cpu.A);
      break;

    case INS_EOR_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      em->cpu.A ^= read_byte(em, ZP_addr);
      set_flags(em, &em->cpu.A);
      break;

    case INS_EOR_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      em->cpu.A ^= read_byte(em, ABS_addr);
      set_flags(em, &em->cpu.A);
      break;

    case INS_EOR_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.X, &cycles);
      em->cpu.A ^= read_byte(em, ABS_addr);
      set_flags(em, &em->cpu.A);
      break;

    case INS_EOR_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.Y, &cycles);
      em->cpu.A ^= read_byte(em, ABS_addr);
      set_flags(em, &em->cpu.A);
      break;

    case INS_EOR_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      em->cpu.A ^= read_byte(em, read_word(em, ZP_addr));
      set_flags(em, &em->cpu.A);
      break;

    case INS_EOR_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + em->cpu.Y;
      if((ABS_addr ^ ABS_addrY) >> 8) cycles++; //page boundry is crossed
      em->cpu.A ^= read_byte(em, ABS_addrY);
      set_flags(em, &em->cpu.A);
      break;
    /**
     *  BRANCH instructions
     **/
    case INS_BCC:
      cycles = 1;
      branch_if(em, &cycles, em->cpu.C, false);
      break;

    case INS_BCS:
      cycles = 1;
      branch_if(em, &cycles, em->cpu.C, true);
      break;

    case INS_BEQ:
      cycles = 1;
      branch_if(em, &cycles, em->cpu.Z, true);
      break;

    case INS_BMI:
      cycles = 1;
      branch_if(em, &cycles, em->cpu.N, true);
      break;

    case INS_BNE:
      cycles = 1;
      branch_if(em, &cycles, em->cpu.Z, false);
      break;

    case INS_BPL:
      cycles = 1;
      branch_if(em, &cycles, em->cpu.N, false);
      break;

    case INS_BVC:
      cycles = 1;
      branch_if(em, &cycles, em->cpu.V, false);
      break;

    case INS_BVS:
      cycles = 1;
      branch_if(em, &cycles, em->cpu.V, true);
      break;
    /**
     *  BIT instructions
     **/
    case INS_BIT_ZP:
      cycles = 2;
      ZP_addr = fetch_byte(em);
      data = read_byte(em, ZP_addr);
      em->cpu.Z = !(data & em->cpu.A);
      em->cpu.N =  (data & 0b10000000) != 0;
      em->cpu.V =  (data & 0b01000000) != 0;
      break;

    case INS_BIT_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      data = read_byte(em, ABS_addr);
      em->cpu.Z = !(data & em->cpu.A);
      em->cpu.N =  (data & 0b10000000) != 0;
      em->cpu.V =  (data & 0b01000000) != 0;
      break;

    case INS_CLC:
      cycles = 2;
      em->cpu.C = 0;
      break;

    case INS_CLD:
      cycles = 2;
      em->cpu.D = 0;
      break;

    case INS_CLI:
      cycles = 2;
      em->cpu.I = 0;
      break;

    case INS_CLV:
      cycles = 2;
      em->cpu.V = 0;
      break;
    /**
     *  DEC instructions
     **/
    case INS_DEC_ZP:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      data = read_byte(em, ZP_addr);
      data--;
      set_flags(em, &data);
      write_byte(em, data, ZP_addr);
      break;

    case INS_DEC_ZPX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      data = read_byte(em, ZP_addr);
      data--;
      set_flags(em, &data);
      write_byte(em, data, ZP_addr);
      break;

    case INS_DEC_ABS:
      cycles = 6;
      ABS_addr = fetch_word(em);
      data = read_byte(em, ABS_addr);
      data--;
      set_flags(em, &data);
      write_byte(em, data, ABS_addr);
      break;

    case INS_DEC_ABSX:
      cycles = 7;
      ABS_addr = fetch_word(em) + em->cpu.X;
      data = read_byte(em, ABS_addr);
      data--;
      set_flags(em, &data);
      write_byte(em, data, ABS_addr);
      break;
    /**
     *  DEY-DEX instructions
     **/
    case INS_DEX:
      cycles = 2;
      em->cpu.X--;
      set_flags(em, &em->cpu.X);
      break;

    case INS_DEY:
      cycles = 2;
      em->cpu.Y--;
      set_flags(em, &em->cpu.Y);
      break;
    /**
     *  INC instructions
     **/
    case INS_INC_ZP:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      data = read_byte(em, ZP_addr);
      data++;
      set_flags(em, &data);
      write_byte(em, data, ZP_addr);
      break;

    case INS_INC_ZPX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      data = read_byte(em, ZP_addr);
      data++;
      set_flags(em, &data);
      write_byte(em, data, ZP_addr);
      break;

    case INS_INC_ABS:
      cycles = 6;
      ABS_addr = fetch_word(em);
      data = read_byte(em, ABS_addr);
      data++;
      set_flags(em, &data);
      write_byte(em, data, ABS_addr);
      break;

    case INS_INC_ABSX:
      cycles = 7;
      ABS_addr = fetch_word(em) + em->cpu.X;
      data = read_byte(em, ABS_addr);
      data++;
      set_flags(em, &data);
      write_byte(em, data, ABS_addr);
      break;
    /**
     *  INX-INY instructions
     **/
    case INS_INX:
      cycles = 2;
      em->cpu.X++;
      set_flags(em, &em->cpu.X);
      break;

    case INS_INY:
      cycles = 2;
      em->cpu.Y++;
      set_flags(em, &em->cpu.Y);
      break;
    /**
     *  JSR-RTS instructions
     **/
    case INS_JSR:
      cycles = 6;
      sub_addr = fetch_word(em);
      push_word(em, em->cpu.PC);
      em->cpu.PC = sub_addr;
      break;

    case INS_RTS:
      cycles = 6;
      em->cpu.PC = pull_word(em);
      break;

    case INS_JMP_ABS:
      cycles = 3;
      em->cpu.PC = fetch_word(em);
      break;

    case INS_JMP_IND:
      cycles = 5;
      em->cpu.PC = read_word(em, fetch_word(em));
      break;
    /**
     *  LDA instructions
     **/
    case INS_LDA_IM:
      cycles = 2;
      em->cpu.A = fetch_byte(em);
      set_flags(em, &em->cpu.A);
      break;

    case INS_LDA_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      load_register(em, &em->cpu.A, ZP_addr);
      break;

    case INS_LDA_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      load_register(em, &em->cpu.A, ZP_addr);
      break;

    case INS_LDA_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      load_register(em, &em->cpu.A, ABS_addr);
      break;

    case INS_LDA_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.X, &cycles);
      load_register(em, &em->cpu.A, ABS_addr);
      break;

    case INS_LDA_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.Y, &cycles);
      load_register(em, &em->cpu.A, ABS_addr);
      break;

    case INS_LDA_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      load_register(em, &em->cpu.A, read_word(em, ZP_addr));
      break;

    case INS_LDA_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + em->cpu.Y;
      if((ABS_addr ^ ABS_addrY) >> 8) cycles++; //page boundry is crossed
      load_register(em, &em->cpu.A, ABS_addrY);
      break;
    /**
     *  LDX instructions
     **/
    case INS_LDX_IM:
      cycles = 2;
      em->cpu.X = fetch_byte(em);
      set_flags(em, &em->cpu.X);
      break;

    case INS_LDX_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      load_register(em, &em->cpu.X, ZP_addr);
    break;

    case INS_LDX_ZPY:
      cycles = 4;
      ZP_addr = fetch_byte(em) + em->cpu.Y;
      load_register(em, &em->cpu.X, ZP_addr);
      break;

    case INS_LDX_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      load_register(em, &em->cpu.X, ABS_addr);
      break;

    case INS_LDX_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.Y, &cycles);
      load_register(em, &em->cpu.X, ABS_addr);
      break;
    /**
     *  LDY instructions
     **/
    case INS_LDY_IM:
      cycles = 2;
      em->cpu.Y = fetch_byte(em);
      set_flags(em, &em->cpu.Y);
      break;

    case INS_LDY_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      load_register(em, &em->cpu.Y, ZP_addr);
    break;

    case INS_LDY_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      load_register(em, &em->cpu.Y, ZP_addr);
      break;

    case INS_LDY_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      load_register(em, &em->cpu.Y, ABS_addr);
      break;

    case INS_LDY_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.X, &cycles);
      load_register(em, &em->cpu.Y, ABS_addr);
      break;
    /**
     *  STA instructions
     **/
    case INS_STA_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      write_byte(em, em->cpu.A, ZP_addr);
      break;

    case INS_STA_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      write_byte(em, em->cpu.A, ZP_addr);
      break;

    case INS_STA_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      write_byte(em, em->cpu.A, ABS_addr);
      break;

    case INS_STA_ABSX:
      cycles = 5;
      ABS_addr = addr_abs_reg_5(em, &em->cpu.X);
      write_byte(em, em->cpu.A, ABS_addr);
      break;

    case INS_STA_ABSY:
      cycles = 5;
      ABS_addr = addr_abs_reg_5(em, &em->cpu.Y);
      write_byte(em, em->cpu.A, ABS_addr);
      break;

    case INS_STA_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      write_byte(em, em->cpu.A, read_word(em, ZP_addr));
      break;

    case INS_STA_INDY:
      cycles = 6;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      write_byte(em, em->cpu.A, ABS_addr + em->cpu.Y);
      break;
    /**
     *  STX instructions
     **/
    case INS_STX_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      write_byte(em, em->cpu.X, ZP_addr);
      break;

    case INS_STX_ZPY:
      cycles = 4;
      ZP_addr = fetch_byte(em) + em->cpu.Y;
      write_byte(em, em->cpu.X, ZP_addr);
      break;

    case INS_STX_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      write_byte(em, em->cpu.X, ABS_addr);
      break;
    /**
     *  STY instructions
     **/
    case INS_STY_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      write_byte(em, em->cpu.Y, ZP_addr);
      break;

    case INS_STY_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      write_byte(em, em->cpu.Y, ZP_addr);
      break;

    case INS_STY_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      write_byte(em, em->cpu.Y, ABS_addr);
      break;
    /**
     *  LSR instructions
     **/
    case INS_LSR_ACC:
      cycles = 2;
      em->cpu.A = LSR(em, em->cpu.A);
      break;

    case INS_LSR_ZP:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      data = read_byte(em, ZP_addr);
      write_byte(em, LSR(em, data), ZP_addr);
      break;

    case INS_LSR_ZPX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      data = read_byte(em, ZP_addr);
      write_byte(em, LSR(em, data), ZP_addr);
      break;

    case INS_LSR_ABS:
      cycles = 6;
      ABS_addr = fetch_word(em);
      data = read_byte(em, ABS_addr);
      write_byte(em, LSR(em, data), ABS_addr);
      break;

    case INS_LSR_ABSX:
      cycles = 7;
      ABS_addr = fetch_word(em) + em->cpu.X;
      data = read_byte(em, ABS_addr);
      write_byte(em, LSR(em, data), ABS_addr);
      break;
    /**
     *  ASL instructions
     **/
    case INS_ASL_ACC:
      cycles = 2;
      em->cpu.A = ASL(em, em->cpu.A);
      break;

    case INS_ASL_ZP:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      data = read_byte(em, ZP_addr);
      write_byte(em, ASL(em, data), ZP_addr);
      break;

    case INS_ASL_ZPX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      data = read_byte(em, ZP_addr);
      write_byte(em, ASL(em, data), ZP_addr);
      break;

    case INS_ASL_ABS:
      cycles = 6;
      ABS_addr = fetch_word(em);
      data = read_byte(em, ABS_addr);
      write_byte(em, ASL(em, data), ABS_addr);
      break;

    case INS_ASL_ABSX:
      cycles = 7;
      ABS_addr = fetch_word(em) + em->cpu.X;
      data = read_byte(em, ABS_addr);
      write_byte(em, ASL(em, data), ABS_addr);
      break;
    /**
     *  NOP instructions
     **/
    case INS_NOP:
      cycles = 1;
      break;
    /**
     *  STACK instructions
     **/
    case INS_PHA:
      cycles = 3;
      push_byte(em, em->cpu.A);
      break;

    case INS_PHP:
      cycles = 3;
      push_byte(em, em->cpu.status | 0b00001100);
      break;

    case INS_PLA:
      cycles = 4;
      em->cpu.A = pull_byte(em);
      set_flags(em, &em->cpu.A);
      break;

    case INS_PLP:
      cycles = 4;
      em->cpu.status = pull_byte(em);
      break;

    case INS_ROL_ACC:  printf("unsuported instruction"); break;
    case INS_ROL_ZP:   printf("unsuported instruction"); break;
    case INS_ROL_ZPX:  printf("unsuported instruction"); break;
    case INS_ROL_ABS:  printf("unsuported instruction"); break;
    case INS_ROL_ABSX: printf("unsuported instruction"); break;
    case INS_ROR_ACC:  printf("unsuported instruction"); break;
    case INS_ROR_ZP:   printf("unsuported instruction"); break;
    case INS_ROR_ZPX:  printf("unsuported instruction"); break;
    case INS_ROR_ABS:  printf("unsuported instruction"); break;
    case INS_ROR_ABSX: printf("unsuported instruction"); break;
    case INS_RTI:      printf("unsuported instruction"); break;
    case INS_SEC:      printf("unsuported instruction"); break;
    case INS_SED:      printf("unsuported instruction"); break;
    case INS_SEI:      printf("unsuported instruction"); break;
    /**
     *  TRANSFER instructions
     **/
    case INS_TAX:
      cycles = 2;
      em->cpu.X = em->cpu.A;
      set_flags(em, &em->cpu.X);
      break;

    case INS_TAY:
      cycles = 2;
      em->cpu.Y = em->cpu.SP;
      set_flags(em, &em->cpu.Y);
      break;

    case INS_TSX:
      cycles = 2;
      em->cpu.X = em->cpu.SP;
      set_flags(em, &em->cpu.X);
      break;

    case INS_TXA:
      cycles = 2;
      em->cpu.A = em->cpu.X;
      set_flags(em, &em->cpu.A);
      break;

    case INS_TXS:
      cycles = 2;
      em->cpu.SP = em->cpu.X;
      break;

    case INS_TYA:
      cycles = 2;
      em->cpu.A = em->cpu.Y;
      set_flags(em, &em->cpu.A);
      break;

    case INS_BRK: return -1;

    default:
      printf("not a valid instruction");
      break;
  }
  return cycles;
}
