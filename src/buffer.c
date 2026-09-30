/* format -- C formatting library
 * Copyright (C) 2026 ef3d0c3e
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#include "fmt.h"

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
		.nwritten = 0,
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
		.nwritten = 0,
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
		.nwritten = 0,
		.malloc = NULL,
		.free = NULL,
		.realloc = NULL,
	};
}

struct format_output
format_output_none(void)
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
		.capacity = (size_t)-1,
		.nwritten = 0,
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
	if (output->data) {
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
	/* Memory outputs do not flush, setting the mode has no effect */
	if (output->fd == -1 && output->file == NULL)
		return;

	if (mode != kFormatFlushNever_) {
		format_output_flush(output);
		output->flush_mode = mode;
	}
}

int
format_output_flush(struct format_output* output)
{
	assert(output != NULL);

	if (output->fd != -1) {
		size_t total = 0;
		while (total != output->size) {
			const ssize_t n = write(output->fd, output->data + total, output->size - total);
			if (n == -1) {
				if (errno == EINTR)
					continue;
				output->size = 0;
				return -1;
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

		/* No buffering */
		if (output->flush_mode == kFormatFlushAlways) {
			size_t pos = 0;
			do {
				const ssize_t n = write(output->fd, buf + pos, len - pos);
				if (n < 0 && errno == EINTR)
					continue;
				else if (n < 0) {
					return -1;
					break;
				} else if (n == 0)
					continue;
				output->nwritten += (size_t)n;
				pos += (size_t)n;
			} while (pos < len);
			return 0;
		}

		/* Allocate buffer */
		if (output->data == NULL) {
			if (output->malloc)
				output->data = output->malloc(1024);
			else
				output->data = malloc(1024);
			if (output->data == NULL)
				return -1;
			output->capacity = 1024;
		}
		assert(output->data != NULL);

		size_t pos = 0;
		if (output->flush_mode == kFormatFlushNewline) {
			/* Here we know that the internal buffer cannot contain a `\n`, so we scan the @p buf
			 * for a `\n`, then we write everything until that `\n`, and the rest is handled by
			 * the normal buffer case. */

			const void* nl = memrchr(buf, '\n', len);
			// Write until newline
			if (nl) {
				const size_t nl_pos = (size_t)((const char*)nl - buf);
				// Write internal buffer first
				if (format_output_flush(output))
					return -1;

				// Write until the last newline
				while (pos <= nl_pos) {
					const ssize_t n = write(output->fd, buf + pos, nl_pos - pos + 1);
					if (n < 0 && errno == EINTR) {
						continue;
					} else if (n < 0) {
						return -1;
					}
					if (n == 0) {
						continue;
					}
					output->nwritten += (size_t)n;
					pos += (size_t)n;
				}
			}
			assert(len >= pos);
		}
		const size_t left = len - pos;
		/* If leftover is small, copy it to buffer */
		if (left <= output->capacity - output->size) {
			memcpy(output->data + output->size, buf + pos, left);
			output->size += left;
			output->nwritten += left;
			return 0;
		}

		/* Not enough space in buffer: flush, to buffer again, or write without buffering if writes are large */
		if (format_output_flush(output))
			return -1;
		if (left <= output->capacity)
		{
			memcpy(output->data, buf + pos, left);
			output->size += left;
			output->nwritten += left;
			return 0;
		}

		while (pos != len) {
			assert(pos < len);
			const ssize_t n = write(output->fd, buf + pos, len - pos);
			if (n < 0 && errno == EINTR) {
				continue;
			} else if (n < 0) {
				return -1;
			}
			if (n == 0) {
				continue;
			}
			output->nwritten += (size_t)n;
			pos += (size_t)n;
		}
		assert(pos == len);
	}
	/* STDIO file */
	else if (output->file != NULL) {
		assert(output->fd == -1);

		size_t pos = 0;
		while (pos != len) {
			const size_t n = fwrite(buf + pos, 1, len - pos, output->file);
			if (n != len - pos) {
				if (errno != EINTR)
					return -1;
			}
			pos += n;
			output->nwritten += n;
		}
		assert(pos == len);
	}
	/* Memory buffer */
	else {
		assert(output->file == NULL && output->fd == -1);

		if (output->capacity != (size_t)-1) {

			/* Compute new capacity */
			size_t new_cap = output->capacity ? output->capacity : 1;
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
			if (output->data == NULL) {
				output->capacity = 0;
				return -1;
			}
			output->capacity = new_cap;

			/* Copy */
			memcpy(output->data + output->size, buf, len);
			output->nwritten += len;
		}
		output->size += len;
	}

	return 0;
}
