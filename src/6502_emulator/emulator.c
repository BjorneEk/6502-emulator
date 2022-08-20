////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////

#include "emulator.h"
#include <stdio.h>
#include <string.h>
void reset(emulator_t * em) {
  init_mem(&em->mem);
  reset_cpu(&em->cpu);
  em->debug = false;
}
void start_program(emulator_t * em) {
  em->cpu.PC = read_word(em, RESET_CPU);
}

void enable_debug(emulator_t * em) {
  em->debug = true;
}
void disable_debug(emulator_t * em) {
  em->debug = true;
}

emulator_t copy_state(emulator_t *em) {
  emulator_t new;
  reset(&new);
  memcpy(new.mem.data, em->mem.data, MEMORY_SIZE);
  new.cpu.PC = em->cpu.PC;
  new.cpu.SP = em->cpu.SP;
  new.cpu.A = em->cpu.A;
  new.cpu.X = em->cpu.X;
  new.cpu.Y = em->cpu.Y;
  new.cpu.status = em->cpu.status;
  return new;
}
void debug(emulator_t * em) {
  printf("6502 emulator status\n");
  printf("Registers:\n");
  printf("A: %02X %i\n", em->cpu.A, em->cpu.A);
  printf("X: %02X %i\n", em->cpu.X, em->cpu.X);
  printf("Y: %02X %i\n", em->cpu.Y, em->cpu.Y);
  printf("PC: %04X %i\n", em->cpu.PC, em->cpu.PC);
  printf("SP: %04X %i\n", em->cpu.SP, em->cpu.SP);
  printf("STATUS: CZIDBUVN:%i%i%i%i%i%i%i%i\n",
  em->cpu.C,em->cpu.Z,em->cpu.I,em->cpu.D,em->cpu.B,em->cpu.U,em->cpu.V,em->cpu.N);

  printf("MEMORY:\n");
  print_mem(&em->mem);
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
  em->cpu.V = same_sign && (em->cpu.A & 0b10000000);
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

u8_t ROR(emulator_t * em, u8_t operand) {
  bool old_zero_bit = (operand & 0b00000001) > 0;
  operand = operand >> 1;
  if (em->cpu.C) operand |= 0b10000000;
  em->cpu.C = old_zero_bit;
  set_flags(em, &operand);
  return operand;
}
u8_t ROL(emulator_t * em, u8_t operand) {
  u8_t new_zero_bit = em->cpu.C ? 0b00000001 : 0;
  em->cpu.C = (operand & 0b10000000) > 0;
  operand = operand << 1;
  operand |= new_zero_bit;
  set_flags(em, &operand);
  return operand;
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


int execute(emulator_t * em) {
  u8_t ins, data;
  u8_t ZP_addr, addr_lb;
  u16_t ABS_addr, ABS_addrX, ABS_addrY, sub_addr, DB_addr;
  i32_t cycles;
  ins = fetch_byte(em);
  switch(ins){
    /**
     *  ADC instructions
     **/
    case INS_ADC_IM:
      cycles = 2;
      data = fetch_byte(em);
      ADC(em, data);
      if(em->debug) printf("ADC #$%02X\n", data);
      break;

    case INS_ADC_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      ADC(em, read_byte(em, ZP_addr));
      if(em->debug) printf("ADC $%02X\n", ZP_addr);
      break;

    case INS_ADC_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      ADC(em, read_byte(em, ZP_addr + em->cpu.X));
      if(em->debug) printf("ADC $%02X, x\n", ZP_addr);
      break;

    case INS_ADC_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      ADC(em, read_byte(em, ABS_addr));
      if(em->debug) printf("ADC $%04X\n", ABS_addr);
      break;

    case INS_ADC_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.X, &cycles);
      ADC(em, read_byte(em, ABS_addr));
      if(em->debug) printf("ADC $%04X, x\n", ABS_addr -  + em->cpu.X);
      break;

    case INS_ADC_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.Y, &cycles);
      ADC(em, read_byte(em, ABS_addr));
      if(em->debug) printf("ADC $%04X, y\n", ABS_addr - em->cpu.Y);
      break;

    case INS_ADC_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em);
      ADC(em, read_byte(em, read_word(em, ZP_addr + em->cpu.X)));
      if(em->debug) printf("ADC ($%02X, x)\n", ZP_addr);
      break;

    case INS_ADC_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + em->cpu.Y;
      if((ABS_addr ^ ABS_addrY) >> 8) cycles++; //page boundry is crossed
      ADC(em, read_byte(em, ABS_addrY));
      if(em->debug) printf("ADC ($%02X), y\n", ZP_addr);
      break;
    /**
     *  SBC instructions
     **/
    case INS_SBC_IM:
      cycles = 2;
      data = fetch_byte(em);
      SBC(em, data);
      if(em->debug) printf("SBC #$%02X\n", data);
      break;

    case INS_SBC_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      SBC(em, read_byte(em, ZP_addr));
      if(em->debug) printf("SBC $%02X\n", ZP_addr);
      break;

    case INS_SBC_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      SBC(em, read_byte(em, ZP_addr + em->cpu.X));
      if(em->debug) printf("SBC $%02X, x\n", ZP_addr);
      break;

    case INS_SBC_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      SBC(em, read_byte(em, ABS_addr));
      if(em->debug) printf("SBC $%04X\n", ABS_addr);
      break;

    case INS_SBC_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.X, &cycles);
      SBC(em, read_byte(em, ABS_addr));
      if(em->debug) printf("SBC $%04X, x\n", ABS_addr - em->cpu.X);
      break;

    case INS_SBC_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.Y, &cycles);
      SBC(em, read_byte(em, ABS_addr));
      if(em->debug) printf("SBC $%04X, y\n", ABS_addr - em->cpu.Y);
      break;

    case INS_SBC_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em);
      SBC(em, read_byte(em, read_word(em, ZP_addr + em->cpu.X)));
      if(em->debug) printf("SBC ($%02X, x)\n", ZP_addr);
      break;

    case INS_SBC_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + em->cpu.Y;
      if((ABS_addr ^ ABS_addrY) >> 8) cycles++; //page boundry is crossed
      SBC(em, read_byte(em, ABS_addrY));
      if(em->debug) printf("ADC ($%02X), y\n", ZP_addr);
      break;
    /**
     *  CMP instructions
     **/
    case INS_CMP_IM:
      cycles = 2;
      data = fetch_byte(em);
      compare(em, data, em->cpu.A);
      if(em->debug) printf("CMP #$%02X\n", data);
      break;

    case INS_CMP_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      compare(em, read_byte(em, ZP_addr), em->cpu.A);
      if(em->debug) printf("CMP $%02X\n", ZP_addr);
      break;

    case INS_CMP_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      compare(em, read_byte(em, ZP_addr + em->cpu.X), em->cpu.A);
      if(em->debug) printf("CMP $%02X, x\n", ZP_addr);
      break;

    case INS_CMP_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      compare(em, read_byte(em, ABS_addr), em->cpu.A);
      if(em->debug) printf("CMP $%04X\n", ABS_addr);
      break;

    case INS_CMP_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.X, &cycles);
      compare(em, read_byte(em, ABS_addr), em->cpu.A);
      if(em->debug) printf("CMP $%04X, x\n", ABS_addr - em->cpu.X);
      break;

    case INS_CMP_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.Y, &cycles);
      compare(em, read_byte(em, ABS_addr), em->cpu.A);
      if(em->debug) printf("CMP $%04X, y\n", ABS_addr - em->cpu.Y);
      break;

    case INS_CMP_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      compare(em, read_byte(em, read_word(em, ZP_addr)), em->cpu.A);
      if(em->debug) printf("CMP ($%02X, x)\n", ZP_addr);
      break;

    case INS_CMP_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + em->cpu.Y;
      if((ABS_addr ^ ABS_addrY) >> 8) cycles++; //page boundry is crossed
      compare(em, read_byte(em, ABS_addrY), em->cpu.A);
      if(em->debug) printf("CMP ($%02X), y\n", ZP_addr);
      break;
    /**
     *  CPX instructions
     **/
    case INS_CPX_IM:
      cycles = 2;
      data = fetch_byte(em);
      compare(em, data, em->cpu.X);
      if(em->debug) printf("CPX #$%02X\n", data);
      break;

    case INS_CPX_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      compare(em, read_byte(em, ZP_addr), em->cpu.X);
      if(em->debug) printf("CPX $%02X\n", ZP_addr);
      break;

    case INS_CPX_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      compare(em, read_byte(em, ABS_addr), em->cpu.X);
      if(em->debug) printf("CPX $%04X\n", ABS_addr);
      break;
    /**
     *  CPY instructions
     **/
    case INS_CPY_IM:
      cycles = 2;
      data = fetch_byte(em);
      compare(em, data, em->cpu.Y);
      if(em->debug) printf("CPY #$%02X\n", data);
      break;

    case INS_CPY_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      compare(em, read_byte(em, ZP_addr), em->cpu.Y);
      if(em->debug) printf("CPY $%02X\n", ZP_addr);
      break;

    case INS_CPY_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      compare(em, read_byte(em, ABS_addr), em->cpu.Y);
      if(em->debug) printf("CPY $%04X\n", ABS_addr);
      break;

    /**
     *  AND instructions
     **/
    case INS_AND_IM:
      cycles = 2;
      data = fetch_byte(em);
      em->cpu.A &= data;
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("AND #$%02X\n", data);
      break;

    case INS_AND_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      em->cpu.A &= read_byte(em, ZP_addr);
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("AND $%02X\n", ZP_addr);
      break;

    case INS_AND_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      em->cpu.A &= read_byte(em, ZP_addr + em->cpu.X);
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("AND $%02X, x\n", ZP_addr);
      break;

    case INS_AND_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      em->cpu.A &= read_byte(em, ABS_addr);
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("AND $%04X\n", ABS_addr);
      break;

    case INS_AND_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.X, &cycles);
      em->cpu.A &= read_byte(em, ABS_addr);
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("AND $%04X, x\n", ABS_addr - em->cpu.X);
      break;

    case INS_AND_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.Y, &cycles);
      em->cpu.A &= read_byte(em, ABS_addr);
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("AND $%04X, y\n", ABS_addr - em->cpu.Y);
      break;

    case INS_AND_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      em->cpu.A &= read_byte(em, read_word(em, ZP_addr));
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("AND ($%02X, x)\n", ZP_addr);
      break;

    case INS_AND_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + em->cpu.Y;
      if((ABS_addr ^ ABS_addrY) >> 8) cycles++; //page boundry is crossed
      em->cpu.A &= read_byte(em, ABS_addrY);
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("AND ($%02X), y\n", ZP_addr);
      break;
    /**
     *  ORA instructions
     **/
    case INS_ORA_IM:
      cycles = 2;
      data = fetch_byte(em);
      em->cpu.A |= data;
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("ORA #$%02X\n", data);
      break;

    case INS_ORA_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      em->cpu.A &= read_byte(em, ZP_addr);
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("ORA $%02X\n", ZP_addr);
      break;

    case INS_ORA_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      em->cpu.A |= read_byte(em, ZP_addr + em->cpu.X);
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("ORA $%02X, x\n", ZP_addr);
      break;

    case INS_ORA_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      em->cpu.A |= read_byte(em, ABS_addr);
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("ORA $%04X\n", ABS_addr);
      break;

    case INS_ORA_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.X, &cycles);
      em->cpu.A |= read_byte(em, ABS_addr);
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("ORA $%04X, x\n", ABS_addr - em->cpu.X);
      break;

    case INS_ORA_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.Y, &cycles);
      em->cpu.A |= read_byte(em, ABS_addr);
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("ORA $%04X, y\n", ABS_addr - em->cpu.Y);
      break;

    case INS_ORA_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      em->cpu.A |= read_byte(em, read_word(em, ZP_addr));
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("ORA ($%02X, x)\n", ZP_addr);
      break;

    case INS_ORA_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + em->cpu.Y;
      if((ABS_addr ^ ABS_addrY) >> 8) cycles++; //page boundry is crossed
      em->cpu.A |= read_byte(em, ABS_addrY);
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("ORA ($%02X), y\n", ZP_addr);
      break;
    /**
     *  EOR instructions
     **/
    case INS_EOR_IM:
      cycles = 2;
      data = fetch_byte(em);
      em->cpu.A ^= data;
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("EOR #$%02X\n", data);
      break;

    case INS_EOR_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      em->cpu.A |= read_byte(em, ZP_addr);
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("EOR $%02X\n", ZP_addr);
      break;

    case INS_EOR_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      em->cpu.A ^= read_byte(em, ZP_addr + em->cpu.X);
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("EOR $%02X, x\n", ZP_addr);
      break;

    case INS_EOR_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      em->cpu.A ^= read_byte(em, ABS_addr);
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("EOR $%04X\n", ABS_addr);
      break;

    case INS_EOR_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.X, &cycles);
      em->cpu.A ^= read_byte(em, ABS_addr);
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("EOR $%04X, x\n", ABS_addr - em->cpu.X);
      break;

    case INS_EOR_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.Y, &cycles);
      em->cpu.A ^= read_byte(em, ABS_addr);
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("EOR $%04X, y\n", ABS_addr - em->cpu.Y);
      break;

    case INS_EOR_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      em->cpu.A ^= read_byte(em, read_word(em, ZP_addr));
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("EOR ($%02X, x)\n", ZP_addr);
      break;

    case INS_EOR_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + em->cpu.Y;
      if((ABS_addr ^ ABS_addrY) >> 8) cycles++; //page boundry is crossed
      em->cpu.A ^= read_byte(em, ABS_addrY);
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("EOR ($%02X), y\n", ZP_addr);
      break;
    /**
     *  BRANCH instructions
     **/
    case INS_BCC:
      cycles = 1;
      if(em->debug) {
        data = read_byte(em, em->cpu.PC);
        printf("BCC $%04X\n", data);
      }
      branch_if(em, &cycles, em->cpu.C, false);
      break;

    case INS_BCS:
      cycles = 1;
      if(em->debug) {
        data = read_byte(em, em->cpu.PC);
        printf("BCS $%04X\n", data);
      }
      branch_if(em, &cycles, em->cpu.C, true);
      break;

    case INS_BEQ:
      cycles = 1;
      if(em->debug) {
        data = read_byte(em, em->cpu.PC);
        printf("BEQ $%04X\n", data);
      }
      branch_if(em, &cycles, em->cpu.Z, true);
      break;

    case INS_BMI:
      cycles = 1;
      if(em->debug) {
        data = read_byte(em, em->cpu.PC);
        printf("BMI $%04X\n", data);
      }
      branch_if(em, &cycles, em->cpu.N, true);
      break;

    case INS_BNE:
      cycles = 1;
      if(em->debug) {
        data = read_byte(em, em->cpu.PC);
        printf("BNE $%04X\n", data);
      }
      branch_if(em, &cycles, em->cpu.Z, false);
      break;

    case INS_BPL:
      cycles = 1;
      if(em->debug) {
        data = read_byte(em, em->cpu.PC);
        printf("BPL $%04X\n", data);
      }
      branch_if(em, &cycles, em->cpu.N, false);
      break;

    case INS_BVC:
      cycles = 1;
      if(em->debug) {
        data = read_byte(em, em->cpu.PC);
        printf("BVC $%04X\n", data);
      }
      branch_if(em, &cycles, em->cpu.V, false);
      break;

    case INS_BVS:
      cycles = 1;
      if(em->debug) {
        data = read_byte(em, em->cpu.PC);
        printf("BVS $%04X\n", data);
      }
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
      if(em->debug) printf("BIT $%02X\n", ZP_addr);
      break;

    case INS_BIT_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      data = read_byte(em, ABS_addr);
      em->cpu.Z = !(data & em->cpu.A);
      em->cpu.N =  (data & 0b10000000) != 0;
      em->cpu.V =  (data & 0b01000000) != 0;
      if(em->debug) printf("BIT $%04X\n", ABS_addr);
      break;

    case INS_CLC:
      cycles = 2;
      em->cpu.C = 0;
      if(em->debug) printf("CLC\n");
      break;

    case INS_CLD:
      cycles = 2;
      em->cpu.D = 0;
      if(em->debug) printf("CLD\n");
      break;

    case INS_CLI:
      cycles = 2;
      em->cpu.I = 0;
      if(em->debug) printf("CLI\n");
      break;

    case INS_CLV:
      cycles = 2;
      em->cpu.V = 0;
      if(em->debug) printf("CLV\n");
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
      if(em->debug) printf("DEC $%02X\n", ZP_addr);
      break;

    case INS_DEC_ZPX:
      cycles = 6;
      ZP_addr = fetch_byte(em);
      data = read_byte(em, ZP_addr + em->cpu.X);
      data--;
      set_flags(em, &data);
      write_byte(em, data, ZP_addr);
      if(em->debug) printf("DEC $%02X, x\n", ZP_addr);
      break;

    case INS_DEC_ABS:
      cycles = 6;
      ABS_addr = fetch_word(em);
      data = read_byte(em, ABS_addr);
      data--;
      set_flags(em, &data);
      write_byte(em, data, ABS_addr);
      if(em->debug) printf("DEC $%04X\n", ABS_addr);
      break;

    case INS_DEC_ABSX:
      cycles = 7;
      ABS_addr = fetch_word(em);
      data = read_byte(em, ABS_addr + em->cpu.X);
      data--;
      set_flags(em, &data);
      write_byte(em, data, ABS_addr);
      if(em->debug) printf("DEC $%04X, x\n", ABS_addr);
      break;
    /**
     *  DEY-DEX instructions
     **/
    case INS_DEX:
      cycles = 2;
      em->cpu.X--;
      set_flags(em, &em->cpu.X);
      if(em->debug) printf("DEX\n");
      break;

    case INS_DEY:
      cycles = 2;
      em->cpu.Y--;
      set_flags(em, &em->cpu.Y);
      if(em->debug) printf("DEY\n");
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
      if(em->debug) printf("INC $%02X\n", ZP_addr);
      break;

    case INS_INC_ZPX:
      cycles = 6;
      ZP_addr = fetch_byte(em);
      data = read_byte(em, ZP_addr + em->cpu.X);
      data++;
      set_flags(em, &data);
      write_byte(em, data, ZP_addr);
      if(em->debug) printf("INC $%02X, x\n", ZP_addr);
      break;

    case INS_INC_ABS:
      cycles = 6;
      ABS_addr = fetch_word(em);
      data = read_byte(em, ABS_addr);
      data++;
      set_flags(em, &data);
      write_byte(em, data, ABS_addr);
      if(em->debug) printf("INC $%04X\n", ABS_addr);
      break;

    case INS_INC_ABSX:
      cycles = 7;
      ABS_addr = fetch_word(em);
      data = read_byte(em, ABS_addr + em->cpu.X);
      data++;
      set_flags(em, &data);
      write_byte(em, data, ABS_addr);
      if(em->debug) printf("INC $%04X, x\n", ABS_addr);
      break;
    /**
     *  INX-INY instructions
     **/
    case INS_INX:
      cycles = 2;
      em->cpu.X++;
      set_flags(em, &em->cpu.X);
      if(em->debug) printf("INX\n");
      break;

    case INS_INY:
      cycles = 2;
      em->cpu.Y++;
      set_flags(em, &em->cpu.Y);
      if(em->debug) printf("INY\n");
      break;
    /**
     *  JSR-RTS instructions
     **/
    case INS_JSR:
      cycles = 6;
      sub_addr = fetch_word(em);
      push_word(em, em->cpu.PC);
      em->cpu.PC = sub_addr;
      if(em->debug) printf("JSR $%04X\n", sub_addr);
      break;

    case INS_RTS:
      cycles = 6;
      em->cpu.PC = pull_word(em);
      if(em->debug) printf("RTS\n");
      break;

    case INS_JMP_ABS:
      cycles = 3;
      em->cpu.PC = fetch_word(em);
      if(em->debug) printf("JMP $%04X\n", em->cpu.PC);
      break;

    case INS_JMP_IND:
      cycles = 5;
      ABS_addr = fetch_word(em);
      em->cpu.PC = read_word(em, ABS_addr);
      if(em->debug) printf("JMP ($%04X)\n", ABS_addr);
      break;
    /**
     *  LDA instructions
     **/
    case INS_LDA_IM:
      cycles = 2;
      data = fetch_byte(em);
      em->cpu.A = data;
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("LDA #$%02X\n", data);
      break;

    case INS_LDA_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      load_register(em, &em->cpu.A, ZP_addr);
      if(em->debug) printf("LDA $%02X\n", ZP_addr);
      break;

    case INS_LDA_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      load_register(em, &em->cpu.A, ZP_addr + em->cpu.X);
      if(em->debug) printf("LDA $%02X, x\n", ZP_addr);
      break;

    case INS_LDA_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      load_register(em, &em->cpu.A, ABS_addr);
      if(em->debug) printf("LDA $%04X\n", ABS_addr);
      break;

    case INS_LDA_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.X, &cycles);
      load_register(em, &em->cpu.A, ABS_addr);
      if(em->debug) printf("LDA $%04X, x\n", ABS_addr - em->cpu.X);
      break;

    case INS_LDA_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.Y, &cycles);
      load_register(em, &em->cpu.A, ABS_addr);
      if(em->debug) printf("LDA $%04X, y\n", ABS_addr - em->cpu.Y);
      break;

    case INS_LDA_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      load_register(em, &em->cpu.A, read_word(em, ZP_addr));
      if(em->debug) printf("LDA ($%02X, x)\n", ZP_addr - em->cpu.X);
      break;

    case INS_LDA_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + em->cpu.Y;
      if((ABS_addr ^ ABS_addrY) >> 8) cycles++; //page boundry is crossed
      load_register(em, &em->cpu.A, ABS_addrY);
      if(em->debug) printf("LDA ($%02X), y\n", ZP_addr);
      break;
    /**
     *  LDX instructions
     **/
    case INS_LDX_IM:
      cycles = 2;
      data = fetch_byte(em);
      em->cpu.X = data;
      set_flags(em, &em->cpu.X);
      if(em->debug) printf("LDX #$%02X\n", data);
      break;

    case INS_LDX_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      load_register(em, &em->cpu.X, ZP_addr);
      if(em->debug) printf("LDX $%02X\n", ZP_addr);
    break;

    case INS_LDX_ZPY:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      load_register(em, &em->cpu.X, ZP_addr + em->cpu.Y);
      if(em->debug) printf("LDX $%02X, y\n", ZP_addr);
      break;

    case INS_LDX_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      load_register(em, &em->cpu.X, ABS_addr);
      if(em->debug) printf("LDX $%04X\n", ABS_addr);
      break;

    case INS_LDX_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.Y, &cycles);
      load_register(em, &em->cpu.X, ABS_addr);
      if(em->debug) printf("LDX $%04X, y\n", ABS_addr - em->cpu.Y);
      break;
    /**
     *  LDY instructions
     **/
    case INS_LDY_IM:
      cycles = 2;
      data = fetch_byte(em);
      em->cpu.Y = data;
      set_flags(em, &em->cpu.Y);
      if(em->debug) printf("LDY #$%02X\n", data);
      break;

    case INS_LDY_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      load_register(em, &em->cpu.Y, ZP_addr);
      if(em->debug) printf("LDY $%02X\n", ZP_addr);
    break;

    case INS_LDY_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      load_register(em, &em->cpu.Y, ZP_addr + em->cpu.X);
      if(em->debug) printf("LDY $%02X, x\n", ZP_addr);
      break;

    case INS_LDY_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      load_register(em, &em->cpu.Y, ABS_addr);
      if(em->debug) printf("LDY $%04X\n", ABS_addr);
      break;

    case INS_LDY_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &em->cpu.X, &cycles);
      load_register(em, &em->cpu.Y, ABS_addr);
      if(em->debug) printf("LDY $%04X, x\n", ABS_addr - em->cpu.X);
      break;
    /**
     *  STA instructions
     **/
    case INS_STA_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      write_byte(em, em->cpu.A, ZP_addr);
      if(em->debug) printf("STA $%02X\n", ZP_addr);
      break;

    case INS_STA_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      write_byte(em, em->cpu.A, ZP_addr + em->cpu.X);
      if(em->debug) printf("STA $%02X, x\n", ZP_addr);
      break;

    case INS_STA_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      write_byte(em, em->cpu.A, ABS_addr);
      if(em->debug) printf("STA $%04X\n", ABS_addr);
      break;

    case INS_STA_ABSX:
      cycles = 5;
      ABS_addr = addr_abs_reg_5(em, &em->cpu.X);
      write_byte(em, em->cpu.A, ABS_addr);
      if(em->debug) printf("STA $%04X, x\n", ABS_addr - em->cpu.X);
      break;

    case INS_STA_ABSY:
      cycles = 5;
      ABS_addr = addr_abs_reg_5(em, &em->cpu.Y);
      write_byte(em, em->cpu.A, ABS_addr);
      if(em->debug) printf("STA $%04X, y\n", ABS_addr - em->cpu.Y);
      break;

    case INS_STA_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      write_byte(em, em->cpu.A, read_word(em, ZP_addr));
      if(em->debug) printf("STA ($%02X, x)\n", ZP_addr);
      break;

    case INS_STA_INDY:
      cycles = 6;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      write_byte(em, em->cpu.A, ABS_addr + em->cpu.Y);
      if(em->debug) printf("STA ($%02X), y\n", ZP_addr);
      break;
    /**
     *  STX instructions
     **/
    case INS_STX_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      write_byte(em, em->cpu.X, ZP_addr);
      if(em->debug) printf("STX $%02X\n", ZP_addr);
      break;

    case INS_STX_ZPY:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      write_byte(em, em->cpu.X, ZP_addr + em->cpu.Y);
      if(em->debug) printf("STX $%02X, y\n", ZP_addr);
      break;

    case INS_STX_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      write_byte(em, em->cpu.X, ABS_addr);
      if(em->debug) printf("STX $%04X\n", ABS_addr);
      break;
    /**
     *  STY instructions
     **/
    case INS_STY_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      write_byte(em, em->cpu.Y, ZP_addr);
      if(em->debug) printf("STY $%02X\n", ZP_addr);
      break;

    case INS_STY_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      write_byte(em, em->cpu.Y, ZP_addr + em->cpu.X);
      if(em->debug) printf("STY $%02X, x\n", ZP_addr);
      break;

    case INS_STY_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      write_byte(em, em->cpu.Y, ABS_addr);
      if(em->debug) printf("STY $%04X\n", ABS_addr);
      break;
    /**
     *  LSR instructions
     **/
    case INS_LSR_ACC:
      cycles = 2;
      em->cpu.A = LSR(em, em->cpu.A);
      if(em->debug) printf("LSR A\n");
      break;

    case INS_LSR_ZP:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      data = read_byte(em, ZP_addr);
      write_byte(em, LSR(em, data), ZP_addr);
      if(em->debug) printf("LSR $%02X\n", ZP_addr);
      break;

    case INS_LSR_ZPX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      data = read_byte(em, ZP_addr);
      write_byte(em, LSR(em, data), ZP_addr);
      if(em->debug) printf("LSR $%02X, x\n", ZP_addr - em->cpu.X);
      break;

    case INS_LSR_ABS:
      cycles = 6;
      ABS_addr = fetch_word(em);
      data = read_byte(em, ABS_addr);
      write_byte(em, LSR(em, data), ABS_addr);
      if(em->debug) printf("LSR $%04X\n", ABS_addr);
      break;

    case INS_LSR_ABSX:
      cycles = 7;
      ABS_addr = fetch_word(em) + em->cpu.X;
      data = read_byte(em, ABS_addr);
      write_byte(em, LSR(em, data), ABS_addr);
      if(em->debug) printf("LSR $%04X, x\n", ABS_addr - em->cpu.X);
      break;
    /**
     *  ASL instructions
     **/
    case INS_ASL_ACC:
      cycles = 2;
      em->cpu.A = ASL(em, em->cpu.A);
      if(em->debug) printf("ASL A\n");
      break;

    case INS_ASL_ZP:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      data = read_byte(em, ZP_addr);
      write_byte(em, ASL(em, data), ZP_addr);
      if(em->debug) printf("ASL $%02X\n", ZP_addr);
      break;

    case INS_ASL_ZPX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      data = read_byte(em, ZP_addr);
      write_byte(em, ASL(em, data), ZP_addr);
      if(em->debug) printf("ASL $%02X, x\n", ZP_addr - em->cpu.X);
      break;

    case INS_ASL_ABS:
      cycles = 6;
      ABS_addr = fetch_word(em);
      data = read_byte(em, ABS_addr);
      write_byte(em, ASL(em, data), ABS_addr);
      if(em->debug) printf("ASL $%04X\n", ABS_addr);
      break;

    case INS_ASL_ABSX:
      cycles = 7;
      ABS_addr = fetch_word(em) + em->cpu.X;
      data = read_byte(em, ABS_addr);
      write_byte(em, ASL(em, data), ABS_addr);
      if(em->debug) printf("ASL $%04X, x\n", ABS_addr - em->cpu.X);
      break;
    /**
     *  ROL instructions
     **/
    case INS_ROL_ACC:
      cycles = 2;
      em->cpu.A = ROL(em, em->cpu.A);
      if(em->debug) printf("ROL A\n");
      break;

    case INS_ROL_ZP:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      data = read_byte(em, ZP_addr);
      write_byte(em, ROL(em, data), ZP_addr);
      if(em->debug) printf("ROL $%02X\n", ZP_addr);
      break;

    case INS_ROL_ZPX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      data = read_byte(em, ZP_addr);
      write_byte(em, ROL(em, data), ZP_addr);
      if(em->debug) printf("ROL $%02X, x\n", ZP_addr - em->cpu.X);
      break;

    case INS_ROL_ABS:
      cycles = 6;
      ABS_addr = fetch_word(em);
      data = read_byte(em, ABS_addr);
      write_byte(em, ROL(em, data), ABS_addr);
      if(em->debug) printf("ROL $%04X\n", ABS_addr);
      break;

    case INS_ROL_ABSX:
      cycles = 7;
      ABS_addr = fetch_word(em) + em->cpu.X;
      data = read_byte(em, ABS_addr);
      write_byte(em, ROL(em, data), ABS_addr);
      if(em->debug) printf("ROL $%04X, x\n", ABS_addr - em->cpu.X);
      break;
    /**
     *  ROR instructions
     **/
    case INS_ROR_ACC:
      cycles = 2;
      em->cpu.A = ROR(em, em->cpu.A);
      if(em->debug) printf("ROR A\n");
      break;

    case INS_ROR_ZP:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      data = read_byte(em, ZP_addr);
      write_byte(em, ROR(em, data), ZP_addr);
      if(em->debug) printf("ROR $%02X\n", ZP_addr);
      break;

    case INS_ROR_ZPX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + em->cpu.X;
      data = read_byte(em, ZP_addr);
      write_byte(em, ROR(em, data), ZP_addr);
      if(em->debug) printf("ROR $%02X, x\n", ZP_addr - em->cpu.X);
      break;

    case INS_ROR_ABS:
      cycles = 6;
      ABS_addr = fetch_word(em);
      data = read_byte(em, ABS_addr);
      write_byte(em, ROR(em, data), ABS_addr);
      if(em->debug) printf("ROR $%04X\n", ABS_addr);
      break;

    case INS_ROR_ABSX:
      cycles = 7;
      ABS_addr = fetch_word(em) + em->cpu.X;
      data = read_byte(em, ABS_addr);
      write_byte(em, ROR(em, data), ABS_addr);
      if(em->debug) printf("ROR $%04X, x\n", ABS_addr - em->cpu.X);
      break;

    /**
     *  NOP instructions
     **/
    case INS_NOP:
      cycles = 1;
      if(em->debug) printf("NOP\n");
      break;
    /**
     *  STACK instructions
     **/
    case INS_PHA:
      cycles = 3;
      push_byte(em, em->cpu.A);
      if(em->debug) printf("PHA\n");
      break;

    case INS_PHP:
      cycles = 3;
      push_byte(em, em->cpu.status | 0b00001100);
      if(em->debug) printf("PHP\n");
      break;

    case INS_PLA:
      cycles = 4;
      em->cpu.A = pull_byte(em);
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("PLA\n");
      break;

    case INS_PLP:
      cycles = 4;
      em->cpu.status = pull_byte(em);
      if(em->debug) printf("PLP\n");
      break;

    case INS_RTI:
      cycles = 6;
      em->cpu.status = pull_byte(em);
      em->cpu.PC = pull_word(em);
      if(em->debug) printf("RTI\n");
      break;

    case INS_SEC:
      cycles = 2;
      em->cpu.C = 1;
      if(em->debug) printf("SEC\n");
      break;

    case INS_SED:
      cycles = 2;
      em->cpu.D = 1;
      if(em->debug) printf("SED\n");
      break;

    case INS_SEI:
      cycles = 2;
      em->cpu.I = 1;
      if(em->debug) printf("SEI\n");
      break;
    /**
     *  TRANSFER instructions
     **/
    case INS_TAX:
      cycles = 2;
      em->cpu.X = em->cpu.A;
      set_flags(em, &em->cpu.X);
      if(em->debug) printf("TAX\n");
      break;

    case INS_TAY:
      cycles = 2;
      em->cpu.Y = em->cpu.SP;
      set_flags(em, &em->cpu.Y);
      if(em->debug) printf("TAY\n");
      break;

    case INS_TSX:
      cycles = 2;
      em->cpu.X = em->cpu.SP;
      set_flags(em, &em->cpu.X);
      if(em->debug) printf("TSX\n");
      break;

    case INS_TXA:
      cycles = 2;
      em->cpu.A = em->cpu.X;
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("TXA\n");
      break;

    case INS_TXS:
      cycles = 2;
      em->cpu.SP = em->cpu.X;
      if(em->debug) printf("TXS\n");
      break;

    case INS_TYA:
      cycles = 2;
      em->cpu.A = em->cpu.Y;
      set_flags(em, &em->cpu.A);
      if(em->debug) printf("TYA\n");
      break;

    case INS_BRK:
      if(em->debug) printf("BRK\n");
      return -1;

    default:
      printf("not a valid instruction");
      break;
  }
  return cycles;
}
