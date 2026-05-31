#ifndef LIBFORMAT_FMT_H
#define LIBFORMAT_FMT_H

#include "buffer.h"
#include <assert.h>

// TODO: Enforce 4096 max args
struct fmt_env {
	/** @brief Stack allocated array of arguments */
	struct fmt_arg *args;
	/** @brief Number of arguments */
	size_t size;
};

struct fmt_arg {
	/** @brief Custom formatter */
	void (*formatter)(struct format_output *output, const char *fmt_spec, const struct fmt_env, size_t idx);
	/** @brief Raw data */
	void *data;
};

/**
 * @brief Format a `long` argument
 *
 * @param output Output to write to
 * @param fmt_spec Format specifier for this argument
 * @param env Format environment
 * @param idx Index of this argument in @p env
 */
void
format_fmt_long(struct format_output* output,
                const char* fmt_spec,
                const struct fmt_env env,
                size_t idx);


#endif // LIBFORMAT_FMT_H
