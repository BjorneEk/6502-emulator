
////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////
#include <sdl/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include "emulator.h"

#define WIDTH  100
#define HEIGHT 64
#define PIXEL_SIZE 20
#define VRAM_START 0x2000
#define vram_at(_x, _y) (uint16_t) (((VIDPAGE)+(_x))+((160) * (_y)))

void init_window(SDL_Window ** window, SDL_Renderer ** renderer) {
  SDL_Init(SDL_INIT_EVERYTHING);

  SDL_CreateWindowAndRenderer(WIDTH * PIXEL_SIZE, HEIGHT * PIXEL_SIZE, SDL_WINDOW_SHOWN, window, renderer);

  if(*window == NULL || *renderer == NULL)
  {
    log_error("Failed to initialize SDL2 window");
    return;
  }

  SDL_SetWindowTitle(*window, "window");
  SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 0);
  SDL_RenderClear(*renderer);
  SDL_RenderPresent(*renderer);
}

bool event_loop(SDL_Event * event)
{
  while (SDL_PollEvent(event))
  {
    switch ((*event).type)
    {
      case SDL_MOUSEBUTTONDOWN:
        switch ((*event).button.button)
        {
          case SDL_BUTTON_LEFT:  break;
          case SDL_BUTTON_RIGHT: break;
          default:               break;
        }
        break;
      case SDL_MOUSEBUTTONUP:
        switch ((*event).button.button)
        {
          case SDL_BUTTON_LEFT:  break;
          case SDL_BUTTON_RIGHT: break;
          default:               break;
        }
        break;
      case SDL_MOUSEMOTION:
          /* event.motion.x event.motion.y */
        break;
      case SDL_KEYDOWN: break;
      case SDL_KEYUP:   break;
      case SDL_QUIT: return true; break;
    }
  }
  return false;
}

void main_loop(SDL_Window * window, SDL_Renderer * renderer,
               SDL_Event * event, emulator_t * em) {
  bool quit = false;

  while (!quit)
  {
    for(int i = 0; i < 100*1290; i++) execute(em);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    for(int j = 0; j < 64; j++) {
      for (int i = 0; i < 100; i ++) {
        u8_t pbyte = read_byte(em, (VRAM_START + i + (128*j)));
        SDL_SetRenderDrawColor(renderer,0,0,0, 0);
        SDL_Rect pixel = (SDL_Rect) {
          .y=j*PIXEL_SIZE,.x=i*PIXEL_SIZE,
          .h=PIXEL_SIZE,.w=PIXEL_SIZE
        };
        SDL_RenderFillRect(renderer, &pixel);

        SDL_SetRenderDrawColor(renderer,
          (0xFF) * ((float)((pbyte & 0b00110000) >> 4)/3),
          (0xFF) * ((float)((pbyte & 0b00001100) >> 2)/3),
          (0xFF) * (float)(pbyte & 0b00000011)/3, 0
        );
        pixel = (SDL_Rect){
          .y=(j*PIXEL_SIZE)+1,.x=(i*PIXEL_SIZE)+1,
          .h=PIXEL_SIZE-2,.w=PIXEL_SIZE-2
        };
        SDL_RenderFillRect(renderer, &pixel);
      }

    }
    /**
     *   update screen?
     **/
    SDL_RenderPresent(renderer);
    quit = event_loop(event);
  }
}


int main(int argc, char * args[]) {
  SDL_Window   * window   = NULL;
  SDL_Renderer * renderer = NULL;
  emulator_t em;
  SDL_Event event;
  
  reset(&em);
  read_mem_file(&em.mem, "program.bin");
  start_program(&em);
  for(int i = 1; i < 0xFF+1; i++) {
    execute(&em);
  }
  init_window(&window, &renderer);
  main_loop(window, renderer, &event, &em);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  exit(0);
}
