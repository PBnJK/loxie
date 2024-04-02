/**
 * @file chunk.c
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Definição de uma sequência de bytes (<i>bytecode</i>)
 */

#include "chunk.h"
#include "memory.h"

void chunkInit( Chunk *chunk ) {
	chunk->count = 0;
	chunk->size  = 0;

	/* Não alocamos nada aqui, chunkWrite lidará com isso */
	chunk->code  = NULL;
}

void chunkFree( Chunk *chunk ) {
	MEM_FREE_ARRAY(uint8_t, chunk->code, chunk->size);
	chunkInit(chunk);
}

void chunkWrite( Chunk *chunk, const uint8_t BYTE ) {
	if( chunk->size < chunk->count + 1 ) {
		const size_t OLD_SIZE = chunk->size;
		chunk->size = MEM_GROW_SIZE(OLD_SIZE);

		chunk->code = MEM_GROW_ARRAY(
			uint8_t, chunk->code, OLD_SIZE, chunk->size
		);
	}

	chunk->code[chunk->count] = BYTE;
	++chunk->count;
}
