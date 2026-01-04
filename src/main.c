#include <c64.h>
#include <peekpoke.h>

// https://cc65.github.io/mailarchive/2008-03/6026.html

unsigned char sprite_y=50;
unsigned int sprite_x=300;

void my_irq_2(void);

// place a sprite on the screen
void position_sprite(unsigned char nr, unsigned int x, unsigned char y) {

    VIC.spr_pos[nr].y = y;
    VIC.spr_pos[nr].x = (unsigned char)(x&0xff);

    // magic magix
    if (x&0x100) 
        VIC.spr_hi_x|=(1<<nr);
    else
        VIC.spr_hi_x&=(0xff ^ (1<<nr));
    
}

void my_irq(void) {

    VIC.bordercolor=COLOR_YELLOW;
    VIC.rasterline = 255;
   // VIC.ctrl1&=0xf7;

  /* ack raster IRQ */
  VIC.irr = 1;
   POKEW(0x0314, (int)&my_irq_2);

  __asm__(" jmp $ea31");
}

void my_irq_2(void) {
    VIC.bordercolor=COLOR_BLACK;
    VIC.rasterline = 249;
  //  VIC.ctrl1 |=0x08;
  /* ack raster IRQ */
  VIC.irr = 1;
   POKEW(0x0314, (int)&my_irq);

   sprite_y++;
   sprite_x+=2;

    position_sprite(0,sprite_x,sprite_y);


  __asm__(" jmp $ea31");
}


void __fastcall__ irq_setup(void (*irqh)(void)) {
  /* disable IRQ sources, SEI not needed */
  CIA1.icr = 0x7f;
  VIC.imr = 0;

  VIC.rasterline = 249;
  VIC.ctrl1 = 0x1b;

  /* set kernal IRQ vector */
  POKEW(0x0314, (int)irqh);

  /* enable raster IRQs */
  VIC.imr = 1;
}

extern char sprite1[];
char *smem=(char *)832;

void main(void) {
    unsigned char n;
  irq_setup(&my_irq);
  VIC.spr_ena=1;
//   VIC.spr0_y=sprite_y;
//   VIC.spr0_x=sprite_x;


  POKE(2040,13);

  // https://www.commodore.ca/manuals/c64_users_guide/c64-users_guide-06-sprite_graphics.pdf


for (n = 0 ; n <= 62; n++) {
 		POKE(832+n,sprite1[n]);
    }

  while (PEEK(0xc6) == 0) {
    ;
  }
}