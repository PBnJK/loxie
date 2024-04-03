/**
 * @file memory.c
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Lida com operações relacionadas à memória
 */

#include <stdlib.h>

#include "memory.h"
#include "error.h"

void *memRealloc( void *pointer, const size_t OLD_SIZE, const size_t NEW_SIZE ) {
	if( NEW_SIZE == 0 ) {
		/* Se o novo tamanho for 0, libere a memória, já que
		 * realloc com tamanho 0 é UB.
		 */
		free(pointer);
		return NULL;
	}

	void *result = realloc(pointer, NEW_SIZE);
	if( result == NULL ) {
		/* Um erro aconteceu quando tentamos alocar mais memória
		 * Ou a memória RAM acabou ( x _ x ) ...
		 * ... ou um erro bisonho aconteceu.
		 *
		 * De qualquer forma, sai com código ERR_UNAVAILABLE
		 */
		errFatal(0, "Nao foi possivel alocar memoria!");
		exit(69);
	}

	return result;
}

