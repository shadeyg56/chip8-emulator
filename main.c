/*
MIT License

Copyright (c) 2022 shadeyg56

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "headers/chip-8.h"
#include "headers/screen.h"
#include <stdio.h>

#include <SDL2/SDL.h>

// const float target_framerate = 1.0f/60.0f;

int main(int argc, char *argv[]){
    run = 1;
    if (argc < 2){
        printf("Run the program: main.exe path/to/rom");
        return 1;
    }
    CHIP8 C8;
    chip8_init(&C8, argv[1]);

    init_display();

    float dt = 0.0f;
    uint64_t start_time = SDL_GetPerformanceCounter();

    while(run){
        SDL_Delay(2);
        uint64_t end_time = SDL_GetPerformanceCounter();
        float diff = ((end_time-start_time) / (float)(SDL_GetPerformanceFrequency()));
        float delta = diff * 1000.0f;
        dt += delta;
        if (dt > 16.666f){
            dt -= 16.666f;
            if (C8.delay_timer > 0){
                C8.delay_timer--;
            }
            if (C8.sound_timer > 0){
                C8.sound_timer--;
            }
        }
        start_time = SDL_GetPerformanceCounter();
        execute(&C8);
        input(&C8);
        if (draw_flag){
            unsigned char *display = C8.display;
            draw(display);
        }
    }
    return 0;
}