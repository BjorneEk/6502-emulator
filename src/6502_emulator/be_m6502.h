////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////
#ifndef _BE_M6502_H_
#define _BE_M6502_H_

#include <sdl/SDL.h>
#include <stdbool.h>
#include "m6502.h"

#define MEM_START (0x0000)
#define MEM_END   (0xFFFF)

#define RAM_START (0x0000)
#define RAM_END   (0x3FFF)

#define VRAM_START (0x3000)
#define VRAM_END   (0x3FFF)

#define IO_START   (0x4000)
#define IO_END     (0x7FFF)

#define ROM_START  (0x8000)
#define ROM_END    (0x8FFF)

#define VGA_WIDTH  100
#define VGA_HEIGHT 64
#define VGA_PAGEWIDTH 128

#define RED_BM   (0b00110000)
#define GREEN_BM (0b00001100)
#define BLUE_BM  (0b00000011)

#define RED_VALUE(clr)   ((0xFF) * ((float)((clr & RED_BM) >> 4)/3))
#define GREEN_VALUE(clr) ((0xFF) * ((float)((clr & GREEN_BM) >> 2)/3))
#define BLUE_VALUE(clr)  ((0xFF) * (float)(clr & BLUE_BM)/3)

#define VGA_VRAM_CORD(vga, _x, _y) (u8_t) (*(((vga->vram) + (_x)) + ((VGA_PAGEWIDTH) * (_y))))

typedef struct {

  SDL_Window * window;

  SDL_Renderer * renderer;

  u32_t w, h;

  u32_t pixel_size;

  u8_t * vram;
} vga_t;

void vga_init(vga_t * vga, m6502_t * em, u32_t pixel_size);
void vga_repaint(vga_t * vga);
void vga_destroy_window(vga_t * vga);

#endif
