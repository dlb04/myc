#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>

#include <err.h>

#include "common.h"
#include "lexer.h"
#include "parser.h"

static ASTNode *astnode(NodeType type, ASTNode *children, ASTNode *sibling) {
	ASTNode *nd = malloc(sizeof(*nd));

	if (!nd)
		err(errno, "failed allocation");

	nd->tk = NULL;
	nd->type = type;
	nd->children = children;
	nd->sibling = sibling;

	return nd;
}

static ASTNode *astnode_from_token(Token *tk) {
	ASTNode *nd = malloc(sizeof(*nd));

	if (!nd)
		err(errno, "failed allocation");

	nd->tk = tk;
	nd->type = ND_FROM_TOKEN;
	nd->children = NULL;
	nd->sibling = NULL;

	return nd;
}

/**
 * Displays an error of the form:
 * `file:row:col: expected X but found Y`
 */
static inline void display_expected_tk_error(const Token *tk, TokenType expected) {
	printf("%s:%zu:%zu: expected %s but found %s\n",
			tk->metadata.filepath,
			tk->metadata.row,
			tk->metadata.col,
			token_type_str(expected),
			token_str(tk));
}

/**
 * Operations on the tree
 */

static void push_sibling(ASTNode *root, ASTNode *sibling) {
	if (root->sibling)
		push_sibling(root->sibling, sibling);
	else
		root->sibling = sibling;
}

static void push_children(ASTNode *root, ASTNode *children) {
	if (root->children)
		push_sibling(root->children, children);
	else
		root->children = children;
}

/**
 * Returns the nth, starting from 0, child of a node.
 * This is mainly used for code generation.
 */
ASTNode *nth_child(ASTNode *root, size_t n) {
	ASTNode *child = root->children;
	for (size_t i = 1; i <= n; ++i) {
		assert(child != NULL && "there are less children than requested");
		child = child->sibling;
	}

	return child;
}

static inline Token *expect(Lexer *l, TokenType expected) {
	Token *tk = lexer_next_token(l);
	if (tk->type == expected) {
		return tk;
	}
	else {
		display_expected_tk_error(tk, expected);
		exit(1);
	}
}

static inline Token *match_and_fail(Token *tk, TokenType expected) {
	if (tk->type == expected)
		return tk;
	else {
		display_expected_tk_error(tk, expected);
		exit(1);
	}
}

static inline bool match(Token *tk, TokenType type) {
	return tk->type == type;
}

static ASTNode *fn_arg_list(Lexer *l) {
	ASTNode *_fn_arg_list = astnode(ND_FN_ARG_LIST, NULL, NULL);

	push_children(_fn_arg_list,
			astnode_from_token(expect(l, TK_LPAREN)));

	push_children(_fn_arg_list,
			astnode_from_token(expect(l, TK_RPAREN)));

	return _fn_arg_list;
}

static ASTNode *return_stmt(Lexer *l) {
	ASTNode *retstmt = astnode(ND_RETURN_STMT, NULL, NULL);

	push_children(retstmt,
			astnode_from_token(expect(l, TK_RETURN)));
	push_children(retstmt,
			astnode_from_token(expect(l, TK_INT)));
	push_children(retstmt,
			astnode_from_token(expect(l, TK_SEMICOLON)));

	return retstmt;
}

/**
 * CompoundStmt -> "{" Statement* "}"
 */
static ASTNode *compound_stmt(Lexer *l) {
	ASTNode *_compound_stmt = astnode(ND_COMPOUND_STMT, NULL, NULL);

	push_children(_compound_stmt,
			astnode_from_token(expect(l, TK_LBRACKET)));

	Token *tk = lexer_peek(l);

	if (match(tk, TK_RETURN)) {
		push_children(_compound_stmt, return_stmt(l));
	} else if (match(tk, TK_SEMICOLON)) {
		push_children(_compound_stmt, astnode(ND_EMPTY_STMT, NULL, NULL));
	}

	push_children(_compound_stmt,
			astnode_from_token(expect(l, TK_RBRACKET)));

	return _compound_stmt;
}

/**
 * FnDcl -> "fn" Identifer FnArgList CompoundStmt
 */
static ASTNode *fndef(Lexer *l) {
	Token *tk = lexer_next_token(l);

	if (match(tk, TK_FN)) {
		ASTNode *_fndcl = astnode(ND_FN_DEF, NULL, NULL);

		push_children(_fndcl, astnode_from_token(tk));
		push_children(_fndcl, astnode_from_token(expect(l, TK_IDN)));
		push_children(_fndcl, fn_arg_list(l));
		push_children(_fndcl, compound_stmt(l));
		
		return _fndcl;
	}

	return NULL;
}

/**
 * See README.md:Grammar
 *
 * Program -> FnDef Eof
*/
ASTNode *parse_program(Lexer *lexer) {

	ASTNode *f= fndef(lexer);
	Token *t = lexer_next_token(lexer);
	assert(t->type == TK_EOF);
	ASTNode *eof = astnode_from_token(t);
	ASTNode *program = astnode(ND_PROGRAM, NULL, NULL);

	push_children(program, f);
	push_children(program, eof);
	
	return program;
}

const char *node_type_as_str(ASTNode *nd) {
	NodeType type = nd->type;
	switch (type) {
		RETURN_CASE_LABEL(ND_PROGRAM);
		RETURN_CASE_LABEL(ND_FN_DEF);
		RETURN_CASE_LABEL(ND_FN_ARG_LIST);
		RETURN_CASE_LABEL(ND_COMPOUND_STMT);
		RETURN_CASE_LABEL(ND_RETURN_STMT);
		RETURN_CASE_LABEL(ND_EMPTY_STMT);
		case ND_FROM_TOKEN:
			assert(nd->tk != NULL);
			return token_type_str(nd->tk->type);
		default:
			return "UNKNDTYPE";
	}
}

static void _print_tree(ASTNode *root, size_t spacing, size_t inc) {
	if (root == NULL)
		return;

	for (size_t i = 0; i < spacing; i++)
		putchar(' ');

	const char *type = node_type_as_str(root);
	const char *repr = "";
	if (root->type == ND_FROM_TOKEN)
		repr = token_str(root->tk);

	printf("%s, `%s`\n", type, repr);

	_print_tree(root->children, spacing + inc, inc);
	_print_tree(root->sibling, spacing, inc);
}

void print_tree(ASTNode *root, size_t spacing) {
	printf("[INFO]: Dumping AST\n");
	_print_tree(root, spacing, 2UL);
}
