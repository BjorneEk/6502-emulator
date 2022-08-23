////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////

#ifndef _IO6522_H_
#define _IO6522_H_

#include "util.h"


/**
 * +-----+-----+-----+-----+----------+-------------------------------------+----------------------+
 * | RS3 | RS2 | RS1 | RS0 | Dest reg |               w desc                |        r desc        |
 * +-----+-----+-----+-----+----------+-------------------------------------+----------------------+
 * |   0 |   0 |   0 |   0 | ORB/IRB  | Output Register "B"                 | Input Register "B"   |
 * |   0 |   0 |   0 |   1 | ORA/IRA  | Output Register "A"                 | Input Register "A"   |
 * |   0 |   0 |   1 |   0 | DDRB     | Data Direction Register "B"         |                      |
 * |   0 |   0 |   1 |   1 | DDRA     | Data Direction Register "A"         |                      |
 * |   0 |   1 |   0 |   0 | T1C-L    | T1 Low-Order Latches                | T1 Low-Order Counter |
 * |   0 |   1 |   0 |   1 | T1C-H    | T1 High-Order Counter               |                      |
 * |   0 |   1 |   1 |   0 | T1L-L    | T1 Low-Order Latches                |                      |
 * |   0 |   1 |   1 |   1 | T1L-H    | T1 High-Order Latches               |                      |
 * |   1 |   0 |   0 |   0 | T2C-L    | T2 Low-Order Latches                | T2 Low-Order Counter |
 * |   1 |   0 |   0 |   1 | T2C-H    | T2 High-Order Counter               |                      |
 * |   1 |   0 |   1 |   0 | SR       | Shift Register                      |                      |
 * |   1 |   0 |   1 |   1 | ACR      | Auxiliary Control Register          |                      |
 * |   1 |   1 |   0 |   0 | PCR      | Peripheral Control Register         |                      |
 * |   1 |   1 |   0 |   1 | IFR      | Interrupt Flag Register             |                      |
 * |   1 |   1 |   1 |   0 | IER      | Interrupt Enable Register           |                      |
 * |   1 |   1 |   1 |   1 | ORA/IRA  | Same as Reg 1 except no "Handshake" |                      |
 * +-----+-----+-----+-----+----------+-------------------------------------+----------------------+
 **/
typedef struct {

  union { u8_t ORB, IRB; };
  union { u8_t ORA, IRA; };

  u8_t DDRB;
  u8_t DDRA;

  u8_t T1C_L, T1C_H;
  u8_t T1L_L, T1L_H;
  u8_t T2C_L, t2C_H;
  u8_t SR;
  u8_t ACR;
  u8_t PCR;
  u8_t IFR;
  u8_t IER;

} io6522_t


#endif
