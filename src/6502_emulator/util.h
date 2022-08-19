////////////////////////////////////////////////////////////////////////////
///        @author Gustaf Franzén :: https://github.com/BjorneEk;        ///
////////////////////////////////////////////////////////////////////////////

#ifndef _UTIL_H_
#define _UTIL_H_


#include <stdbool.h>

typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned int u32_t;
typedef unsigned long long u64_t;

typedef char i8_t;
typedef short i16_t;
typedef int i32_t;
typedef long i64_t;


void log_warning(const char * msg);
void log_warning_str(const char * msg, const char * str);
void log_error(const char * msg);
void log_error_str(const char * msg, const char * str);
void log_error_char(const char * msg, char c);
void log_error_int(const char * msg, int i);

void assert_w(bool cond, const char * msg);
void assert_err(bool cond, const char * msg);

void hexdump(const u8_t * data, u64_t len);


#endif /* _UTIL_H_ */
