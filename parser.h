#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum {
	ND_PROGRAM,
	ND_FN_DEF,
	ND_FN_ARG_LIST,
	ND_COMPOUND_STMT,
	ND_RETURN_STMT,
	ND_EMPTY_STMT,
	ND_FROM_TOKEN,
} NodeType;

typedef struct __ast_node__ {
	const Token *tk;

	NodeType type;

	struct __ast_node__ *children;
	struct __ast_node__ *sibling;
} ASTNode;

ASTNode *parse_program(Lexer *lexer);

const char *node_type_as_str(ASTNode *nd);

void print_tree(ASTNode *root, size_t init_spacing);

ASTNode *nth_child(ASTNode *root, size_t n);

#endif /* PARSER_H */
