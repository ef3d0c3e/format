#ifndef LIBFORMAT_H
#define LIBFORMAT_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

// TODO: Enforce 4096 max args
struct fmt_env
{
	/** @brief Stack allocated array of arguments */
	struct fmt_arg* args;
	/** @brief Number of arguments */
	size_t size;
};

struct fmt_arg
{
	/** @brief Custom formatter */
	int (*formatter)(struct format_output* output,
	                 const char* fmt_spec,
	                 const struct fmt_env*,
	                 size_t idx);
	/** @brief Raw data */
	uintptr_t data;
};

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

/**
 * @defgroup Formatters Default formatters
 * @{
 */

/**
 * @brief Format a `long` argument
 *
 * @param output Output to write to
 * @param fmt_spec Format specifier for this argument
 * @param env Format environment
 * @param idx Index of this argument in @p env
 *
 * @return 0 on success, -1 on errors
 */
int
format_fmt_long(struct format_output* output,
                const char* fmt_spec,
                const struct fmt_env* env,
                size_t idx);

// TEMP
int
format_fmt_long_long(struct format_output*, const char*, const struct fmt_env*, size_t);
int
format_fmt_int(struct format_output*, const char*, const struct fmt_env*, size_t);
int
format_fmt_short(struct format_output*, const char*, const struct fmt_env*, size_t);
int
format_fmt_char(struct format_output*, const char*, const struct fmt_env*, size_t);
int
format_fmt_unsigned_long_long(struct format_output*, const char*, const struct fmt_env*, size_t);
int
format_fmt_unsigned_long(struct format_output*, const char*, const struct fmt_env*, size_t);
int
format_fmt_unsigned_int(struct format_output*, const char*, const struct fmt_env*, size_t);
int
format_fmt_unsigned_short(struct format_output*, const char*, const struct fmt_env*, size_t);
int
format_fmt_unsigned_char(struct format_output*, const char*, const struct fmt_env*, size_t);
int
format_fmt_float(struct format_output*, const char*, const struct fmt_env*, size_t);
int
format_fmt_double(struct format_output*, const char*, const struct fmt_env*, size_t);
int
format_fmt_str(struct format_output*, const char*, const struct fmt_env*, size_t);

/** @} */

void
format_args(struct format_output* output, const char* fmt, const struct fmt_env env);

/**
 * @defgroup Macros Helper macros
 * @{
 */

#define FMT__EXPAND(...) __VA_ARGS__
#define FMT__EMPTY()
#define FMT__DEFER1(m) m FMT__EMPTY()
#define FMT__DEFER2(m) m FMT__EMPTY FMT__EMPTY()()
#define FMT__EVAL(...) FMT__EVAL1024(__VA_ARGS__)
#define FMT__EVAL1024(...) FMT__EVAL512(FMT__EVAL512(__VA_ARGS__))
#define FMT__EVAL512(...) FMT__EVAL256(FMT__EVAL256(__VA_ARGS__))
#define FMT__EVAL256(...) FMT__EVAL128(FMT__EVAL128(__VA_ARGS__))
#define FMT__EVAL128(...) FMT__EVAL64(FMT__EVAL64(__VA_ARGS__))
#define FMT__EVAL64(...) FMT__EVAL32(FMT__EVAL32(__VA_ARGS__))
#define FMT__EVAL32(...) FMT__EVAL16(FMT__EVAL16(__VA_ARGS__))
#define FMT__EVAL16(...) FMT__EVAL8(FMT__EVAL8(__VA_ARGS__))
#define FMT__EVAL8(...) FMT__EVAL4(FMT__EVAL4(__VA_ARGS__))
#define FMT__EVAL4(...) FMT__EVAL2(FMT__EVAL2(__VA_ARGS__))
#define FMT__EVAL2(...) FMT__EVAL1(FMT__EVAL1(__VA_ARGS__))
#define FMT__EVAL1(...) __VA_ARGS__

#define FMT__CAR(a, ...) a
#define FMT__CDR(a, ...) __VA_ARGS__
#define FMT__SECOND(a, b, ...) b

#define FMT__PROBE() ~, 1
#define FMT__IS_PROBE(...) FMT__SECOND(__VA_ARGS__, 0)
#define FMT__NOT(x) FMT__IS_PROBE(FMT__CAT(FMT___NOT_, x))
#define FMT___NOT_0 FMT__PROBE()
#define FMT__BOOL(x) FMT__NOT(FMT__NOT(x))

#define FMT__IF_ELSE(cond) FMT___IF_ELSE(FMT__BOOL(cond))
#define FMT___IF_ELSE(cond) FMT__CAT(FMT___IF_, cond)
#define FMT___IF_1(...) __VA_ARGS__ FMT___IF_1_ELSE
#define FMT___IF_0(...) FMT___IF_0_ELSE
#define FMT___IF_1_ELSE(...)
#define FMT___IF_0_ELSE(...) __VA_ARGS__

#define FMT__HAS_ARGS(...) FMT__BOOL(__VA_OPT__(1 +) 0)
#define FMT___END_OF_ARGS_() 0

#define FMT__CAT(a, b) a##b

#define FMT__MAP(__counter_base, __m, __first, ...)                                              \
	__m(__COUNTER__ - __counter_base - 1, __first) FMT__IF_ELSE(FMT__HAS_ARGS(__VA_ARGS__))(     \
	  FMT__DEFER2(FMT___MAP)()(__counter_base, __m, __VA_ARGS__))()
#define FMT___MAP() FMT__MAP

#define FMT__CONSUME(...)
#define FMT__IS_TUPLE(X) FMT__NOT(FMT__HAS_ARGS(FMT__CONSUME X))
#define FMT__TUPLE_FORMATTER(X) FMT__CAR(FMT__EXPAND X)
#define FMT__TUPLE_EXPR(X) FMT__CDR(FMT__EXPAND X)

#define FMT__ARG_EXPAND(X)                                                                       \
	FMT__IF_ELSE(FMT__IS_TUPLE(X))                                                               \
	(FMT__TUPLE_FORMATTER(X), FMT__TUPLE_EXPR(X))(NULL, X)
#define FMT___SELECT_0(X, ...) X
#define FMT___SELECT_1(X, ...) FMT___SELECT_0(__VA_ARGS__)
#define FMT__SELECT(N, ...) FMT___SELECT_##N(__VA_ARGS__)

#define FMT__MAPPER_DEFAULT(N, ARG)                                                              \
	__builtin_choose_expr(                                                                       \
	  __builtin_types_compatible_p(long long, typeof(ARG)),                                      \
	  ((struct fmt_arg){ format_fmt_long_long, 0 }),                                             \
	  __builtin_choose_expr(                                                                     \
	    __builtin_types_compatible_p(long, typeof(ARG)),                                         \
	    ((struct fmt_arg){ format_fmt_long, 0 }),                                                \
	    __builtin_choose_expr(                                                                   \
	      __builtin_types_compatible_p(int, typeof(ARG)),                                        \
	      ((struct fmt_arg){ format_fmt_int, 0 }),                                               \
	      __builtin_choose_expr(                                                                 \
	        __builtin_types_compatible_p(short, typeof(ARG)),                                    \
	        ((struct fmt_arg){ format_fmt_short, 0 }),                                           \
	        __builtin_choose_expr(                                                               \
	          __builtin_types_compatible_p(char, typeof(ARG)),                                   \
	          ((struct fmt_arg){ format_fmt_char, 0 }),                                          \
	          __builtin_choose_expr(                                                             \
	            __builtin_types_compatible_p(unsigned long long, typeof(ARG)),                   \
	            ((struct fmt_arg){ format_fmt_unsigned_long_long, 0 }),                          \
	            __builtin_choose_expr(                                                           \
	              __builtin_types_compatible_p(unsigned long, typeof(ARG)),                      \
	              ((struct fmt_arg){ format_fmt_unsigned_long, 0 }),                             \
	              __builtin_choose_expr(                                                         \
	                __builtin_types_compatible_p(unsigned int, typeof(ARG)),                     \
	                ((struct fmt_arg){ format_fmt_unsigned_int, 0 }),                            \
	                __builtin_choose_expr(                                                       \
	                  __builtin_types_compatible_p(unsigned short, typeof(ARG)),                 \
	                  ((struct fmt_arg){ format_fmt_unsigned_short, 0 }),                        \
	                  __builtin_choose_expr(                                                     \
	                    __builtin_types_compatible_p(unsigned char, typeof(ARG)),                \
	                    ((struct fmt_arg){ format_fmt_unsigned_char, 0 }),                       \
	                    __builtin_choose_expr(                                                   \
	                      __builtin_types_compatible_p(double, typeof(ARG)),                     \
	                      ((struct fmt_arg){ format_fmt_double, 0 }),                            \
	                      __builtin_choose_expr(                                                 \
	                        __builtin_types_compatible_p(float, typeof(ARG)),                    \
	                        ((struct fmt_arg){ format_fmt_float, 0 }),                           \
	                        __builtin_choose_expr(                                               \
	                          __builtin_types_compatible_p(const char*, typeof(ARG)) ||          \
	                            __builtin_types_compatible_p(char*, typeof(ARG)) ||              \
	                            __builtin_types_compatible_p(const char[], typeof(ARG)) ||       \
	                            __builtin_types_compatible_p(char[], typeof(ARG)),               \
	                          ((struct fmt_arg){ format_fmt_str, 0 }),                           \
	                          ((struct fmt_arg){ NULL, 0 }))))))))))))))
#define FMT__MAPPER(N, ARG)                                                                      \
	FMT__IF_ELSE(FMT__IS_TUPLE(ARG))(                                                            \
	  ((struct fmt_arg){ FMT__SELECT(0, FMT__ARG_EXPAND(ARG)), 0 }))(                            \
	  FMT__MAPPER_DEFAULT(N, FMT__SELECT(1, FMT__ARG_EXPAND(ARG)))),

#define FMT__SET(N, ARG)                                                                         \
	{                                                                                            \
		_Static_assert(sizeof(typeof(FMT__SELECT(1, FMT__ARG_EXPAND(ARG)))) <= sizeof(uint64_t), \
		               "Invalid argument type");                                                 \
		if (__builtin_types_compatible_p(const void*,                                            \
		                                 typeof(FMT__SELECT(1, FMT__ARG_EXPAND(ARG))))) {        \
			args[N].data = (uintptr_t)FMT__SELECT(1, FMT__ARG_EXPAND(ARG));                            \
		} else {                                                                                 \
			const typeof(FMT__SELECT(1, FMT__ARG_EXPAND(ARG))) temp =                            \
			  FMT__SELECT(1, FMT__ARG_EXPAND(ARG));                                              \
			memcpy(&args[N].data, &temp, sizeof(temp));                                          \
		}                                                                                        \
	}

#define format(output, fmt, ...)                                                                 \
	do {                                                                                         \
		_Static_assert(__builtin_types_compatible_p(typeof(output), struct format_output*),      \
		               "Invalid output type");                                                   \
		_Static_assert(__builtin_types_compatible_p(typeof(fmt), const char*) ||                 \
		                 __builtin_types_compatible_p(typeof(fmt), const char[]),                \
		               "Invalid format string");                                                 \
		struct fmt_arg args[] = { FMT__IF_ELSE(FMT__HAS_ARGS(__VA_ARGS__))(                      \
		  FMT__EXPAND(FMT__EVAL(FMT__MAP(0, FMT__MAPPER, __VA_ARGS__))))()(struct fmt_arg){      \
		  NULL, 0 } /* Sentinel */ };                                                            \
		enum                                                                                     \
		{                                                                                        \
			counter_base = __COUNTER__                                                           \
		};                                                                                       \
		FMT__IF_ELSE(FMT__HAS_ARGS(__VA_ARGS__))(                                                \
		  FMT__EXPAND(FMT__EVAL(FMT__MAP(counter_base, FMT__SET, __VA_ARGS__))))()               \
		  format_args(                                                                           \
		    output,                                                                              \
		    fmt,                                                                                 \
		    (const struct fmt_env){ .args = args, .size = sizeof(args) / sizeof(args[0]) - 1 }); \
	} while (0)

/** @} */

#endif // LIBFORMAT_H
