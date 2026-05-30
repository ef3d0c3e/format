#ifndef LIBFORMAT_BUFFER_H
#define LIBFORMAT_BUFFER_H

#include "../include/format.h"
#include <stdio.h>

struct format_output {
	// Style + Color
	/** @brief Foreground color */
	format_color fg;
	/** @brief Background color */
	format_color bg;
	/** @brief Output style */
	enum format_output_style style;

	// Output data
	/** @brief Output file descriptor, `-1` for none */
	int fd;
	/** @brief Stdio `FILE` output, `NULL` for none */
	FILE* file;
	/** @brief Flushing mode */
	enum format_output_flush_mode flush_mode;

	// Output buffer
	/** @brief Buffer data */
	char *data;
	/** @brief Buffer size */
	size_t size;
	/** @brief Buffer allocated capacity */
	size_t capacity;

	// Allocators
	void* (*malloc)(size_t);
	void (*free)(void*, size_t);
	void* (*realloc)(void*, size_t, size_t);
};

int format_output_write(struct format_output *output, const char *buf, size_t len);



#endif // LIBFORMAT_BUFFER_H
