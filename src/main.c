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

extern unsigned char snowflake[];

#define VIC_BANK_START 0x8000
#define SPRITEPTR (((unsigned int)snowflake)/64)
#define SPRITE_POINTERS (VIC_BANK_START+2040)
#define P_SCREEN ((unsigned char *)0x2000)
#define P_COLOR  ((unsigned char *)0x0400)

void show_image() {   
 
  // //memset(P_COLOR,0x11,0x400);
  VIC.ctrl1 = 0b00111000;
  // // set up screen memory
  VIC.addr  = 0b00011000;
  // // point at 0x8000
  CIA2.pra &=0xfc;
  CIA2.pra |=0x1;

  // set the ghostbyte
  //POKE(0x3fff,0x0);
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


}
      
// used for border removal 
void my_irq_2(void) {
  VIC.ctrl1 = 0b00110000;

}

unsigned char animtimeout=0;

#define ROT_SPRITE(sprite)\
    VIC.bordercolor=COLOR_BLUE;\
      sprits.anim[sprite]=(sprits.anim[sprite]+1)&3;\
      *(unsigned char *)(SPRITE_POINTERS+sprite) = SPRITEPTR+sprits.anim[sprite];

void rot_anim() {
  if (++animtimeout&0x40) {
    animtimeout=0;
    ROT_SPRITE(0);
    ROT_SPRITE(1);
    ROT_SPRITE(2);
    ROT_SPRITE(3);
    ROT_SPRITE(4);
    ROT_SPRITE(5);
    ROT_SPRITE(6);
    ROT_SPRITE(7);
    VIC.bordercolor=COLOR_BLACK;
  } 
}

void setup_sprites() {

  VIC.spr_ena=0x1F;

  for (i=0;i<8;i++) {
    *(unsigned char *)(SPRITE_POINTERS+i) = SPRITEPTR+sprits.anim[i];
    VIC.spr_color[i] = COLOR_GRAY3;
  } 
  VIC.spr_hi_x=0;
  for(i=0;i<8;i++){
    VIC.spr_pos[i].x=sprits.x[i];
    VIC.spr_pos[i].y=sprits.y[i];
  }

}

void testirq() {
  VIC.bordercolor=COLOR_BLACK;
}

void testirq2() {
  VIC.bordercolor=COLOR_BLUE;
}

void ptloop();

void  *pt=(void *)ptloop;

// completely b0rks if optimizer is running
#pragma optimize (push,off)
void ptrunner() { 
  __asm__("jmp (%v)",pt);
}

void ptloop() {

}
#pragma optimize (pop)

void __fastcall__ irqhandler() {  

  irq_save();
  // set ghostbyte to transparent and put sprites in top of graphics. 
  POKE(VIC_BANK_START+0x3FFF,0x0);
  VIC.spr_bg_prio=0x0;


  ptrunner();
  rot_anim();


  // irq_wait(48);
  // // turn on graphics mode
  // VIC.ctrl1=0b00111000;

  irq_wait(247);
  // set ghostbyte to solid black and put sprites behind graphics
  POKE(VIC_BANK_START+0x3FFF,0xff);
  VIC.spr_bg_prio=0xFF;


  irq_wait(249);
  my_irq_2();

  irq_wait(252);
  my_irq();

  irq_set(irqhandler, 0);
  irq_restore();

}

extern unsigned char lowres_image[];

void idle50() {
  unsigned int a;
  for(a=0;a<50;a++);
}

void show_lowres_image() {
  unsigned int n,k;
// start with the lowres image

  for(n=0;n<1000;n++) {
    *(unsigned char *)(0x400+n) = 0;
    *(unsigned char *)(0xD800+n) = COLOR_WHITE;
  }

  
  VIC.bgcolor0=COLOR_BLACK;
  VIC.bordercolor=COLOR_BLACK;

  k=0;
  for(n=0;n<1000;n++) {
    *(unsigned char *)(0x400+n) = lowres_image[k];
    ++k;
    *(unsigned char *)(0xD800+n) = lowres_image[k];
    ++k;
    idle50();
  }

}

void shake_screen() {
  unsigned int i,j;

  //for(j=0;j<20;j++)
   for (i=0;i<150;i++) {
    VIC.ctrl2=(VIC.ctrl2&0xF8)|(i&0x7);
    VIC.ctrl1=(VIC.ctrl1&0xF8)|(i&0x7);
    idle50();
    }

}

// moves the screen downwards, exposing the hires image gradually, before we move to the snowy landscape
void slide_into_hires() {
  static int tehline=51;
  irq_save();
  
  // go to hires
  show_image();
  VIC.bordercolor=COLOR_BLACK;
  VIC.spr_ena=0xFF;
  my_irq();

  // go to lowres
  irq_wait(tehline);
  VIC.spr_ena=0x0;

  // restore lowres settings
  VIC.ctrl1 = 0x1c;
  VIC.addr  = 0b00010101;
  CIA2.pra = 0x97;

  if (tehline>253) {
    irq_set(irqhandler,0);
    show_image();
    VIC.spr_ena=0xFF;
    irq_restore();
  }

  tehline+=2;
  irq_set(slide_into_hires, 0);
  irq_restore();
}

void main(void) {
  unsigned char a,b;

  POKE(198,0);
  while(!PEEK(198));


  printf("%x %x %x ",CIA2.pra, VIC.ctrl1, VIC.addr);

  show_lowres_image();
  a=VIC.ctrl1;
  b=VIC.ctrl2;
  shake_screen();
  setup_sprites();

  CIA1.icr = 0x7f; // turn off cia interrupts
  VIC.imr = 0;     // turn off vic interrupts
  VIC.ctrl1 = a;  
  VIC.ctrl2 = b;

  /* enable raster  IRQs */
  // disable KERNEL and BASIC roms, keep I/O enabled
  *((unsigned char *)1) = 0x35; // 0b00111001

  irq_set(slide_into_hires,0); 
  VIC.imr = 1;  // enable vic interrupts

  //show_image();
  //setup_sprites();

  loop:
    goto loop;

}