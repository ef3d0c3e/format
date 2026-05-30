#include "buffer.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

// Public API

struct format_output
format_output_fd(int fd)
{
	assert(fd != -1);
	return (struct format_output){
		.fg = ~0U,
		.bg = ~0U,
		.style = kFormatStyleNone,
		.fd = fd,
		.file = NULL,
		.flush_mode = kFormatFlushNever,
		.data = NULL,
		.size = 0,
		.capacity = 0,
		.malloc = NULL,
		.free = NULL,
		.realloc = NULL,
	};
}

struct format_output
format_output_file(FILE* file)
{
	assert(file != NULL);
	return (struct format_output){
		.fg = ~0U,
		.bg = ~0U,
		.style = kFormatStyleNone,
		.fd = -1,
		.file = file,
		.flush_mode = kFormatFlushNewline,
		.data = NULL,
		.size = 0,
		.capacity = 0,
		.malloc = NULL,
		.free = NULL,
		.realloc = NULL,
	};
}

struct format_output
format_output_buf(void)
{
	return (struct format_output){
		.fg = ~0U,
		.bg = ~0U,
		.style = kFormatStyleNone,
		.fd = -1,
		.file = NULL,
		.flush_mode = kFormatFlushNever,
		.data = NULL,
		.size = 0,
		.capacity = 0,
		.malloc = NULL,
		.free = NULL,
		.realloc = NULL,
	};
}

void
format_output_destroy(struct format_output* output);

void
format_output_set_allocator(struct format_output* output,
                            void* (*malloc)(size_t),
                            void (*free)(void*),
                            void* (*realloc)(void*, size_t, size_t))
{
	assert(output->data == NULL);
	assert(output->size == 0);
	assert(output->capacity == 0);
	output->malloc = malloc;
	output->free = free;
	output->realloc = realloc;
}

void
format_output_set_flush(struct format_output* output, enum format_output_flush_mode mode)
{
	assert(output->fd != -1 || output->file != NULL);
	output->flush_mode = mode;
	// TODO: stdio
}

int
format_output_flush(struct format_output* output)
{
	if (output->fd != -1) {
		const ssize_t r = write(output->fd, output->data, output->size);
		output->size = 0;
		return (r == -1) ? -1 : 0;
	} else if (output->file != NULL) {
		return fflush(output->file);
	}
	return 0;
}

/// INTERNAL

int
format_output_write(struct format_output* output, const char* buf, size_t len)
{
	/* File descriptor */
	if (output->fd != -1) {
		if (output->flush_mode == kFormatFlushAlways) {
			size_t total = 0;
			do {
				ssize_t n = write(output->fd, buf + total, len - total);
				if (n < 0 && errno == EINTR)
					continue;
				else if (n < 0) {
					return -1;
					break;
				} else if (n == 0)
					continue;
				total += (size_t)n;
			} while (total < len);
			return 0;
		}

		/* Allocate buffer */
		if (unlikely(output->data == NULL)) {
			if (output->malloc)
				output->data = output->malloc(1024);
			else
				output->data = malloc(1024);
			if (unlikely(output->data == NULL))
				return -1;
			output->capacity = 1024;
		}

		/* Write first bytes */
		size_t total = 0;
		if (output->size < output->capacity)
		{
			const size_t avail = output->capacity - output->size;
			const size_t to_write = (avail > len) ? len : avail;
			memcpy(output->data + output->size, buf, to_write);
			total += to_write;
		}

		/* Write leftover */
		while (total < len)
		{

		}
	}

	return 0;
}
