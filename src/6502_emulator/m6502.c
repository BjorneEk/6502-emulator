////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////

#include "m6502.h"
#include <stdio.h>
#include <string.h>

#define PC_ (em->cpu.PC)
#define SP_ (em->cpu.SP)
#define A_  (em->cpu.A)
#define X_  (em->cpu.X)
#define Y_  (em->cpu.Y)
#define PS_ (em->cpu.status)
#define C_  (em->cpu.C)
#define Z_  (em->cpu.Z)
#define I_  (em->cpu.I)
#define D_  (em->cpu.D)
#define B_  (em->cpu.B)
#define U_  (em->cpu.U)
#define V_  (em->cpu.V)
#define N_  (em->cpu.N)

#define FLAG_N_BITMASK (0b10000000)
#define FLAG_C_BITMASK (0b00000001)

#define HIGH_BYTE(word) (u8_t)(word >> 8)
#define LOW_BYTE(word) (u8_t)(word & 0xFF)

#define SET_ZN_FLAGS(register) Z_ = (*register == 0);\
                               N_ = (*register & FLAG_N_BITMASK) != 0;

#define BG_YELLOW \033[43m

void load_register(m6502_t * em, u8_t * reg, u16_t addr) {
  *reg = read_byte(em, addr);
  SET_ZN_FLAGS(reg);
}

u16_t addr_abs_reg(m6502_t * em, u8_t * reg, i32_t * cycles){
  u16_t ABS_addr = fetch_word(em);
  u16_t ABS_addr_reg = ABS_addr + *reg;
  if((ABS_addr ^ ABS_addr_reg) >> 8) (*cycles)++; //page boundry is crossed
  return ABS_addr_reg;
}
u16_t addr_abs_reg_5(m6502_t * em, u8_t * reg){
  return fetch_word(em) + *reg;
}


void ADC(m6502_t * em, u8_t operand) {
  assert_w(D_, "ADC: Decimal mode not yet suported");
  const bool same_sign = !((A_ ^ operand) & FLAG_N_BITMASK);
  u16_t sum = A_;
  sum += operand;
  sum += C_;
  A_ = LOW_BYTE(sum);
  SET_ZN_FLAGS(&A_);
  C_ = sum > 0xFF;
  // set overflow if both operands are neggative
  V_ = same_sign && (A_ & FLAG_N_BITMASK);
}
void SBC(m6502_t * em, u8_t operand) {
  ADC(em, ~operand);
}

u8_t ASL(m6502_t * em, u8_t operand) {
  C_ = (operand & FLAG_N_BITMASK) > 0;
  u8_t res = operand << 1;
  SET_ZN_FLAGS(&res);
  return res;
}

u8_t LSR(m6502_t * em, u8_t operand) {
  C_ = (operand & FLAG_C_BITMASK) > 0;
  u8_t res = operand >> 1;
  SET_ZN_FLAGS(&res);
  return res;
}

u8_t ROR(m6502_t * em, u8_t operand) {
  bool old_zero_bit = (operand & FLAG_C_BITMASK) > 0;
  operand = operand >> 1;
  if (C_) operand |= FLAG_N_BITMASK;
  C_ = old_zero_bit;
  SET_ZN_FLAGS(&operand);
  return operand;
}
u8_t ROL(m6502_t * em, u8_t operand) {
  u8_t new_zero_bit = C_ ? FLAG_C_BITMASK : 0;
  C_ = (operand & FLAG_N_BITMASK) > 0;
  operand = operand << 1;
  operand |= new_zero_bit;
  SET_ZN_FLAGS(&operand);
  return operand;
}

void compare(m6502_t * em, u8_t operand, u8_t reg) {
  u8_t tmp = reg - operand;
  N_ = (tmp & FLAG_N_BITMASK) > 0;
  Z_ = reg == operand;
  C_ = reg >= operand;
}

void branch_if(m6502_t * em, i32_t * cycles, bool test, bool expected){
  i8_t Offset = fetch_s_byte(em);
  if ( test == expected ) {
    cycles++;
    const u16_t PC_old = PC_;
    PC_ += Offset;
    if(HIGH_BYTE(PC_) != HIGH_BYTE(PC_old)) cycles++;
  }
}


int execute(m6502_t * em) {
  u8_t ins, data, ZP_addr;
  u16_t ABS_addr, ABS_addrY, sub_addr;
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
      ADC(em, read_byte(em, ZP_addr + X_));
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
      ABS_addr = addr_abs_reg(em, &X_, &cycles);
      ADC(em, read_byte(em, ABS_addr));
      if(em->debug) printf("ADC $%04X, x\n", ABS_addr -  + X_);
      break;

    case INS_ADC_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &Y_, &cycles);
      ADC(em, read_byte(em, ABS_addr));
      if(em->debug) printf("ADC $%04X, y\n", ABS_addr - Y_);
      break;

    case INS_ADC_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em);
      ADC(em, read_byte(em, read_word(em, ZP_addr + X_)));
      if(em->debug) printf("ADC ($%02X, x)\n", ZP_addr);
      break;

    case INS_ADC_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + Y_;
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
      SBC(em, read_byte(em, ZP_addr + X_));
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
      ABS_addr = addr_abs_reg(em, &X_, &cycles);
      SBC(em, read_byte(em, ABS_addr));
      if(em->debug) printf("SBC $%04X, x\n", ABS_addr - X_);
      break;

    case INS_SBC_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &Y_, &cycles);
      SBC(em, read_byte(em, ABS_addr));
      if(em->debug) printf("SBC $%04X, y\n", ABS_addr - Y_);
      break;

    case INS_SBC_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em);
      SBC(em, read_byte(em, read_word(em, ZP_addr + X_)));
      if(em->debug) printf("SBC ($%02X, x)\n", ZP_addr);
      break;

    case INS_SBC_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + Y_;
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
      compare(em, data, A_);
      if(em->debug) printf("CMP #$%02X\n", data);
      break;

    case INS_CMP_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      compare(em, read_byte(em, ZP_addr), A_);
      if(em->debug) printf("CMP $%02X\n", ZP_addr);
      break;

    case INS_CMP_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      compare(em, read_byte(em, ZP_addr + X_), A_);
      if(em->debug) printf("CMP $%02X, x\n", ZP_addr);
      break;

    case INS_CMP_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      compare(em, read_byte(em, ABS_addr), A_);
      if(em->debug) printf("CMP $%04X\n", ABS_addr);
      break;

    case INS_CMP_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &X_, &cycles);
      compare(em, read_byte(em, ABS_addr), A_);
      if(em->debug) printf("CMP $%04X, x\n", ABS_addr - X_);
      break;

    case INS_CMP_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &Y_, &cycles);
      compare(em, read_byte(em, ABS_addr), A_);
      if(em->debug) printf("CMP $%04X, y\n", ABS_addr - Y_);
      break;

    case INS_CMP_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + X_;
      compare(em, read_byte(em, read_word(em, ZP_addr)), A_);
      if(em->debug) printf("CMP ($%02X, x)\n", ZP_addr);
      break;

    case INS_CMP_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + Y_;
      if((ABS_addr ^ ABS_addrY) >> 8) cycles++; //page boundry is crossed
      compare(em, read_byte(em, ABS_addrY), A_);
      if(em->debug) printf("CMP ($%02X), y\n", ZP_addr);
      break;
    /**
     *  CPX instructions
     **/
    case INS_CPX_IM:
      cycles = 2;
      data = fetch_byte(em);
      compare(em, data, X_);
      if(em->debug) printf("CPX #$%02X\n", data);
      break;

    case INS_CPX_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      compare(em, read_byte(em, ZP_addr), X_);
      if(em->debug) printf("CPX $%02X\n", ZP_addr);
      break;

    case INS_CPX_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      compare(em, read_byte(em, ABS_addr), X_);
      if(em->debug) printf("CPX $%04X\n", ABS_addr);
      break;
    /**
     *  CPY instructions
     **/
    case INS_CPY_IM:
      cycles = 2;
      data = fetch_byte(em);
      compare(em, data, Y_);
      if(em->debug) printf("CPY #$%02X\n", data);
      break;

    case INS_CPY_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      compare(em, read_byte(em, ZP_addr), Y_);
      if(em->debug) printf("CPY $%02X\n", ZP_addr);
      break;

    case INS_CPY_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      compare(em, read_byte(em, ABS_addr), Y_);
      if(em->debug) printf("CPY $%04X\n", ABS_addr);
      break;

    /**
     *  AND instructions
     **/
    case INS_AND_IM:
      cycles = 2;
      data = fetch_byte(em);
      A_ &= data;
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("AND #$%02X\n", data);
      break;

    case INS_AND_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      A_ &= read_byte(em, ZP_addr);
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("AND $%02X\n", ZP_addr);
      break;

    case INS_AND_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      A_ &= read_byte(em, ZP_addr + X_);
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("AND $%02X, x\n", ZP_addr);
      break;

    case INS_AND_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      A_ &= read_byte(em, ABS_addr);
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("AND $%04X\n", ABS_addr);
      break;

    case INS_AND_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &X_, &cycles);
      A_ &= read_byte(em, ABS_addr);
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("AND $%04X, x\n", ABS_addr - X_);
      break;

    case INS_AND_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &Y_, &cycles);
      A_ &= read_byte(em, ABS_addr);
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("AND $%04X, y\n", ABS_addr - Y_);
      break;

    case INS_AND_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + X_;
      A_ &= read_byte(em, read_word(em, ZP_addr));
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("AND ($%02X, x)\n", ZP_addr);
      break;

    case INS_AND_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + Y_;
      if((ABS_addr ^ ABS_addrY) >> 8) cycles++; //page boundry is crossed
      A_ &= read_byte(em, ABS_addrY);
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("AND ($%02X), y\n", ZP_addr);
      break;
    /**
     *  ORA instructions
     **/
    case INS_ORA_IM:
      cycles = 2;
      data = fetch_byte(em);
      A_ |= data;
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("ORA #$%02X\n", data);
      break;

    case INS_ORA_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      A_ &= read_byte(em, ZP_addr);
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("ORA $%02X\n", ZP_addr);
      break;

    case INS_ORA_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      A_ |= read_byte(em, ZP_addr + X_);
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("ORA $%02X, x\n", ZP_addr);
      break;

    case INS_ORA_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      A_ |= read_byte(em, ABS_addr);
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("ORA $%04X\n", ABS_addr);
      break;

    case INS_ORA_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &X_, &cycles);
      A_ |= read_byte(em, ABS_addr);
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("ORA $%04X, x\n", ABS_addr - X_);
      break;

    case INS_ORA_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &Y_, &cycles);
      A_ |= read_byte(em, ABS_addr);
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("ORA $%04X, y\n", ABS_addr - Y_);
      break;

    case INS_ORA_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + X_;
      A_ |= read_byte(em, read_word(em, ZP_addr));
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("ORA ($%02X, x)\n", ZP_addr);
      break;

    case INS_ORA_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + Y_;
      if((ABS_addr ^ ABS_addrY) >> 8) cycles++; //page boundry is crossed
      A_ |= read_byte(em, ABS_addrY);
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("ORA ($%02X), y\n", ZP_addr);
      break;
    /**
     *  EOR instructions
     **/
    case INS_EOR_IM:
      cycles = 2;
      data = fetch_byte(em);
      A_ ^= data;
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("EOR #$%02X\n", data);
      break;

    case INS_EOR_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      A_ |= read_byte(em, ZP_addr);
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("EOR $%02X\n", ZP_addr);
      break;

    case INS_EOR_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      A_ ^= read_byte(em, ZP_addr + X_);
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("EOR $%02X, x\n", ZP_addr);
      break;

    case INS_EOR_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      A_ ^= read_byte(em, ABS_addr);
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("EOR $%04X\n", ABS_addr);
      break;

    case INS_EOR_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &X_, &cycles);
      A_ ^= read_byte(em, ABS_addr);
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("EOR $%04X, x\n", ABS_addr - X_);
      break;

    case INS_EOR_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &Y_, &cycles);
      A_ ^= read_byte(em, ABS_addr);
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("EOR $%04X, y\n", ABS_addr - Y_);
      break;

    case INS_EOR_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + X_;
      A_ ^= read_byte(em, read_word(em, ZP_addr));
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("EOR ($%02X, x)\n", ZP_addr);
      break;

    case INS_EOR_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + Y_;
      if((ABS_addr ^ ABS_addrY) >> 8) cycles++; //page boundry is crossed
      A_ ^= read_byte(em, ABS_addrY);
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("EOR ($%02X), y\n", ZP_addr);
      break;
    /**
     *  BRANCH instructions
     **/
    case INS_BCC:
      cycles = 1;
      if(em->debug) {
        data = read_byte(em, PC_);
        printf("BCC $%04X\n", data);
      }
      branch_if(em, &cycles, C_, false);
      break;

    case INS_BCS:
      cycles = 1;
      if(em->debug) {
        data = read_byte(em, PC_);
        printf("BCS $%04X\n", data);
      }
      branch_if(em, &cycles, C_, true);
      break;

    case INS_BEQ:
      cycles = 1;
      if(em->debug) {
        data = read_byte(em, PC_);
        printf("BEQ $%04X\n", data);
      }
      branch_if(em, &cycles, Z_, true);
      break;

    case INS_BMI:
      cycles = 1;
      if(em->debug) {
        data = read_byte(em, PC_);
        printf("BMI $%04X\n", data);
      }
      branch_if(em, &cycles, N_, true);
      break;

    case INS_BNE:
      cycles = 1;
      if(em->debug) {
        data = read_byte(em, PC_);
        printf("BNE $%04X\n", data);
      }
      branch_if(em, &cycles, Z_, false);
      break;

    case INS_BPL:
      cycles = 1;
      if(em->debug) {
        data = read_byte(em, PC_);
        printf("BPL $%04X\n", data);
      }
      branch_if(em, &cycles, N_, false);
      break;

    case INS_BVC:
      cycles = 1;
      if(em->debug) {
        data = read_byte(em, PC_);
        printf("BVC $%04X\n", data);
      }
      branch_if(em, &cycles, V_, false);
      break;

    case INS_BVS:
      cycles = 1;
      if(em->debug) {
        data = read_byte(em, PC_);
        printf("BVS $%04X\n", data);
      }
      branch_if(em, &cycles, V_, true);
      break;
    /**
     *  BIT instructions
     **/
    case INS_BIT_ZP:
      cycles = 2;
      ZP_addr = fetch_byte(em);
      data = read_byte(em, ZP_addr);
      Z_ = !(data & A_);
      N_ =  (data & 0b10000000) != 0;
      V_ =  (data & 0b01000000) != 0;
      if(em->debug) printf("BIT $%02X\n", ZP_addr);
      break;

    case INS_BIT_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      data = read_byte(em, ABS_addr);
      Z_ = !(data & A_);
      N_ =  (data & 0b10000000) != 0;
      V_ =  (data & 0b01000000) != 0;
      if(em->debug) printf("BIT $%04X\n", ABS_addr);
      break;

    case INS_CLC:
      cycles = 2;
      C_ = 0;
      if(em->debug) printf("CLC\n");
      break;

    case INS_CLD:
      cycles = 2;
      D_ = 0;
      if(em->debug) printf("CLD\n");
      break;

    case INS_CLI:
      cycles = 2;
      I_ = 0;
      if(em->debug) printf("CLI\n");
      break;

    case INS_CLV:
      cycles = 2;
      V_ = 0;
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
      SET_ZN_FLAGS(&data);
      write_byte(em, data, ZP_addr);
      if(em->debug) printf("DEC $%02X\n", ZP_addr);
      break;

    case INS_DEC_ZPX:
      cycles = 6;
      ZP_addr = fetch_byte(em);
      data = read_byte(em, ZP_addr + X_);
      data--;
      SET_ZN_FLAGS(&data);
      write_byte(em, data, ZP_addr);
      if(em->debug) printf("DEC $%02X, x\n", ZP_addr);
      break;

    case INS_DEC_ABS:
      cycles = 6;
      ABS_addr = fetch_word(em);
      data = read_byte(em, ABS_addr);
      data--;
      SET_ZN_FLAGS(&data);
      write_byte(em, data, ABS_addr);
      if(em->debug) printf("DEC $%04X\n", ABS_addr);
      break;

    case INS_DEC_ABSX:
      cycles = 7;
      ABS_addr = fetch_word(em);
      data = read_byte(em, ABS_addr + X_);
      data--;
      SET_ZN_FLAGS(&data);
      write_byte(em, data, ABS_addr);
      if(em->debug) printf("DEC $%04X, x\n", ABS_addr);
      break;
    /**
     *  DEY-DEX instructions
     **/
    case INS_DEX:
      cycles = 2;
      X_--;
      SET_ZN_FLAGS(&X_);
      if(em->debug) printf("DEX\n");
      break;

    case INS_DEY:
      cycles = 2;
      Y_--;
      SET_ZN_FLAGS(&Y_);
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
      SET_ZN_FLAGS(&data);
      write_byte(em, data, ZP_addr);
      if(em->debug) printf("INC $%02X\n", ZP_addr);
      break;

    case INS_INC_ZPX:
      cycles = 6;
      ZP_addr = fetch_byte(em);
      data = read_byte(em, ZP_addr + X_);
      data++;
      SET_ZN_FLAGS(&data);
      write_byte(em, data, ZP_addr);
      if(em->debug) printf("INC $%02X, x\n", ZP_addr);
      break;

    case INS_INC_ABS:
      cycles = 6;
      ABS_addr = fetch_word(em);
      data = read_byte(em, ABS_addr);
      data++;
      SET_ZN_FLAGS(&data);
      write_byte(em, data, ABS_addr);
      if(em->debug) printf("INC $%04X\n", ABS_addr);
      break;

    case INS_INC_ABSX:
      cycles = 7;
      ABS_addr = fetch_word(em);
      data = read_byte(em, ABS_addr + X_);
      data++;
      SET_ZN_FLAGS(&data);
      write_byte(em, data, ABS_addr);
      if(em->debug) printf("INC $%04X, x\n", ABS_addr);
      break;
    /**
     *  INX-INY instructions
     **/
    case INS_INX:
      cycles = 2;
      X_++;
      SET_ZN_FLAGS(&X_);
      if(em->debug) printf("INX\n");
      break;

    case INS_INY:
      cycles = 2;
      Y_++;
      SET_ZN_FLAGS(&Y_);
      if(em->debug) printf("INY\n");
      break;
    /**
     *  JSR-RTS instructions
     **/
    case INS_JSR:
      cycles = 6;
      sub_addr = fetch_word(em);
      push_word(em, PC_);
      PC_ = sub_addr;
      if(em->debug) printf("JSR $%04X\n", sub_addr);
      break;

    case INS_RTS:
      cycles = 6;
      PC_ = pull_word(em);
      if(em->debug) printf("RTS\n");
      break;

    case INS_JMP_ABS:
      cycles = 3;
      PC_ = fetch_word(em);
      if(em->debug) printf("JMP $%04X\n", PC_);
      break;

    case INS_JMP_IND:
      cycles = 5;
      ABS_addr = fetch_word(em);
      PC_ = read_word(em, ABS_addr);
      if(em->debug) printf("JMP ($%04X)\n", ABS_addr);
      break;
    /**
     *  LDA instructions
     **/
    case INS_LDA_IM:
      cycles = 2;
      data = fetch_byte(em);
      A_ = data;
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("LDA #$%02X\n", data);
      break;

    case INS_LDA_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      load_register(em, &A_, ZP_addr);
      if(em->debug) printf("LDA $%02X\n", ZP_addr);
      break;

    case INS_LDA_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      load_register(em, &A_, ZP_addr + X_);
      if(em->debug) printf("LDA $%02X, x\n", ZP_addr);
      break;

    case INS_LDA_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      load_register(em, &A_, ABS_addr);
      if(em->debug) printf("LDA $%04X\n", ABS_addr);
      break;

    case INS_LDA_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &X_, &cycles);
      load_register(em, &A_, ABS_addr);
      if(em->debug) printf("LDA $%04X, x\n", ABS_addr - X_);
      break;

    case INS_LDA_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &Y_, &cycles);
      load_register(em, &A_, ABS_addr);
      if(em->debug) printf("LDA $%04X, y\n", ABS_addr - Y_);
      break;

    case INS_LDA_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + X_;
      load_register(em, &A_, read_word(em, ZP_addr));
      if(em->debug) printf("LDA ($%02X, x)\n", ZP_addr - X_);
      break;

    case INS_LDA_INDY:
      cycles = 5;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      ABS_addrY = ABS_addr + Y_;
      if((ABS_addr ^ ABS_addrY) >> 8) cycles++; //page boundry is crossed
      load_register(em, &A_, ABS_addrY);
      if(em->debug) printf("LDA ($%02X), y\n", ZP_addr);
      break;
    /**
     *  LDX instructions
     **/
    case INS_LDX_IM:
      cycles = 2;
      data = fetch_byte(em);
      X_ = data;
      SET_ZN_FLAGS(&X_);
      if(em->debug) printf("LDX #$%02X\n", data);
      break;

    case INS_LDX_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      load_register(em, &X_, ZP_addr);
      if(em->debug) printf("LDX $%02X\n", ZP_addr);
    break;

    case INS_LDX_ZPY:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      load_register(em, &X_, ZP_addr + Y_);
      if(em->debug) printf("LDX $%02X, y\n", ZP_addr);
      break;

    case INS_LDX_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      load_register(em, &X_, ABS_addr);
      if(em->debug) printf("LDX $%04X\n", ABS_addr);
      break;

    case INS_LDX_ABSY:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &Y_, &cycles);
      load_register(em, &X_, ABS_addr);
      if(em->debug) printf("LDX $%04X, y\n", ABS_addr - Y_);
      break;
    /**
     *  LDY instructions
     **/
    case INS_LDY_IM:
      cycles = 2;
      data = fetch_byte(em);
      Y_ = data;
      SET_ZN_FLAGS(&Y_);
      if(em->debug) printf("LDY #$%02X\n", data);
      break;

    case INS_LDY_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      load_register(em, &Y_, ZP_addr);
      if(em->debug) printf("LDY $%02X\n", ZP_addr);
    break;

    case INS_LDY_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      load_register(em, &Y_, ZP_addr + X_);
      if(em->debug) printf("LDY $%02X, x\n", ZP_addr);
      break;

    case INS_LDY_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      load_register(em, &Y_, ABS_addr);
      if(em->debug) printf("LDY $%04X\n", ABS_addr);
      break;

    case INS_LDY_ABSX:
      cycles = 4;
      ABS_addr = addr_abs_reg(em, &X_, &cycles);
      load_register(em, &Y_, ABS_addr);
      if(em->debug) printf("LDY $%04X, x\n", ABS_addr - X_);
      break;
    /**
     *  STA instructions
     **/
    case INS_STA_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      write_byte(em, A_, ZP_addr);
      if(em->debug) printf("STA $%02X\n", ZP_addr);
      break;

    case INS_STA_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      write_byte(em, A_, ZP_addr + X_);
      if(em->debug) printf("STA $%02X, x\n", ZP_addr);
      break;

    case INS_STA_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      write_byte(em, A_, ABS_addr);
      if(em->debug) printf("STA $%04X\n", ABS_addr);
      break;

    case INS_STA_ABSX:
      cycles = 5;
      ABS_addr = addr_abs_reg_5(em, &X_);
      write_byte(em, A_, ABS_addr);
      if(em->debug) printf("STA $%04X, x\n", ABS_addr - X_);
      break;

    case INS_STA_ABSY:
      cycles = 5;
      ABS_addr = addr_abs_reg_5(em, &Y_);
      write_byte(em, A_, ABS_addr);
      if(em->debug) printf("STA $%04X, y\n", ABS_addr - Y_);
      break;

    case INS_STA_INDX:
      cycles = 6;
      ZP_addr = fetch_byte(em) + X_;
      write_byte(em, A_, read_word(em, ZP_addr));
      if(em->debug) printf("STA ($%02X, x)\n", ZP_addr);
      break;

    case INS_STA_INDY:
      cycles = 6;
      ZP_addr = fetch_byte(em);
      ABS_addr = read_word(em, ZP_addr);
      write_byte(em, A_, ABS_addr + Y_);
      if(em->debug) printf("STA ($%02X), y\n", ZP_addr);
      break;
    /**
     *  STX instructions
     **/
    case INS_STX_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      write_byte(em, X_, ZP_addr);
      if(em->debug) printf("STX $%02X\n", ZP_addr);
      break;

    case INS_STX_ZPY:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      write_byte(em, X_, ZP_addr + Y_);
      if(em->debug) printf("STX $%02X, y\n", ZP_addr);
      break;

    case INS_STX_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      write_byte(em, X_, ABS_addr);
      if(em->debug) printf("STX $%04X\n", ABS_addr);
      break;
    /**
     *  STY instructions
     **/
    case INS_STY_ZP:
      cycles = 3;
      ZP_addr = fetch_byte(em);
      write_byte(em, Y_, ZP_addr);
      if(em->debug) printf("STY $%02X\n", ZP_addr);
      break;

    case INS_STY_ZPX:
      cycles = 4;
      ZP_addr = fetch_byte(em);
      write_byte(em, Y_, ZP_addr + X_);
      if(em->debug) printf("STY $%02X, x\n", ZP_addr);
      break;

    case INS_STY_ABS:
      cycles = 4;
      ABS_addr = fetch_word(em);
      write_byte(em, Y_, ABS_addr);
      if(em->debug) printf("STY $%04X\n", ABS_addr);
      break;
    /**
     *  LSR instructions
     **/
    case INS_LSR_ACC:
      cycles = 2;
      A_ = LSR(em, A_);
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
      ZP_addr = fetch_byte(em) + X_;
      data = read_byte(em, ZP_addr);
      write_byte(em, LSR(em, data), ZP_addr);
      if(em->debug) printf("LSR $%02X, x\n", ZP_addr - X_);
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
      ABS_addr = fetch_word(em) + X_;
      data = read_byte(em, ABS_addr);
      write_byte(em, LSR(em, data), ABS_addr);
      if(em->debug) printf("LSR $%04X, x\n", ABS_addr - X_);
      break;
    /**
     *  ASL instructions
     **/
    case INS_ASL_ACC:
      cycles = 2;
      A_ = ASL(em, A_);
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
      ZP_addr = fetch_byte(em) + X_;
      data = read_byte(em, ZP_addr);
      write_byte(em, ASL(em, data), ZP_addr);
      if(em->debug) printf("ASL $%02X, x\n", ZP_addr - X_);
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
      ABS_addr = fetch_word(em) + X_;
      data = read_byte(em, ABS_addr);
      write_byte(em, ASL(em, data), ABS_addr);
      if(em->debug) printf("ASL $%04X, x\n", ABS_addr - X_);
      break;
    /**
     *  ROL instructions
     **/
    case INS_ROL_ACC:
      cycles = 2;
      A_ = ROL(em, A_);
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
      ZP_addr = fetch_byte(em) + X_;
      data = read_byte(em, ZP_addr);
      write_byte(em, ROL(em, data), ZP_addr);
      if(em->debug) printf("ROL $%02X, x\n", ZP_addr - X_);
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
      ABS_addr = fetch_word(em) + X_;
      data = read_byte(em, ABS_addr);
      write_byte(em, ROL(em, data), ABS_addr);
      if(em->debug) printf("ROL $%04X, x\n", ABS_addr - X_);
      break;
    /**
     *  ROR instructions
     **/
    case INS_ROR_ACC:
      cycles = 2;
      A_ = ROR(em, A_);
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
      ZP_addr = fetch_byte(em) + X_;
      data = read_byte(em, ZP_addr);
      write_byte(em, ROR(em, data), ZP_addr);
      if(em->debug) printf("ROR $%02X, x\n", ZP_addr - X_);
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
      ABS_addr = fetch_word(em) + X_;
      data = read_byte(em, ABS_addr);
      write_byte(em, ROR(em, data), ABS_addr);
      if(em->debug) printf("ROR $%04X, x\n", ABS_addr - X_);
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
      push_byte(em, A_);
      if(em->debug) printf("PHA\n");
      break;

    case INS_PHP:
      cycles = 3;
      push_byte(em, PS_ | 0b00001100);
      if(em->debug) printf("PHP\n");
      break;

    case INS_PLA:
      cycles = 4;
      A_ = pull_byte(em);
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("PLA\n");
      break;

    case INS_PLP:
      cycles = 4;
      PS_ = pull_byte(em);
      if(em->debug) printf("PLP\n");
      break;

    case INS_RTI:
      cycles = 6;
      PS_ = pull_byte(em);
      PC_ = pull_word(em);
      if(em->debug) printf("RTI\n");
      break;

    case INS_SEC:
      cycles = 2;
      C_ = 1;
      if(em->debug) printf("SEC\n");
      break;

    case INS_SED:
      cycles = 2;
      D_ = 1;
      if(em->debug) printf("SED\n");
      break;

    case INS_SEI:
      cycles = 2;
      I_ = 1;
      if(em->debug) printf("SEI\n");
      break;
    /**
     *  TRANSFER instructions
     **/
    case INS_TAX:
      cycles = 2;
      X_ = A_;
      SET_ZN_FLAGS(&X_);
      if(em->debug) printf("TAX\n");
      break;

    case INS_TAY:
      cycles = 2;
      Y_ = SP_;
      SET_ZN_FLAGS(&Y_);
      if(em->debug) printf("TAY\n");
      break;

    case INS_TSX:
      cycles = 2;
      X_ = SP_;
      SET_ZN_FLAGS(&X_);
      if(em->debug) printf("TSX\n");
      break;

    case INS_TXA:
      cycles = 2;
      A_ = X_;
      SET_ZN_FLAGS(&A_);
      if(em->debug) printf("TXA\n");
      break;

    case INS_TXS:
      cycles = 2;
      SP_ = X_;
      if(em->debug) printf("TXS\n");
      break;

    case INS_TYA:
      cycles = 2;
      A_ = Y_;
      SET_ZN_FLAGS(&A_);
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


void reset(m6502_t * em) {
  init_mem(&em->mem);
  reset_cpu(&em->cpu);
  em->debug = false;
}
void start_program(m6502_t * em) {
  PC_ = read_word(em, RESET_CPU);
}

void enable_debug(m6502_t * em) {
  em->debug = true;
}
void disable_debug(m6502_t * em) {
  em->debug = true;
}

m6502_t copy_state(m6502_t *em) {
  m6502_t new;
  reset(&new);
  memcpy(new.mem.data, em->mem.data, MEMORY_SIZE);
  new.cpu.PC = PC_;
  new.cpu.SP = SP_;
  new.cpu.A = A_;
  new.cpu.X = X_;
  new.cpu.Y = Y_;
  new.cpu.status = PS_;
  return new;
}


void debug(m6502_t * em) {
  u32_t i, j, k, cnt;
  u8_t r;
  u16_t addr;

  printf("6502 emulator status\n");
  printf("Registers:\n");
  printf("A: %02X %i\n", A_, A_);
  printf("X: %02X %i\n", X_, X_);
  printf("Y: %02X %i\n", Y_, Y_);
  printf("\033[43mPC\033[0m: %04X %i\n", PC_, PC_);
  printf("\033[42mSP\033[0m: %04X %i\n\n", SP_, SP_);

  printf("STATUS REGISTER: \n");
  printf("┌─┬─┬─┬─┬─┬─┬─┬─┐\n");
  printf("│C│Z│I│D│B│U│V│N│\n");
  printf("│%i│%i│%i│%i│%i│%i│%i│%i│\n",
  C_,Z_,I_,D_,B_,U_,V_,N_);
  printf("└─┴─┴─┴─┴─┴─┴─┴─┘\n");

  printf("MEMORY:\n");
  printf("┌─────────┬─────────────────────────┬─────────────────────────┬────────┬────────┐\n");

  for (i = r = cnt = 0; i < MEMORY_SIZE; i += 0x10) {
    if(i != 0 && !memcmp(&em->mem.data[i], &em->mem.data[i - 0x10], 0x10) &&
      (PC_ < i || PC_ > (i + 0x0F)) && (SP_ < i || SP_ > (i + 0x0F))){
      r = 1; cnt++; continue;
    }

    if (r) {
      printf("│*%7i │                         │                         │        │        │\n",cnt);
      r = cnt = 0;
    }

    printf("│%08x ", i);

    j = 0;
    while (j < 2) {
      printf("│ ");
      for(k = 0; k < 0x8; k++) {
        addr = (i+k+(j*0x8));
        if (addr == PC_)
          printf("\033[43m%02x\033[0m ", em->mem.data[addr]);
        else if (addr == SP_)
          printf("\033[42m%02x\033[0m ", em->mem.data[addr]);
        else
          printf("%02x ", em->mem.data[addr]);
      }
      j++;
    }

    j = 0;
    while (j < 2) {
      printf("│");
      for(k = 0; k < 0x8; k++) {
        addr = (i+k+(j*0x8));
        if(31 < em->mem.data[addr] && 127 > em->mem.data[addr])
          printf("%c", (char)em->mem.data[addr]);
        else if(em->mem.data[addr] == 0)
          printf("0");
        else printf("×");
      }
      j++;
    }
    printf("│");
    printf("\n");
  }
  printf("└─────────┴─────────────────────────┴─────────────────────────┴────────┴────────┘\n");
}

void memset_word(m6502_t * em, u16_t data, u16_t addr) {
  mem_write(&em->mem, HIGH_BYTE(data), addr);
  mem_write(&em->mem, LOW_BYTE(data), addr+1);
}

u8_t fetch_byte(m6502_t * em) {
  u8_t data;
  data = mem_read(&em->mem, PC_);
  PC_++;
  return data;
}
i8_t fetch_s_byte(m6502_t * em) {
  return fetch_byte(em);
}

u16_t fetch_word(m6502_t * em) {
  u8_t lb;
  u16_t hb;
  lb = fetch_byte(em);
  hb = fetch_byte(em);
  hb = hb << 8;
  hb |= lb;
  return hb;
}

u8_t read_byte(m6502_t * em, u16_t addr) {
  u8_t data;
  data = mem_read(&em->mem, addr);
  return data;
}

u16_t read_word(m6502_t * em, u16_t addr) {
  u8_t lb;
  u16_t hb;
  lb = read_byte(em, addr);
  hb = read_byte(em, addr + 1);
  hb = hb << 8;
  hb |= lb;
  return hb;
}

void write_byte(m6502_t * em, u8_t data, u16_t addr) {
  mem_write(&em->mem, data, addr);
}

void write_word(m6502_t * em, u16_t data, u16_t addr) {
  u8_t lb;
  u8_t hb;

  hb = HIGH_BYTE(data);
  lb = LOW_BYTE(data);
  write_byte(em, lb, addr);
  write_byte(em, hb, addr + 1);
}

void push_byte(m6502_t * em, u8_t data) {
  write_word(em, data, STACK_OFFSET | (SP_--));
}
void push_word(m6502_t * em, u16_t data) {
  write_word(em, data, STACK_OFFSET | (SP_-1));
  SP_ -= 2;
}

u8_t pull_byte(m6502_t * em) {
  u8_t data;
  SP_++;
  data = read_byte(em, STACK_OFFSET | SP_);
  write_byte(em, 0x00, STACK_OFFSET | SP_);
  return data;
}
u16_t pull_word(m6502_t * em) {
  u8_t lb;
  u16_t hb;
  lb = pull_byte(em);
  hb = pull_byte(em);
  hb = hb << 8;
  hb |= lb;
  return hb;
}
