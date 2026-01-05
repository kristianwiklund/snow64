#include <c64.h>
#include <peekpoke.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

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
#define position_sprite_m(nr) {\
  VIC.spr_pos[nr].y = sprits.y[nr];\
  VIC.spr_pos[nr].x = sprits.x[nr];\
}

// lookup table to avoid shifts
const unsigned  char spritebit[] = {0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80};

const unsigned  char hexor[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

unsigned char i,j,n;

// "locals" for addition routine
unsigned char azc;
unsigned int aza;

 
#define NR_SPRITES 5
// struct of lists, not list of structs - see https://github.com/ilmenit/CC65-Advanced-Optimizations?tab=readme-ov-file
struct sprit {
    unsigned char x[NR_SPRITES];
    unsigned char hisprites;
    unsigned char y[NR_SPRITES];
    signed char dx[NR_SPRITES];
    signed char dy[NR_SPRITES];
};

struct sprit sprits = {
                        {120,100,75,180,60},
                        0,
                        {130,80,180,75,60},
                        {1,2,5,2,1},
                        {0,-3,-1,-2,1}
};
// unsigned char i,j,n;

struct _vic2 *V=(void *)0xD000;

#define abouncesprite(sprite)

#define bouncesprite(sprite){\
  if (!(VIC.spr_hi_x&spritebit[sprite])) {if (VIC.spr_pos[sprite].x <= BORDER_LEFT) { sprits.dx[sprite]=-sprits.dx[sprite]; VIC.spr_pos[sprite].x=BORDER_LEFT+1;}}\
  else{\
   if (VIC.spr_pos[sprite].x >= (BORDER_RIGHT-SPRITE_WIDTH-255)) { sprits.dx[sprite]=-sprits.dx[sprite]; VIC.spr_pos[sprite].x=(BORDER_RIGHT-SPRITE_WIDTH-1-255);}\
  }}
  
// not interesting with the borders gone. 
//   {
//   if (VIC.spr_pos[sprite].y <= BORDER_TOP){\
//         sprits.dy[sprite]=-sprits.dy[sprite];\
//         VIC.spr_pos[sprite].y=BORDER_TOP+1;\
//   }\
//   else if (VIC.spr_pos[sprite].y >= (BORDER_BOTTOM-SPRITE_HEIGHT)){\
//      sprits.dy[sprite]=-sprits.dy[sprite];VIC.spr_pos[sprite].y=(BORDER_BOTTOM-SPRITE_HEIGHT-1);}\
// }

//       addzor(sprite,&VIC.spr_pos[sprite].x,&sprits.hisprites,sprits.dx[sprite]);\

#define addzor(sprite) {\
 __asm__("clc");\
 __asm__("lda %w",((const int)&VIC+offsetof(struct __vic2, spr_pos[sprite].x)));\
 __asm__("ldy #%b", offsetof(struct sprit, dx[sprite]));\
 __asm__("adc %v,y",sprits);\
 __asm__("sta %w",((const int)&VIC+offsetof(struct __vic2, spr_pos[sprite].x)));\
 __asm__("bcc %g",_label##sprite##);\
 __asm__("lda %w", (const int)&VIC.spr_hi_x);\
 __asm__("eor #%b",1<<sprite);\
 __asm__("sta %w", (const int)&VIC.spr_hi_x);\
_label##sprite##:\
 __asm__("nop");\
}

    // clc                   ; $08F5 18      
    // ldy #$02              ; $08F6 A0 02   
    // lda $d000,Y           ; $08F8 B9 00D0 
    // ldy #$0c              ; $08FB A0 0C   
    // adc $0c3a,Y           ; $08FD 79 3A0C 
    // ldy #$02              ; $0900 A0 02   
    // sta $d000,Y           ; $0902 99 00D0 
    // bcc label5            ; $0905 90 08           bcc $090f
    // lda $d010             ; $0907 AD 10D0         load VIC sprite x bit 8
    // eor #$02              ; $090A 49 02   
    // sta $d010             ; $090C 8D 10D0         store VIC sprite x bit 8



 #define baddzor(sprite) {\
  aza = (unsigned int)(VIC.spr_pos[sprite].x)+sprits.dx[sprite];\
  (VIC.spr_pos[sprite].x)=(unsigned char)(aza);\
  azc = (aza&0x100)>>1;\
  if (azc) {\
    (VIC.spr_hi_x) ^= spritebit[sprite];\
  }\
 }

 #define pokezor(sprite){\
  POKE(1024,hexor[VIC.spr_hi_x&0x1]);\
  POKE(1025,hexor[VIC.spr0_x>>4]);\
  POKE(1026,hexor[VIC.spr0_x&0xf]);\
 }

#define movemovecheck(sprite) {\
      VIC.bordercolor=sprite;\
      addzor(sprite);\
      VIC.spr_pos[sprite].y+=sprits.dy[sprite];\
      if (!sprite) pokezor(sprite);\
      bouncesprite(sprite);\
}

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
      // movemovecheck(1);
      // movemovecheck(2);
      // movemovecheck(3);
      // movemovecheck(4);

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
    VIC.spr_pos[i].x=0;
    VIC.spr_pos[i].y=0;
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