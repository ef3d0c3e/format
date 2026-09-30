#include "util.h"

#include <errno.h>

static int
make_shm_fd(const char* tag)
{
	char name[256];
	sprintf(name, "/fmt-test-%s-%d", tag, (int)getpid());
	int fd = shm_open(name, O_CLOEXEC | O_CREAT | O_EXCL | O_RDWR, 0600);
	cr_assert(fd != -1, "Failed to open memory fd");
	shm_unlink(name);
	return fd;
}

static size_t
fd_size(int fd)
{
	const off_t end = lseek(fd, 0, SEEK_END);
	cr_assert(end >= 0);
	return (size_t)end;
}

static size_t
read_fd(int fd, char* buf, size_t cap)
{
	cr_assert(lseek(fd, 0, SEEK_SET) == 0);
	const ssize_t n = read(fd, buf, cap);
	cr_assert(n >= 0);
	return (size_t)n;
}

Test(fmt_flush, always)
{
	int fd = make_shm_fd("flush-always");
	struct format_output out = format_output_fd(fd);
	format_output_set_flush(&out, kFormatFlushAlways);

	const int result_always = format(&out, "Hello, {}!", "World");
	cr_assert_eq(result_always, 0);
	/* Everything should already be in the fd, without any explicit flush */
	cr_assert_eq(fd_size(fd), 13);

	char buf[64];
	cr_assert_eq(read_fd(fd, buf, sizeof(buf)), 13);
	cr_assert(memcmp(buf, "Hello, World!", 13) == 0);

	format_output_destroy(&out);
	cr_assert_eq(fd_size(fd), 13);
	close(fd);
}

Test(fmt_flush, none)
{
	int fd = make_shm_fd("flush-none");
	struct format_output out = format_output_fd(fd);
	format_output_set_flush(&out, kFormatFlushNone);

	const int result_small = format(&out, "abc");
	cr_assert_eq(result_small, 0);
	/* Small writes stay buffered */
	cr_assert_eq(fd_size(fd), 0);

	char* big = malloc(2001);
	memset(big, 'x', 2000);
	big[2000] = '\0';
	const int result_big = format(&out, "{}", big);
	cr_assert_eq(result_big, 0);
	/* The 1024-byte internal buffer got flushed once full */
	cr_assert_eq(fd_size(fd), 1024);

	format_output_destroy(&out);
	cr_assert_eq(fd_size(fd), 2003);

	char* buf = malloc(2003);
	cr_assert_eq(read_fd(fd, buf, 2003), 2003);
	cr_assert(memcmp(buf, "abc", 3) == 0);
	cr_assert(memcmp(buf + 3, big, 2000) == 0);
	free(buf);
	free(big);
	close(fd);
}

Test(fmt_flush, newline)
{
	int fd = make_shm_fd("flush-newline");
	struct format_output out = format_output_fd(fd); /* newline flush by default */

	cr_assert_eq(format(&out, "abc"), 0);
	cr_assert_eq(fd_size(fd), 0);

	cr_assert_eq(format(&out, "def\n"), 0);
	cr_assert_eq(fd_size(fd), 7); /* "abcdef\n" flushed on newline */

	cr_assert_eq(format(&out, "gh\nij"), 0);
	cr_assert_eq(fd_size(fd), 10); /* "gh\n" flushed, "ij" buffered */

	format_output_destroy(&out);
	cr_assert_eq(fd_size(fd), 12);

	char buf[16] = {0};
	cr_assert_eq(read_fd(fd, buf, sizeof(buf)), 12);
	cr_assert(memcmp(buf, "abcdef\ngh\nij", 12) == 0);
	close(fd);
}

Test(fmt_flush, buffered_overflow)
{
	int fd = make_shm_fd("flush-overflow");
	struct format_output out = format_output_fd(fd);

	char a[600], b[600];
	memset(a, 'a', sizeof(a));
	memset(b, 'b', sizeof(b));

	cr_assert_eq(format_output_write(&out, a, sizeof(a)), 0);
	cr_assert_eq(fd_size(fd), 0);
	/* This write does not fit in the 1024-byte buffer: the buffered content
	 * must not be overwritten by the flushing logic */
	cr_assert_eq(format_output_write(&out, b, sizeof(b)), 0);

	format_output_destroy(&out);
	cr_assert_eq(fd_size(fd), 1200);

	char buf[1200];
	cr_assert_eq(read_fd(fd, buf, sizeof(buf)), 1200);
	cr_assert(memcmp(buf, a, 600) == 0);
	cr_assert(memcmp(buf + 600, b, 600) == 0);
	close(fd);
}

Test(fmt_flush, memory_noop)
{
	/* Setting the flush mode on memory outputs has no effect */
	struct format_output out = format_output_buf();
	format_output_set_flush(&out, kFormatFlushAlways);
	const int result_buf = format(&out, "hello");
	cr_assert_eq(result_buf, 0);
	cr_assert_eq(out.size, 5);
	cr_assert(memcmp(out.data, "hello", 5) == 0);
	format_output_destroy(&out);

	struct format_output none = format_output_none();
	format_output_set_flush(&none, kFormatFlushNone);
	const int result_none = format(&none, "hello");
	cr_assert_eq(result_none, 0);
	cr_assert_eq(none.size, 5);
	format_output_destroy(&none);
}

static int g_eintr_read_fd;
static size_t g_eintr_drain;

static void
eintr_handler(int signo)
{
	(void)signo;
	char buf[64];
	while (g_eintr_drain > 0) {
		const size_t chunk = g_eintr_drain < sizeof(buf) ? g_eintr_drain : sizeof(buf);
		const ssize_t n = read(g_eintr_read_fd, buf, chunk);
		if (n <= 0)
			break;
		g_eintr_drain -= (size_t)n;
	}
}

Test(fmt_flush, eintr)
{
	int fds[2];
	cr_assert_eq(pipe(fds), 0);

	/* Fill the pipe so that the next write blocks */
	const int flags = fcntl(fds[1], F_GETFL);
	cr_assert(flags != -1);
	cr_assert(fcntl(fds[1], F_SETFL, flags | O_NONBLOCK) != -1);
	char fill[4096];
	memset(fill, 'x', sizeof(fill));
	size_t filled = 0;
	ssize_t n;
	while ((n = write(fds[1], fill, sizeof(fill))) > 0)
		filled += (size_t)n;
	cr_assert(errno == EAGAIN || errno == EWOULDBLOCK);
	cr_assert(fcntl(fds[1], F_SETFL, flags) != -1);

	g_eintr_read_fd = fds[0];
	/* Drain whole pages: pipe writes are accounted per buffer slot, so
	 * freeing a few bytes is not enough to unblock the writer */
	g_eintr_drain = 8192;

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = eintr_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0; /* no SA_RESTART: the blocked write must return EINTR */
	cr_assert_eq(sigaction(SIGALRM, &sa, NULL), 0);

	struct format_output out = format_output_fd(fds[1]);
	const int result_eintr = format(&out, "abc");
	cr_assert_eq(result_eintr, 0);

	ualarm(100000, 0);
	cr_assert_eq(format_output_flush(&out), 0);
	ualarm(0, 0);

	format_output_destroy(&out);
	/* Close the write end so the drain loop below sees EOF */
	close(fds[1]);

	/* The interrupted flush must have written its 3 bytes after the signal
	 * handler freed some room in the pipe */
	size_t total = 0;
	size_t last_pos = 0;
	char last[3] = {0};
	char buf[4096];
	for (;;) {
		const ssize_t r = read(fds[0], buf, sizeof(buf));
		if (r <= 0)
			break;
		for (ssize_t k = 0; k < r; ++k) {
			last[last_pos] = buf[k];
			last_pos = (last_pos + 1) % sizeof(last);
		}
		total += (size_t)r;
	}
	cr_assert_eq(total, filled - 8192 + 3);
	char ordered[3];
	for (size_t k = 0; k < sizeof(ordered); ++k)
		ordered[k] = last[(last_pos + k) % sizeof(last)];
	cr_assert(memcmp(ordered, "abc", 3) == 0);

	close(fds[0]);
	close(fds[1]);
}
