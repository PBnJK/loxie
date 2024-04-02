/**
 * @file debug.c
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Funções para auxiliar o desenvolvimento
 */

#include <stdio.h>

#include "debug.h"

/**
 * @brief Imprime uma operação sem operandos
 *
 * @param[in] NAME Nome da operação
 * @param[in] offset Índice no array de bytes
 *
 * @return O próximo índice
 */
static size_t _debugSimpleOp( const char *NAME, size_t offset );

void debugDisassembleChunk( Chunk *chunk, const char *NAME ) {
	printf("=== %s ===\n", NAME);
	
	for( size_t offset = 0; offset < chunk->count; ) {
		offset = debugDisassembleInstruction(chunk, offset);
	}
}

size_t debugDisassembleInstruction( Chunk *chunk, size_t offset ) {
	printf("%04d ", offset);

	const uint8_t OP = chunk->code[offset];
	switch(OP) {
		case OP_RETURN:
			return _debugSimpleOp("OP_RETURN", offset);
		default:
			printf("Instrução desconhecida %02x\n", OP);
			return offset + 1;
	}
}

static size_t _debugSimpleOp( const char *NAME, size_t offset ) {
	printf("%-16s\n", NAME);
	return offset + 1;
}

