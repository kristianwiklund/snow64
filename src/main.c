#include <c64.h>
#include <peekpoke.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "spritemovement.h"
#include "irqhandler.h"

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

#define NR_SPRITES 8
// struct of lists, not list of structs - see https://github.com/ilmenit/CC65-Advanced-Optimizations?tab=readme-ov-file
struct sprit {
    unsigned char dirx[NR_SPRITES];
    signed char dx[NR_SPRITES];
    unsigned char x[NR_SPRITES];
    unsigned char hisprites;
    unsigned char y[NR_SPRITES]; 
    signed char dy[NR_SPRITES];
    unsigned char anim[NR_SPRITES];
};

struct sprit sprits = {
                        {0,0,0,0,0,0,0,0},
                        {1,2,1,2,1,2,1,1},  
                        {120,100,75,180,60,50,200,150},
                        0,
                        {130,80,180,75,60,50,60,70},
                        {2,3,1,2,1,1,1,2},
                        {0,1,2,0,1,2,0,1}
};
  
// set up bitmap

extern unsigned char cliffs_bitmap[];
extern unsigned char cliffs_colors[];
extern unsigned char *_BITMAP_LOAD__,*_BITMAP_RUN__,*_COLORS_LOAD__,*_COLORS_RUN__;
extern unsigned int _BITMAP_SIZE__, _COLORS_SIZE__;



void show_image() {   
  unsigned int i;
  // https://www.c64-wiki.com/wiki/Standard_Bitmap_Mode

  #define P_SCREEN ((unsigned char *)0x2000)
  #define P_COLOR  ((unsigned char *)0x0400)

  //printf("sb=%x, ss=%x, cb=%x, cc=%x\n", cliffs_bitmap, P_SCREEN, cliffs_colors, P_COLOR);

  memcpy(P_COLOR, cliffs_colors, 0x400);
  //memset(P_COLOR,0x11,0x400);
  VIC.ctrl1 = 0b00111000;
  VIC.addr  = 0b00011000;


  // set the ghostbyte
  POKE(0x3fff,0x0);
  VIC.bgcolor0 = COLOR_BLACK;
  VIC.bordercolor = COLOR_BLACK;
}

void my_irq(void) {

  //signed char tx,ty;
  // VIC.rasterline = 249;
  VIC.ctrl1 = 0b00111000;


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

// here be known preprocessor warnings about pasting
      movemovecheck(0);
      movemovecheck(1);
      movemovecheck(2);
      movemovecheck(3);
      movemovecheck(4);
      movemovecheck(5);
      movemovecheck(6);
      movemovecheck(7);

// end preprocessor warnings
      //VIC.spr_hi_x = sprits.hisprites;
  

  // normal interrupt processing
//__asm__(" jmp $ea31");
}
      
// used for border removal 
void my_irq_2(void) {
  VIC.ctrl1 = 0b00110000;
}


extern unsigned char snowflake[];
#define smem 832;

void setup_sprites() {

  VIC.spr_ena=0x1F;
  // VIC.spr1_y=100;
  // VIC.spr1_x=255;
  // VIC.spr2_y=160;
  // VIC.spr2_x=0;
  // sprits.hisprites=0x04;

  for (i=0;i<8;i++) {
    *(unsigned char *)(2040+i) = 13+sprits.anim[i];
    VIC.spr_color[i] = COLOR_WHITE;
  } 
  VIC.spr_hi_x=0;
  for(i=0;i<8;i++){
    VIC.spr_pos[i].x=sprits.x[i];
    VIC.spr_pos[i].y=sprits.y[i];
  }

  // https://www.commodore.ca/manuals/c64_users_guide/c64-users_guide-06-sprite_graphics.pdf


for (n = 0 ; n < 64+64+64; n++) {
 		POKE(832+n,snowflake[n]);

    }
   // printf("%u\n",&VIC.spr_coll);

}
void testirq() {
  VIC.bordercolor=COLOR_BLACK;
}

void testirq2() {
  VIC.bordercolor=COLOR_BLUE;
}

void __fastcall__ irqhandler() {  

  irq_save();
  // set ghostbyte to transparent and put sprites in top of graphics. 
  POKE(0x3FFF,0x0);
  VIC.spr_bg_prio=0x0;


  // irq_wait(48);
  // // turn on graphics mode
  // VIC.ctrl1=0b00111000;

  irq_wait(247);
  // set ghostbyte to solid black and put sprites behind graphics
  POKE(0x3fff,0xff);
  VIC.spr_bg_prio=0xFF;


  irq_wait(249);
  my_irq_2();

  irq_wait(252);
  my_irq();

  irq_set(irqhandler, 0);
  irq_restore();

}
void main(void) {

  show_image();
  setup_sprites();


  CIA1.icr = 0x7f;
  VIC.imr = 0;
  VIC.ctrl1 = 0x1b;
  /* enable raster  IRQs */
  // disable KERNEL and BASIC roms, keep I/O enabled
  *((unsigned char *)1) = 0x35;
  irq_set(irqhandler,0); 
  VIC.imr = 1;


  loop:
    goto loop;

}