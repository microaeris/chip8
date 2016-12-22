// sudo apt-get install libsdl2-2.0
// sudo apt-get install libsdl2-dev
// gcc -o chip8 -O2 -Wall -std=c11 -g chip8.c `sdl2-config --cflags --libs`

//✦ sdl2-config --cflags --libs
//-I/usr/include/SDL2 -D_REENTRANT
//-L/usr/lib/x86_64-linux-gnu -lSDL2

// chip8.c then -I(lib name) -L(path)




//Using SDL and standard IO
#include <SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

//Screen dimension constants
const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;

int tick=0, done=0;

// Chip 8
uint16_t opcode;
uint8_t mem[4096];
uint8_t V[16];
uint16_t I; // Index register
uint16_t pc; 
uint8_t gfx[64*32];
uint8_t delay_timer;
uint8_t sound_timer;
uint16_t stack[16];
uint16_t sp;
uint8_t key[16];

typedef enum {
    ERROR_NONE,
    ERROR_SDL_INIT,
} ERROR_T;

int init_sdl(SDL_Window **window, SDL_Surface **surface) 
{
    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return ERROR_SDL_INIT;
    }

    //Create window
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

    //Get window surface
    *surface = SDL_GetWindowSurface(*window);
    return ERROR_NONE;
}

void init_chip8() 
{
    
}

void emulate_cycle ()
{

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
        printf("Did not read ROM correctly\n");
        exit(0);
    }

    // // Check that I read the correct thing
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

    // setupInput();
    // myChip8.initialize();
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