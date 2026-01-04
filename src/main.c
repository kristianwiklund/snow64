#include <c64.h>
#include <peekpoke.h>

#define SPRITE_HEIGHT 21
#define SPRITE_WIDTH 24
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

#define BORDER_LEFT 24
#define BORDER_RIGHT (SCREEN_WIDTH+BORDER_LEFT)
#define BORDER_TOP 51
#define BORDER_BOTTOM (SCREEN_HEIGHT+BORDER_TOP)

// https://cc65.github.io/mailarchive/2008-03/6026.html

signed int sprite_x=50,sprite_y=50;
signed int sprite2_x=150,sprite2_y=150;

void my_irq_2(void);

// macro defined place sprite on screen. Uses about half a raster line. Avoids the function call overhead. 
#define position_sprite_m(nr,xxx,yyy) {VIC.spr_pos[nr].y = (yyy);\
    VIC.spr_pos[nr].x = (unsigned char)((xxx)&0xff);\
    if ((xxx)&0x100) \
        VIC.spr_hi_x|=(1<<nr);\
    else\
        VIC.spr_hi_x&=(0xff ^ (1<<nr));}




// interrupt processing. Yellow bar shows the used time

signed char dx=1,dy=3,dx2=-2,dy2=1;

void my_irq(void) {

    VIC.bordercolor=COLOR_YELLOW;
   // VIC.rasterline = 255;
    // VIC.ctrl1&=0xf7;

    /* ack raster IRQ */
    VIC.irr = 1;
    //  POKEW(0x0314, (int)&my_irq_2);

    sprite_x+=dx;
    sprite_y+=dy;
    sprite2_x+=dx2;
    sprite2_y+=dy2;

    if (sprite_x <= BORDER_LEFT|| sprite_x >= (BORDER_RIGHT-SPRITE_WIDTH)) dx=-dx;
    if (sprite_y <= BORDER_TOP || sprite_y  >= (BORDER_BOTTOM-SPRITE_HEIGHT)) dy=-dy;
    if (sprite2_x <= BORDER_LEFT || sprite2_x >= (BORDER_RIGHT-SPRITE_WIDTH)) dx2=-dx2;
    if (sprite2_y <= BORDER_TOP || sprite2_y  >= (BORDER_BOTTOM-SPRITE_HEIGHT)) dy2=-dy2;


    // an exceptionally inefficient routine
    position_sprite_m(0,sprite_x,sprite_y);
    position_sprite_m(1,sprite2_x,sprite2_y);

    VIC.bordercolor=COLOR_BLACK;
 

  __asm__(" jmp $ea31");
}

// used for border removal - not active now
void my_irq_2(void) {
    VIC.rasterline = 249;
  //  VIC.ctrl1 |=0x08;
  /* ack raster IRQ */
  VIC.irr = 1;
   POKEW(0x0314, (int)&my_irq);



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
  VIC.spr_ena=3;
//   VIC.spr0_y=sprite_y;
//   VIC.spr0_x=sprite_x;


    // two identical sprites
  POKE(2040,13);
  POKE(2041,13);

  // https://www.commodore.ca/manuals/c64_users_guide/c64-users_guide-06-sprite_graphics.pdf


for (n = 0 ; n <= 62; n++) {
 		POKE(832+n,sprite1[n]);

    }

  while (PEEK(0xc6) == 0) {
    ;
  }
}