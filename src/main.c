#include <c64.h>
#include <peekpoke.h>
#include <stdlib.h>
#include <stdio.h>

#define SPRITE_HEIGHT 21
#define SPRITE_WIDTH 24
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

#define BORDER_LEFT 24
#define BORDER_RIGHT (SCREEN_WIDTH+BORDER_LEFT)
#define BORDER_TOP 51
#define BORDER_BOTTOM (SCREEN_HEIGHT+BORDER_TOP)

// https://cc65.github.io/mailarchive/2008-03/6026.html

signed int sprite_y=BORDER_TOP+1,sprite_x=BORDER_LEFT+1;
signed int sprite2_y=BORDER_BOTTOM-1-SPRITE_HEIGHT,sprite2_x=BORDER_RIGHT-1-SPRITE_WIDTH;

void my_irq_2(void);

// macro defined place sprite on screen. Uses about half a raster line. Avoids the function call overhead. 
#define position_sprite_m(nr,xxx,yyy) {VIC.spr_pos[nr].y = (yyy);\
    VIC.spr_pos[nr].x = (unsigned char)((xxx)&0xff);\
    if ((xxx)&0x100) \
        VIC.spr_hi_x|=(1<<nr);\
    else\
        VIC.spr_hi_x&=(~(1<<nr));}




// interrupt processing. Yellow bar shows the used time

signed char dx=1,dy=-3,dx2=2,dy2=1;
signed char mycol=1;

struct sprit {

    unsigned int x;
    unsigned char y;
    signed char dx,dy;
};
#define NR_SPRITES 2
struct sprit sprits[NR_SPRITES] = {{50,50,1,-3},{100,80,2,1}};


void my_irq(void) {

    unsigned char irr = VIC.irr;
    unsigned char i;
    struct sprit *s=sprits;
 

    // raster interrupt
    if (irr&1) {
        //signed char tx,ty;
        VIC.bordercolor=mycol;
        // VIC.rasterline = 255;
        // VIC.ctrl1&=0xf7;

        /* ack raster IRQ */
        VIC.irr = 1;
        //  POKEW(0x0314, (int)&my_irq_2);

        // if (VIC.spr_coll) {
        //     mycol++;
        //     VIC.spr_coll=255;
        //      tx=dx;
        //      ty=dy;
        //      dx=dx2;
        //      dy=dy2;
        //      dx2=tx;
        //      dy2=ty;
        // }

        for (i=0;i<NR_SPRITES;i++) {
            VIC.bordercolor=COLOR_GREEN;
            sprits[i].x+=sprits[i].dx;
            sprits[i].y+=sprits[i].dy;
            VIC.bordercolor=COLOR_LIGHTBLUE;


            if (sprits[i].x <= BORDER_LEFT || sprits[i].x >= (BORDER_RIGHT-SPRITE_WIDTH)) sprits[i].dx=-sprits[i].dx;
            if (sprits[i].y <= BORDER_TOP || sprits[i].y >= (BORDER_BOTTOM-SPRITE_HEIGHT)) sprits[i].dy=-sprits[i].dy;
    
          //  position_sprite_m(i,sprits[i].x,sprits[i].y);
        }

        // sprite_x+=dx;
        // sprite_y+=dy;
        // sprite2_x+=dx2;
        // sprite2_y+=dy2;

        // if (sprite_x <= BORDER_LEFT|| sprite_x >= (BORDER_RIGHT-SPRITE_WIDTH)) dx=-dx;
        // if (sprite_y <= BORDER_TOP || sprite_y  >= (BORDER_BOTTOM-SPRITE_HEIGHT)) dy=-dy;
        // if (sprite2_x <= BORDER_LEFT || sprite2_x >= (BORDER_RIGHT-SPRITE_WIDTH)) dx2=-dx2;
        // if (sprite2_y <= BORDER_TOP || sprite2_y  >= (BORDER_BOTTOM-SPRITE_HEIGHT)) dy2=-dy2;


        // an exceptionally inefficient routine
        // position_sprite_m(0,sprite_x,sprite_y);
        // position_sprite_m(1,sprite2_x,sprite2_y);


        VIC.bordercolor=COLOR_BLACK;

        // normal interrupt processing
        __asm__(" jmp $ea31");

    }


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

  /* enable raster and sprite IRQs */
  VIC.imr = 1;
  VIC.spr_coll=255;
}

extern char sprite1[];
char *smem=(char *)832;

void main(void) {
    unsigned char n;
  irq_setup(&my_irq);
  VIC.spr_ena=3;
 //  VIC.spr0_y=sprite_y;
 //  VIC.spr0_x=sprite_x;


    // two identical sprites
  POKE(2040,13);
  POKE(2041,13);    
  VIC.spr0_color = COLOR_CYAN;
  VIC.spr1_color = COLOR_YELLOW;

  // https://www.commodore.ca/manuals/c64_users_guide/c64-users_guide-06-sprite_graphics.pdf


for (n = 0 ; n <= 62; n++) {
 		POKE(832+n,sprite1[n]);

    }
   // printf("%u\n",&VIC.spr_coll);

  while (PEEK(0xc6) == 0) {
    ;
  }
}