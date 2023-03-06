  .sect    .data
  io_fbyte equ 1
  io_fline equ 2
  io_pend  equ 0
  io_sstrg equ 7
  sd_chenq equ $b
  err_ok   equ 0
  err_bp   equ -15
  .align  2
ssp_save:
  ds.w    2
buf_ptr:
  ds.w    2
err_code:
  ds.w    1
  .globl  _ch_io_stub
  .sect   .text
;; IOSS CH_IO ENTRY POINT
_ch_io_stub:
  move.l  a4,-(a7)
  ;; Save system stack pointer and switch to alternate stack reserved by us
  lea     ssp_save,a4
  move.l  a7,(a4)
  movea.l _altsysstack_top,a7
  ;; A1 is saved separately, A5 is FP, A6 is fixed with compiler flags
  movem.l a0/a2-a3/d2-d7,-(a7)
is_op_sd_chenq:
  cmp.b   #sd_chenq,d0
  bne     is_op_pend
  move.w  #err_bp,d0
  bra     ch_io_stub_exit
is_op_pend:
  cmp.b   #io_pend,d0
  bne     is_op_fbyte
  pea     err_code              ; Error code for exit
  move.l  a0,-(a7)              ; Channel block
  lea     _pend,a4
  jsr     (a4)
  addq    #8,a7
  move.w  err_code,d0
  bra     ch_io_stub_exit
is_op_fbyte:
  cmp.b   #io_fbyte,d0
  bne     is_op_fline
  ;; Call the actual fbyte routine
  pea     err_code
  move.l  a0,-(a7)
  lea     _fbyte,a4
  jsr     (a4)
  addq    #8,a7
  move.b  d0,d1                 ; Byte returned from fbyte
  move.w  err_code,d0
  bra     ch_io_stub_exit
is_op_fline:
  cmp.b   #io_fline,d0
  bne     is_op_sstrg
  pea     err_code
  lea     buf_ptr,a4
  move.l  a1,(a4)               ; Location of first byte of buffer to read to
  move.l  a4,-(a7)
  move.w  d2,-(a7)              ; Buffer size
  move.l  a0,-(a7)              ; Channel block pointer
  lea     _fline,a4
  jsr     (a4)
  lea     14(a7),a7
  moveq   #0,d1
  move.w  d0,d1                 ; Number of bytes read
  lea     buf_ptr,a4
  move.l  (a4),a1
  lea     err_code,a4
  moveq   #0,d0
  move.w  (a4),d0
  bra     ch_io_stub_exit
is_op_sstrg:
  cmp.b   #io_sstrg,d0
  bne     op_no_match
  lea     buf_ptr,a4
  move.l  a1,(a4)               ; Location of first byte to be sent
  move.l  a4,-(a7)
  move.w  d2,-(a7)              ; Number of characters to write
  move.w  d3,-(a7)              ; Timeout
  move.l  a0,-(a7)              ; Channel block pointer
  lea     _sstrg,a4
  jsr     (a4)
  adda.l  #12,a7
  lea     buf_ptr,a4
  movea.l (a4),a1
  move.w  d0,d1                 ; # of bytes sent
  moveq   #0,d0               ; Error code
  bra     ch_io_stub_exit
op_no_match:
  moveq   #0,d0
ch_io_stub_exit:
  ;; Restore registers
  movem.l (a7)+,a0/a2-a3/d2-d7
  ;; Restore original system stack pointer
  lea     ssp_save,a4
  movea.l (a4),a7
  movea.l (a7)+,a4
  ;; D0=return value (error code)
  rts

