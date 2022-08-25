vidpage = $0000
start_color = $0002
 .org $8000

reset:
  cli
  lda #$0
  sta start_color

loop:
 lda #$30
 sta vidpage + 1
 lda #$00
 sta vidpage

 ldx #$20
 ldy #$0
 inc start_color
 lda start_color

page:
  sta (vidpage), y

  and #$7f
  bne inc_color
  clc
  adc #$1

inc_color:
  clc
  adc #$1
  iny
  bne page

  inc vidpage + 1
  dex

  bne page

 jmp loop

nmi:
  rti
irq:
  ldy #$0
  lda #$0
  ldx #$20
irq_loop:
  sta (vidpage), y
  and #$7f
  bne inc_page
  clc

inc_page:
  iny
  bne irq_loop
  inc vidpage + 1
  dex
  bne irq_loop
exit_irq:
  rti



 .org $fffa
 .word nmi
 .word reset
 .word irq
