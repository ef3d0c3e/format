#define _GNU_SOURCE
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
		.flush_mode = kFormatFlushNever_,
		.data = NULL,
		.size = 0,
		.capacity = 0,
		.malloc = NULL,
		.free = NULL,
		.realloc = NULL,
	};
}

void
format_output_destroy(struct format_output* output)
{
	assert(output != NULL);
	format_output_flush(output);
	if (output->data)
	{
		if (output->free)
			output->free(output->data, output->capacity);
		else
			free(output->data);
		output->data = NULL;
		output->size = 0;
		output->capacity = 0;
	}
}

void
format_output_set_allocator(struct format_output* output,
                            void* (*malloc)(size_t),
                            void (*free)(void*, size_t),
                            void* (*realloc)(void*, size_t, size_t))
{
	assert(output != NULL);
	assert(malloc != NULL);
	assert(free != NULL);
	assert(realloc != NULL);
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
	assert(output != NULL);
	assert((output->fd != -1 || output->file != NULL) && "Invalid output type");

	format_output_flush(output);
	output->flush_mode = mode;
	// TODO: stdio
}

int
format_output_flush(struct format_output* output)
{
	assert(output != NULL);

	if (output->fd != -1) {
		size_t total = 0;
		while (total != output->size)
		{
			const ssize_t n = write(output->fd, output->data + total, output->size - total);
			if (n == -1)
			{
				if (errno != EINTR) {
					output->size = 0;
					return -1;
				}
			}
			total += (size_t)n;
		}
		output->size = 0;
	} else if (output->file != NULL) {
		return fflush(output->file);
	}
	return 0;
}

int
format_output_write(struct format_output* output, const char* buf, size_t len)
{
	assert(output != NULL);
	assert(buf != NULL);

	/* File descriptor */
	if (output->fd != -1) {
		assert(output->file == NULL);
		assert(output->flush_mode != kFormatFlushNever_);
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
		assert(output->data != NULL);

		size_t total = 0;
		if (output->size + len > output->capacity) {
			/* Write and flush until last block */
			while (1) {
				if (output->size + len - total <= output->capacity)
					break;
				assert(total <= len);
				const size_t avail = output->capacity - output->size;
				assert(avail <= len - total);
				memcpy(output->data, buf + total, avail);
				if (unlikely(format_output_flush(output) == -1))
					return -1;
				total += avail;
			}

			assert(total <= len && len - total < output->capacity);
			assert(output->size == 0);
			/* Write last block, optionally flush */
			if (output->flush_mode == kFormatFlushNewline) {
				const void* nl = memrchr(buf + total, '\n', len - total);

				if (nl) {
					/* Write until last flush point, flush, then write leftover */
					memcpy(output->data,
					       output->data + total,
					       (uintptr_t)nl - (uintptr_t)output->data + total + 1);
					if (unlikely(format_output_flush(output) == -1))
						return -1;
					total += (uintptr_t)nl - (uintptr_t)output->data + 1;
					memcpy(output->data, buf + total, len - total);
					output->size = len - total;
					total = len;
				} else {
					/* Write everything */
					memcpy(output->data, buf + total, len - total);
					output->size = len - total;
					total = len;
				}
			} else {
				/* Write last block */
				assert(output->flush_mode == kFormatFlushNone);
				memcpy(output->data, buf + total, total - len);
				output->size = total - len;
				total = len;
			}
		}
		assert(total == len);
	}
	/* STDIO file */
	else if (output->file != NULL) {
		assert(output->fd == -1);

		size_t total = 0;
		while (total != len) {
			const size_t n = fwrite(buf + total, 1, len - total, output->file);
			if (n != len - total) {
				if (errno != EINTR)
					return -1;
			}
			total += n;
		}
		assert(total == len);
	}
	/* Memory buffer */
	else {
		assert(output->file == NULL && output->fd == -1);

		/* Compute new capacity */
		size_t new_cap = output->capacity;
		while (new_cap < output->size + len) {
			new_cap *= 2;
		}
		assert(new_cap >= len + output->size);

		/* Make space */
		if (output->realloc) {
			output->data = output->realloc(output->data, output->capacity, new_cap);
		} else {
			output->data = realloc(output->data, new_cap);
		}
		if (unlikely(output->data == NULL)) {
			output->capacity = 0;
			return -1;
		}
		output->capacity = new_cap;

		/* Copy */
		memcpy(output->data + output->size, buf, len);
		output->size += len;
	}

	return 0;
}
