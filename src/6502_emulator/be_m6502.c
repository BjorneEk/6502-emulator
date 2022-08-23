


#include "be_m6502.h"





void vga_init(vga_t * vga, m6502_t * em, u32_t pixel_size) {
  vga->pixel_size = pixel_size;
  vga->vram = VRAM(em->mem);
  vga->h = VGA_HEIGHT;
  vga->w = VGA_WIDTH;
  SDL_Init(SDL_INIT_EVERYTHING);

  SDL_CreateWindowAndRenderer(
    VGA_WIDTH  * pixel_size,
    VGA_HEIGHT * pixel_size,
    SDL_WINDOW_SHOWN, &vga->window, &vga->renderer);

  assert_err(vga->window == NULL || vga->renderer == NULL, "Failed to initialize SDL2 window");

  SDL_SetWindowTitle(vga->window, "100 x 64 vga emulator");
  SDL_SetRenderDrawColor(vga->renderer, 0, 0, 0, 0);
  SDL_RenderClear(vga->renderer);
  SDL_RenderPresent(vga->renderer);
}

void vga_setpixel(vga_t * vga, u32_t x, u32_t y, u8_t color) {
  SDL_Rect pixel;

  pixel = (SDL_Rect){
    .y=(y*vga->pixel_size),.x=(x*vga->pixel_size),
    .h=vga->pixel_size,.w=vga->pixel_size
  };
  SDL_SetRenderDrawColor(vga->renderer,
    RED_VALUE(color),
    GREEN_VALUE(color),
    BLUE_VALUE(color), 0);
  SDL_RenderFillRect(vga->renderer, &pixel);
}

void vga_repaint(vga_t * vga) {
  u8_t pbyte;
  u32_t i, j;

  SDL_SetRenderDrawColor(vga->renderer, 0, 0, 0, 0);
  SDL_RenderClear(vga->renderer);

  for(i = 0; i < VGA_WIDTH; i++)
    for (j = 0; j < VGA_HEIGHT; j ++)
      vga_setpixel(vga, i, j, VGA_VRAM_CORD(vga, i, j));
  SDL_RenderPresent(vga->renderer);
}

void vga_destroy_window(vga_t * vga) {
  SDL_DestroyRenderer(vga->renderer);
  SDL_DestroyWindow(vga->window);
}
