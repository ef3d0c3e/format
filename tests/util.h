#ifndef FORMAT_TESTS_UTIL_H
#define FORMAT_TESTS_UTIL_H

#include <criterion/criterion.h>
#include <criterion/logging.h>

#include <format.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>
#include <signal.h>

static inline size_t
utf8_len(const char* str, size_t len)
{
	if (!len)
		return 0;
	size_t s;
	if ((str[0] & 0x80) == 0)
		s = 1;
	else if ((str[0] & 0xE0) == 0xC0)
		s = 2;
	else if ((str[0] & 0xF0) == 0xE0)
		s = 3;
	else if ((str[0] & 0xF8) == 0xF0)
		s = 4;
	else if ((str[0] & 0xFC) == 0xF8)
		s = 5;
	else if ((str[0] & 0xFE) == 0xFC)
		s = 6;
	else
		return 0;
	if (s > len)
		return 0;
	if (strnlen(str, s) < s)
		return 0;
	return s;
}

static inline void print_buffer(const char *buf, size_t len)
{
	printf(" \"");
	for (size_t i = 0; i < len; ++i)
	{
		if (isprint(buf[i]))
			printf("%c", buf[i]);
		else
		{
			const size_t clen = utf8_len(&buf[i], len - i);
			if (clen > 1)
			{
				printf("'%zu'",clen);
				printf("%.*s", (int)len, &buf[i]);
				i += clen - 1;
			}
			else
				printf("\033[36m\\x%hhx\033[0m", (unsigned char)buf[i]);
		}
	}
	printf("\" [%zu]\n", len);
}

static inline void cleanup_buf(char **buf)
{
	free(*buf);
}

#define test_printf(fmt_format_, fmt_printf_, ...) \
do { \
	FORMAT__START_DIAG(gcc) \
	FORMAT__START_DIAG(clang) \
	FORMAT__DIAG(gcc, ignored "-Wformat") \
	FORMAT__DIAG(clang, ignored "-Wformat") \
	char __attribute__((cleanup(cleanup_buf))) *buf = NULL; \
	int len = asprintf(&buf, fmt_printf_, __VA_ARGS__); \
	{ \
		struct format_output out__ = format_output_buf(); \
		const int result__ = format(&out__, fmt_format_, __VA_ARGS__); \
		cr_assert(result__ == 0); \
		if (out__.size != (size_t)len || memcmp(out__.data, buf, out__.size) != 0) \
		{ \
			printf("Got:\n"); \
			print_buffer(out__.data, out__.size); \
			printf("Expected:\n"); \
			print_buffer(buf, (size_t)len); \
			cr_assert(0, "test %s", #fmt_format_ "/" #fmt_printf_); \
		} \
		else \
			cr_assert(1); \
		format_output_destroy(&out__); \
	} \
	{ \
		char *data_buf = NULL; \
		size_t data_size = 0; \
		FILE *f = open_memstream(&data_buf, &data_size); \
		struct format_output out__ = format_output_file(f); \
		const int result__ = format(&out__, fmt_format_, __VA_ARGS__); \
		cr_assert(result__ == 0); \
		format_output_destroy(&out__); \
		fclose(f); \
		cr_assert(out__.nwritten == data_size, "Mismatch between output size and number of bytes written"); \
		if (data_size != (size_t)len || memcmp(data_buf, buf, data_size) != 0) \
		{ \
			printf("Got:\n"); \
			print_buffer(data_buf, data_size); \
			printf("Expected:\n"); \
			print_buffer(buf, (size_t)len); \
			cr_assert(0, "test %s", #fmt_format_ "/" #fmt_printf_); \
		} \
		else \
			cr_assert(1); \
		free(data_buf); \
	} \
	{ \
		char name[256]; \
		sprintf(name, "/fmt-test-%d", (int)getpid()); \
		int fd = shm_open(name, O_CLOEXEC | O_CREAT | O_EXCL | O_RDWR, 0600); \
		cr_assert(fd != -1, "Failed to open memory fd"); \
		shm_unlink(name); \
		struct format_output out__ = format_output_fd(fd); \
		const int result__ = format(&out__, fmt_format_, __VA_ARGS__); \
		cr_assert(result__ == 0); \
		format_output_destroy(&out__); \
		off_t size = lseek(fd, 0, SEEK_END); \
		lseek(fd, 0, SEEK_SET); \
		char *data = malloc((size_t)size); \
		read(fd, data, (size_t)size); \
		cr_assert(out__.nwritten == (size_t)size, "Mismatch between output size and number of bytes written"); \
		if ((size_t)size != (size_t)len || memcmp(data, buf, (size_t)size) != 0) \
		{ \
			printf("Got:\n"); \
			print_buffer(data, (size_t)size); \
			printf("Expected:\n"); \
			print_buffer(buf, (size_t)len); \
			cr_assert(0, "test %s", #fmt_format_ "/" #fmt_printf_); \
		} \
		else \
			cr_assert(1); \
		close(fd); \
		free(data); \
	} \
	FORMAT__END_DIAG(clang) \
	FORMAT__END_DIAG(gcc) \
} while (0)

#define test_manual(fmt_format_, expected_, ...) \
do { \
	FORMAT__START_DIAG(gcc) \
	FORMAT__START_DIAG(clang) \
	FORMAT__DIAG(gcc, ignored "-Wformat") \
	FORMAT__DIAG(clang, ignored "-Wformat") \
	const size_t len = strlen(expected_); \
	{ \
		struct format_output out__ = format_output_buf(); \
		const int result__ = format(&out__, fmt_format_, __VA_ARGS__); \
		cr_assert(result__ == 0); \
		if (out__.size != len || memcmp(out__.data, expected_, out__.size) != 0) \
		{ \
			printf("Got:\n"); \
			print_buffer(out__.data, out__.size); \
			printf("Expected:\n"); \
			print_buffer(expected_, len); \
			cr_assert(0, "test %s", #fmt_format_); \
		} \
		else \
			cr_assert(1); \
		format_output_destroy(&out__); \
	} \
	{ \
		char *data_buf = NULL; \
		size_t data_size = 0; \
		FILE *f = open_memstream(&data_buf, &data_size); \
		struct format_output out__ = format_output_file(f); \
		const int result__ = format(&out__, fmt_format_, __VA_ARGS__); \
		cr_assert(result__ == 0); \
		format_output_destroy(&out__); \
		fclose(f); \
		cr_assert(out__.nwritten == data_size); \
		if (data_size != (size_t)len || memcmp(data_buf, expected_, data_size) != 0) \
		{ \
			printf("Got:\n"); \
			print_buffer(data_buf, data_size); \
			printf("Expected:\n"); \
			print_buffer(expected_, (size_t)len); \
			cr_assert(0, "test %s", #fmt_format_); \
		} \
		else \
			cr_assert(1); \
		free(data_buf); \
	} \
	{ \
		char name[256]; \
		sprintf(name, "/fmt-test-%d", (int)getpid()); \
		int fd = shm_open(name, O_CLOEXEC | O_CREAT | O_EXCL | O_RDWR, 0600); \
		cr_assert(fd != -1, "Failed to open memory fd"); \
		shm_unlink(name); \
		struct format_output out__ = format_output_fd(fd); \
		const int result__ = format(&out__, fmt_format_, __VA_ARGS__); \
		cr_assert(result__ == 0); \
		format_output_destroy(&out__); \
		off_t size = lseek(fd, 0, SEEK_END); \
		lseek(fd, 0, SEEK_SET); \
		char *data = malloc((size_t)size); \
		read(fd, data, (size_t)size); \
		cr_assert(out__.nwritten == (size_t)size); \
		if ((size_t)size != (size_t)len || memcmp(data, expected_, (size_t)size) != 0) \
		{ \
			printf("Got:\n"); \
			print_buffer(data, (size_t)size); \
			printf("Expected:\n"); \
			print_buffer(expected_, (size_t)len); \
			cr_assert(0, "test %s", #fmt_format_); \
		} \
		else \
			cr_assert(1); \
		close(fd); \
		free(data); \
	} \
	FORMAT__END_DIAG(clang) \
	FORMAT__END_DIAG(gcc) \
} while (0)

#define CONCAT_(x, y) x ## y
#define CONCAT(x, y) CONCAT_(x, y)

#define for_each__(id_, type_, varname_, ...) \
	type_ CONCAT(foreach_array_, id_)[] = {__VA_ARGS__}; \
	type_ varname_ = CONCAT(foreach_array_, id_)[0]; \
	for (size_t idx_ = 0; idx_ < sizeof(CONCAT(foreach_array_, id_)) / sizeof(type_) && (varname_ = CONCAT(foreach_array_, id_)[idx_], 1); varname_ = CONCAT(foreach_array_, id_)[idx_++])
#define for_each_(id_, type_, varname_, ...) for_each__(id_, type_, varname_, __VA_ARGS__)
#define for_each(type_, varname_, ...) FORMAT__START_DIAG(clang) FORMAT__DIAG(clang, ignored "-Wc2y-extensions") for_each_(__COUNTER__, type_, varname_, __VA_ARGS__) FORMAT__END_DIAG(clang)

	
#endif // FORMAT_TESTS_UTIL_H
