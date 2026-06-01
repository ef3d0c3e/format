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
	int (*formatter)(struct format_output *output, const char *fmt_spec, const struct fmt_env*, size_t idx);
	/** @brief Raw data */
	void *data;
};


#endif // LIBFORMAT_FMT_H
