#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <err.h>

#include "strbuf.h"

static void strbuf_grow(StrBuf *buf, size_t increment) {
	const size_t newcap = buf->capacity + increment + 8;

	char *np = realloc(buf->data, newcap);
	if (!np)
		err(errno, "failed realloc");

	buf->data = np;
	buf->capacity = newcap;
}

StrBuf *strbuf(size_t capacity) {
	capacity += 8;
	char *data = malloc(capacity);

	if (data == NULL)
		err(errno, "failed allocation");

	StrBuf *buf = malloc(sizeof(*buf));

	*buf = (StrBuf) {
		.data = data,
		.capacity = capacity,
		.len = 0,
	};

	return buf;
}

void strbuf_writeln(StrBuf *buf, const char* str) {
	const size_t line_len = strlen(str) + 1;
	if (buf->len + line_len >= buf->capacity)
		strbuf_grow(buf, line_len);

	char *p = buf->data + buf->len;
	memcpy(p, str, line_len - 1);
	p[line_len - 1] = '\n';
	buf->len += line_len;
}

void strbuf_writec(StrBuf *buf, char c) {
	if (buf->len + 1 >= buf->capacity)
		strbuf_grow(buf, 8);

	buf->data[buf->len++] = c;
}

void strbuf_vwriteln(StrBuf *buf, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	size_t remaining_cap = buf->capacity - buf->len;
	char *p = buf->data + buf->len;
	int inc = 0;
	while ( (inc = vsnprintf(p, remaining_cap, fmt, ap)) < 0) {
		strbuf_grow(buf, 64);
		p = buf->data + buf->len;
		remaining_cap = buf->capacity - buf->len;
	}
	buf->len += inc;
	strbuf_writec(buf, '\n');
}

void strbuf_to_file(FILE *output, StrBuf *buf) {
	fwrite(buf->data, 1, buf->len, output);
}
