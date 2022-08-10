// Most of the screen code and frame limiting code was taken from Github with minor changes

#include "headers/screen.h"

#include <SDL2/SDL.h>
#include <stdio.h>

#define FG_COLOR 0xFF1976D2
#define BG_COLOR 0xFF212121

SDL_Window *screen;

SDL_Renderer *renderer;



void init_display(){
    SDL_InitSubSystem(SDL_INIT_EVERYTHING);
    screen = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 64*8, 32*8, 0);
    renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_PRESENTVSYNC);
 
}

void draw(unsigned char *display){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int y = 0; y < 32; y++){
        for (int x = 0; x < 64; x++){ 
            if (display[x +(y * 64)]) {
                SDL_Rect rect;
                rect.x = x * 8;
                rect.y = y * 8;
                rect.w = 8;
                rect.h = 8;
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
    SDL_RenderPresent(renderer);
}