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
	fprintf(out, "Usage: %s [OPTIONS] FILE\n", pnam);
	fprintf(out, "OPTIONS:\n");
	fprintf(out, "\t-h\t display this help\n");
	fprintf(out, "\t-l\t lex the file and print the tokens\n");
	fprintf(out, "\t-r\t parse the file and print the parse tree\n");
}

size_t filesz(FILE *f) {
	fseek(f, 0, SEEK_END);
	size_t sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	return sz;
}

void print_tokens(Lexer *lexer) {
	printf("[INFO]: Printing tokens\n");
	Token *tk;
	do {
		tk = lexer_next_token(lexer);
		printf("%s:%zu:%zu: <%02X, %s>\n",
				tk->metadata.filepath,
				tk->metadata.row,
				tk->metadata.col,
				tk->type,
				token_str(tk));
	} while(tk->type != TK_EOF);
}

enum {
	Compile,
	LexAndPrint,
	ParseAndPrint,
};

int main(int argc, char **argv) {
	
	const char *program_name = *argv;

	int what_to_do = Compile;
	const char *filepath = NULL;

	// argument parsing
	if (argc < 2) {
		usage(program_name, stderr);
		errx(1, "no output file provided");
	} else if (argc == 2) {
		filepath = argv[1];
	} else {
		filepath = argv[2];
		if (!strcmp(argv[1], "-h")) {
			usage(program_name, stdout);
			exit(0);
		} else if (!strcmp(argv[1], "-l")) {
			what_to_do = LexAndPrint;
		} else if (!strcmp(argv[1], "-t")) {
			what_to_do = ParseAndPrint;
		}
	} 

	FILE *input = fopen(filepath, "r");
	if (input == NULL)
		err(errno, "%s: ", filepath);

	Lexer *lexer = lexer_from_file(input, filepath);
	fclose(input);

	if (what_to_do == LexAndPrint) {
		print_tokens(lexer);
		exit(0);
	}

	ASTNode *program = parse_program(lexer);
	
	if (what_to_do == ParseAndPrint) {
		print_tree(program, 0);
		exit(0);
	}

	FILE *output = fopen("output.asm", "w");
	if (output == NULL)
		err(errno, "cannot create output file");

	StrBuf *code = codegen(program);
	strbuf_to_file(output, code);
	fclose(output);

	return 0;
}
