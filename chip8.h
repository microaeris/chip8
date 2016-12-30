#include <stdint.h>
#include <stdbool.h>

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
uint32_t gfx[64*32]; // RGBA buffer
uint8_t delay_timer;
uint8_t sound_timer;
uint16_t stack[16];
uint16_t sp; // Points to the next free spot on the stack
uint8_t key[16];
uint8_t key_down; // Stores the key pressed in this cycle. can be none. can be checked once (and then it will be reset)
bool drawFlag;

unsigned char chip8_fontset[80] =
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
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

typedef enum {
    ERROR_NONE,
    ERROR_SDL_INIT,
    ERROR_BAD_OPCODE,
} ERROR_T;

//SDL
SDL_Event e;

int init_sdl(SDL_Window **window, SDL_Renderer **renderer,
             SDL_Surface **surface, SDL_Texture **texture);
void init_chip8();
void emulate_cycle();
void decode(uint16_t opcode);
void load_rom();
uint8_t handle_key_down(SDL_Keycode keycode);
uint8_t handle_key_up(SDL_Keycode keycode);
void render_screen(SDL_Window *window, SDL_Renderer *renderer,
                   SDL_Surface *surface, SDL_Texture *texture);
