#ifndef _SPRITEMOVEMENT_H
#define _SPRITEMOVEMENT_H

#define SPRITE_HEIGHT 21
#define SPRITE_WIDTH 24
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

#define BORDER_LEFT 24
#define BORDER_RIGHT (SCREEN_WIDTH+BORDER_LEFT)
#define BORDER_TOP 51
#define BORDER_BOTTOM (SCREEN_HEIGHT+BORDER_TOP)

// ---------------------------------------------------------

// macro defined place sprite on screen. Uses about half a raster line. Avoids the function call overhead. 
#define position_sprite_m(nr) {\
  VIC.spr_pos[nr].y = sprits.y[nr];\
  VIC.spr_pos[nr].x = sprits.x[nr];\
}

// lookup table to avoid shifts
const unsigned  char shiftleft[] = {0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80};

#define bouncesprite(sprite){\
  if (!(VIC.spr_hi_x&shiftleft[sprite])) {if (VIC.spr_pos[sprite].x <= BORDER_LEFT) { sprits.dirx[sprite]=!sprits.dirx[sprite]; VIC.spr_pos[sprite].x=BORDER_LEFT+2;}}\
  else{\
   if (VIC.spr_pos[sprite].x >= (BORDER_RIGHT-SPRITE_WIDTH-255)) { sprits.dirx[sprite]=!sprits.dirx[sprite]; VIC.spr_pos[sprite].x=(BORDER_RIGHT-SPRITE_WIDTH-2-255);}\
  }\
}

// not interesting with the borders gone. 
//   {
//   if (VIC.spr_pos[sprite].y <= BORDER_TOP){\
//         sprits.dy[sprite]=-sprits.dy[sprite];\
//         VIC.spr_pos[sprite].y=BORDER_TOP+1;\
//   }\
//   else if (VIC.spr_pos[sprite].y >= (BORDER_BOTTOM-SPRITE_HEIGHT)){\
//      sprits.dy[sprite]=-sprits.dy[sprite];VIC.spr_pos[sprite].y=(BORDER_BOTTOM-SPRITE_HEIGHT-1);}\
// }

// add a delta to a sprite. the delta need to be defined in a struct (struct sprit) containing at least one list "dx"
// directly manipulates the vic registers - sprite positions are not stored elsewhere

#define addzor(sprite) {\
 __asm__("clc");\
 __asm__("lda %w",((const int)&VIC+offsetof(struct __vic2, spr_pos[sprite].x)));\
 __asm__("ldy #%b", offsetof(struct sprit, dx[sprite]));\
 __asm__("adc %v,y",sprits);\
 __asm__("sta %w",((const int)&VIC+offsetof(struct __vic2, spr_pos[sprite].x)));\
 __asm__("bcc %g",_labela##sprite##);\
 __asm__("lda %w", (const int)&VIC.spr_hi_x);\
 __asm__("eor #%b",(unsigned char)(1<<sprite));\
 __asm__("sta %w", (const int)&VIC.spr_hi_x);\
_labela##sprite##:\
 __asm__("nop");\
}

#define subzor(sprite) {\
 __asm__("sec");\
 __asm__("lda %w",((const int)&VIC+offsetof(struct __vic2, spr_pos[sprite].x)));\
 __asm__("ldy #%b", offsetof(struct sprit, dx[sprite]));\
 __asm__("sbc %v,y",sprits);\
 __asm__("sta %w",((const int)&VIC+offsetof(struct __vic2, spr_pos[sprite].x)));\
 __asm__("bcs %g",_labels##sprite##);\
 __asm__("lda %w", (const int)&VIC.spr_hi_x);\
 __asm__("eor #%b",(unsigned char)(1<<sprite));\
 __asm__("sta %w", (const int)&VIC.spr_hi_x);\
_labels##sprite##:\
 __asm__("nop");\
}

#define movezor(sprite) {\
  if(sprits.dirx[sprite])\
    {subzor(sprite);}\
  else\
    {addzor(sprite);}\
}


// does the actual move of the sprites
#define movemovecheck(sprite) {\
      movezor(sprite);\
      VIC.spr_pos[sprite].y+=sprits.dy[sprite];\
      bouncesprite(sprite);\
}
#endif
