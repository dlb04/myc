#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>

#include <err.h>

#include "common.h"

#include "lexer.h"

extern size_t filesz(FILE *);

inline static char *pos(Lexer *l) {
	return l->src + l->pos;
}

inline static char chr(Lexer *l) {
	return l->src[l->pos];
}

inline static bool can_read(Lexer *l) {
	return l->src_len > l->pos;
}

inline static bool can_peek(Lexer *l, size_t offset) {
	const size_t p = l->pos + offset;
	return p < l->src_len;
}

inline static bool peek(Lexer *l, size_t offset) {
	return l->src[l->pos + offset];
}

inline static void adv(Lexer *l) {
	l->col++;
	l->pos++;
}

static Token *token(const char *repr, size_t len,
		TokenType type, TokenMetadata *metadata) {

	Token *token = malloc(sizeof(*token));
	if (token == NULL)
		err(errno, "failed allocation");

	*token = (Token) {
		.repr = repr,
		.len = len,
		.type = type,
	};

	if (metadata)
		memcpy(&token->metadata, metadata, sizeof(*metadata));

	return token;
}

static TokenType iden_or_keyword(const char *start, size_t len) {
	static char *kw_list[] = {
		"fn", "return",
	};
	static size_t kws = sizeof(kw_list)/sizeof(char*);
	for (size_t i = 0; i < kws; i++) {
		if (memcmp(start, kw_list[i], len) == 0)
			return __START_OF_KEYWORDS__ + i + 1;
	}

	return TK_IDN;
}

Lexer *lexer_from_file(FILE *input, const char* filepath) {

	Lexer *lexer = malloc(sizeof(*lexer));

	if (!lexer)
		err(errno, "failed allocation");

	size_t src_len = filesz(input);
	char *src = malloc(src_len);
	fread(src, 1, src_len, input);
	src[--src_len] = 0;

	*lexer = (Lexer) {
		.src = src,
		.src_len = src_len,
		.filepath = filepath,
		.pos = 0,
		.col = 0,
		.row = 0,
	};

	return lexer;
}

static inline void adv_line(Lexer *l) {
	l->row++;
	l->col = 0;
}

Token *lexer_next_token(Lexer *lexer) {
	const char *repr = NULL;
	size_t len = 0;
	TokenType type = 0;

	TokenMetadata metadata = {
		.filepath = lexer->filepath,
	};

	for (; can_read(lexer); adv(lexer)) {
		metadata.row = lexer->row;
		metadata.col = lexer->col;

		repr = pos(lexer);

		const int c = chr(lexer);

		/**
		 * Tokens whose type can be known by just looking at the current
		 * character
		 */
		switch (c) {
			case '\n':
				adv_line(lexer);
			case ' ': case '\t':
				continue;

			case ';':
				type = TK_SEMICOLON;
				len = 1;
				adv(lexer);
				goto valid_token;

			case '(': case ')':
				type = TK_LPAREN + (c == ')');
				len = 1;
				adv(lexer);
				goto valid_token;

			case '}': case '{':
				type = TK_LBRACKET + (c == '}');
				len = 1;
				adv(lexer);
				goto valid_token;

			case '-':
				// Handle negative integers
				if (!can_peek(lexer, 1) || !isdigit(peek(lexer, 1)))
					errx(1, "expected integer, but only found `-`");

			__attribute__ ((fallthrough));
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				// TODO: support multidigit integers.
				type = TK_INT;
				len = 1;
				adv(lexer);
				goto valid_token;

			default: 
				break;
		}

		/* Handle identifiers or keywords */
		while (chr(lexer) == '_' || isalpha(chr(lexer))) {
			len++;
			adv(lexer);
		}

		if (len != 0) {
			type = iden_or_keyword(repr, len);
			goto valid_token;
		}

		errx(1, "invalid character `%c`", c);
	}

	/* End of file */
	return token(NULL, 0, TK_EOF, NULL);

valid_token:
	return token(repr, len, type, &metadata);
}

/**
 * Lookahead of one token.
 */
Token *lexer_peek(Lexer *l) {
	Token *tk = lexer_next_token(l);
	l->pos -= tk->len;
	return tk;
}

const char *token_str(const Token *tk) {
	static char buf[64] = {};
	const size_t bufsiz = sizeof(buf);

	if (tk->type == TK_EOF)
		return "EOF";

	assert(tk->len < bufsiz && "Token is too large");
	memset(buf, 0, bufsiz);
	return memcpy(buf, tk->repr, tk->len);
}

const char *token_type_str(TokenType type) {
	switch (type) {
		RETURN_CASE_LABEL(TK_EOF);
		RETURN_CASE_LABEL(TK_INT);
		RETURN_CASE_LABEL(TK_IDN);
		RETURN_CASE_LABEL(TK_LPAREN);
		RETURN_CASE_LABEL(TK_RPAREN);
		RETURN_CASE_LABEL(TK_LBRACKET);
		RETURN_CASE_LABEL(TK_RBRACKET);
		RETURN_CASE_LABEL(TK_SEMICOLON);
		RETURN_CASE_LABEL(TK_FN);
		RETURN_CASE_LABEL(TK_RETURN);
		default:
			return "UNKTKTYPE";
	}
}

void print_file(Lexer *l) {
	printf("[INFO]: Printing input file\n");
	printf("%s\n", l->src); 
}
