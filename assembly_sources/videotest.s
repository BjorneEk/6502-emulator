vidpage = $0000
start_color = $0002
 .org $8000

reset:
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

 .org $fffc
 .word $8000
 .word $0000
