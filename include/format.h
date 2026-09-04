#ifndef LIBFORMAT_H
#define LIBFORMAT_H

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
	void* (*next)(struct format_arg_collection* collection);
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

#define FORMAT__CAR(a, ...) a
#define FORMAT__CDR(a, ...) __VA_ARGS__
#define FORMAT__CDR2(...) FORMAT__CDR(__VA_ARGS__)
#define FORMAT__SECOND(a, b, ...) b

#define FORMAT__PROBE() ~, 1
#define FORMAT__IS_PROBE(...) FORMAT__SECOND(__VA_ARGS__, 0)
#define FORMAT__NOT(x) FORMAT__IS_PROBE(FORMAT__CAT(FORMAT___NOT_, x))
#define FORMAT___NOT_0 FORMAT__PROBE()
#define FORMAT__BOOL(x) FORMAT__NOT(FORMAT__NOT(x))

#define FORMAT__IF_ELSE(cond) FORMAT___IF_ELSE(FORMAT__BOOL(cond))
#define FORMAT___IF_ELSE(cond) FORMAT__CAT(FORMAT___IF_, cond)
#define FORMAT___IF_1(...) __VA_ARGS__ FORMAT___IF_1_ELSE
#define FORMAT___IF_0(...) FORMAT___IF_0_ELSE
#define FORMAT___IF_1_ELSE(...)
#define FORMAT___IF_0_ELSE(...) __VA_ARGS__

#define FORMAT__HAS_ARGS(...) FORMAT__BOOL(__VA_OPT__(1 +) 0)
#define FORMAT___END_OF_ARGS_() 0

#define FORMAT__CAT(a, b) a##b

#define FORMAT__MAP(__counter_base, __m, __first, ...)                                           \
	__m(__COUNTER__ - __counter_base - 1, __first) FORMAT__IF_ELSE(FORMAT__HAS_ARGS(             \
	  __VA_ARGS__))(FORMAT__DEFER2(FORMAT___MAP)()(__counter_base, __m, __VA_ARGS__))()
#define FORMAT___MAP() FORMAT__MAP

#define FORMAT__CONSUME(...)
#define FORMAT__IS_TUPLE(X) FORMAT__NOT(FORMAT__HAS_ARGS(FORMAT__CONSUME X))
#define FORMAT__TUPLE_FORMATTER(X) FORMAT__CAR(FORMAT__EXPAND X)
#define FORMAT__TUPLE_EXPR(X) FORMAT__CDR(FORMAT__EXPAND X)

#define FORMAT__IS_PAIR(X)                                                                       \
	FORMAT__IF_ELSE(FORMAT__HAS_ARGS(FORMAT__CDR X))                                             \
	(FORMAT__IS_PAIR_2 X)(0)

#define FORMAT__IS_PAIR_2(a, b, ...) FORMAT__NOT(FORMAT__HAS_ARGS(__VA_ARGS__))

#define FORMAT__IS_TRIPLET(X)                                                                    \
	FORMAT__IF_ELSE(FORMAT__HAS_ARGS(FORMAT__CDR2(FORMAT__CDR X)))                               \
	(FORMAT__IS_TRIPLET_3 X)(0)

#define FORMAT__IS_TRIPLET_3(a, b, c, ...) FORMAT__NOT(FORMAT__HAS_ARGS(__VA_ARGS__))

#define FORMAT__ARG_EXPAND(X)                                                                    \
	FORMAT__IF_ELSE(FORMAT__IS_TUPLE(X))(FORMAT__TUPLE_FORMATTER(X),                             \
	                                     FORMAT__TUPLE_EXPR(X))(NULL, X)
#define FORMAT___SELECT_0(X, ...) X
#define FORMAT___SELECT_1(X, ...) FORMAT___SELECT_0(__VA_ARGS__)
#define FORMAT___SELECT_2(X, ...) FORMAT___SELECT_1(__VA_ARGS__)
#define FORMAT___SELECT(N, ...) FORMAT___SELECT_##N(__VA_ARGS__)
#define FORMAT__SELECT(N, ...) FORMAT___SELECT(N, __VA_ARGS__)

#define FORMAT__MAPPER_DEFAULT(N, ARG)                                                           \
	__builtin_choose_expr(                                                                       \
	  __builtin_types_compatible_p(long long, typeof(ARG)),                                      \
	  ((struct format_arg){                                                                      \
	    .type = kFormatScalar, .formatter = format_fmt_long_long, .data = 0 }),                  \
	  __builtin_choose_expr(                                                                     \
	    __builtin_types_compatible_p(long, typeof(ARG)),                                         \
	    ((struct format_arg){ .type = kFormatScalar, .formatter = format_fmt_long, .data = 0 }), \
	    __builtin_choose_expr(                                                                   \
	      __builtin_types_compatible_p(int, typeof(ARG)),                                        \
	      ((struct format_arg){                                                                  \
	        .type = kFormatScalar, .formatter = format_fmt_int, .data = 0 }),                    \
	      __builtin_choose_expr(                                                                 \
	        __builtin_types_compatible_p(short, typeof(ARG)),                                    \
	        ((struct format_arg){                                                                \
	          .type = kFormatScalar, .formatter = format_fmt_short, .data = 0 }),                \
	        __builtin_choose_expr(                                                               \
	          __builtin_types_compatible_p(char, typeof(ARG)),                                   \
	          ((struct format_arg){                                                              \
	            .type = kFormatScalar, .formatter = format_fmt_char, .data = 0 }),               \
	          __builtin_choose_expr(                                                             \
	            __builtin_types_compatible_p(unsigned long long, typeof(ARG)),                   \
	            ((struct format_arg){ .type = kFormatScalar,                                     \
				                      .formatter = format_fmt_unsigned_long_long,                \
				                      .data = 0 }),                                              \
	            __builtin_choose_expr(                                                           \
	              __builtin_types_compatible_p(unsigned long, typeof(ARG)),                      \
	              ((struct format_arg){                                                          \
	                .type = kFormatScalar, .formatter = format_fmt_unsigned_long, .data = 0 }),  \
	              __builtin_choose_expr(                                                         \
	                __builtin_types_compatible_p(unsigned int, typeof(ARG)),                     \
	                ((struct format_arg){                                                        \
	                  .type = kFormatScalar, .formatter = format_fmt_unsigned_int, .data = 0 }), \
	                __builtin_choose_expr(                                                       \
	                  __builtin_types_compatible_p(unsigned short, typeof(ARG)),                 \
	                  ((struct format_arg){ .type = kFormatScalar,                               \
					                        .formatter = format_fmt_unsigned_short,              \
					                        .data = 0 }),                                        \
	                  __builtin_choose_expr(                                                     \
	                    __builtin_types_compatible_p(unsigned char, typeof(ARG)),                \
	                    ((struct format_arg){ .type = kFormatScalar,                             \
						                      .formatter = format_fmt_unsigned_char,             \
						                      .data = 0 }),                                      \
	                    __builtin_choose_expr(                                                   \
	                      __builtin_types_compatible_p(signed char, typeof(ARG)),                \
	                      ((struct format_arg){ .type = kFormatScalar,                           \
						                        .formatter = format_fmt_signed_char,             \
						                        .data = 0 }),                                    \
	                      __builtin_choose_expr(                                                 \
	                        __builtin_types_compatible_p(double, typeof(ARG)),                   \
	                        ((struct format_arg){ .type = kFormatScalar,                         \
							                      .formatter = format_fmt_double,                \
							                      .data = 0 }),                                  \
	                        __builtin_choose_expr(                                               \
	                          __builtin_types_compatible_p(float, typeof(ARG)),                  \
	                          ((struct format_arg){ .type = kFormatScalar,                       \
							                        .formatter = format_fmt_float,               \
							                        .data = 0 }),                                \
	                          __builtin_choose_expr(                                             \
	                            __builtin_types_compatible_p(const char*, typeof(ARG)) ||        \
	                              __builtin_types_compatible_p(char*, typeof(ARG)) ||            \
	                              __builtin_types_compatible_p(const char[], typeof(ARG)) ||     \
	                              __builtin_types_compatible_p(char[], typeof(ARG)),             \
	                            ((struct format_arg){ .type = kFormatScalar,                     \
								                      .formatter = format_fmt_str,               \
								                      .data = 0 }),                              \
	                            ((struct format_arg){ .type = kFormatScalar,                     \
								                      .formatter = NULL,                         \
								                      .data = 0 })))))))))))))))
#define FORMAT__MAPPER_TRIPLET_COLLECTION(X, Y)                                                  \
	((struct format_arg){ .type = kFormatCollection, .collection = X, .data = 0 })
#define FORMAT__MAPPER_TRIPLET(TAG, X, Y) FORMAT__CAT(FORMAT__MAPPER_TRIPLET_, TAG)(X, Y)

#define FORMAT__MAPPER(N, ARG)                                                                   \
	FORMAT__IF_ELSE(FORMAT__IS_TRIPLET(ARG))(FORMAT__MAPPER_TRIPLET ARG)(                        \
	  FORMAT__IF_ELSE(FORMAT__IS_PAIR(ARG))(((struct format_arg){                                \
	    .type = kFormatScalar,                                                                   \
	    .formatter = FORMAT__SELECT(0, FORMAT__ARG_EXPAND(ARG)),                                 \
	    .data = 0 }))(FORMAT__MAPPER_DEFAULT(N, FORMAT__SELECT(1, FORMAT__ARG_EXPAND(ARG))))),

#define FORMAT__SET_COLLECTION(N, X, Y)                                                          \
	{                                                                                            \
		args[N].data = (uintptr_t)(Y);                                                           \
	}
#define FORMAT__SET_TRIPLET_(N, TAG, X, Y) FORMAT__CAT(FORMAT__SET_, TAG)(N, X, Y)

#define FORMAT__IS_POINTER_VAR_P(VAR)                                                            \
	(__builtin_types_compatible_p(const void*, typeof(VAR)) ||                                   \
	 __builtin_types_compatible_p(char*, typeof(VAR)) ||                                         \
	 __builtin_types_compatible_p(const char*, typeof(VAR)) ||                                   \
	 __builtin_types_compatible_p(char[], typeof(VAR)) ||                                        \
	 __builtin_types_compatible_p(const char[], typeof(VAR)))

#define FORMAT__SET_DEFAULT(N, ARG)                                                              \
	{                                                                                            \
		_Static_assert(sizeof(typeof(FORMAT__SELECT(1, FORMAT__ARG_EXPAND(ARG)))) <=             \
		                 sizeof(uint64_t),                                                       \
		               "Invalid argument type");                                                 \
		if (FORMAT__IS_POINTER_VAR_P(FORMAT__SELECT(1, FORMAT__ARG_EXPAND(ARG)))) {              \
			args[N].data = (uintptr_t)FORMAT__SELECT(1, FORMAT__ARG_EXPAND(ARG));                \
		} else {                                                                                 \
			const typeof(FORMAT__SELECT(1, FORMAT__ARG_EXPAND(ARG))) temp =                      \
			  FORMAT__SELECT(1, FORMAT__ARG_EXPAND(ARG));                                        \
			memcpy(&args[N].data, &temp, sizeof(temp));                                          \
		}                                                                                        \
	}
#define FORMAT__SET(N, ARG)                                                                      \
	FORMAT__IF_ELSE(FORMAT__IS_TRIPLET(ARG))                                                     \
	(FORMAT__SET_TRIPLET_(N,                                                                     \
	                      FORMAT__SELECT(0, FORMAT__EXPAND ARG),                                 \
	                      FORMAT__SELECT(1, FORMAT__EXPAND ARG),                                 \
	                      FORMAT__SELECT(2, FORMAT__EXPAND ARG)))(FORMAT__SET_DEFAULT(N, ARG))

#define format(output, fmt, ...)                                                                 \
	do {                                                                                         \
		FORMAT__START_DIAG(clang)                                                                \
		FORMAT__DIAG(clang, ignored "-Wc2y-extensions")                                          \
		_Static_assert(__builtin_types_compatible_p(typeof(output), struct format_output*),      \
		               "Invalid output type");                                                   \
		_Static_assert(__builtin_types_compatible_p(typeof(fmt), const char*) ||                 \
		                 __builtin_types_compatible_p(typeof(fmt), const char[]),                \
		               "Invalid format string");                                                 \
		struct format_arg args[] = { FORMAT__IF_ELSE(FORMAT__HAS_ARGS(__VA_ARGS__))(             \
		  FORMAT__EXPAND(FORMAT__EVAL(FORMAT__MAP(0, FORMAT__MAPPER, __VA_ARGS__))))()(          \
		  struct format_arg){                                                                    \
		  .type = kFormatScalar, .formatter = NULL, .data = 0 } /* Sentinel */ };                \
		enum                                                                                     \
		{                                                                                        \
			counter_base = __COUNTER__                                                           \
		};                                                                                       \
		FORMAT__IF_ELSE(FORMAT__HAS_ARGS(__VA_ARGS__))(                                          \
		  FORMAT__EXPAND(FORMAT__EVAL(FORMAT__MAP(counter_base, FORMAT__SET, __VA_ARGS__))))()   \
		  format_args(output,                                                                    \
		              fmt,                                                                       \
		              (const struct format_env){ .args = args,                                   \
		                                         .size = sizeof(args) / sizeof(args[0]) - 1 });  \
		FORMAT__END_DIAG(clang)                                                                  \
	} while (0)

/** @} */

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
	void* cur;
};

static inline void*
format__collection_array_next(struct format_arg_collection* collection)
{
	struct format_collection_array_cursor* c = collection->state;
	void* p = c->cur;
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
	   .state = &(struct format_collection_array_cursor){ .cur = (ARRAY) },                      \
	   .next = format__collection_array_next,                                                    \
	   .width = format__collection_array_width,                                                  \
	   .formatter = FORMAT__MAPPER_DEFAULT(0, *(ARRAY)).formatter,                               \
	   .elem_size = sizeof(*(ARRAY)),                                                            \
	   .is_pointer = FORMAT__IS_POINTER_VAR_P(*(ARRAY)) }),                                      \
	 ARRAY)
#define FORMAT__ARRAY_1(ARRAY, FORMATTER)                                                        \
	(COLLECTION,                                                                                 \
	 ((struct format_arg_collection){                                                            \
	   .state = &(struct format_collection_array_cursor){ .cur = (ARRAY) },                      \
	   .next = format__collection_array_next,                                                    \
	   .width = format__collection_array_width,                                                  \
	   .formatter = FORMATTER,                                                                   \
	   .elem_size = sizeof(*(ARRAY)),                                                            \
	   .is_pointer = FORMAT__IS_POINTER_VAR_P(*(ARRAY)) }),                                      \
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

#endif // LIBFORMAT_H
