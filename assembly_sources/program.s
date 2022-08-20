 .org $8000



  lda #$01
  clc
  adc #$01
  sta $FFFF

loop:
  lda #$55
  sta $6000

  lda #$aa
  sta $6000

  jmp loop

 .org $fffc
 .word $8000
 .word $0000
