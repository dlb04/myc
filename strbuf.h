#ifndef STRBUF_H
#define STRBUF_H

#include <stddef.h>

typedef struct {
	char* data;
	size_t capacity;
	size_t len; // write offset
} StrBuf;

StrBuf *strbuf(size_t capacity);
void strbuf_writeln(StrBuf *buf, const char* str);
void strbuf_to_file(FILE *output, StrBuf *buf);
void strbuf_vwriteln(StrBuf *buf, const char *fmt, ...);

#endif /* STRBUF_H */
