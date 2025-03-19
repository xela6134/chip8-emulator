#include <iostream>
#include <zconf.h>
#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL_events.h>
#include <SDL.h>
#include "chip8.h"

uint8_t keymap[16] = {
    SDLK_x, SDLK_1, SDLK_2, SDLK_3,
    SDLK_q, SDLK_w, SDLK_e, SDLK_a,
    SDLK_s, SDLK_d, SDLK_z, SDLK_c,
    SDLK_4, SDLK_r, SDLK_f, SDLK_v
};

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        std::cerr << "Path to ROM to be loaded must be given as argument\nType -help to see usage\n";
        exit(1);
    }

    // Loading ROM provided as argument
    Chip8 chip8;
    if (not chip8.load_rom(argv[1])) {
        std::cerr << "ROM could not be loaded. Possibly invalid path given\n";
        exit(1);
    }

    // Set up SDL
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    const int ht = 320, wt = 640;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cerr << "Error in initializing SDL " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    window = SDL_CreateWindow("Chip8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, wt, ht, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Error in creating window " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == nullptr) {
        std::cerr << "Error in initializing rendering " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    SDL_RenderSetLogicalSize(renderer, wt, ht);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if (texture == nullptr) {
        std::cerr << "Error in setting up texture " << SDL_GetError() << std::endl;
        SDL_Quit();
        exit(1);
    }

    // Set initial emulation speed (delay in microseconds)
    unsigned int cycle_delay = 1500;

    while (true) {
        chip8.single_cycle();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0);
            }
            if (event.type == SDL_KEYDOWN) {
                // Allow ESC to quit
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    exit(0);
                }
                // Adjust speed with UP and DOWN arrow keys.
                if (event.key.keysym.sym == SDLK_UP) {
                    if (cycle_delay > 100) { // Avoid negative or too low delay
                        cycle_delay -= 100;
                        std::cout << "Speeding up, delay = " << cycle_delay << " microseconds" << std::endl;
                    }
                }
                if (event.key.keysym.sym == SDLK_DOWN) {
                    cycle_delay += 100;
                    std::cout << "Slowing down, delay = " << cycle_delay << " microseconds" << std::endl;
                }
                // Set Chip8 keypad values for game controls.
                for (int i = 0; i < 16; ++i) {
                    if (event.key.keysym.sym == keymap[i]) {
                        chip8.set_keypad_value(i, 1);
                    }
                }
            }
            if (event.type == SDL_KEYUP) {
                for (int i = 0; i < 16; ++i) {
                    if (event.key.keysym.sym == keymap[i]) {
                        chip8.set_keypad_value(i, 0);
                    }
                }
            }
        }

        if (chip8.get_draw_flag()) {
            chip8.set_draw_flag(false);
            uint32_t pixels[32 * 64];
            for (int i = 0; i < 32 * 64; i++) {
                if (chip8.get_display_value(i) == 0) {
                    pixels[i] = 0xFF000000;
                } else {
                    pixels[i] = 0xFFFFFFFF;
                }
            }
            SDL_UpdateTexture(texture, NULL, pixels, 64 * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        usleep(cycle_delay);
    }

    return 0;
}
