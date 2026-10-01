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

static void*
null_malloc(size_t size)
{
	(void)size;
	return NULL;
}

static void
null_free(void *ptr, size_t size)
{
	(void)ptr;
	(void)size;
}

static void*
null_realloc(void *ptr, size_t old_size, size_t new_size)
{
	(void)ptr;
	(void)old_size;
	(void)new_size;
	return NULL;
}

/* Allocator failure must fail and return properly */
Test(fmt_custom, null_alloc) {
	struct format_output out = format_output_buf();
	format_output_set_allocator(&out, null_malloc, null_free, null_realloc);

	int result = format(&out, "Hello, {}!", "World");
	cr_assert_neq(result, 0);

	format_output_destroy(&out);
}

Test(fmt_custom, null_alloc_fd) {
	char name[256];
	sprintf(name, "/fmt-test-custom-%d", (int)getpid());
	int fd = shm_open(name, O_CLOEXEC | O_CREAT | O_EXCL | O_RDWR, 0600);
	cr_assert(fd != -1, "Failed to open memory fd");
	shm_unlink(name);

	struct format_output out = format_output_fd(fd);
	format_output_set_flush(&out, kFormatFlushNone);
	format_output_set_allocator(&out, null_malloc, null_free, null_realloc);

	int result = format(&out, "Hello, {}!", "World");
	cr_assert_neq(result, 0);

	format_output_destroy(&out);
	close(fd);
}

/* Simple mmap-based allocator */
static void*
mmap_malloc(size_t size)
{
	const size_t page = (size_t)sysconf(_SC_PAGESIZE);
	size_t n = 1;
	while (page * n < size)
		n *= 2;
	void *ptr = mmap(NULL, n * page, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (ptr == MAP_FAILED)
		return NULL;
	return ptr;
}

static void
mmap_free(void *ptr, size_t size)
{
	const size_t page = (size_t)sysconf(_SC_PAGESIZE);
	// size does not need to be aligned to a page boundary
	munmap(ptr, size);
}

static void*
mmap_realloc(void *ptr, size_t old_size, size_t new_size)
{
	if (!ptr)
	{
		cr_assert_eq(old_size, 0);
		return mmap_malloc(new_size);
	}

	const size_t page = (size_t)sysconf(_SC_PAGESIZE);

	// Must align old_size to page boundaries
	{
		size_t n = old_size / page;
		while (n * page < old_size)
			++n;
		old_size = n * page;
	}

	size_t n = 1;
	while (page * n < new_size)
		n *= 2;
	void *new_ptr = mremap(ptr, old_size, n * page, MREMAP_MAYMOVE);
	if (new_ptr == MAP_FAILED)
		return NULL;
	return new_ptr;
}

Test(fmt_custom, mmap_alloc) {
	struct format_output out = format_output_buf();
	format_output_set_allocator(&out, mmap_malloc, mmap_free, mmap_realloc);

	int result = format(&out, "Hello, {}!", "World");
	cr_assert_eq(result, 0);
	cr_assert_eq(out.nwritten, 13);
	cr_assert_eq(memcmp(out.data, "Hello, World!", 13), 0);

	char buf[4096];
	memset(buf, 'x', sizeof(buf));
	cr_assert_eq(format_output_write(&out, buf, 4096), 0);
	cr_assert_eq(out.nwritten, 13 + 4096);
	cr_assert_eq(memcmp(out.data + 13, buf, 4096), 0);

	format_output_destroy(&out);
}
