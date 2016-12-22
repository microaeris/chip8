#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "chip8.h"

int init_sdl(SDL_Window **window, SDL_Surface **surface) 
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return ERROR_SDL_INIT;
    }

    *window = SDL_CreateWindow("Chip8", 
                               SDL_WINDOWPOS_UNDEFINED, 
                               SDL_WINDOWPOS_UNDEFINED, 
                               SCREEN_WIDTH, 
                               SCREEN_HEIGHT, 
                               SDL_WINDOW_SHOWN);

    if (*window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return ERROR_SDL_INIT;
    }

    *surface = SDL_GetWindowSurface(*window);
    return ERROR_NONE;
}

void init_chip8() 
{
    pc      = 0x200;  
    opcode  = 0;
    I       = 0;
    sp      = 0;      

    // Clear display  
    // Clear stack
    // Clear registers V0-VF
    // Clear memory
    // Reset timers

    // Load fontset
    memcpy(mem + 0x050, chip8_fontset, sizeof(chip8_fontset));
}

void emulate_cycle()
{
    // Fetch Opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    // Decode Opcode

    // Execute Opcode

    // Update timers
    if (delay_timer > 0) {
        --delay_timer;
    }

    if (sound_timer > 0) {
        if(sound_timer == 1) {
            printf("BEEP!\n");
        }
        --sound_timer;
    } 
}

void load_rom()
{
    // Copy rom from file to mem array
    FILE *rom_file = fopen("./roms/MAZE", "r");
    fseek(rom_file, 0, SEEK_END); // seek to end of file
    size_t size = ftell(rom_file); // get current file pointer
    fseek(rom_file, 0, SEEK_SET); // seek back to beginning of file
    size_t read_size = fread(mem + 0x200, 1, size, rom_file);

    if (size != read_size) {
        printf("Did not read the ROM file correctly\n");
        exit(0);
    }

    // printf("size: %zu\n", size);
    // printf("read_size: %zu\n", read_size);
    // for (int i = 0; i < size; i++) {
    //     printf("%x ", mem[0x200+i]);
    // }
    // printf("\n");
}

void setpixel(SDL_Surface *surface, int x, int y, uint8_t r, uint8_t g, uint8_t b) 
{
    *((uint32_t*)surface->pixels + (y*surface->pitch/4) + x) = SDL_MapRGB(surface->format, r, g, b);
}

void render_screen(SDL_Window *window, SDL_Surface *surface)
{
    // render
    SDL_LockSurface(surface);
    for(int y=0; y<surface->h; y++) {
        for(int x=0; x<surface->w; x++) {
            setpixel(surface, x, y, 
                     (x*x)/256+3*y+tick, 
                     (y*y)/256+x+tick, 
                     tick);
        }
    }
    SDL_UnlockSurface(surface);
    SDL_UpdateWindowSurface(window);
}

int main(int argc, char *args[])
{
    SDL_Window *window = NULL;
    SDL_Surface *surface = NULL;
    SDL_Event e;

    int res = 0;
    if ((res = init_sdl(&window, &surface)) != ERROR_NONE) {
        return res;
    }

    // TODO: Setup keypad 
    init_chip8();
    load_rom();

    while(!done) {
        emulate_cycle();
        render_screen(window, surface);

        // // If the draw flag is set, update the screen
        // if(myChip8.drawFlag)
        //   drawGraphics();
     
        // // Store key press state (Press and Release)
        // myChip8.setKeys();  
        tick++;

        // check for key/quit
        while(SDL_PollEvent(&e)) {
            done |= (e.type == SDL_QUIT || e.type == SDL_KEYDOWN);
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return ERROR_NONE;
}
