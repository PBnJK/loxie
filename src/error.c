/**
 * @file error.c
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Helper para registro de erros
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "error.h"

void errFatal( const size_t LINE, const char *MSG, ... ) {
	if( LINE ) {
		fprintf(stderr, COLOR_RED "ERRO" COLOR_RESET " [linha %d]: ", LINE);
	} else {
		fputs(COLOR_RED "ERRO" COLOR_RESET ": ", stderr);
	}

	va_list args;
	va_start(args, MSG);

	vfprintf(stderr, MSG, args);

	va_end(args);

	printf("\n");
}

void errWarn( const size_t LINE, const char *MSG, ... ) {
	if( LINE ) {
		fprintf(stderr, COLOR_YELLOW "AVISO" COLOR_RESET " [linha %d]: ", LINE);
	} else {
		fputs(COLOR_YELLOW "AVISO" COLOR_RESET ": ", stderr);
	}
	
	va_list args;
	va_start(args, MSG);

	vfprintf(stderr, MSG, args);

	va_end(args);

	printf("\n");
}

