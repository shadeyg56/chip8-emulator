#ifndef _CHIP8
#define _CHIP8

unsigned char draw_flag;
int run;

typedef struct CHIP8{
    unsigned short opcode;
    unsigned char memory[4096]; // 4 KB of mem
    unsigned short I;
    unsigned short pc;
    unsigned char V[16];
    unsigned char display[64 * 32];
    unsigned short stack[16];
    unsigned char sp;
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned char keypad[16];
} CHIP8;

int load_rom(CHIP8 *, char *);
void chip8_init(CHIP8 *, char *);
void input(CHIP8 *);
void execute(CHIP8 *);

#endif