  .sect .data
  bp_init equ $110
  .align 2
procdef:
  .dc.w 2
  .globl dhcpinit_ptr
dhcpinit_ptr:
  .dc.w 0
  .dc.b 3,'DHI'
  .globl omanreset_ptr
omanreset_ptr:
  .dc.w 0
  .dc.b 3,'OMR'
  .dc.w 0
  .dc.w 0
  .dc.w 0
    ;; 0,1, /* number of procedures (word) */
    ;; 0,0, /* relative pointer to routine */
    ;; 3, /* length of proc name */
    ;; 'B','O','O', /* proc name */
    ;; 0,0, /* end of procedures */
    ;; 0,0, /* number of functions */
    ;; 0,0 /* end of functions */
  .align 2
  .globl _setup_basic_keywords
_setup_basic_keywords:
  movem.l a0-a2/d0-d3,-(sp)
  ;; procedure definition table pointers
  ;; need to be patched runtime
  ;; because C startup module will patch effective addresses of labels
  ;; only when code gets called
  lea     dhcpinit_stub,a0
  lea     dhcpinit_ptr,a1
  sub.l   a1,a0
  move.w  a0,(a1)
  lea     omanreset_stub,a0
  lea     omanreset_ptr,a1
  sub.l   a1,a0
  move.w  a0,(a1)
  lea     procdef,a1
  suba.l  a2,a2
  move.w  bp_init,a2
  jsr     (a2)
  movem.l (sp)+,a0-a2/d0-d3
  rts
  .globl dhcpinit_stub
dhcpinit_stub:
  lea     _dhcpinit_impl,a0
  jsr     (a0)
  moveq   #0,d0
  rts
  .globl  omanreset_stub
omanreset_stub:
  lea     _omanreset_impl,a0
  jsr     (a0)
  moveq   #0,d0
  rts
