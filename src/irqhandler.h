#ifndef _IRQHANDLER_H
#define _IRQHANDLER_H

// irq_wait (see slides from trident / fjälldata 2025), targeted to normal kernel irqs
#define kernel_irq_wait(tralla)\
    VIC.irr = 1;\
    __asm__("lda #<%g",iwnext##tralla##);\
    __asm__("sta $0314");\
    __asm__("lda #>%g",iwnext##tralla##);\
    __asm__("sta $0315");\
    VIC.rasterline=tralla;\
    __asm__("jmp $ea31");\
iwnext##tralla##:

#define kernel_irq_set(handler, tralla)\
    VIC.irr = 1;\
    POKEW(0x0314, (int)&handler);\
    VIC.rasterline=tralla; 

// irq_wait (see slides from trident / fjälldata 2025), targeted to normal kernel irqs
#define irq_wait(tralla)\
    __asm__("lda #<%g",iwnext##tralla##);\
    __asm__("sta $fffe");\
    __asm__("lda #>%g",iwnext##tralla##);\
    __asm__("sta $ffff");\
    VIC.rasterline=tralla;\
    irq_restore();\
iwnext##tralla##:\
    irq_save();

#define irq_set(handler, tralla)\
    VIC.irr = 1;\
    POKEW(0xfffe, (int)&handler);\
    VIC.rasterline=tralla; 

#define irq_save()\
    __asm__("pha");\
	__asm__("txa");\
	__asm__("pha");\
	__asm__("tya");\
	__asm__("pha");\
	__asm__("lda $01");\
	__asm__("pha");\
	__asm__("lda #$35");\
	__asm__("sta $01");

#define irq_restore()\
    __asm__("asl $d019");\
	__asm__("pla");\
	__asm__("sta $01");\
	__asm__("pla");\
	__asm__("tay");\
	__asm__("pla");\
	__asm__("tax");\
	__asm__("pla");\
	__asm__("rti")

#endif