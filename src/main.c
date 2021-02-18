#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>

/* SDL */
SDL_Window *window;
SDL_Texture *texture;
SDL_Renderer *renderer;
#define SCALE 10 /* scale from screen to window */
#define SCREEN_WIDTH 64 /* screen buffer width */
#define SCREEN_HEIGHT 32 /* screen buffer height */
#define WINDOW_WIDTH SCREEN_WIDTH*SCALE /* window width */
#define WINDOW_HEIGHT SCREEN_HEIGHT*SCALE /* window height */

/* check if window should close ** quit must be defined as an integer */
#define POLL_EVENTS(evt) while (SDL_PollEvent(&evt)) if (evt.type == SDL_QUIT) quit=1;
/* calculate pixel value from single bit */
#define BIT_TO_PIXEL(bit) (0xFFFFFF00 * (uint8_t)bit) | 0x000000FF
/* map pointer to section in memory (will erase all data in that section) */
#define MAP_TO(ptr0, ptr1, start, size) ptr0 = (ptr1+start); memset(ptr0, 0, size)
/* get lower 8-bits of address */
#define LOW_8(address) (uint8_t)(address & 0xFF)
/* get upper 8-bits of address */
#define HIGH_8(address) (uint8_t)((address>>8) & 0xFF)
/* combine two 8-bit values (little endian) */
#define MAKE_16(a, b) (uint16_t)(((b & 0xFF) << 8) | (a & 0xFF))

/* instructions */
#define CLS() for (int i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++) display[i]=0

/* memory map */
#define RAM_START  0x0000
#define ROM_START  0x0200
#define RAM_END    0x0FFF
#define RAM_SIZE   0x1000
#define ROM_SIZE   0x0E00
#define FONT_START 0x0000
#define FONT_END   0x004F
#define FONT_SIZE  0x0050
#define RES_START  0x0000
#define RES_END    0x01FF
#define RES_SIZE   0x0200

/* ram */
uint8_t ram[RAM_SIZE];
/* rom */
uint8_t *rom; /* mapped to 0x200-0xFFF of RAM */
/* fontset */
uint8_t fontset[0x50] = {
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
    0xF0, 0x90, 0xF0, 0x90, 0x90, // a
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // b
    0xF0, 0x80, 0x80, 0x80, 0xF0, // c
    0xE0, 0x90, 0x90, 0x90, 0xE0, // d
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // e
    0xF0, 0x80, 0xF0, 0x80, 0x80, // f
};
/* reserved */
uint8_t *res;
/* stack */
uint16_t stack[0xF];

/* video memory */
uint32_t display[SCREEN_WIDTH*SCREEN_HEIGHT];

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
/* print preview of rom file */
void print_rom( void );
/* take a snapshot of ram */
void ram_snapshot( void );

/* initialize display */
void initialize_sdl( void );
/* update display */
void sdl_update( void );
/* quit sdl */
void sdl_close( void );


/* execute next instruction in ROM */
void execute_next( void ) {
    uint16_t inst = rom[PC+=2];
    uint8_t low = LOW_8(inst);
    uint8_t high = HIGH_8(inst);
    uint16_t addr = inst & 0xfff; // 12 bit address
    uint8_t l = (low >> 4) & 0xF; // highest 4 bits of the lower byte
    uint8_t n = low & 0xF; // lowest 4 bits
    uint8_t x = high & 0xF; // lower 4 bits of the higher byte
    uint8_t y = (high >> 4) & 0xF; // highest 4 bits
    uint8_t b = low; // lower byte

    switch (y) {
        default:
            break;
        case 0: {
            if (n == 0) {

            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* initialize the chip8 */
    chip_initialize();

    FILE *fp;

    /* load rom file */
    load_rom_file(fp, "tetris.rom");
    /* init sdl */
    initialize_sdl();

    /* print rom contents */
    print_rom();
    /* print the memory map */
    print_memory_map();

    /* main loop */
    int quit = 0;
    while (!quit) {
        SDL_Event e;
        POLL_EVENTS(e);
        sdl_update();
    }

    ram_snapshot();

    sdl_close();
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
    /* map rom to ram starting at 0x200 */
    MAP_TO(rom, ram, ROM_START, ROM_SIZE);
    /* map reserved space to ram */
    MAP_TO(res, ram, RES_START, RES_SIZE);

    /* copy fontset to ram */
    memcpy(res, fontset, FONT_SIZE);
}

void print_memory_map( void ) {
    printf("RAM:    0x%04X - 0x%04X   (0x%05X)  (%ikb)\n", RAM_START, RAM_END, RAM_SIZE, RAM_SIZE/1024);
    printf("FONT:   0x%04X - 0x%04X   (0x%05X)  (0kb)\n", FONT_START, FONT_END, FONT_SIZE);
    printf("ROM:    0x%04X - 0x%04X   (0x%05X)  (%ikb)\n", ROM_START, RAM_END, RAM_SIZE-ROM_START, (RAM_SIZE-ROM_START)/1024);
    printf("STACK:  0x%04X - 0x%04X   (0x%05X)  (0kb)\n", 0, 31, 32);
    printf("REG:    0x%04X - 0x%04X   (0x%05X)  (0kb)\n\n", 0, 15, 16);
}

void print_rom( void ) {
    printf("ROM Preview:\n");
    for (size_t i = 0; i < 64; i++) {
        if (i % 16 == 0) printf("0x%08x  ", i);
        printf("0x%x ", rom[i]);
        if ((i+1) % 16 == 0) printf("\n");
    }
    printf("[...]\n");
    printf("0x%08x\n\n", ROM_SIZE);
}

void ram_snapshot( void ) {
    FILE *fp = fopen("ram.txt", "w");
    for (size_t i = 0; i < RAM_SIZE; i++) {
        if (i % 16 == 0) fprintf(fp, "0x%08x  ", i);
        fprintf(fp, "0x%02x ", ram[i]);
        if ((i+1) % 16 == 0) fprintf(fp, "\n");
    }
    fclose(fp);
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

void sdl_update( void ) {
    SDL_UpdateTexture(texture, NULL, display, 64 * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void sdl_close( void ) {
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(texture);
    SDL_Quit();
}
