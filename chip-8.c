// Main resource for CHIP-8 architecture and instructions: https://tobiasvl.github.io/blog/write-a-chip-8-emulator/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <SDL2/SDL.h>
#include <time.h>

#include "headers/chip-8.h"
#include "headers/screen.h"

#define DEBUG 0

unsigned char fontset[80] = 
{
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80 // F
};

SDL_Scancode keymappings[16] = {
    SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4,
    SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_R,
    SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_F,
    SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_C, SDL_SCANCODE_V};


int load_rom(CHIP8 *C8, char *filename){
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL){
        printf("Failed to load ROM: File not found\n");
        return 1;
    }
    // get size of rom
    fseek(fp, 0, SEEK_END);
    uint64_t rom_size = ftell(fp);
    rewind(fp);
    printf("ROM Size: %lld B\n", rom_size);
    // load ROM to memory starting at 512th byte
    if (rom_size > (sizeof(C8->memory) - 0x200)){
        printf("Failed to load ROM: ROM is too large\n");
        return 1;
    }
    fread(C8->memory+0x200, 1, sizeof(C8->memory) - 0x200, fp);
    fclose(fp);
    printf("ROM loaded succesfully\n");
    return 0;
}


void chip8_init(CHIP8 * C8, char *rom){
    printf("CPU Initializing\n");
    // set seed
    srand(time(NULL));
    // load memory
    memset(C8->memory, 0, sizeof(C8->memory));
    printf("Loading fontset to memory\n");
    memcpy(C8->memory, fontset, sizeof(fontset));
    printf("Fontset loaded succesfully\n");
    printf("Loading ROM\n");
    int rom_code = load_rom(C8, rom);

    // clear everything
    memset(C8->display, 0, sizeof(C8->display));
    memset(C8->stack, 0, sizeof(C8->stack));
    memset(C8->V, 0, sizeof(C8->V));
    C8->sp = 0;

    draw_flag = 0;

    // ROM is loaded at 0x200 so first instruction is at 0x200
    C8->pc = 0x200;
    C8->opcode = 0;
    if (rom_code == 0){
        printf("CPU Initialized");
    } else {
        printf("CPU failed to initialize\n");
        run = 0;
    }
}

void input(CHIP8 *C8){
    SDL_Event event;

    if (SDL_PollEvent(&event)){
        const Uint8 *state = SDL_GetKeyboardState(NULL);

        if (event.type == SDL_QUIT){
            run = 0;
        }
        else if (state[SDL_SCANCODE_ESCAPE]){
            run = 0;
        } 
        else{
            for (int keycode = 0; keycode < 16; keycode++){
                C8->keypad[keycode] = state[keymappings[keycode]];
            }
        }
    }
}

void execute(CHIP8 *C8){

    draw_flag = 0;
    C8->opcode = C8->memory[C8->pc] << 8 | C8->memory[C8->pc + 1];
    
    //Vx register
    unsigned short x = (C8->opcode & 0x0F00) >> 8;

    //Vy register
    unsigned short y = (C8->opcode & 0x00F0) >> 4;

    if (DEBUG){
        printf("Executing %04X at %04X , I:%02X SP:%02X\n", C8->opcode, C8->pc, C8->I, C8->sp);
    }
    // mask off first nibble
    switch(C8->opcode & 0xF000){

        case 0x0000:
            switch(C8->opcode & 0x00FF){
                // 00E0: Clear screen
                case 0x00E0:
                    memset(C8->display, 0, sizeof(C8->display));
                    C8->pc += 2;
                    break;
                // 00EE: Continue from last address in stack
                case 0x00EE:
                    C8->pc = C8->stack[C8->sp];
                    C8->sp -= 1;
                    C8->pc += 2;
                    break;
            }
            break;

        // 1NNN: Jump to NNN
        case 0x1000:
            C8->pc = C8->opcode & 0x0FFF;
            break;

        // 2NNN: Add current PC to stack and jump to NNN;
        case 0x2000:
            C8->sp += 1;
            C8->stack[C8->sp] = C8->pc;
            C8->pc = C8->opcode & 0xFFF;
            break;

        // 3XNN: Skip if Vx == NN
        case 0x3000:
            if (C8->V[x] == (C8->opcode & 0x00FF)){
                C8->pc += 2;
            }
            C8->pc += 2;
            break;

        // 4XNN: Skip if Vx != NN
        case 0x4000:
            if (C8->V[x] != (C8->opcode & 0x00FF)){
                C8->pc += 2;
            }
            C8->pc += 2;
            break;

        // 5XY0: skip if Vx == Vy
        case 0x5000:
            if (C8->V[x] == C8->V[y]){
                C8->pc += 2;
            }
            C8->pc += 2;
            break;
        
        // 6XNN: set register Vx to NN
        case 0x6000:
            C8->V[x] = (C8->opcode & 0x00FF);
            C8->pc += 2;
            break;

        // 7XNN: add NN to register Vx
        case 0x7000:
            C8->V[x] += (C8->opcode & 0x00FF);
            C8->pc += 2;
            break;


        // Logic and arithmetic
        case 0x8000:
            switch (C8->opcode & 0x000F){
                // 8XY0: Set Vx = Vy
                case 0x0000:
                    C8->V[x] = C8->V[y];
                    C8->pc += 2;
                    break;

                // 8XY1: Set Vx = Vx | Vy
                case 0x0001:
                    C8->V[x] = C8->V[x] | C8->V[y];
                    C8->pc += 2;
                    break;

                // 8XY2: Set Vx = Vx & Vy
                case 0x0002:
                    C8->V[x] = C8->V[x] & C8->V[y];
                    C8->pc += 2;
                    break;

                // 8XY3: Set Vx = Vx ^ Vy
                case 0x0003:
                    C8->V[x] = C8->V[x] ^ C8->V[y];
                    C8->pc += 2;
                    break;

                // 8XY4: Set Vx = Vx + Vy. Vf indicates a carry
                case 0x0004:
                    C8->V[0xF] = ((C8->V[x] + C8->V[y]) > 0xFF) ? 1 : 0;
                    C8->V[x] = C8->V[x] + (C8->V[y]);
                    C8->pc += 2;
                    break;

                // 8XY5: Subtract Vy from Vx. Vf indicates borrow
                case 0x0005:
                    C8->V[0xF] = (C8->V[x] > C8->V[y]) ? 1 : 0;
                    C8->V[x] = C8->V[x] - (C8->V[y]);
                    C8->pc += 2;
                    break;

                // 8XY6: set VF to least significant digit of Vx and shift Vx right 1
                case 0x0006:
                    C8->V[0xF] = C8->V[x] & 0x1;
                    C8->V[x] = C8->V[x] >> 1;
                    C8->pc += 2;
                    break;

                // 8XY7: Subtract Vx from Vy. Vf indicates borrow
                case 0x0007:
                    C8->V[0xF] = (C8->V[y] > C8->V[x]) ? 1 : 0;
                    C8->V[x] = C8->V[y] - (C8->V[x]);
                    C8->pc += 2;
                    break;
                // 8XYE: set VF to most significant digit of Vx and shift Vx left 1
                case 0x000E:
                    C8->V[0xF] = (C8->V[x] >> 7 ) & 0x1;
                    C8->V[x] = C8->V[x] << 1;
                    C8->pc += 2;
                    break;
            }
            break;

        // 9XY0: skip if Vx != Vy
        case 0x9000:
            if (C8->V[x] != C8->V[y]){
                C8->pc += 2;
            }
            C8->pc += 2;
            break;

        // ANNN: set index register to NNN
        case 0xA000:
            C8->I = (C8->opcode & 0x0FFF);
            C8->pc += 2;
            break;

        // BNNN: Jump to NNN + V0
        case 0xB000:
            C8->pc= (C8->opcode & 0x0FFF) + C8->V[0];
            break;

        // CXNN Set Vx to a random num & NN
        case 0xC000:
            C8->V[x] = (rand() & (C8->opcode & 0x00FF));
            C8->pc += 2;
            break;

        // DXYN: draw to display
        case 0xD000:
            draw_flag = 1;
            unsigned short n = (C8->opcode & 0x000F);
            int y_pos = (C8->V[y] & 0x1F);
            int x_pos = (C8->V[x] & 0x3F);
            C8->V[0xf] = 0;
            for (int sprite_row = 0; sprite_row < n; sprite_row++)
            {
                if (y_pos + sprite_row >= 32)
                {
                    break;
                }
                for (int sprite_bit = 0; sprite_bit < 8; sprite_bit++)
                {
                    int screen_y = (y_pos + sprite_row) % 32;
                    int screen_x = (x_pos + sprite_bit) % 64;
                    if (x_pos + sprite_bit >= 64)
                    {
                        break;
                    }
                    uint32_t screen_pixel = C8->display[screen_x + (screen_y * 64)];
                    uint8_t sprite_pixel = ((C8->memory[C8->I + sprite_row]) >> (7 - sprite_bit)) & 1;
                    if (screen_pixel == 1 && sprite_pixel != 0)
                    {
                        C8->display[screen_x + (screen_y * 64)] = 0;
                        C8->V[0xf] = 1;
                    }
                    else if (screen_pixel == 0 && sprite_pixel != 0)
                    {
                        C8->display[screen_x + (screen_y * 64)] = 1;
                    }
                }
            }
            C8->pc += 2;
            break;

        case 0xE000:
            switch (C8->opcode & 0x00FF){
                case 0x009E:
                    if (C8->keypad[C8->V[x]]){
                        C8->pc += 2;
                    }
                    C8->pc += 2;
                    break;

                case 0x00A1:
                    if (!(C8->keypad[C8->V[x]])){
                        C8->pc += 2;
                    }
                    C8->pc += 2;
                    break;
            }


        case 0xF000:
            switch (C8->opcode & 0x00FF){
                
                // FX07: Sets Vx to value of delay timer
                case 0x0007:
                    C8->V[x] = C8->delay_timer;
                    C8->pc += 2;
                    break;
                
                // FX15: Sets delay timer to value of Vx
                case 0x0015:
                    C8->delay_timer = C8->V[x];
                    C8->pc += 2;
                    break;
                
                // FX18: Sets sound timer to value of Vx
                case 0x0018:
                    C8->sound_timer = C8->V[x];
                    C8->pc += 2;
                    break;

                // FX1E: Add Vx to register I
                case 0x001E:
                    C8->I += C8->V[x];
                    C8->pc += 2;
                    break;
                
                // FX0A: Blocks going to next instruction until a key is pressed, then stores that key in Vx
                case 0x000A:
                    for (int i = 0; i < 16; i++){
                        if (C8->keypad[i]){
                            C8->V[x] = C8->keypad[i];
                            C8->pc += 2;
                            break;
                        }
                    }
                    break;

                // FX29: Sets register I to character in Vx. Multiply by 5 because digits are 4x5
                case 0x0029:
                    C8->I = C8->V[x] * 5;
                    C8->pc += 2;
                    break;

                // FX33: Convert Vx to 3 decimal degits and stores them in memory at address I to I + 2
                case 0x0033:
                    C8->memory[C8->I] = (C8->V[x] % 1000) / 100;
                    C8->memory[C8->I + 1] = (C8->V[x] % 100) / 10;
                    C8->memory[C8->I + 2] = (C8->V[x] % 10);
                    C8->pc += 2;
                    break;

                // FX55: Load V0 - Vx into system memory starting at address in register I
                case 0x0055:
                    for (int i=0; i <= x; i++){
                        C8->memory[C8->I + i] = C8->V[i];
                    }
                    C8->pc += 2;
                    break;

                // FX65: Load system memory into registers V0 - Vx starting at address in register I
                case 0x0065:
                    for (int i=0; i <= x; i++){
                        C8->V[i]= C8->memory[C8->I + i];
                    }
                    C8->pc += 2;
                    break;
            }
    }
}
