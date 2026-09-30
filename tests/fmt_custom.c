#include "util.h"

struct point
{
	int x, y;
};

static int
format_point(struct format_output* output,
             const char* fmt_spec,
             const struct format_env* env,
             size_t idx)
{
	(void)fmt_spec;
	const struct point* p = (const struct point*)env->args[idx].data;
	return format(output, "({}, {})", p->x, p->y);
}

static int
format_fail(struct format_output* output,
            const char* fmt_spec,
            const struct format_env* env,
            size_t idx)
{
	(void)output;
	(void)fmt_spec;
	(void)env;
	(void)idx;
	return -1;
}

Test(fmt_custom, formatter)
{
	struct point p = {6, 7};
	struct point p1 = {1, 2};
	struct point p2 = {3, 4};
	test_manual("{}", "(6, 7)", (format_point, &p));
	test_manual("{} and {}", "(1, 2) and (3, 4)", (format_point, &p1), (format_point, &p2));
}

Test(fmt_custom, error)
{
	struct format_output out = format_output_buf();
	const int result = format(&out, "{}", (format_fail, 1));
	cr_assert_eq(result, -1);
	format_output_destroy(&out);
}

static size_t g_malloc_count;
static size_t g_free_count;
static size_t g_realloc_count;

static void*
counting_malloc(size_t size)
{
	++g_malloc_count;
	return malloc(size);
}

static void
counting_free(void* ptr, size_t size)
{
	(void)size;
	++g_free_count;
	free(ptr);
}

static void*
counting_realloc(void* ptr, size_t old_size, size_t new_size)
{
	(void)old_size;
	++g_realloc_count;
	return realloc(ptr, new_size);
}

Test(fmt_custom, allocator_buf)
{
	struct format_output out = format_output_buf();
	format_output_set_allocator(&out, counting_malloc, counting_free, counting_realloc);

	char* big = malloc(5001);
	memset(big, 'z', 5000);
	big[5000] = '\0';

	const int result = format(&out, "{}", big);
	cr_assert_eq(result, 0);
	cr_assert_eq(out.size, 5000);
	cr_assert(memcmp(out.data, big, 5000) == 0);
	cr_assert(g_realloc_count > 0);
	cr_assert_eq(g_free_count, 0);

	format_output_destroy(&out);
	cr_assert_eq(g_free_count, 1);

	free(big);
}

Test(fmt_custom, allocator_fd)
{
	char name[256];
	sprintf(name, "/fmt-test-custom-%d", (int)getpid());
	int fd = shm_open(name, O_CLOEXEC | O_CREAT | O_EXCL | O_RDWR, 0600);
	cr_assert(fd != -1, "Failed to open memory fd");
	shm_unlink(name);

	struct format_output out = format_output_fd(fd);
	format_output_set_allocator(&out, counting_malloc, counting_free, counting_realloc);

	const int result = format(&out, "hello");
	cr_assert_eq(result, 0);
	cr_assert_eq(g_malloc_count, 1);
	cr_assert_eq(g_free_count, 0);

	format_output_destroy(&out);
	cr_assert_eq(g_free_count, 1);

	close(fd);
}