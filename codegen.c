/**
 * Code generation family of functions.
 * Generate x64 ASM.
 * Every function that generates ASM has the name "gen_{thing that generates}"
 */

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "parser.h"
#include "strbuf.h"

static StrBuf *output = NULL;

static void gen_return_stmt(ASTNode *root, bool use_tab) {
	const Token *retval_tk = nth_child(root, 1)->tk;
	assert (retval_tk && retval_tk->type == TK_INT);
	int retval = atoi(token_str(retval_tk));
	if (use_tab) {
		strbuf_vwriteln(output, "\tmov rax, %d", retval);
		strbuf_writeln(output, "\tret");
	} else {
		strbuf_vwriteln(output, "mov rax, %d", retval);
		strbuf_writeln(output, "ret");
	}
}

static void gen_compound_stmt(ASTNode *root) {
	assert(root->type == ND_COMPOUND_STMT);

	ASTNode *cur_stmt;
	for (size_t i = 1; (cur_stmt=nth_child(root,i))->type != ND_FROM_TOKEN; ++i) {
		switch (cur_stmt->type) {
		case ND_RETURN_STMT:
			gen_return_stmt(cur_stmt, true);
			break;
		case ND_EMPTY_STMT:
			continue;
		default:
			assert(0 && "i am tired");
		}
	}
}

static void gen_fndef(ASTNode *root) {
	assert(root->type == ND_FN_DEF);
	
	ASTNode *fn_name = nth_child(root, 1);
	// ignore argument list for now.
	strbuf_vwriteln(output, "%s:", token_str(fn_name->tk));
	ASTNode *compound_stmt = nth_child(root, 3);
	gen_compound_stmt(compound_stmt);
}

StrBuf *codegen(ASTNode *root) {
	StrBuf *generated_asm = strbuf(64);
	output = generated_asm;

	assert(root->type == ND_PROGRAM);
	assert(root->children->type == ND_FN_DEF);

	strbuf_writeln(output, "section .text");

	gen_fndef(root->children);

	return generated_asm;
}
