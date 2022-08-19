

#include "util.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

void log_warning(const char * msg) {
  printf("[\033[33;1;4mWarning\033[0m]: %s\n", msg);
}

void assert_w(bool cond, const char * msg) {
  if(cond) log_warning(msg);
}
void assert_err(bool cond, const char * msg) {
  if(cond) log_error(msg);
}

void log_warning_str(const char * msg, const char * str) {
  printf("[\033[33;1;4mWarning\033[0m]: %s: '%s'\n", msg, str);
}
void log_error(const char * msg) {
  fprintf(stderr, "[\033[31;1;4mError\033[0m]: %s | %s\n",msg, strerror( errno ));
}
void log_error_str(const char * msg, const char * str) {
  fprintf(stderr, "[\033[31;1;4mError\033[0m]: %s: '%s' | %s\n",msg, str, strerror( errno ));
}
void log_error_char(const char * msg, char c) {
  fprintf(stderr, "[\033[31;1;4mError\033[0m]: %s: %c\n",msg, c);
}
void log_error_int(const char * msg, int i) {
  fprintf(stderr, "[\033[31;1;4mError\033[0m]: %s: %i\n",msg, i);
}

void hexdump(const u8_t * data, u64_t len) {
  u32_t i, j, k, cnt;
  u8_t r;

  printf("┌─────────┬─────────────────────────┬─────────────────────────┬────────┬────────┐\n");

  for (i = r = cnt = 0; i < len; i += 0x10) {
    if(i != 0 && !memcmp(&data[i], &data[i - 0x10], 0x10)){
      r = 1; cnt++; continue;
    }

    if (r) {
      printf("│*%7i │                         │                         │        │        │\n",cnt);
      r = cnt = 0;
    }

    printf("│%08x ", i);

    j = 0;
    while (j < 2) {
      printf("│ ");
      for(k = 0; k < 0x8; k++) printf("%02x ", data[i+k+(j*0x8)]);
      j++;
    }

    j = 0;
    while (j < 2) {
      printf("│");
      for(k = 0; k < 0x8; k++) {
        if(31 < data[i+k+(j*0x8)] && 127 > data[i+k+(j*0x8)])
          printf("%c", (char)data[i+k+(j*0x8)]);
        else if(data[i+k+(j*0x8)] == 0)printf("0");
        else printf("×");
      }
      j++;
    }
    printf("│");
    printf("\n");
  }
  printf("└─────────┴─────────────────────────┴─────────────────────────┴────────┴────────┘\n");
}
