#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>

/* SDL */
SDL_Window *window;
SDL_Texture *texture;
SDL_Renderer *renderer;
#define SCALE 10
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define WINDOW_WIDTH SCREEN_WIDTH*SCALE
#define WINDOW_HEIGHT SCREEN_HEIGHT*SCALE
#define BIT_TO_PIXEL(bit) (0xFFFFFF00 * (uint8_t)bit) | 0x000000FF

/* memory map */
#define RAM_START 0x0000
#define ROM_START 0x0200
#define RAM_END   0x0FFF
#define RAM_SIZE  0x1000
#define ROM_SIZE  0x0E00

/* ram */
uint8_t ram[RAM_SIZE];
/* rom */
uint8_t *rom;
/* stack */
uint16_t stack[0xF];

/* video memory */
uint32_t video_buffer[SCREEN_WIDTH*SCREEN_HEIGHT];

/* general purpose 8-bit regs */
uint8_t V[0xF];
/* memory register */
uint16_t I;
/* sound timer */
uint8_t st;
/* delay timer */
uint8_t dt;
/* program counter */
uint16_t PC;
/* stack pointer */
uint8_t SP;

/* chip8 keypad layout */
const unsigned char keypad[16] = {
    '1','2','3','4', 
    'q','w','e','r', 
    'a','s','d','f', 
    'z','x','c','v'
};

/* initialize chip8 */
void chip_initialize( void );
/* print chip8 memory map */
void print_memory_map( void );

void initialize_sdl( void );

int main(int argc, char *argv[]) {
    chip_initialize();
    print_memory_map();

    FILE *fp;

    /* load rom file */
    load_rom_file(fp, "tetris.rom");

    printf("ROM Preview:\n");
    for (size_t i = 0; i < 64; i++) {
        if (i % 16 == 0) printf("0x%08x  ", i);
        printf("0x%x ", rom[i]);
        if ((i+1) % 16 == 0) printf("\n");
    }
    printf("[...]\n");
    printf("0x%08x\n\n", ROM_SIZE);

    initialize_sdl();

    int quit = 0;
    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }
        video_buffer[150] = BIT_TO_PIXEL(1);
        SDL_UpdateTexture(texture, NULL, video_buffer, 64 * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(texture);
    SDL_Quit();
}


void load_rom_file(FILE *fp, const char *fname) {
    fp = fopen(fname, "rb");
    memset(rom, 0, ROM_SIZE);
    if (fp == NULL) {
        printf("ERROR: ROM FILE '%s' DOES NOT EXIST.\n", fname);
        exit(-1);
    }
    printf("Successfuly opened '%s'.\n", fname);
    printf("Reading from binary file...\n");
    const size_t fsize = fread(rom, sizeof(uint8_t), ROM_SIZE, fp);
    printf("Successfuly read %d bytes (%dkb) from '%s'\n", fsize, fsize/1024, fname);
    printf("Now closing file\n\n");
    fclose(fp);
}

void chip_initialize( void ) {
    memset(ram, 0, RAM_SIZE*sizeof(uint8_t));
    memset(stack, 0, 0xF*sizeof(uint16_t));
    memset(V, 0, 0xF*sizeof(uint8_t));
    st = 0;
    dt = 0;
    PC = 0;
    SP = 0;
    /* map rom to ram starting at ROM */
    rom = (ram+ROM_START);
}

void print_memory_map( void ) {
    printf("RAM:    0x%04X - 0x%04X   (0x%05X)  (%ikb)\n", RAM_START, RAM_END, RAM_SIZE, RAM_SIZE/1024);
    printf("ROM:    0x%04X - 0x%04X   (0x%05X)  (%ikb)\n", ROM_START, RAM_END, RAM_SIZE-ROM_START, (RAM_SIZE-ROM_START)/1024);
    printf("STACK:  0x%04X - 0x%04X   (0x%05X)\n", 0, 31, 32);
    printf("REG:    0x%04X - 0x%04X   (0x%05X)\n\n", 0, 15, 16);
}

void initialize_sdl( void ) {
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("Chip8", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    renderer = SDL_CreateRenderer(window, -1, 0);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 64, 32);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}
