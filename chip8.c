// TODO
// write blog post about SDL and resizing windows, and general windowing
// Rom selection screen

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL.h>
#include "chip8.h"

#define WHITE 0xFFFFFFFF

#define X(op)   (op & 0x0F00) >> 8
#define Y(op)   (op & 0x00F0) >> 4
#define N(op)   (op & 0x000F)
#define NN(op)  (op & 0x00FF)
#define NNN(op) (op & 0x0FFF)
#define screen_idx(x, y) ((x) + ((y) * SCREEN_WIDTH))

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
                               SDL_WINDOW_SHOWN);

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
    srand(time(NULL));

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
    uint32_t index;

    switch(opcode & 0xF000) {
        case 0x0000:
            switch(NNN(opcode)) {
                case 0x00E0:
                    memset(gfx, 0, sizeof(gfx));
                    drawFlag = true;
                    pc += 2;
                    break;
                case 0x00EE:
                    pc = stack[sp--];
                    break;
                default:
                    // Call a RCA 1802 program! FIXME
                    stack[sp++] = pc;
                    pc = NNN(opcode);
                    break;
            }
            break;
        case 0x1000:
            pc = NNN(opcode);
            break;
        case 0x2000:
            stack[sp++] = pc;
            pc = NNN(opcode);
        case 0x3000:
            if (V[x] == NN(opcode)) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        case 0x4000:
            if (V[x] != NN(opcode)) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        case 0x5000:
            if (V[x] == V[y]) {
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
            switch(N(opcode)) {
                case 0x0:
                    V[x] = V[y];
                    break;
                case 0x1:
                    V[x] |= V[y];
                    break;
                case 0x2:
                    V[x] &= V[y];
                    break;
                case 0x3:
                    V[x] ^= V[y];
                    break;
                case 0x4:
                    V[0xF] = V[y] > (0xFF - V[x]); // carry bit
                    V[x] += V[y];
                    break;
                case 0x5:
                    V[0xF] = V[x] < V[y]; // borrow bit
                    V[x] -= V[y];
                    break;
                case 0x6:
                    V[0xF] = V[x] & 0x1;
                    V[x] >>= 1;
                    break;
                case 0x7:
                    V[0xF] = V[y] < V[x]; // borrow bit
                    V[x] = V[y] - V[x];
                    break;
                case 0xE:
                    V[0xF] = V[x] & 0x80;
                    V[x] <<= 1;
                    break;
                default:
                    printf ("Unknown opcode: 0x%X\n", opcode); // FIXME this is redundant
                    break;
            }
            pc += 2;
            break;
        case 0x9000:
            if (V[x] != V[y]) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        case 0xA000:
            I = NNN(opcode);
            pc += 2;
            break;
        case 0xB000:
            pc = NNN(opcode) + V[0];
            break;
        case 0xC000: // rand, and
            V[x] = (rand() % 256) & NN(opcode);
            pc += 2;
            break;
        case 0xD000: // draw
            V[0xF] = 0;
            for (uint8_t row = 0; row < N(opcode); ++row) {
                pixels = mem[I + row];
                for(uint8_t col = 0; col < 8; ++col) {
                    if((pixels & (0x80 >> col)) != 0) {
                        index = screen_idx(V[x] + row, V[y] + col);
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
        case 0xE000:
            switch(NN(opcode)) {
                case 0x9E:
                    if (last_key == V[x]) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
                case 0xA1:
                    if (last_key != V[x]) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
                default:
                    printf ("Unknown opcode: 0x%X\n", opcode); // FIXME this is redundant
                    break;
            }
            break;
        case 0xF000:
            switch(NN(opcode)) {
                case 0x07:
                    V[x] = delay_timer;
                    break;
                case 0x0A:
                    // Wait for a key press
                    SDL_WaitEvent(&e);
                    if (e.type == SDL_KEYDOWN) {
                        V[x] = handle_key_down(e.key.keysym.sym);
                    }
                    break;
                case 0x15:
                    delay_timer = V[x];
                    break;
                case 0x18:
                    sound_timer = V[x];
                    break;
                case 0x1E:
                    I += V[x];
                    break;
                case 0x29:
                    // FIXME Character sprite address
                    I = 0x50 + V[x] * 5;
                    break;
                case 0x33:
                    // BCD
                    mem[I] = V[x] / 100;
                    mem[I+1] = (V[x] / 10 ) % 10;
                    mem[I+2] = V[x] % 10;
                    break;
                case 0x55:
                    // Reg dump
                    memcpy(mem + I, V, sizeof(V));
                    break;
                case 0x65:
                    // Reg load
                    memcpy(V, mem + I, sizeof(V));
                    break;
                default:
                    printf ("Unknown opcode: 0x%X\n", opcode); // FIXME this is redundant
                    break;
            }
            pc += 2;
            break;
        default:
            printf ("Totally unknown opcode: 0x%X\n", opcode);
            break;
    }
}

void load_rom()
{
    // Copy rom from file to mem array
    FILE *rom_file = fopen("./roms/GUESS", "r");
    fseek(rom_file, 0, SEEK_END);   // seek to end of file
    size_t size = ftell(rom_file);  // get current file pointer
    fseek(rom_file, 0, SEEK_SET);   // seek back to beginning of file
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

    // for (int j = 0; j < 32; ++j) {
    //     for (int i = 0; i < 64; ++i) {
    //         printf("%x ", gfx[screen_idx(i,j)]&0xF);
    //     }
    //     printf("\n");
    // }
    // printf("\n");
}

/* Handle hex key press
 * Keypad                   Keyboard
 * +-+-+-+-+                +-+-+-+-+
 * |1|2|3|C|                |1|2|3|4|
 * +-+-+-+-+                +-+-+-+-+
 * |4|5|6|D|                |Q|W|E|R|
 * +-+-+-+-+       =>       +-+-+-+-+
 * |7|8|9|E|                |A|S|D|F|
 * +-+-+-+-+                +-+-+-+-+
 * |A|0|B|F|                |Z|X|C|V|
 * +-+-+-+-+                +-+-+-+-+
 */
uint8_t handle_key_down(SDL_Keycode keycode) // TODO move this to its own file
{
    switch(keycode) {
        case SDLK_1:
            key[0x1] = 1;
            return 0x1;
        case SDLK_2:
            key[0x2] = 1;
            return 0x2;
        case SDLK_3:
            key[0x3] = 1;
            return 0x3;
        case SDLK_4:
            key[0xC] = 1;
            return 0xC;
        case SDLK_q:
            key[0x4] = 1;
            return 0x4;
        case SDLK_w:
            key[0x5] = 1;
            return 0x5;
        case SDLK_e:
            key[0x6] = 1;
            return 0x6;
        case SDLK_r:
            key[0xD] = 1;
            return 0xD;
        case SDLK_a:
            key[0x7] = 1;
            return 0x7;
        case SDLK_s:
            key[0x8] = 1;
            return 0x8;
        case SDLK_d:
            key[0x9] = 1;
            return 0x9;
        case SDLK_f:
            key[0xE] = 1;
            return 0xE;
        case SDLK_z:
            key[0xA] = 1;
            return 0xA;
        case SDLK_x:
            key[0x0] = 1;
            return 0x0;
        case SDLK_c:
            key[0xB] = 1;
            return 0xB;
        case SDLK_v:
            key[0xF] = 1;
            return 0xF;
        default:
            return -1;
    }
}

uint8_t handle_key_up(SDL_Keycode keycode)
{
    switch(keycode) {
        case SDLK_1:
            key[0x1] = 0;
            return 0x1;
        case SDLK_2:
            key[0x2] = 0;
            return 0x2;
        case SDLK_3:
            key[0x3] = 0;
            return 0x3;
        case SDLK_4:
            key[0xC] = 0;
            return 0xC;
        case SDLK_q:
            key[0x4] = 0;
            return 0x4;
        case SDLK_w:
            key[0x5] = 0;
            return 0x5;
        case SDLK_e:
            key[0x6] = 0;
            return 0x6;
        case SDLK_r:
            key[0xD] = 0;
            return 0xD;
        case SDLK_a:
            key[0x7] = 0;
            return 0x7;
        case SDLK_s:
            key[0x8] = 0;
            return 0x8;
        case SDLK_d:
            key[0x9] = 0;
            return 0x9;
        case SDLK_f:
            key[0xE] = 0;
            return 0xE;
        case SDLK_z:
            key[0xA] = 0;
            return 0xA;
        case SDLK_x:
            key[0x0] = 0;
            return 0x0;
        case SDLK_c:
            key[0xB] = 0;
            return 0xB;
        case SDLK_v:
            key[0xF] = 0;
            return 0xF;
        default:
            return -1;
    }
}

int main(int argc, char *args[])
{
    // TODO make these global
    SDL_Window *window = NULL;
    SDL_Surface *surface = NULL; // TODO remove this later
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;

    int res = 0;
    if ((res = init_sdl(&window, &renderer, &surface, &texture)) != ERROR_NONE) {
        return res;
    }

    init_chip8();
    load_rom();

    while(!done) {
        // check for key press
        while (SDL_PollEvent(&e)) {
            done |= (e.type == SDL_QUIT);
            if (e.type == SDL_KEYDOWN) {
                last_key = handle_key_down(e.key.keysym.sym);
            } else if (e.type == SDL_KEYUP) {
                last_key = handle_key_up(e.key.keysym.sym);
            }
        }

        emulate_cycle();
        if (drawFlag) {
            render_screen(window, renderer, surface, texture);
            drawFlag = false;
        }

        tick++; // TODO remove this later
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return ERROR_NONE;
}
