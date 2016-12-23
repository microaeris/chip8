#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "chip8.h"

#define WHITE 0xFFFFFFFF

#define X(op)   (op & 0x0F00) >> 8
#define Y(op)   (op & 0x00F0) >> 4
#define N(op)   (op & 0x000F)
#define NN(op)  (op & 0x00FF)
#define NNN(op) (op & 0x0FFF)
#define screen_idx(x, y) (x + y * SCREEN_WIDTH)

int init_sdl(SDL_Window **window, SDL_Renderer **renderer,
             SDL_Surface **surface, SDL_Texture **texture)
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
                               SDL_WINDOW_SHOWN); // SDL_WINDOW_FULLSCREEN

    if (*window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return ERROR_SDL_INIT;
    }

    *renderer = SDL_CreateRenderer(*window, -1, 0);
    if (*renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n",
            SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return ERROR_SDL_INIT;
    }

    *surface = SDL_GetWindowSurface(*window);
    *texture = SDL_CreateTexture(*renderer,
                                 SDL_PIXELFORMAT_ARGB8888,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 SCREEN_WIDTH, SCREEN_HEIGHT);
    if (*texture == NULL) {
        printf("Texture could not be created! SDL_Error: %s\n",
            SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return ERROR_SDL_INIT;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_RenderSetLogicalSize(*renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetWindowSize(*window, 640, 320);

    SDL_SetRenderDrawColor(*renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(*renderer);
    SDL_RenderPresent(*renderer);

    return ERROR_NONE;
}

void init_chip8()
{
    pc      = 0x200;
    opcode  = 0;
    I       = 0;
    sp      = 0;

    // TODO
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
    opcode = mem[pc] << 8 | mem[pc + 1];
    decode(opcode);

    // TODO this needs to run at 60 Hz
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

void decode(uint16_t opcode)
{
    uint8_t x = X(opcode);
    uint8_t y = Y(opcode);
    uint8_t pixels;
    uint8_t index;

    switch(opcode & 0xF000) {
        case 0x1000:
            pc = NNN(opcode);
            break;
        case 0x3000:
            if (V[x] == NN(opcode)) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        case 0x6000:
            V[x] = NN(opcode);
            pc += 2;
            break;
        case 0x7000:
            V[x] += NN(opcode);
            pc += 2;
            break;
        case 0x8000:
            V[x] = V[y];
            pc += 2;
            break;
        case 0xA000:
            I = NNN(opcode);
            pc += 2;
            break;
        case 0xC000: // rand, and
            V[x] = (rand() % 256) & NN(opcode);
            pc += 2;
            break;
        case 0xD000: // draw
            V[0xF] = 0;
            for (int row = 0; row < N(opcode); ++row) {
                pixels = mem[I + row];
                for(int col = 0; col < 8; ++col) {
                    if((pixels & (0x80 >> col)) != 0) {
                        index = screen_idx(V[x] + col, V[y] + row);
                        if(gfx[index] == WHITE) {
                            V[0xF] = 1;
                        }
                        gfx[index] ^= WHITE;
                    }
                }
            }
            drawFlag = true;
            pc += 2;
            break;
        default:
            printf ("Unknown opcode: 0x%X\n", opcode);
            break;
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
}

void render_screen(SDL_Window *window, SDL_Renderer *renderer,
                   SDL_Surface *surface, SDL_Texture *texture)
{
    SDL_UpdateTexture(texture, NULL, gfx, 64 * sizeof(uint32_t)); // pitch is the number of bytes from the start of one row to the next--and since we have a linear RGBA buffer in this example, it's just 640 times 4 (r,g,b,a).
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    for (int j = 0; j < 32; ++j) {
        for (int i = 0; i < 64; ++i) {
            printf("%x ", gfx[screen_idx(i,j)]&0xF);
        }
        printf("\n");
    }
    printf("\n");
    printf("\n");
}

int main(int argc, char *args[])
{
    SDL_Window *window = NULL;
    SDL_Surface *surface = NULL; // TODO remove this later
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;
    SDL_Event e;

    int res = 0;
    if ((res = init_sdl(&window, &renderer, &surface, &texture)) != ERROR_NONE) {
        return res;
    }

    // TODO: Setup keypad
    init_chip8();
    load_rom();

    while(!done) {
        emulate_cycle();
        if (drawFlag) {
            render_screen(window, renderer, surface, texture);
            drawFlag = false;
        }

        // TODO
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


//128 draws
//