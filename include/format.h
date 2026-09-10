#ifndef LIBFORMAT_H
#define LIBFORMAT_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Diagnostic helpers */
#define FORMAT___DO_PRAGMA_(x) _Pragma(#x)
#define FORMAT___DO_PRAGMA(x) FORMAT___DO_PRAGMA_(x)
#if defined(__GNUC__) && !defined(__clang__)
#define FORMAT___START_DIAG_gcc FORMAT___DO_PRAGMA(GCC diagnostic push)
#define FORMAT___DIAG_gcc(diag) FORMAT___DO_PRAGMA(GCC diagnostic diag)
#define FORMAT___END_DIAG_gcc FORMAT___DO_PRAGMA(GCC diagnostic pop)
#else
#define FORMAT___START_DIAG_gcc
#define FORMAT___DIAG_gcc(diagnostic)
#define FORMAT___END_DIAG_gcc
#endif

#if defined(__clang__)
#define FORMAT___START_DIAG_clang FORMAT___DO_PRAGMA(clang diagnostic push)
#define FORMAT___DIAG_clang(diag) FORMAT___DO_PRAGMA(clang diagnostic diag)
#define FORMAT___END_DIAG_clang FORMAT___DO_PRAGMA(clang diagnostic pop)
#else
#define FORMAT___START_DIAG_clang
#define FORMAT___DIAG_clang(diagnostic)
#define FORMAT___END_DIAG_clang
#endif

#define FORMAT__START_DIAG(target) FORMAT___START_DIAG_##target
#define FORMAT__DIAG(target, diagnostic) FORMAT___DIAG_##target(diagnostic)
#define FORMAT__END_DIAG(target) FORMAT___END_DIAG_##target

// Format output buffer
struct format_output;

/**
 * @defgroup Style Format style
 * @{
 */

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

/** @} */

/**
 * @defgroup Arguments Format arguments
 * @{
 */

/**
 * @class format_env
 * @brief Holds the list of format arguments
 */
struct format_env
{
	/** @brief List of arguments */
	struct format_arg* args;
	/** @brief Number of arguments */
	size_t size;
};

/**
 * @class format_arg_collection
 * @brief Format argument for collections
 *
 * @ref next Is an iterator-like function that returns the current value of the collection and
 * advance the internal cursor
 * @ref width Return the total width required to format the collection, it muse use @ref formatter
 * @ref formatter Is the individual formater for the values inside the collection
 */
struct format_arg_collection
{
	/** @brief Opaque iterator state, managed by the caller */
	void* state;
	/** @brief Get the next element in the collection, NULL when exhausted */
	const void* (*next)(struct format_arg_collection* collection);
	size_t (*width)(struct format_arg_collection* collection, const char* fmt, size_t n);
	/** @brief Formatter for elements in the collection */
	int (*formatter)(struct format_output*, const char*, const struct format_env*, size_t);
	/** @brief `sizeof(array[0])` */
	size_t elem_size;
	/** @brief `true` if the values inside the array are pointers */
	int is_pointer;
};

/** @brief Type of format argument */
enum format_arg_type
{
	/** @brief Single value */
	kFormatScalar,
	/** @brief Collection of values */
	kFormatCollection,
};

/**
 * @class format_arg
 * @brief Individual format argument
 */
struct format_arg
{
	/** @brief Type of format argument */
	enum format_arg_type type;
	union
	{
		/** @brief Formatter */
		int (*formatter)(struct format_output* output,
		                 const char* fmt_spec,
		                 const struct format_env*,
		                 size_t idx);
		/** @brief Collector accessor */
		struct format_arg_collection collection;
	};
	/** @brief Raw data, value to format */
	uintptr_t data;
};

/** @} */

/**
 * @defgroup Output Output buffer
 * @{
 */

struct format_output
{
	/** @brief Foreground color */
	format_color fg;
	/** @brief Background color */
	format_color bg;
	/** @brief Output style */
	enum format_output_style style;

	/** @brief Output file descriptor, `-1` for none */
	int fd;
	/** @brief Stdio `FILE` output, `NULL` for none */
	FILE* file;
	/** @brief Flushing mode */
	enum format_output_flush_mode flush_mode;

	/** @brief Buffer data */
	char* data;
	/** @brief Buffer size */
	size_t size;
	/** @brief Buffer allocated capacity */
	size_t capacity;

	// Allocators
	void* (*malloc)(size_t);
	void (*free)(void*, size_t);
	void* (*realloc)(void*, size_t, size_t);

	// TODO: Store target color/style rendering data
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
 * @brief Create a new @ref format_output that keep track of size but does not write anything.
 * Calling @ref format_output_destroy on the returned value does nothing.
 *
 * @return A new @ref format_output that will only store size
 */
struct format_output
format_output_none(void);
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

/** @} */

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
                const struct format_env* env,
                size_t idx);

// TEMP
int
format_fmt_long_long(struct format_output*, const char*, const struct format_env*, size_t);
int
format_fmt_int(struct format_output*, const char*, const struct format_env*, size_t);
int
format_fmt_short(struct format_output*, const char*, const struct format_env*, size_t);
int
format_fmt_char(struct format_output*, const char*, const struct format_env*, size_t);
int
format_fmt_unsigned_long_long(struct format_output*,
                              const char*,
                              const struct format_env*,
                              size_t);
int
format_fmt_unsigned_long(struct format_output*, const char*, const struct format_env*, size_t);
int
format_fmt_unsigned_int(struct format_output*, const char*, const struct format_env*, size_t);
int
format_fmt_unsigned_short(struct format_output*, const char*, const struct format_env*, size_t);
int
format_fmt_signed_char(struct format_output*, const char*, const struct format_env*, size_t);
int
format_fmt_unsigned_char(struct format_output*, const char*, const struct format_env*, size_t);
int
format_fmt_float(struct format_output*, const char*, const struct format_env*, size_t);
int
format_fmt_double(struct format_output*, const char*, const struct format_env*, size_t);
int
format_fmt_str(struct format_output*, const char*, const struct format_env*, size_t);

/** @} */

void
format_args(struct format_output* output, const char* fmt, const struct format_env env);

/**
 * @defgroup Macros Helper macros
 * @{
 */

#define FORMAT__EXPAND(...) __VA_ARGS__
#define FORMAT__EXPAND2(...) __VA_ARGS__
#define FORMAT__EMPTY()
#define FORMAT__DEFER1(m) m FORMAT__EMPTY()
#define FORMAT__DEFER2(m) m FORMAT__EMPTY FORMAT__EMPTY()()

#define FORMAT__EVAL(...) FORMAT__EVAL1024(__VA_ARGS__)
#define FORMAT__EVAL1024(...) FORMAT__EVAL512(FORMAT__EVAL512(__VA_ARGS__))
#define FORMAT__EVAL512(...) FORMAT__EVAL256(FORMAT__EVAL256(__VA_ARGS__))
#define FORMAT__EVAL256(...) FORMAT__EVAL128(FORMAT__EVAL128(__VA_ARGS__))
#define FORMAT__EVAL128(...) FORMAT__EVAL64(FORMAT__EVAL64(__VA_ARGS__))
#define FORMAT__EVAL64(...) FORMAT__EVAL32(FORMAT__EVAL32(__VA_ARGS__))
#define FORMAT__EVAL32(...) FORMAT__EVAL16(FORMAT__EVAL16(__VA_ARGS__))
#define FORMAT__EVAL16(...) FORMAT__EVAL8(FORMAT__EVAL8(__VA_ARGS__))
#define FORMAT__EVAL8(...) FORMAT__EVAL4(FORMAT__EVAL4(__VA_ARGS__))
#define FORMAT__EVAL4(...) FORMAT__EVAL2(FORMAT__EVAL2(__VA_ARGS__))
#define FORMAT__EVAL2(...) FORMAT__EVAL1(FORMAT__EVAL1(__VA_ARGS__))
#define FORMAT__EVAL1(...) __VA_ARGS__

#define FORMAT__EVAL_T(...) FORMAT__EVAL_T1024(__VA_ARGS__)
#define FORMAT__EVAL_T1024(...) FORMAT__EVAL_T512(FORMAT__EVAL_T512(__VA_ARGS__))
#define FORMAT__EVAL_T512(...) FORMAT__EVAL_T256(FORMAT__EVAL_T256(__VA_ARGS__))
#define FORMAT__EVAL_T256(...) FORMAT__EVAL_T128(FORMAT__EVAL_T128(__VA_ARGS__))
#define FORMAT__EVAL_T128(...) FORMAT__EVAL_T64(FORMAT__EVAL_T64(__VA_ARGS__))
#define FORMAT__EVAL_T64(...) FORMAT__EVAL_T32(FORMAT__EVAL_T32(__VA_ARGS__))
#define FORMAT__EVAL_T32(...) FORMAT__EVAL_T16(FORMAT__EVAL_T16(__VA_ARGS__))
#define FORMAT__EVAL_T16(...) FORMAT__EVAL_T8(FORMAT__EVAL_T8(__VA_ARGS__))
#define FORMAT__EVAL_T8(...) FORMAT__EVAL_T4(FORMAT__EVAL_T4(__VA_ARGS__))
#define FORMAT__EVAL_T4(...) FORMAT__EVAL_T2(FORMAT__EVAL_T2(__VA_ARGS__))
#define FORMAT__EVAL_T2(...) FORMAT__EVAL_T1(FORMAT__EVAL_T1(__VA_ARGS__))
#define FORMAT__EVAL_T1(...) __VA_ARGS__

#define FORMAT__EVAL_O(...) FORMAT__EVAL_O1024(__VA_ARGS__)
#define FORMAT__EVAL_O1024(...) FORMAT__EVAL_O512(FORMAT__EVAL_O512(__VA_ARGS__))
#define FORMAT__EVAL_O512(...) FORMAT__EVAL_O256(FORMAT__EVAL_O256(__VA_ARGS__))
#define FORMAT__EVAL_O256(...) FORMAT__EVAL_O128(FORMAT__EVAL_O128(__VA_ARGS__))
#define FORMAT__EVAL_O128(...) FORMAT__EVAL_O64(FORMAT__EVAL_O64(__VA_ARGS__))
#define FORMAT__EVAL_O64(...) FORMAT__EVAL_O32(FORMAT__EVAL_O32(__VA_ARGS__))
#define FORMAT__EVAL_O32(...) FORMAT__EVAL_O16(FORMAT__EVAL_O16(__VA_ARGS__))
#define FORMAT__EVAL_O16(...) FORMAT__EVAL_O8(FORMAT__EVAL_O8(__VA_ARGS__))
#define FORMAT__EVAL_O8(...) FORMAT__EVAL_O4(FORMAT__EVAL_O4(__VA_ARGS__))
#define FORMAT__EVAL_O4(...) FORMAT__EVAL_O2(FORMAT__EVAL_O2(__VA_ARGS__))
#define FORMAT__EVAL_O2(...) FORMAT__EVAL_O1(FORMAT__EVAL_O1(__VA_ARGS__))
#define FORMAT__EVAL_O1(...) __VA_ARGS__

#define FORMAT__CAR(a, ...) a
#define FORMAT__CDR(a, ...) __VA_ARGS__
#define FORMAT__CDR2(...) FORMAT__CDR(__VA_ARGS__)
#define FORMAT__SECOND(a, b, ...) b

#define FORMAT__PROBE() ~, 1
#define FORMAT__IS_PROBE(...) FORMAT__SECOND(__VA_ARGS__, 0)
#define FORMAT__NOT(x) FORMAT__IS_PROBE(FORMAT__CAT(FORMAT___NOT_, x))
#define FORMAT___NOT_0 FORMAT__PROBE()
#define FORMAT__BOOL(x) FORMAT__NOT(FORMAT__NOT(x))

#define FORMAT___IF_ELSE(cond) FORMAT__CAT(FORMAT___IF_, cond)
#define FORMAT___IF_1(...) __VA_ARGS__ FORMAT___IF_1_ELSE
#define FORMAT___IF_0(...) FORMAT___IF_0_ELSE
#define FORMAT___IF_1_ELSE(...)
#define FORMAT___IF_0_ELSE(...) __VA_ARGS__
#define FORMAT__IF_ELSE(cond) FORMAT___IF_ELSE(FORMAT__BOOL(cond))

#define FORMAT___END_OF_ARGS_() 0
#define FORMAT__HAS_ARGS(...) FORMAT__BOOL(__VA_OPT__(1 +) 0)

#define FORMAT___CAT(a, b) a##b
#define FORMAT__CAT(a, b) FORMAT___CAT(a, b)
#define FORMAT___STRINGIFY(x) #x
#define FORMAT__STRINGIFY(x) FORMAT___STRINGIFY(x)

#define FORMAT___MAP() FORMAT__MAP
#define FORMAT__MAP(__m, __first, ...)                                                           \
	__m(__first) FORMAT__IF_ELSE(FORMAT__HAS_ARGS(__VA_ARGS__))(                                 \
	  FORMAT__DEFER2(FORMAT___MAP)()(__m, __VA_ARGS__))()

#define FORMAT___MAP_CONST() FORMAT__MAP_CONST
#define FORMAT__MAP_CONST(__m, __const, __first, ...)                                            \
	__m(__const, __first) FORMAT__IF_ELSE(FORMAT__HAS_ARGS(__VA_ARGS__))(                        \
	  FORMAT__DEFER2(FORMAT___MAP_CONST)()(__m, __const, __VA_ARGS__))()

#define FORMAT___MAP_CONST_N() FORMAT__MAP_CONST_N
#define FORMAT__MAP_CONST_N(__counter_base, __m, __const, __first, ...)                          \
	__m(__COUNTER__ - __counter_base - 1, __const, __first)                                      \
	  FORMAT__IF_ELSE(FORMAT__HAS_ARGS(__VA_ARGS__))(                                            \
	    FORMAT__DEFER2(FORMAT___MAP_CONST_N)()(__counter_base, __m, __const, __VA_ARGS__))()

#define FORMAT__CONSUME(...)

#define FORMAT___IS_PAIR_1(first, ...)                                                           \
	FORMAT__IF_ELSE(FORMAT__HAS_ARGS(__VA_ARGS__))(0)(FORMAT___IS_PAIR_2(first))
#define FORMAT___IS_PAIR_2_IS_TUPLE(X) FORMAT__IS_PROBE(FORMAT___IS_PAIR_2_TUPLE_PROBE X)
#define FORMAT___IS_PAIR_2_TUPLE_PROBE(...) FORMAT__PROBE()
#define FORMAT___IS_PAIR_2(X)                                                                    \
	FORMAT__IF_ELSE(FORMAT___IS_PAIR_2_IS_TUPLE(X))(FORMAT___IS_PAIR_3 X)(0)
#define FORMAT___IS_PAIR_3(a, ...)                                                               \
	FORMAT__IF_ELSE(FORMAT__HAS_ARGS(__VA_ARGS__))(FORMAT___IS_PAIR_4(__VA_ARGS__))(0)
#define FORMAT___IS_PAIR_4(b, ...) FORMAT__NOT(FORMAT__HAS_ARGS(__VA_ARGS__))
#define FORMAT__IS_PAIR(...) FORMAT___IS_PAIR_1(__VA_ARGS__)

_Static_assert(!FORMAT__IS_PAIR());
_Static_assert(!FORMAT__IS_PAIR(0));
_Static_assert(!FORMAT__IS_PAIR(0, 1));
_Static_assert(!FORMAT__IS_PAIR(0, 1, 2));
_Static_assert(!FORMAT__IS_PAIR(()));
_Static_assert(!FORMAT__IS_PAIR((0)));
_Static_assert(FORMAT__IS_PAIR((0, 1)));
_Static_assert(!FORMAT__IS_PAIR((0, 1, 2)));

#define FORMAT___IS_TRIPLET_1(first, ...)                                                        \
	FORMAT__IF_ELSE(FORMAT__HAS_ARGS(__VA_ARGS__))(0)(FORMAT___IS_TRIPLET_2(first))
#define FORMAT___IS_TRIPLET_2_IS_TUPLE(X) FORMAT__IS_PROBE(FORMAT___IS_TRIPLET_2_TUPLE_PROBE X)
#define FORMAT___IS_TRIPLET_2_TUPLE_PROBE(...) FORMAT__PROBE()
#define FORMAT___IS_TRIPLET_2(X)                                                                 \
	FORMAT__IF_ELSE(FORMAT___IS_TRIPLET_2_IS_TUPLE(X))(FORMAT___IS_TRIPLET_3 X)(0)
#define FORMAT___IS_TRIPLET_3(a, ...)                                                            \
	FORMAT__IF_ELSE(FORMAT__HAS_ARGS(__VA_ARGS__))(FORMAT___IS_TRIPLET_4(__VA_ARGS__))(0)
#define FORMAT___IS_TRIPLET_4(b, ...)                                                            \
	FORMAT__IF_ELSE(FORMAT__HAS_ARGS(__VA_ARGS__))(FORMAT___IS_TRIPLET_5(__VA_ARGS__))(0)
#define FORMAT___IS_TRIPLET_5(c, ...) FORMAT__NOT(FORMAT__HAS_ARGS(__VA_ARGS__))
#define FORMAT__IS_TRIPLET(...) FORMAT___IS_TRIPLET_1(__VA_ARGS__)

_Static_assert(!FORMAT__IS_TRIPLET());
_Static_assert(!FORMAT__IS_TRIPLET(0));
_Static_assert(!FORMAT__IS_TRIPLET(0, 1));
_Static_assert(!FORMAT__IS_TRIPLET(0, 1, 2));
_Static_assert(!FORMAT__IS_TRIPLET(0, 1, 2, 4));
_Static_assert(!FORMAT__IS_TRIPLET(()));
_Static_assert(!FORMAT__IS_TRIPLET((0)));
_Static_assert(!FORMAT__IS_TRIPLET((0, 1)));
_Static_assert(FORMAT__IS_TRIPLET((0, 1, 2)));
_Static_assert(!FORMAT__IS_TRIPLET((0, 1, 2, 4)));

#define FORMAT__IS_POINTER_VAR_P(VAR)                                                            \
	(__builtin_classify_type(VAR) == __builtin_classify_type((void*)0))

#define FORMAT___SELECT__GET_0(a0, ...) a0
#define FORMAT___SELECT__GET_1(a0, a1, ...) a1
#define FORMAT___SELECT__GET_2(a0, a1, a2, ...) a2
#define FORMAT___SELECT__GET_3(a0, a1, a2, a3, ...) a3
#define FORMAT___SELECT__GET_4(a0, a1, a2, a3, a4, ...) a4
#define FORMAT___SELECT__GET_5(a0, a1, a2, a3, a4, a5, ...) a5
#define FORMAT___SELECT__GET_6(a0, a1, a2, a3, a4, a5, a6, ...) a6
#define FORMAT___SELECT__GET_7(a0, a1, a2, a3, a4, a5, a6, a7, ...) a7
#define FORMAT___SELECT_GET(n, tuple) FORMAT__CAT(FORMAT___SELECT__GET_, n) tuple

#define FORMAT___SELECT_NARG(...) FORMAT___SELECT__NARG(__VA_ARGS__, FORMAT___SELECT_RSEQ_N())
#define FORMAT___SELECT__NARG(...) FORMAT__EXPAND(FORMAT___SELECT_ARG_N(__VA_ARGS__))
#define FORMAT___SELECT_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define FORMAT___SELECT_RSEQ_N() 8, 7, 6, 5, 4, 3, 2, 1, 0

#define FORMAT___SELECT_2(N, T) FORMAT___SELECT_GET(N, T)
#define FORMAT___SELECT_3(N1, N2, T) FORMAT___SELECT_GET(N2, FORMAT___SELECT_GET(N1, T))
#define FORMAT___SELECT_4(N1, N2, N3, T)                                                         \
	FORMAT___SELECT_GET(N3, FORMAT___SELECT_GET(N2, FORMAT___SELECT_GET(N1, T)))
#define FORMAT___SELECT_5(N1, N2, N3, N4, T)                                                     \
	FORMAT___SELECT_GET(                                                                         \
	  N4, FORMAT___SELECT_GET(N3, FORMAT___SELECT_GET(N2, FORMAT___SELECT_GET(N1, T))))
#define FORMAT___SELECT_6(N1, N2, N3, N4, N5, T)                                                 \
	FORMAT___SELECT_GET(                                                                         \
	  N5,                                                                                        \
	  FORMAT___SELECT_GET(                                                                       \
	    N4, FORMAT___SELECT_GET(N3, FORMAT___SELECT_GET(N2, FORMAT___SELECT_GET(N1, T)))))
#define FORMAT___SELECT_7(N1, N2, N3, N4, N5, N6, T)                                             \
	FORMAT___SELECT_GET(                                                                         \
	  N6,                                                                                        \
	  FORMAT___SELECT_GET(                                                                       \
	    N5,                                                                                      \
	    FORMAT___SELECT_GET(                                                                     \
	      N4, FORMAT___SELECT_GET(N3, FORMAT___SELECT_GET(N2, FORMAT___SELECT_GET(N1, T))))))
#define FORMAT___SELECT_8(N1, N2, N3, N4, N5, N6, N7, T)                                         \
	FORMAT___SELECT_GET(                                                                         \
	  N7,                                                                                        \
	  FORMAT___SELECT_GET(                                                                       \
	    N6,                                                                                      \
	    FORMAT___SELECT_GET(                                                                     \
	      N5,                                                                                    \
	      FORMAT___SELECT_GET(                                                                   \
	        N4, FORMAT___SELECT_GET(N3, FORMAT___SELECT_GET(N2, FORMAT___SELECT_GET(N1, T)))))))

#define FORMAT__SELECT(...)                                                                      \
	FORMAT__EXPAND(FORMAT__CAT(FORMAT___SELECT_, FORMAT___SELECT_NARG(__VA_ARGS__))(__VA_ARGS__))

_Static_assert(FORMAT___SELECT_GET(0, (7, 8, 9)) == 7);
_Static_assert(FORMAT___SELECT_GET(1, (7, 8, 9)) == 8);
_Static_assert(FORMAT___SELECT_GET(2, (7, 8, 9)) == 9);

_Static_assert(FORMAT__SELECT(0, (10, 20, 30, 40)) == 10);
_Static_assert(FORMAT__SELECT(1, (10, 20, 30, 40)) == 20);
_Static_assert(FORMAT__SELECT(2, (10, 20, 30, 40)) == 30);
_Static_assert(FORMAT__SELECT(3, (10, 20, 30, 40)) == 40);

_Static_assert(FORMAT__SELECT(1, 0, (100, (200, 300))) == 200);
_Static_assert(FORMAT__SELECT(1, 1, (100, (200, 300))) == 300);
_Static_assert(FORMAT__SELECT(0, 0, ((1, 2), (3, 4))) == 1);
_Static_assert(FORMAT__SELECT(0, 1, ((1, 2), (3, 4))) == 2);

_Static_assert(FORMAT__SELECT(1, 1, 0, (1, (2, (3, 4)))) == 3);
_Static_assert(FORMAT__SELECT(1, 1, 1, (1, (2, (3, 4)))) == 4);

_Static_assert(FORMAT__SELECT(1, 1, 1, 0, (1, (2, (3, (4, 5))))) == 4);
_Static_assert(FORMAT__SELECT(1, 1, 1, 1, (1, (2, (3, (4, 5))))) == 5);

#define FORMAT___HAS_ARG_0 (0, 0, 0, 0, 0, 0, 0, 0)
#define FORMAT___HAS_ARG_1 (1, 0, 0, 0, 0, 0, 0, 0)
#define FORMAT___HAS_ARG_2 (1, 1, 0, 0, 0, 0, 0, 0)
#define FORMAT___HAS_ARG_3 (1, 1, 1, 0, 0, 0, 0, 0)
#define FORMAT___HAS_ARG_4 (1, 1, 1, 1, 0, 0, 0, 0)
#define FORMAT___HAS_ARG_5 (1, 1, 1, 1, 1, 0, 0, 0)
#define FORMAT___HAS_ARG_6 (1, 1, 1, 1, 1, 1, 0, 0)
#define FORMAT___HAS_ARG_7 (1, 1, 1, 1, 1, 1, 1, 0)
#define FORMAT___HAS_ARG_8 (1, 1, 1, 1, 1, 1, 1, 1)

#define FORMAT__HAS_ARG(N, ...)                                                                  \
	FORMAT___SELECT_GET(N, FORMAT__CAT(FORMAT___HAS_ARG_, FORMAT___SELECT_NARG(__VA_ARGS__)))

_Static_assert(!FORMAT__HAS_ARG(3));

_Static_assert(FORMAT__HAS_ARG(0, 42));
_Static_assert(!FORMAT__HAS_ARG(1, 42));

_Static_assert(FORMAT__HAS_ARG(0, 1, 2, 3));
_Static_assert(FORMAT__HAS_ARG(1, 1, 2, 3));
_Static_assert(FORMAT__HAS_ARG(2, 1, 2, 3));
_Static_assert(!FORMAT__HAS_ARG(3, 1, 2, 3));
_Static_assert(!FORMAT__HAS_ARG(7, 1, 2, 3));

_Static_assert(FORMAT__HAS_ARG(7, 0, 1, 2, 3, 4, 5, 6, 7));
_Static_assert(!FORMAT__HAS_ARG(7, 0, 1, 2, 3, 4, 5, 6));

_Static_assert(FORMAT__HAS_ARG(1, (1, 2), (3, 4)));
_Static_assert(!FORMAT__HAS_ARG(2, (1, 2), (3, 4)));
_Static_assert(FORMAT__HAS_ARG(2, (1, 2), 5, (3, 4)));

/** @} */

#define FORMAT__MAPPER_CHOOSE(ARG, RULE)                                                         \
	__builtin_choose_expr(__builtin_types_compatible_p(FORMAT__SELECT(0, RULE), typeof(ARG)),    \
	                      format_arg__.formatter = FORMAT__SELECT(1, RULE),                      \
	                      (void)0);
#define FORMAT__CHOOSE(ARG)                                                                      \
	FORMAT__EXPAND(                                                                              \
	  FORMAT__EVAL_T(FORMAT__MAP_CONST(FORMAT__MAPPER_CHOOSE,                                    \
	                                   ARG,                                                      \
	                                   (long long, format_fmt_long_long),                        \
	                                   (long, format_fmt_long),                                  \
	                                   (int, format_fmt_int),                                    \
	                                   (short, format_fmt_short),                                \
	                                   (signed char, format_fmt_signed_char),                    \
	                                   (unsigned long long, format_fmt_unsigned_long_long),      \
	                                   (unsigned long, format_fmt_unsigned_long),                \
	                                   (unsigned int, format_fmt_unsigned_int),                  \
	                                   (unsigned short, format_fmt_unsigned_short),              \
	                                   (unsigned char, format_fmt_unsigned_char),                \
	                                   (float, format_fmt_float),                                \
	                                   (double, format_fmt_double),                              \
	                                   (char, format_fmt_char),                                  \
	                                   (const char*, format_fmt_str),                            \
	                                   (const char[], format_fmt_str),                           \
	                                   (char*, format_fmt_str),                                  \
	                                   (char[], format_fmt_str))))

#define FORMAT__FORMATTER_TRIPLET_COLLECTION(DATA, FIELD)                                        \
	format_arg__.type = kFormatCollection;                                                       \
	typeof(*FIELD)* format_arg_triplet__ = FIELD;                                                \
	(void)format_arg_triplet__;                                                                  \
	format_arg__.collection = (DATA);                                                            \
	assert(format_arg__.collection.next != NULL);                                                \
	assert(format_arg__.collection.width != NULL);                                               \
	assert(format_arg__.collection.formatter != NULL);                                           \
	format_arg__.data = (uintptr_t)(FIELD);
#define FORMAT__FORMATTER_TRIPLET(TAG, X, Y) FORMAT__CAT(FORMAT__FORMATTER_TRIPLET_, TAG)(X, Y)

#define FORMAT__MAPPER_VALUE(ARG)                                                                \
	FORMAT__IF_ELSE(FORMAT__IS_TRIPLET(ARG))(FORMAT__SELECT(2, ARG))(                            \
	  FORMAT__IF_ELSE(FORMAT__IS_PAIR(ARG))(FORMAT__SELECT(1, ARG))(ARG))

#define FORMAT__MAPPER(ARG)                                                                      \
	__extension__({                                                                              \
		_Static_assert((FORMAT__IS_POINTER_VAR_P(FORMAT__MAPPER_VALUE(ARG)) ||                   \
		                sizeof(FORMAT__MAPPER_VALUE(ARG)) <= sizeof(uint64_t)) &&                \
		               "Cannot format type, did you mean to use a pointer instead?");            \
		struct format_arg format_arg__;                                                          \
		format_arg__.type = kFormatScalar;                                                       \
		FORMAT__IF_ELSE(FORMAT__IS_TRIPLET(ARG))(FORMAT__FORMATTER_TRIPLET(                      \
		  FORMAT__SELECT(0, ARG), FORMAT__SELECT(1, ARG), FORMAT__SELECT(2, ARG)))(              \
		  FORMAT__IF_ELSE(FORMAT__IS_PAIR(ARG))(format_arg__.formatter =                         \
		                                          FORMAT__SELECT(0, ARG))(FORMAT__CHOOSE(ARG))); \
		FORMAT__IF_ELSE(                                                                         \
		  FORMAT__IS_TRIPLET(ARG))()(/* respect strict-aliasing */                               \
		                             assert(format_arg__.formatter != NULL &&                    \
		                                    "Could not find formatter for argument"));           \
		FORMAT__IF_ELSE(FORMAT__IS_TRIPLET(ARG))()((void)__builtin_choose_expr(                  \
		  FORMAT__IS_POINTER_VAR_P(FORMAT__MAPPER_VALUE(ARG)),                                   \
		  __extension__({                                                                        \
			  format_arg__.data = (uintptr_t)FORMAT__MAPPER_VALUE(ARG);                          \
			  0;                                                                                 \
		  }),                                                                                    \
		  __extension__({                                                                        \
			  const typeof(FORMAT__MAPPER_VALUE(ARG)) format_temp__ = FORMAT__MAPPER_VALUE(ARG); \
			  format_arg__.data = 0;                                                             \
			  memcpy(&format_arg__.data, &format_temp__, sizeof(format_temp__));                 \
			  0;                                                                                 \
		  })));                                                                                  \
		format_arg__;                                                                            \
	}),

#define format(output, fmt, ...)                                                                 \
	do {                                                                                         \
		FORMAT__START_DIAG(clang)                                                                \
		FORMAT__DIAG(clang, ignored "-Wc2y-extensions")                                          \
		_Static_assert(__builtin_types_compatible_p(typeof(output), struct format_output*),      \
		               "Invalid output type");                                                   \
		_Static_assert(__builtin_types_compatible_p(typeof(fmt), const char*) ||                 \
		                 __builtin_types_compatible_p(typeof(fmt), const char[]),                \
		               "Invalid format string");                                                 \
		struct format_arg format_args__[] = { FORMAT__IF_ELSE(FORMAT__HAS_ARGS(__VA_ARGS__))(    \
		  FORMAT__EXPAND(FORMAT__EVAL(FORMAT__MAP(FORMAT__MAPPER, __VA_ARGS__))))() };           \
		format_args(output,                                                                      \
		            fmt,                                                                         \
		            (const struct format_env){ .args = format_args__,                            \
		                                       .size = sizeof(format_args__) /                   \
		                                               sizeof(format_args__[0]) });              \
		FORMAT__END_DIAG(clang)                                                                  \
	} while (0)

/**
 * @defgroup CustomFormat Custom formatters
 * @{
 */

/**
 * @class format_collection_array_cursor
 * @brief Array formatter cursor
 */
struct format_collection_array_cursor
{
	const void* cur;
};

static inline const void*
format__collection_array_next(struct format_arg_collection* collection)
{
	struct format_collection_array_cursor* c = collection->state;
	const void* p = c->cur;
	c->cur = (char*)c->cur + collection->elem_size;
	return p;
}

static inline size_t
format__collection_array_width(struct format_arg_collection* collection,
                               const char* fmt,
                               size_t n)
{
	struct format_collection_array_cursor c =
	  *(struct format_collection_array_cursor*)collection->state;

	struct format_arg arg;
	arg.type = kFormatScalar;
	arg.formatter = collection->formatter;

	struct format_env env = {
		.args = (struct format_arg*)&arg,
		.size = 1,
	};

	struct format_output out = format_output_none();
	for (size_t i = 0; i < n; ++i) {
		const void* val = c.cur;

		if (collection->is_pointer)
			arg.data = *(uintptr_t*)val;
		else
			memcpy(&arg.data, val, collection->elem_size);
		collection->formatter(&out, fmt, &env, 0);
		c.cur = (char*)c.cur + collection->elem_size;
	}

	return out.size;
}

#define FORMAT__ARRAY_0(ARRAY)                                                                   \
	(COLLECTION,                                                                                 \
	 ((struct format_arg_collection){                                                            \
	   .state = &(struct format_collection_array_cursor){ .cur = (format_arg_triplet__) },       \
	   .next = format__collection_array_next,                                                    \
	   .width = format__collection_array_width,                                                  \
	   .formatter = __extension__({ format_fmt_int; }),                                          \
	   .elem_size = sizeof(*(format_arg_triplet__)),                                             \
	   .is_pointer = FORMAT__IS_POINTER_VAR_P(*(format_arg_triplet__)) }),                       \
	 ARRAY)
#define FORMAT__ARRAY_1(ARRAY, FORMATTER)                                                        \
	(COLLECTION,                                                                                 \
	 ((struct format_arg_collection){                                                            \
	   .state = &(struct format_collection_array_cursor){ .cur = (format_arg_triplet__) },       \
	   .next = format__collection_array_next,                                                    \
	   .width = format__collection_array_width,                                                  \
	   .formatter = FORMATTER,                                                                   \
	   .elem_size = sizeof(*(format_arg_triplet__)),                                             \
	   .is_pointer = FORMAT__IS_POINTER_VAR_P(*(format_arg_triplet__)) }),                       \
	 ARRAY)

/**
 * @brief Format macro for arrays
 *
 * @param ARRAY Array to format
 * @param ... (optional) Custom formatter for elements in @p ARRAY
 */
#define FORMAT_ARRAY(ARRAY, ...)                                                                 \
	FORMAT__IF_ELSE(FORMAT__HAS_ARGS(__VA_ARGS__))(FORMAT__ARRAY_1(ARRAY, __VA_ARGS__))(         \
	  FORMAT__ARRAY_0(ARRAY))

/** @} */

/**
 * @defgroup Grammar Format expression grammar
 * @{
 *
 * @anchor grammar_number
 * # Number
 *
 * \gb{number} \gb{ := } \gb{ integer } <i>parse literally from the format string</i> <br>
 * \gi{ } \gi{ } \gb{| '\{' integer '\}'} <i>parse integer and retrieve the corresponding argument
 * in the argument list</i>
 *
 * A number is a positive integer parsed directly from the format string or from the list of
 * arguments.
 *
 * #### Examples
 *
 *  - `format(out, "{:.5}", "Hello, World")`
 *  - `format(out, "{:15}", "Hello, World")`
 *  - `format(out, "{:.{1}}", "Hello, World", 4)`
 *  - `format(out, "{:[{1}]}", FORMAT_ARRAY(array), sizeof(array) / sizeof(array[0]))`
 *
 * @anchor grammar_size
 * # Size
 *
 * Parsing rules for size are the same as @ref grammar_number "number", except that they are
 * limited to 16384. When referring to *width*, they commonly refer to the number of UTF-8
 * codepoints, and not the number of bytes.
 *
 * @anchor grammar_spec
 * # Specifier
 *
 * \gb{spec} \gb{ := } \gb{ codepoint } <i>parse literally from the format string</i> <br>
 * \gi{ } \gi{ } \gb{| '\{' integer '\}'} <i>parse integer and retrieve the corresponding argument
 * in the argument list</i>
 *
 * A specifier is a literal string that can either be present as a single UTF-8 codepoint in the
 * format string, or a string retrieved from the list of arguments.
 *
 * #### Examples
 *
 *  - `format(out, "{:#[]}", "Hello") -> "[Hello]"`
 *  - `format(out, "{:#''}", "Hello") -> "'Hello'"`
 *  - `format(out, "{:{1}<10}", 15, ". ") -> "15. . . . "`
 *
 * @anchor grammar_alignment
 * # Alignment
 *
 * \gb{alignment} \gb{ := } \gto{spec, fill string} \gb{'<'} <i>left-aligned</i> <br>
 * \gi{ } \gi{ } \gb{|} \gto{spec, fill string} \gb{'>'} <i>right-aligned</i> <br>
 * \gi{ } \gi{ } \gb{|} \gto{spec, fill string} \gb{'~'} <i>center-aligned</i>
 *
 * Define the alignment mode and set the fill string for a formatted value.
 *
 * #### Examples
 *
 *  - `format(out, "{:>10}", "Hello") -> "     Hello"`
 *  - `format(out, "{:<10}", "Hello") -> "Hello     "`
 *  - `format(out, "{:^10}", "Hello") -> "   Hello  "`
 *  - `format(out, "{:-^10}", "Hello") -> "---Hello--"`
 *  - `format(out, "{:{1}>10}", "Hello", "-_") -> "-_-_-Hello"`
 *
 * # Type formatting
 *
 * ## Scalar
 *
 * @anchor grammar_format_string
 * ### String
 *
 * \gb{format_string} := \gto{alignment, alignment} <br>
 * \gi{ } \gto{size, width of the formatted string} <br>
 * \gi{ } (\gb{'#'} <br>
 * \gi{ } \gi{ } \gt{spec, right quote} <br>
 * \gi{ } \gi{ } \gt{spec, left quote} <br>
 * \gi{ } )? <br>
 * \gi{ } (\gb{'.'} <br>
 * \gi{ } \gi{ } \gt{number, precision\, maximum number of BYTES to display} <br>
 * \gi{ } )? <br>
 * \gi{ } ( \gb{'s'} | \gb{'?'} | \gb{'x'} )? <i>display type</i>
 *
 * **Display type** This is a single character that defines how values in the string are printed:
 *  - `s` *(default)* Values are printed as-is.
 *  - `?` Non-printables are displayed using their common escape sequence, for instance `\n` is
 * displayed as `\n` instead of putting a newline. Other values without a common escape sequence
 * are displayed using the `\xXX` format.
 *  - `x` Non-printables are displayed using the `0xXX` format.
 *
 * #### Examples
 *
 *  - `format(out, "{:}", "Hello") -> "Hello"`
 *  - `format(out, "{:#''}", "Hello") -> "'Hello'"`
 *  - `format(out, "{:.2}", "Hello") -> "He"`
 *  - `format(out, "{:5.2}", "Hello") -> "He   "`
 *  - `format(out, "{:?}", "\nT\x87") -> "\nT\x87"`
 *  - `format(out, "{:x}", "\nT\x87") -> "0x0AT0x87"`
 *
 *
 * @anchor grammar_format_signed
 * ### Signed integer
 *
 * \gb{format_signed} := \gto{alignment, alignment} <br>
 * \gi{ } (\gb{'-'} | \gb{'+'} | \gb{' '})? <i>sign</i> <br>
 * \gi{ } \gb{'#'}? <i>alternate mode</i> <br>
 * \gi{ } \gb{'0'}? <i>align with `0`'s</i> <br>
 * \gi{ } \gto{size, width of the formatted number} <br>
 * \gi{ } (\gb{'.'} <br>
 * \gi{ } \gi{ } \gt{number, precision} <br>
 * \gi{ } )? <br>
 * \gi{ } ( \gb{'x'} | \gb{'X'} | \gb{'b'} | \gb{'B'} )? <i>display type</i>
 *
 * Note that you may not use alignment if you use the align with 0's option.
 *
 * Format an integer following rules similar to `printf`.
 *
 * #### Examples
 *
 *  - `format(out, "{}", 123) -> "123"`
 *  - `format(out, "{:x}", 123) -> "7b"`
 *  - `format(out, "{:#x}", 123) -> "0x7b"`
 *  - `format(out, "{:#X}", 123) -> "0x7B"`
 *  - `format(out, "{:05}", 1) -> "00001"`
 *  - `format(out, "{:05}", -1) -> "-0001"`
 *  - `format(out, "{:+}", 1) -> "+1"`
 *  - `format(out, "{: }", 1) -> " 1"`
 *  - `format(out, "{:.5}", 1) -> "-00001"`
 *
 *
 * ### Unsigned integer
 *
 * @anchor grammar_format_collection
 * ## Collection
 *
 * \gb{format_collection} := \gb{'['} \gt{number, number of elements to display} \gb{']'} <br>
 * \gi{ } \gto{alignment, alignment} <br>
 * \gi{ } \gto{size,width} <br>
 * \gi{ } (\gb{'#'} <br>
 * \gi{ } \gi{ } \gt{spec, start delimiter} <br>
 * \gi{ } \gi{ } \gt{spec, element separator} <br>
 * \gi{ } \gi{ } \gt{spec, end delimiter} <br>
 * \gi{ } )? <br>
 * \gi{ } \gb{':\{'} \gt{expression, format expression for array elements} \gb{'\}'}
 *
 *
 * @}
 */

#define FORMAT__OBJ_MAPPER_S(N, CONST, ARG)                                                      \
	format(output,                                                                               \
	       "{:>{1}}{2} = ",                                                                      \
	       "",                                                                                   \
	       4 * FORMAT__SELECT(0, CONST),                                                         \
	       FORMAT__STRINGIFY(FORMAT__SELECT(0, ARG)));                                           \
	FORMAT__IF_ELSE(FORMAT__HAS_ARG(2, ARG))                                                     \
	(format(output,                                                                              \
	        "{0:" FORMAT__SELECT(0, ARG) "},\n",                                                 \
	        FORMAT_OBJ_STRUCT->FORMAT__SELECT(0, ARG),                                           \
	        FORMAT__SELECT(2, FORMAT__EXPAND ARG)))(                                             \
	  format(output,                                                                             \
	         "{0:" FORMAT__SELECT(1, ARG) "},\n",                                                \
	         FORMAT_OBJ_STRUCT->FORMAT__SELECT(0, ARG)));

#define FORMAT__OBJ_MAPPER_P(N, CONST, ARG, FORMATTER, FIELD)                                    \
	format(output, "{:>{1}}{2} = ", "", 4 * FORMAT__SELECT(0, CONST), FORMAT__STRINGIFY(FIELD)); \
	FORMAT__IF_ELSE(FORMAT__HAS_ARG(2, ARG))                                                     \
	(format(output,                                                                              \
	        "{0:" FORMAT__SELECT(1, ARG) "},\n",                                                 \
	        (FORMATTER, FORMAT_OBJ_STRUCT->FIELD),                                               \
	        FORMAT__SELECT(2, FORMAT__EXPAND ARG)))(                                             \
	  format(                                                                                    \
	    output, "{0:" FORMAT__SELECT(1, ARG) "},\n", (FORMATTER, FORMAT_OBJ_STRUCT->FIELD)));

#define FORMAT__OBJ_MAPPER_T(N, CONST, ARG, TAG, DATA, FIELD)                                    \
	typeof(*FORMAT_OBJ_STRUCT->FIELD)* format_arg_triplet__ = FORMAT_OBJ_STRUCT->FIELD;          \
	(void)format_arg_triplet__;                                                                  \
	format(output, "{:>{1}}{2} = ", "", 4 * FORMAT__SELECT(0, CONST), FORMAT__STRINGIFY(FIELD)); \
	FORMAT__IF_ELSE(FORMAT__HAS_ARG(2, FORMAT__EXPAND ARG))(                                     \
	  format(output,                                                                             \
	         "{0:" FORMAT__SELECT(1, ARG) "},\n",                                                \
	         (TAG, DATA, FORMAT_OBJ_STRUCT->FIELD),                                              \
	         FORMAT__SELECT(2, ARG)))(format(output,                                             \
	                                         "{0:" FORMAT__SELECT(1, ARG) "},\n",                \
	                                         (TAG, DATA, FORMAT_OBJ_STRUCT->FIELD)));

#define FORMAT__OBJ_MAPPER(N, CONST, ARG)                                                        \
	FORMAT__IF_ELSE(FORMAT__IS_PAIR(FORMAT__SELECT(0, ARG)))(                                    \
	  FORMAT__OBJ_MAPPER_P(N, CONST, ARG, FORMAT__SELECT(0, 0, ARG), FORMAT__SELECT(0, 1, ARG)   \
                                                                                                 \
	                         ))(                                                                 \
	  FORMAT__IF_ELSE(FORMAT__IS_TRIPLET(FORMAT__SELECT(0, ARG)))(                               \
	    FORMAT__OBJ_MAPPER_T(N,                                                                  \
		                     CONST,                                                              \
		                     ARG,                                                                \
		                     FORMAT__SELECT(0, 0, ARG),                                          \
		                     FORMAT__SELECT(0, 1, ARG),                                          \
		                     FORMAT__SELECT(0, 2, ARG)))(FORMAT__OBJ_MAPPER_S(N, CONST, ARG)))

#define FORMAT__OBJ_MAPPER_(N, CONST, ARG)                                                       \
	{ FORMAT__IF_ELSE(FORMAT__IS_PAIR(ARG))(FORMAT__OBJ_MAPPER_P(                                \
	  N, CONST, ARG, FORMAT__SELECT(0, 0, ARG), FORMAT__SELECT(0, 1, ARG)))(                     \
	  FORMAT__IF_ELSE(FORMAT__IS_TRIPLET(ARG))(FORMAT__OBJ__MAPPER_T(                            \
		N,                                                                                       \
		CONST,                                                                                   \
		ARG,                                                                                     \
		FORMAT__SELECT(0, 0, ARG),                                                               \
		FORMAT__SELECT(1, ARG),                                                                  \
		FORMAT__SELECT(0, 1, ARG)))(FORMAT__OBJ_MAPPER_S(N, CONST, (ARG)))) }

#define FORMAT_OBJ(TYPE, FUN, ...)                                                               \
	int FUN(struct format_output* output,                                                        \
	        const char* fmt_spec,                                                                \
	        const struct format_env* env,                                                        \
	        size_t idx)                                                                          \
	{                                                                                            \
		const TYPE* FORMAT_OBJ_STRUCT = (const TYPE*)env->args[idx].data;                        \
		assert(FORMAT_OBJ_STRUCT != NULL && "Cannot format a NULL object");                      \
		size_t format_depth__ = 1;                                                               \
                                                                                                 \
		format(output, "{0:>{1}}{2} {{\n", "", 4 * (format_depth__ - 1), #TYPE);                  \
		enum                                                                                     \
		{                                                                                        \
			counter_base = __COUNTER__                                                           \
		};                                                                                       \
		FORMAT__EXPAND(FORMAT__EVAL_O(                                                          \
		  FORMAT__MAP_CONST_N(counter_base, FORMAT__OBJ_MAPPER, (format_depth__), __VA_ARGS__))) \
                                                                                                 \
		format(output, "{0:>{1}}}}", "", 4 * (format_depth__ - 1));                             \
		return 0;                                                                                \
	}

struct Foo
{
	int val;
	const char* str;
	long x;
	int arr[5];
	size_t len;
};

#define FORMAT_OBJ_STRUCT format_object__
FORMAT_OBJ(struct Foo,
           format_foo,
           ((format_fmt_int, val), "x"),
           (val, "b"),
           (FORMAT_ARRAY(arr), "[{1}]:{}", (FORMAT_OBJ_STRUCT->len)))

#endif // LIBFORMAT_H
