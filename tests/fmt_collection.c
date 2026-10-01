#include "util.h"

Test(fmt_collection, basic)
{
	const int int_arr[16] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	};
	const long long_arr[16] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	};
	const char* str_arr[16] = {
		"lorem", "ipsum", "dolor", "sit", "amet", "foo", "bar",   "baz",
		"quz",   "quz",   "",      "-",   "{}",   ".",   "hello", "world"
	};
	for (size_t i = 0; i < 16; ++i) {
		char out[8192];
#define manual(start, end, fmt, fmt_sep, array)                                                  \
	do {                                                                                         \
		size_t pos = 0;                                                                          \
		pos += (size_t)sprintf(out + pos, start);                                                \
		for (size_t j = 0; j < i; ++j) {                                                         \
			if (j != 0)                                                                          \
				pos += (size_t)sprintf(out + pos, fmt_sep, array[j]);                            \
			else                                                                                 \
				pos += (size_t)sprintf(out + pos, fmt, array[j]);                                \
		}                                                                                        \
		sprintf(out + pos, end);                                                                 \
	} while (0)

		manual("{", "}", "%d", ", %d", int_arr);
		test_manual("{:[{1}]:{}}", out, FORMAT_ARRAY(int_arr), i);
		manual("{", "}", "%x", ", %x", int_arr);
		test_manual("{:[{1}]:{x}}", out, FORMAT_ARRAY(int_arr), i);

		manual("{", "}", "%ld", ", %ld", long_arr);
		test_manual("{:[{1}]:{}}", out, FORMAT_ARRAY(long_arr), i);
		manual("{", "}", "%lx", ", %lx", long_arr);
		test_manual("{:[{1}]:{x}}", out, FORMAT_ARRAY(long_arr), i);

		manual("{", "}", "%s", ", %s", str_arr);
		test_manual("{:[{1}]:{}}", out, FORMAT_ARRAY(str_arr, format_fmt_str), i);

		manual("[", "]", "%d", " %d", int_arr);
		test_manual("{:[{1}]#[ ]:{}}", out, FORMAT_ARRAY(int_arr), i);
		manual("[", "]", "%x", " %x", int_arr);
		test_manual("{:[{1}]#[ ]:{x}}", out, FORMAT_ARRAY(int_arr), i);

		manual("[", "]", "%ld", " %ld", long_arr);
		test_manual("{:[{1}]#[ ]:{}}", out, FORMAT_ARRAY(long_arr), i);
		manual("[", "]", "%lx", " %lx", long_arr);
		test_manual("{:[{1}]#[ ]:{x}}", out, FORMAT_ARRAY(long_arr), i);

		manual("[", "]", "%s", " %s", str_arr);
		test_manual("{:[{1}]#[ ]:{}}", out, FORMAT_ARRAY(str_arr, format_fmt_str), i);

		manual("{<", ">}", "%d", "| |%d", int_arr);
		test_manual("{:[{1}]#{2}{3}{4}:{}}", out, FORMAT_ARRAY(int_arr), i, "{<", "| |", ">}");
		manual("{<", ">}", "%x", "| |%x", int_arr);
		test_manual("{:[{1}]#{2}{3}{4}:{x}}", out, FORMAT_ARRAY(int_arr), i, "{<", "| |", ">}");

		manual("{<", ">}", "%ld", "| |%ld", long_arr);
		test_manual("{:[{1}]#{2}{3}{4}:{}}", out, FORMAT_ARRAY(long_arr), i, "{<", "| |", ">}");
		manual("{<", ">}", "%lx", "| |%lx", long_arr);
		test_manual("{:[{1}]#{2}{3}{4}:{x}}", out, FORMAT_ARRAY(long_arr), i, "{<", "| |", ">}");

		manual("{<", ">}", "%s", "| |%s", str_arr);
		test_manual("{:[{1}]#{2}{3}{4}:{}}",
		            out,
		            FORMAT_ARRAY(str_arr, format_fmt_str),
		            i,
		            "{<",
		            "| |",
		            ">}");
#undef manual
	}
}

/* Linked list */
struct node
{
	int val;
	struct node* next;
};

/* Linked-list iterator */
static inline int
format_collection_node(const struct format_arg_collection* collection,
                       const void* head,
                       size_t n,
                       format_collection_callback callback,
                       void* cookie)
{
	struct node* node = (struct node*)head;
	for (size_t i = 0; node && i < n; ++i) {
		int result;
		cr_assert_eq(collection->elem_size, sizeof(struct node));
		cr_assert_eq(collection->is_pointer, 0);
		uint64_t value = node->val;
		result = callback(collection, value, cookie);
		if (result == -1)
			return -1;
		if (result == 0)
			break;
		node = node->next;
	}

	return 0;
}

Test(fmt_collection, custom_collection)
{
	const struct node head = {
		.val = 1,
		.next =
		  &(struct node){
		    .val = 2,
		    .next =
		      &(struct node){
		        .val = 3,
		        .next =
		          &(struct node){
		            .val = 4,
		            .next =
		              &(struct node){
		                .val = 5,
		                .next =
		                  &(struct node){
		                    .val = 6,
		                    .next = 0,
		                  },
		              },
		          },
		      },
		  },
	};

	test_manual("{:[0]:{}}", "{}", FORMAT_COLLECTION(&head, format_collection_node, format_fmt_int));
	test_manual("{:[1]:{}}", "{1}", FORMAT_COLLECTION(&head, format_collection_node, format_fmt_int));
	test_manual("{:[6]:{}}", "{1, 2, 3, 4, 5, 6}", FORMAT_COLLECTION(&head, format_collection_node, format_fmt_int));

	/* The iterator stops at the first NULL node */
	test_manual("{:[10]:{}}", "{1, 2, 3, 4, 5, 6}", FORMAT_COLLECTION(&head, format_collection_node, format_fmt_int));
	test_manual("{:[6]:{b}}", "{1, 10, 11, 100, 101, 110}", FORMAT_COLLECTION(&head, format_collection_node, format_fmt_int));
}
