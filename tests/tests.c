#include "util.h"

ReportHook(POST_TEST)(struct criterion_test_stats* stats)
{
	cr_log_info("Test [%s/%s]: %u passed, %u failed, %u total asserts",
	            stats->test->category,
	            stats->test->name,
	            stats->passed_asserts,
	            stats->failed_asserts,
	            stats->passed_asserts + stats->failed_asserts);
}

ReportHook(POST_ALL)(struct criterion_global_stats *stats)
{
	cr_log_info("Total: %zu/%zu passed asserts", stats->asserts_passed, stats->asserts_passed + stats->asserts_failed);
}
