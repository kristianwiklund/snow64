#include <c64.h>
#include <peekpoke.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#include "spritemovement.h"


// irqs: https://cc65.github.io/mailarchive/2008-03/6026.html


void my_irq_2(void);



// displays a hex number ("num") at the top left of the screen
const unsigned  char hexor[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

 #define pokezor(num){\
  POKE(1024,hexor[num>>4]);\
  POKE(1025,hexor[num&0xf]);\
 }

 // iterators
unsigned char i,j,n;

#define NR_SPRITES 5
// struct of lists, not list of structs - see https://github.com/ilmenit/CC65-Advanced-Optimizations?tab=readme-ov-file
struct sprit {
    unsigned char x[NR_SPRITES];
    unsigned char hisprites;
    unsigned char y[NR_SPRITES];
    signed char dx[NR_SPRITES];
    signed char dy[NR_SPRITES];
    signed char dirx[NR_SPRITES];
};

struct sprit sprits = {
                        {120,100,75,180,60},
                        0,
                        {130,80,180,75,60},
                        {1,2,5,2,1},
                        {2,-3,-1,-2,1},
                        {0,0,0,0,0}
};
// unsigned char i,j,n;



void my_irq(void) {

        //signed char tx,ty;
        VIC.rasterline = 249;
        VIC.ctrl1 |=0x08;
        /* ack raster IRQ */
        VIC.irr = 1;
        VIC.bordercolor=COLOR_BLACK; 
        POKEW(0x0314, (int)&my_irq_2);
      //__asm__(" jmp $ea31");

        // n = VIC.spr_coll;
        // if (n) {
        //   VIC.spr_coll=0xff;
        //   // we have a collision. Check which we are colliding with. 
        //   j=1;
        //   for(i=1;i<=(1<<(NR_SPRITES-1));i=i<<1,j++) {
        //     if (i&n) {
        //       sprits.dx[j]=-sprits.dx[j];
        //       sprits.dy[j]=-sprits.dy[j];

        //     }
        //   }
        // }

        // unrolled movement loop

      movemovecheck(0);
      movemovecheck(1);
      movemovecheck(2);
      movemovecheck(3);
      movemovecheck(4);

      //VIC.spr_hi_x = sprits.hisprites;
  
  VIC.bordercolor=COLOR_BLACK;

  // normal interrupt processing
__asm__(" jmp $ea31");
}
      
// used for border removal 
void my_irq_2(void) {
    VIC.rasterline = 255;
    VIC.ctrl1&=0xf7;
  VIC.bordercolor=COLOR_ORANGE;
  /* ack raster IRQ */
  VIC.irr = 1;
  POKEW(0x0314, (int)&my_irq);


  __asm__(" jmp $ea31");
}

void __fastcall__ irq_setup(void (*irqh)(void)) {
  /* disable IRQ sources, SEI not needed */
  CIA1.icr = 0x7f;
  VIC.imr = 0;

  VIC.rasterline = 255;
  VIC.ctrl1 = 0x1b;

  /* set kernal IRQ vector */
  POKEW(0x0314, (int)irqh);

  /* enable raster and sprite IRQs */
  VIC.imr = 1;
  VIC.spr_coll=255;
}

extern unsigned char mysprites[];
char *smem=(char *)832;

void main(void) {
  irq_setup(&my_irq_2);
  VIC.spr_ena=0x1F;
  // VIC.spr1_y=100;
  // VIC.spr1_x=255;
  // VIC.spr2_y=160;
  // VIC.spr2_x=0;
  // sprits.hisprites=0x04;

    // three identical sprites
  POKE(2040,13);
  POKE(2041,13);   
  POKE(2042,13);    
  POKE(2043,13);    
  POKE(2044,13);    

  VIC.spr0_color = COLOR_CYAN;
  VIC.spr1_color = COLOR_YELLOW;
  VIC.spr2_color = COLOR_WHITE;
  VIC.spr3_color = COLOR_ORANGE;
  VIC.spr4_color = COLOR_PURPLE;
  VIC.spr_hi_x=0;
  for(i=0;i<8;i++){
    VIC.spr_pos[i].x=sprits.x[i];
    VIC.spr_pos[i].y=sprits.y[i];
  }

  // https://www.commodore.ca/manuals/c64_users_guide/c64-users_guide-06-sprite_graphics.pdf


for (n = 0 ; n < 64; n++) {
 		POKE(832+n,mysprites[n]);

    }
   // printf("%u\n",&VIC.spr_coll);

  while (PEEK(0xc6) == 0) {
    ;
  }
}