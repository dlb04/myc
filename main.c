#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <stdnoreturn.h>

#include <err.h>

#include "lexer.h"
#include "parser.h"
#include "codegen.h"

static void usage(const char *pnam, FILE *out) {
	fprintf(out, "Usage: %s FILE\n", pnam);
}

size_t filesz(FILE *f) {
	fseek(f, 0, SEEK_END);
	size_t sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	return sz;
}

int main(int argc, char **argv) {
	
	const char *program_name = *argv;

	// argument parsing
	if (argc != 2) {
		usage(program_name, stderr);
		errx(1, "no output file provided");
	}

	if (!strcmp(argv[1], "-h")) {
		usage(program_name, stdout);
		exit(0);
	}

	FILE *input = fopen(argv[1], "r");
	if (input == NULL)
		err(errno, "%s: ", argv[1]);

	Lexer *lexer = lexer_from_file(input, argv[1]);
	fclose(input);
	ASTNode *program = parse_program(lexer);

	print_tree(program, 0);

	FILE *output = fopen("output.asm", "w");
	if (output == NULL)
		err(errno, "cannot create output file");

	StrBuf *code = codegen(program);
	strbuf_to_file(output, code);
	fclose(output);

	return 0;
}
