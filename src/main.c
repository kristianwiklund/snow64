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
#define position_sprite_m(nr) {\
  VIC.spr_pos[nr].y = sprits.y[nr];\
  VIC.spr_pos[nr].x = sprits.x[nr];\

// lookup table to avoid shifts
unsigned char spritebit[] = {0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80};
unsigned char hexor[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
// adds "of" to a sprite position
// pseudocode - see C implementation

inline void __fastcall__  addzor(unsigned char sprite, unsigned char *low, unsigned char *high, signed char of) {  
  unsigned int a;
  unsigned char c;
  unsigned char mask;
  unsigned char ol=*low;

  // low bit
  a = (unsigned int)(*low);
  a+=of;
  *low=(a&0xff);
  c = (a&0x100)>>1;
  // flip the high bit if the "carry" is set
  mask = spritebit[sprite];
  if (c) {
    *high ^= mask;
    // if (ol==0) {
    //     printf("CM=%x S=%x L=%x H=%x OL=%x\n",mask,sprite,*low,*high, ol);  
    //     //POKEW(0x0314, 0xEA31);
    // }
  }

 // POKE(1025,hexor[*low>>4]);
 // POKE(1026,hexor[*low&0xf]);
 // POKE(1024,(*high&mask)?'1':'0');
}



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
                        {1,2,5,-2,1},
                        {2,-3,-1,-2,1}
                    };
// unsigned char i,j,n;

// struct sprit sprits = {
//                         {120}, // x
//                         0, // hix
//                         {130}, // y
//                         {1}, // dx
//                         {0} // dy
//                     };
unsigned char i,j,n;

void my_irq(void) {

    unsigned char irr = VIC.irr;

    // raster interrupt
    if (irr&1) {
        //signed char tx,ty;
        VIC.bordercolor=COLOR_YELLOW;
        // VIC.rasterline = 255;
        // VIC.ctrl1&=0xf7;

        /* ack raster IRQ */
        VIC.irr = 1;
        //  POKEW(0x0314, (int)&my_irq_2);

        // n = VIC.spr_coll;
        // if (n) {
        //   VIC.spr_coll=0xff;
        //   // we have a collision. Check which we are colliding with. 
        //   j=1;
        //   for(i=1;i<=(1<<(NR_SPRITES-1));i=i<<1,j++) {
        //     if (i&n) {
        //       sprits.dx[j]=-sprits.dx[j];
        //       sprits.dy[j]=-sprits.dy[j];
 
            }
          }
        }

        for (i=0;i<NR_SPRITES;i++) { 
            VIC.bordercolor=COLOR_GREEN;
            // slow additions. 
            addzor(i,sprits.x+i,&sprits.hisprites,sprits.dx[i]);
            //sprits.x[i]+=sprits.dx[i];
            sprits.y[i]+=sprits.dy[i];
            VIC.bordercolor=COLOR_LIGHTBLUE;

            if (!(sprits.hisprites&spritebit[i])) {
              if (sprits.x[i] <= BORDER_LEFT) { sprits.dx[i]=-sprits.dx[i]; sprits.x[i]=BORDER_LEFT+1;}
            }
            else 
              if (sprits.x[i] >= (BORDER_RIGHT-SPRITE_WIDTH-255)) { sprits.dx[i]=-sprits.dx[i]; sprits.x[i]=(BORDER_RIGHT-SPRITE_WIDTH-1-255);}  

            if (sprits.y[i] <= BORDER_TOP) { sprits.dy[i]=-sprits.dy[i]; sprits.y[i]=BORDER_TOP+1;}
            else if (sprits.y[i] >= (BORDER_BOTTOM-SPRITE_HEIGHT)) {sprits.dy[i]=-sprits.dy[i];sprits.y[i]=(BORDER_BOTTOM-SPRITE_HEIGHT-1);}
    
            position_sprite_m(i);
        }
        VIC.spr_hi_x = sprits.hisprites;}

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

extern unsigned char mysprites[];
char *smem=(char *)832;

void main(void) {
  irq_setup(&my_irq);
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

  // https://www.commodore.ca/manuals/c64_users_guide/c64-users_guide-06-sprite_graphics.pdf


for (n = 0 ; n < 64; n++) {
 		POKE(832+n,mysprites[n]);

    }
   // printf("%u\n",&VIC.spr_coll);

  while (PEEK(0xc6) == 0) {
    ;
  }
}