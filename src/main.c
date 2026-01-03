#include <c64.h>
#include <peekpoke.h>

#https://cc65.github.io/mailarchive/2008-03/6026.html

void music_init(void) {
}


void music_play(void) {
}


void my_irq(void) {
  ++VIC.bgcolor0;
  music_play();
  --VIC.bgcolor0;

  /* ack raster IRQ */
  VIC.irr = 1;
  __asm__(" jmp $ea31");
}


void __fastcall__ irq_setup(void (*irqh)(void)) {
  /* disable IRQ sources, SEI not needed */
  CIA1.icr = 0x7f;
  VIC.imr = 0;

  /* irq on line 100 */
  VIC.rasterline = 100;
  VIC.ctrl1 = 0x1b;

  /* set kernal IRQ vector */
  POKEW(0x0314, (int)irqh);

  /* enable raster IRQs */
  VIC.imr = 1;
}


void main(void) {
  music_init();

  irq_setup(&my_irq);

  while (PEEK(0xc6) == 0) {
    ;
  }
}