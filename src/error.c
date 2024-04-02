/**
 * @file error.c
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Helper para registro de erros
 */

#include <stdlib.h>		/* exit()    */
#include <stdio.h>		/* fprintf() */

#include "error.h"

void errFatal( const char *MSG, const uint8_t ERR_CODE ) {
	fprintf(stderr, "FATAL ERROR: %s\n", MSG);

	exit(ERR_CODE);
}
