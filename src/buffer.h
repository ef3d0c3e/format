#ifndef LIBFORMAT_BUFFER_H
#define LIBFORMAT_BUFFER_H

#include "../include/format.h"
#include <stdio.h>

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

int format_output_write(struct format_output *output, const char *buf, size_t len);



#endif // LIBFORMAT_BUFFER_H
