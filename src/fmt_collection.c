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

struct collection_spec
{
	/** @brief Number of values to print */
	size_t precision;

	/** @brief Fill string */
	struct format_spec_placeholder fill;
	/**
	 * @brief Alignment character:
	 *  - `<`: Left (default)
	 *  - `>`: Right
	 *  - `^`: Center
	 *  - `0`: Pad with `0`'s
	 */
	char align;

	size_t width;

	/** @brief Array start placeholder */
	struct format_spec_placeholder start;
	/** @brief Array separator placeholder */
	struct format_spec_placeholder sep;
	/** @brief Array end placeholder */
	struct format_spec_placeholder end;

	const char* subexpr;

	const char* left;
};

static inline struct collection_spec
parse_collection_spec(const char* fmt_spec, const struct format_env* env)
{
	struct collection_spec spec = {
		.precision = 0,
		.align = '<',
		.width = 0,
	};
	size_t i = 0;

	/* Precision */
	assert(fmt_spec[i] == '[' && "Expected array format specifier");
	++i;
	spec.precision = format_parse_number(fmt_spec, &i, env);
	assert(fmt_spec[i] == ']' && "Expected array format specifier");
	++i;

	/* Align */
	format_parse_alignment(fmt_spec, &i, env, &spec.align, &spec.fill, " ");

	/* Width */
	spec.width = format_parse_size(fmt_spec, &i, env);

	/* Display */
	if (fmt_spec[i] == '#') {
		++i;
		spec.start = format_parse_placeholder(fmt_spec, &i, env);
		spec.sep = format_parse_placeholder(fmt_spec, &i, env);
		spec.end = format_parse_placeholder(fmt_spec, &i, env);
	} else {
		spec.start.type = 1;
		spec.start.len = 1;
		spec.start.data.str = "{";
		spec.start.width = 1;

		spec.sep.type = 1;
		spec.sep.len = 2;
		spec.sep.data.str = ", ";
		spec.sep.width = 2;

		spec.end.type = 1;
		spec.end.len = 1;
		spec.end.data.str = "}";
		spec.end.width = 1;
	}

	/* Subexpr */
	assert(fmt_spec[i] == ':' && "Expected format subexpression");
	++i;
	assert(fmt_spec[i] == '{' && "Expected format subexpression");
	++i;
	spec.subexpr = fmt_spec + i;
	int balance = 1;
	while (balance != 0) {
		if (fmt_spec[i] == '{')
			++balance;
		else if (fmt_spec[i] == '}')
			--balance;
		++i;
	}

	spec.left = fmt_spec + i;
	return spec;
}

int
format_fmt_collection(struct format_output* output,
                      const char* fmt_spec,
                      const struct format_env* env,
                      size_t idx)
{
	struct collection_spec spec = parse_collection_spec(fmt_spec, env);
	assert(spec.left[0] == '}' && "Leftover content in format specifier");

	struct format_arg_collection* collection = &env->args[idx].payload.collection;
	assert(collection->formatter);
	assert(collection->width);

	/* Compute width */
	const size_t width = collection->width(collection, spec.subexpr, spec.precision) +
	                     spec.start.width + spec.end.width +
	                     (spec.precision > 1 ? spec.sep.width * (spec.precision - 1) : 0);

	struct format_arg args[2];
	args[0].type = kFormatScalar;
	args[0].payload.formatter = collection->formatter;

	args[1].type = kFormatScalar;
	args[1].payload.formatter = NULL;
	args[1].data = 0;

	struct format_env subenv = {
		.args = (struct format_arg*)&args,
		.size = 1,
	};

	/* Compute alignment */
	size_t left = 0, right = 0;
	switch (spec.align) {
		case '^':
			left = (spec.width > width ? spec.width - width : 0);
			right = left / 2;
			left -= right;
			break;
		case '<':
			right = spec.width > width ? spec.width - width : 0;
			break;
		case '>':
			left = spec.width > width ? spec.width - width : 0;
			break;
		default:
			format_unreachable();
	}

	/* Left spacing */
	if (format_write_placeholder(output, &spec.fill, left, 0))
		return -1;
	/* Left delim */
	if (format_write_placeholder(output, &spec.start, (size_t)-1, 0))
		return -1;
	for (size_t i = 0; i < spec.precision; ++i) {
		if (i != 0) {
			if (format_write_placeholder(output, &spec.sep, (size_t)-1, 0))
				return -1;
		}
		const void* val = collection->next(collection);

		if (collection->is_pointer)
			args[0].data = *(uintptr_t*)val;
		else
			memcpy(&args[0].data, val, collection->elem_size);
		collection->formatter(output, spec.subexpr, &subenv, 0);
	}
	/* Right delim */
	if (format_write_placeholder(output, &spec.end, (size_t)-1, 0))
		return -1;
	/* Right spacing */
	if (format_write_placeholder(output, &spec.fill, right, 1))
		return -1;

	return 0;
}
