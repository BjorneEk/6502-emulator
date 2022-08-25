
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
#include <time.h>
#include "m6502.h"
#include "be_m6502.h"

#define WIDTH  100
#define HEIGHT 64

#define PIXEL_SIZE 20

#define CYCLES_PER_REPAINT (10000)

bool event_loop(SDL_Event * event, m6502_t * em)
{
  while (SDL_PollEvent(event))
  {
    switch ((*event).type)
    {
      case SDL_MOUSEBUTTONDOWN:
        switch ((*event).button.button)
        {
          case SDL_BUTTON_LEFT:
            em->cpu.irq = true;
            break;
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
void delay(double nseconds)  {
  clock_t start_time = clock();
  while (clock() < start_time + nseconds);
}

void main_loop(vga_t * vga, SDL_Event * event, m6502_t * em) {
  i32_t i, cycles;
  double time_total, time_real;
  clock_t begin, end;
  bool quit;

  quit = false;

  while (!quit) {
    begin = clock();
    cycles = 0;
    for(i = 0; i < CYCLES_PER_REPAINT; i++) cycles += execute(em);

    vga_repaint(vga);
    quit = event_loop(event, em);

    time_total = EXECUTION_TIME(cycles);
    end = clock();
    time_real = (10*(double)(end - begin) / CLOCKS_PER_SEC);
    delay((time_total - time_real)*1000000);
  }
}


int main(int argc, char * args[]) {
  vga_t vga;
  m6502_t em;
  SDL_Event event;


  if (argc < 2){
    log_error("invalid argument count, enter a binary program file");
    return -1;
  } if (!strcmp(args[1], "-h")) {
    printf("%s <filename>\n", args[0]);
    printf("Options:\n");
    printf("  <\033[1;33m-d\033[0m> debug   enables debug mode\n");
    printf("  <\033[1;33m-bp\033[0m> <addr> breakpoint");
    printf("  <\033[1;33m-ng\033[0m> no graphics   dissables graphics\n");
    printf("  <\033[1;33m-h\033[0m> help   display user info\n");
    return 0;
  } else if (argc == 2) {
    reset(&em);
    if(read_mem_file(&em.mem, args[1])) return -1;
  } else {
    printf("there are currently unsuported options");
    return -1;
  }

  vga_init(&vga, &em, PIXEL_SIZE);
  enable_debug(&em);
  main_loop(&vga, &event, &em);

  vga_destroy_window(&vga);
  SDL_Quit();
  exit(0);
}
