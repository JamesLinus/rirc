#include "test.h"

#include "../src/utils.c" /* FIXME: word_wrap */
#include "../src/buffer.c"

#include <limits.h>

static char*
_fmt_int(int i)
{
	static char buff[1024] = {0};

	if ((snprintf(buff, sizeof(buff) - 1, "%d", i)) < 0)
		fail_test("snprintf");

	return buff;
}

static void
_buffer_newline(struct buffer *b, const char *t)
{
	/* abstract newline with default values */

	buffer_newline(b, BUFFER_LINE_OTHER, "", t);
}

static void
test_buffer_f(void)
{
	/* Test retrieving the first line after pushing to a full buffer */

	int i;

	struct buffer b = {0};

	assert_equals(buffer_size(&b), 0);

	for (i = 0; i < BUFFER_LINES_MAX + 1; i++)
		_buffer_newline(&b, _fmt_int(i + 1));

	assert_strcmp(buffer_f(&b)->text, _fmt_int(BUFFER_LINES_MAX + 1));
	assert_equals(buffer_size(&b), BUFFER_LINES_MAX);
}

static void
test_buffer_l(void)
{
	/* Test retrieving the last line after pushing to a full buffer */

	int i;

	struct buffer b = {0};

	assert_equals(buffer_size(&b), 0);

	for (i = 0; i < BUFFER_LINES_MAX; i++)
		_buffer_newline(&b, _fmt_int(i + 1));

	assert_strcmp(buffer_l(&b)->text, _fmt_int(1));
	assert_equals(buffer_size(&b), BUFFER_LINES_MAX);

	_buffer_newline(&b, _fmt_int(i + 1));

	assert_strcmp(buffer_l(&b)->text, _fmt_int(2));
	assert_equals(buffer_size(&b), BUFFER_LINES_MAX);
}

static void
test_buffer_index_overflow(void)
{
	/* Test masked indexing after unsigned integer overflow */

	struct buffer b = {
		.head = UINT_MAX,
		.tail = UINT_MAX - 1
	};

	assert_equals(buffer_size(&b), 1);
	assert_equals(MASK(b.head), (BUFFER_LINES_MAX - 1));

	_buffer_newline(&b, _fmt_int(0));

	assert_equals(buffer_size(&b), 2);
	assert_equals(MASK(b.head), 0);

	_buffer_newline(&b, _fmt_int(-1));

	assert_equals(buffer_size(&b), 3);
	assert_strcmp(b.buffer_lines[0].text, _fmt_int(-1));
}

static void
test_buffer_line_overlength(void)
{
	/* Test that lines over the maximum length are recursively split and added separately */

	struct buffer b = {0};

	/* Indices to first and last positions of lines, total length = 2.5 times the maximum */
	unsigned int f1 = 0,
	             l1 = TEXT_LENGTH_MAX - 1,
	             f2 = TEXT_LENGTH_MAX,
	             l2 = TEXT_LENGTH_MAX * 2 - 1,
	             f3 = TEXT_LENGTH_MAX * 2,
	             l3 = TEXT_LENGTH_MAX * 2 + TEXT_LENGTH_MAX / 2 - 1;

	/* Add a line that's 2.5 times the maximum length */
	char text[l3 + 1];

	memset(&text, ' ', sizeof(text) - 1);

	text[f1] = 'a';
	text[l1] = 'A';
	text[f2] = 'b';
	text[l2] = 'B';
	text[f3] = 'c';
	text[l3] = 'C';

	text[sizeof(text)] = 0;

	_buffer_newline(&b, text);

	assert_equals((int)b.buffer_lines[0].text_len, TEXT_LENGTH_MAX);
	assert_equals((int)b.buffer_lines[2].text_len, TEXT_LENGTH_MAX / 2);

	assert_equals(buffer_size(&b), 3);

	assert_equals(b.buffer_lines[0].text[0], 'a');
	assert_equals(b.buffer_lines[0].text[TEXT_LENGTH_MAX - 1], 'A');

	assert_equals(b.buffer_lines[1].text[0], 'b');
	assert_equals(b.buffer_lines[1].text[TEXT_LENGTH_MAX - 1], 'B');

	assert_equals(b.buffer_lines[2].text[0], 'c');
	assert_equals(b.buffer_lines[2].text[TEXT_LENGTH_MAX / 2 - 1], 'C');
}

static void
test_buffer_line_rows(void)
{
	/* Test calculating the number of rows a buffer line occupies */

	struct buffer b = {0};
	struct buffer_line *l;

	char *text = "aa bb cc";

	_buffer_newline(&b, text);

	l = buffer_f(&b);

	/* 1 column: 6 rows. word wrap skips whitespace prefix in line continuations:
	 * a
	 * a
	 * b
	 * b
	 * c
	 * c
	 * */
	assert_equals(buffer_line_rows(l, 1), 6);

	/* 4 columns: 3 rows:
	 * 'aa b' -> wraps to
	 *   'aa'
	 *   'bb c'
	 * 'bb c' -> wraps to
	 *   'bb'
	 *   'cc'
	 * */
	assert_equals(buffer_line_rows(l, 4), 3);

	/* Greater columns than length should always return one row */
	assert_equals(buffer_line_rows(l, sizeof(text) + 1), 1);
}

int
main(void)
{
	testcase tests[] = {
		&test_buffer_f,
		&test_buffer_l,
		&test_buffer_index_overflow,
		&test_buffer_line_overlength,
		&test_buffer_line_rows
	};

	return run_tests(tests);
}