/**
 * @file debug.c
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Funções para auxiliar o desenvolvimento
 */

#include <stdio.h>

#include "debug.h"
#include "opcodes.h"
#include "error.h"

/**
 * @brief Imprime uma operação sem operandos
 *
 * @param[in] NAME Nome da operação
 * @param[in] offset Índice no array de bytes
 *
 * @return O próximo índice
 */
static size_t _simpleOp( const char *NAME, size_t offset );

/**
 * @brief Imprime uma operação com um operando que é um índice no array
 * de constantes
 *
 * @param[in] NAME Nome da operação
 * @param[in] chunk Ponteiro pra chunk onde reside a constante
 * @param[in] offset Índice no array de bytes
 *
 * @return O próximo índice, pulando a instrução + índice
 */
static size_t _const16Op( const char *NAME, Chunk *chunk, size_t offset );

/**
 * @brief Mesmo que @ref _Const16Op, mas com um índice 24-bit
 *
 * @param[in] NAME Nome da operação
 * @param[in] chunk Ponteiro pra chunk onde reside a constante
 * @param[in] offset Índice no array de bytes
 *
 * @return O próximo índice, pulando a instrução + índice
 */
static size_t _const32Op( const char *NAME, Chunk *chunk, size_t offset );

void debugDisassembleChunk( Chunk *chunk, const char *NAME ) {
	printf("=== %s ===\n", NAME);
	
	for( size_t offset = 0; offset < chunk->count; ) {
		offset = debugDisassembleInstruction(chunk, offset);
	}
}

size_t debugDisassembleInstruction( Chunk *chunk, size_t offset ) {
	printf("%04d ", offset);
	const size_t LINE = chunkGetLine(chunk, offset);

	if( offset > 0 && LINE == chunkGetLine(chunk, offset - 1) ) {
		/* Linha é a mesma que a da mesma instrução.
		 * Imprime uma linha pra deixar mais legível
		 */
		printf("   | ");
	} else {
		printf("%4d ", LINE);
	}

	const uint8_t OP = chunk->code[offset];
	switch(OP) {
		case OP_CONST_16:
			return _const16Op("OP_CONST_16", chunk, offset);
		case OP_CONST_32:
			return _const32Op("OP_CONST_32", chunk, offset);
		case OP_ADD:
			return _simpleOp("OP_ADD", offset);
		case OP_SUB:
			return _simpleOp("OP_SUB", offset);
		case OP_MUL:
			return _simpleOp("OP_MUL", offset);
		case OP_DIV:
			return _simpleOp("OP_DIV", offset);
		case OP_MOD:
			return _simpleOp("OP_MOD", offset);
		case OP_NEGATE:
			return _simpleOp("OP_NEGATE", offset);
		case OP_RETURN:
			return _simpleOp("OP_RETURN", offset);
		default:
			errWarn(LINE, "Instrucao desconhecida '%02x'", OP);
			return offset + 1;
	}
}

static size_t _simpleOp( const char *NAME, size_t offset ) {
	printf("%-16s\n", NAME);
	return offset + 1;
}

static size_t _const16Op( const char *NAME, Chunk *chunk, size_t offset ) {
	const uint8_t CONST = chunk->code[++offset];
	
	printf("%-16s %4d '", NAME, CONST);
	valuePrint(chunk->consts.values[CONST]);
	printf("'\n");

	return offset + 1;
}

static size_t _const32Op( const char *NAME, Chunk *chunk, size_t offset ) {
	size_t constant = chunk->code[++offset];
		  constant |= chunk->code[++offset] << 8;
		  constant |= chunk->code[++offset] << 16;
	
	printf("%-16s %4d '", NAME, constant);
	valuePrint(chunk->consts.values[constant]);
	printf("'\n");

	return offset + 1;
}
