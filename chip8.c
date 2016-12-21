// cc -o sdltest -O2 -Wall -std=c11 -g `sdl-config --cflags --libs` sdltest.c


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
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main( int argc, char* args[] )
{
    //The window we'll be rendering to
    SDL_Window* window = NULL;
    
    //The surface contained by the window
    SDL_Surface* screenSurface = NULL;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }
    else
    {
        //Create window
        window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( window == NULL )
        {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Get window surface
            screenSurface = SDL_GetWindowSurface( window );

            //Fill the surface white
            SDL_FillRect( screenSurface, NULL, SDL_MapRGB( screenSurface->format, 0xFF, 0xFF, 0xFF ) );
            
            //Update the surface
            SDL_UpdateWindowSurface( window );

            //Wait two seconds
            SDL_Delay( 2000 );
        }
    }
    //Destroy window
    SDL_DestroyWindow( window );

    //Quit SDL subsystems
    SDL_Quit();

    return 0;
}