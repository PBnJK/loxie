/**
 * @file compiler.c
 * @author Pedro B.
 * @date 2024.04.02
 *
 * @brief Compila a linguagem NEAT
 */

#include <stdio.h>

#include "compiler.h"
#include "scanner.h"
#include "token.h"

void compCompile( const char *SOURCE ) {
	scannerInit(SOURCE);

	size_t line = 0;
	while(true) {
		Token token = scanToken();

		if( token.line != line ) {
			printf("%4d ", token.line);
			line = token.line;
		} else {
			printf("   | ");
		}

		printf("%2d '%.*s'\n", token.type, token.length, token.START);

		if( token.type == TOKEN_EOF ) {
			return;
		}
	}
}

