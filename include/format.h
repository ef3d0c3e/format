#ifndef LIBFORMAT_H
#define LIBFORMAT_H

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/** @brief Format style */
enum format_output_style
{
	/** @brief Do not affect current style */
	kFormatStyleNone = 0,
	/** @brief Enable bold */
	kFormatStyleBold = (1 << 0),
	/** @brief Enable italic */
	kFormatStyleItalic = (1 << 1),
	/** @brief Enable underline */
	kFormatStyleUnderline = (1 << 2),
	/** @brief Enable crossed */
	kFormatStyleCrossed = (1 << 3),
	/** @brief Reset all styles and colors */
	kFormatStyleReset = ~0,
};

/** @brief Format color format, 24-bit RGB, alpha component ignored */
typedef uint32_t format_color;

/** @brief Flushing behavior for @ref format_output */
enum format_output_flush_mode
{
	/** @brief Flush on newlines */
	kFormatFlushNewline = 0,
	/** @brief No specific flushing behavior */
	kFormatFlushNone = 1,
	/** @brief Flush every written buffer */
	kFormatFlushAlways = 2,
	/** @brief Never flush, should be used when `fd == -1 || file == NULL`, to format to a memory
	   buffer */
	kFormatFlushNever_ = 3,
};

// Format output buffer
struct format_output;

/**
 * @brief Create a new @ref format_output from a file descriptor
 *
 * @param fd File descriptor to output to
 *
 * @return A new @ref format_output that will output to @p fd
 */
struct format_output
format_output_fd(int fd);
/**
 * @brief Create a new @ref format_output from a stdio's `FILE` pointer
 *
 * @param file `FILE` pointer to output to
 *
 * @return A new @ref format_output that will output to @p file
 */
struct format_output
format_output_file(FILE* file);
/**
 * @brief Create a new @ref format_output to output to it's internal buffer
 *
 * @return A new @ref format_output that will store output bytes
 */
struct format_output
format_output_buf(void);
/**
 * @brief Destroy a @ref format_output
 *
 * This function will flush if needed
 *
 * @param output Output to destroy
 */
void
format_output_destroy(struct format_output* output);

/**
 * @brief Set allocator for the output
 */
void
format_output_set_allocator(struct format_output* output,
                            void* (*malloc)(size_t),
                            void (*free)(void*, size_t),
                            void* (*realloc)(void*, size_t, size_t));
/**
 * @brief Set the output flushing mode
 */
void
format_output_set_flush(struct format_output* output, enum format_output_flush_mode mode);
/**
 * @brief Flush the format output to it's underlying file descriptor or `FILE`
 *
 * @param output @ref format_output
 *
 * @return `0` on success, `-1` on error and `errno` is set
 */
int
format_output_flush(struct format_output* output);
/**
 * @brief Writes raw bytes to the output
 *
 * @param output @ref format_output to write to
 * @param buf Buffer to write
 * @param len Number of bytes in @p buf to write
 *
 * This function writes the bytes from @p buf into @p output, allocating and flushing as required
 *
 * @return 0 on success, -1 on failure
 */
int
format_output_write(struct format_output* output, const char* buf, size_t len);

#endif // LIBFORMAT_H
