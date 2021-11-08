#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

typedef struct {
	char *src;
	const char *filepath;
	size_t src_len;
	size_t pos;

	size_t row;
	size_t col;
} Lexer;

typedef struct {
	const char *filepath;
	size_t row, col;
} TokenMetadata;

typedef enum {
	TK_EOF,
	TK_INT,
	TK_IDN,
	TK_LPAREN,
	TK_RPAREN,
	TK_LBRACKET,
	TK_RBRACKET,
	TK_SEMICOLON,

	// used to calculate the type in iden_or_keyword()
	// append new keywords at the end.
	__START_OF_KEYWORDS__,
	TK_FN,
	TK_RETURN,

} TokenType;

typedef struct {
	TokenType type;
	const char *repr;
	size_t len;
	
	TokenMetadata metadata; // for better error reporting
}Token;

Lexer *lexer_from_file(FILE *input, const char *filepath);

Token *lexer_next_token(Lexer *lexer);

/**
 * Lookahead of one token.
 */
Token *lexer_peek(Lexer *lexer);

/**
 * Returns a NUL terminated string representing TK
 */
const char *token_str(const Token *tk);

const char *token_type_str(TokenType type);

void print_file(Lexer *l);

#endif /* LEXER_H */
