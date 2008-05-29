#ifndef BIOS_H
#define BIOS_H

#define BDA_SEG 0x0040
#define BDA_FBMS 0x0013
#define BDA_NUM_DRIVES 0x0075

extern unsigned long currticks ( void );
extern void cpu_nap ( void );

#endif /* BIOS_H */