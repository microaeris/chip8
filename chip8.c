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

//Screen dimension constants
const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;

int tick=0, done=0;

void setpixel(SDL_Surface *surface, int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    *((uint32_t*)surface->pixels + (y*surface->pitch/4) + x) = SDL_MapRGB(surface->format, r, g, b);
}

int main(int argc, char *args[])
{
    SDL_Window *window = NULL;
    SDL_Surface *surface = NULL;
    SDL_Event e;

    //Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    //Create window
    window = SDL_CreateWindow("Chip8", 
                              SDL_WINDOWPOS_UNDEFINED, 
                              SDL_WINDOWPOS_UNDEFINED, 
                              SCREEN_WIDTH, 
                              SCREEN_HEIGHT, 
                              SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    //Get window surface
    surface = SDL_GetWindowSurface(window);
    
    while(!done) {
        // render
        SDL_LockSurface(surface);
        for(int y=0; y<surface->h; y++) {
            for(int x=0; x<surface->w; x++) {
                setpixel(surface, x, y, (x*x)/256+3*y+tick, (y*y)/256+x+tick, tick);
            }
        }
        SDL_UnlockSurface(surface);
        SDL_UpdateWindowSurface(window);
        tick++;

        // check for key/quit
        while(SDL_PollEvent(&e)) {
            done |= (e.type == SDL_QUIT || e.type == SDL_KEYDOWN);
        }
    }

    //Destroy window
    SDL_DestroyWindow(window);
    //Quit SDL subsystems
    SDL_Quit();
    return 0;
}