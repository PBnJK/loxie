/**
 * @file debug.c
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Funções para auxiliar o desenvolvimento
 */

#include "debug.h"

#include <stdio.h>

#include "error.h"
#include "opcodes.h"
#include "vm.h"

/**
 * @brief Imprime uma operação sem operandos
 *
 * @param[in] NAME Nome da operação
 * @param[in] offset Índice no array de bytes
 *
 * @return O próximo índice
 */
static size_t _simpleOp(const char* NAME, size_t offset);

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
static size_t _const16Op(const char* NAME, Chunk* chunk, size_t offset);

/**
 * @brief Mesmo que @ref _const16Op, mas com um índice 24-bit
 *
 * @param[in] NAME Nome da operação
 * @param[in] chunk Ponteiro pra chunk onde reside a constante
 * @param[in] offset Índice no array de bytes
 *
 * @return O próximo índice, pulando a instrução + índice
 */
static size_t _const32Op(const char* NAME, Chunk* chunk, size_t offset);

/**
 * @brief Imprime uma operação com um operando que é um índice no array
 * de constantes da VM
 *
 * @param[in] NAME Nome da operação
 * @param[in] chunk Ponteiro pra chunk
 * @param[in] offset Índice no array de bytes
 *
 * @return O próximo índice, pulando a instrução + índice
 */
static size_t _global16Op(const char* NAME, Chunk* chunk, size_t offset);

/**
 * @brief Mesmo que @ref _global16Op, mas com um índice 24-bit
 *
 * @param[in] NAME Nome da operação
 * @param[in] chunk Ponteiro pra chunk
 * @param[in] offset Índice no array de bytes
 *
 * @return O próximo índice, pulando a instrução + índice
 */
static size_t _global32Op(const char* NAME, Chunk* chunk, size_t offset);

/**
 * @brief Imprime uma operação com operando 8-bit
 *
 * @param[in] NAME Nome da operação
 * @param[in] chunk Ponteiro pra chunk
 * @param[in] offset Índice no array de bytes
 *
 * @return O próximo índice
 */
static size_t _local16Op(const char* NAME, Chunk* chunk, size_t offset);

/**
 * @brief Imprime uma operação com operando 24-bit
 *
 * @param[in] NAME Nome da operação
 * @param[in] chunk Ponteiro pra chunk
 * @param[in] offset Índice no array de bytes
 *
 * @return O próximo índice
 */
static size_t _local32Op(const char* NAME, Chunk* chunk, size_t offset);

static size_t _jumpOp(const char* NAME, const int8_t SIGN, Chunk* chunk,
					  size_t offset);

static size_t _16BitOp(const char* NAME, Chunk* chunk, size_t offset);
static size_t _32BitOp(const char* NAME, Chunk* chunk, size_t offset);

static size_t _closureOp(const char* NAME, Chunk* chunk, size_t offset,
						 bool is24Bit);

void debugDisassembleChunk(Chunk* chunk, const char* NAME) {
	printf("=== %s ===\n", NAME);

	for( size_t offset = 0; offset < chunk->count; ) {
		offset = debugDisassembleInstruction(chunk, offset);
	}
}

size_t debugDisassembleInstruction(Chunk* chunk, size_t offset) {
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
	switch( OP ) {
		case OP_CONST_16:
			return _const16Op("OP_CONST_16", chunk, offset);
		case OP_CONST_32:
			return _const32Op("OP_CONST_32", chunk, offset);
		case OP_TRUE:
			return _simpleOp("OP_TRUE", offset);
		case OP_FALSE:
			return _simpleOp("OP_FALSE", offset);
		case OP_NIL:
			return _simpleOp("OP_NIL", offset);
		case OP_POP:
			return _simpleOp("OP_POP", offset);
		case OP_DEF_GLOBAL_16:
			return _global16Op("OP_DEF_GLOBAL_16", chunk, offset);
		case OP_DEF_GLOBAL_32:
			return _global32Op("OP_DEF_GLOBAL_32", chunk, offset);
		case OP_DEF_CONST_16:
			return _global16Op("OP_DEF_CONST_16", chunk, offset);
		case OP_DEF_CONST_32:
			return _global32Op("OP_DEF_CONST_32", chunk, offset);
		case OP_GET_GLOBAL_16:
			return _global16Op("OP_GET_GLOBAL_16", chunk, offset);
		case OP_GET_GLOBAL_32:
			return _global32Op("OP_GET_GLOBAL_32", chunk, offset);
		case OP_GET_LOCAL_16:
			return _local16Op("OP_GET_LOCAL_16", chunk, offset);
		case OP_GET_LOCAL_32:
			return _local32Op("OP_GET_LOCAL_32", chunk, offset);
		case OP_GET_UPVALUE_16:
			return _16BitOp("OP_GET_UPVALUE_16", chunk, offset);
		case OP_GET_UPVALUE_32:
			return _32BitOp("OP_GET_UPVALUE_32", chunk, offset);
		case OP_SET_GLOBAL_16:
			return _global16Op("OP_SET_GLOBAL_16", chunk, offset);
		case OP_SET_GLOBAL_32:
			return _global32Op("OP_SET_GLOBAL_32", chunk, offset);
		case OP_SET_LOCAL_16:
			return _local16Op("OP_SET_LOCAL_16", chunk, offset);
		case OP_SET_LOCAL_32:
			return _local32Op("OP_SET_LOCAL_32", chunk, offset);
		case OP_SET_UPVALUE_16:
			return _16BitOp("OP_GET_UPVALUE_16", chunk, offset);
		case OP_SET_UPVALUE_32:
			return _32BitOp("OP_GET_UPVALUE_32", chunk, offset);
		case OP_EQUAL:
			return _simpleOp("OP_EQUAL", offset);
		case OP_GREATER:
			return _simpleOp("OP_GREATER", offset);
		case OP_GREATER_EQUAL:
			return _simpleOp("OP_GREATER_EQUAL", offset);
		case OP_LESS:
			return _simpleOp("OP_LESS", offset);
		case OP_LESS_EQUAL:
			return _simpleOp("OP_LESS_EQUAL", offset);
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
		case OP_NOT:
			return _simpleOp("OP_NOT", offset);
		case OP_PRINT:
			return _simpleOp("OP_PRINT", offset);
		case OP_JUMP:
			return _jumpOp("OP_JUMP", 1, chunk, offset);
		case OP_JUMP_IF_FALSE:
			return _jumpOp("OP_JUMP_IF_FALSE", 1, chunk, offset);
		case OP_LOOP:
			return _jumpOp("OP_LOOP", -1, chunk, offset);
		case OP_BREAK:
			return _simpleOp("OP_JUMP", offset);
		case OP_DUP:
			return _simpleOp("OP_DUP", offset);
		case OP_CALL:
			return _16BitOp("OP_CALL", chunk, offset);
		case OP_CLOSURE_16:
			return _closureOp("OP_CLOSURE_16", chunk, offset, false);
		case OP_CLOSURE_32:
			return _closureOp("OP_CLOSURE_32", chunk, offset, true);
		case OP_CLOSE_UPVALUE:
			return _simpleOp("OP_CLOSE_UPVALUE", offset);
		case OP_RETURN:
			return _simpleOp("OP_RETURN", offset);
		default:
			errWarn(LINE, "Instrucao desconhecida '%02x'", OP);
			return offset + 1;
	}
}

static size_t _simpleOp(const char* NAME, size_t offset) {
	printf("%-20s\n", NAME);
	return offset + 1;
}

static size_t _const16Op(const char* NAME, Chunk* chunk, size_t offset) {
	const uint8_t CONST = chunk->code[++offset];

	printf("%-20s %4d '", NAME, CONST);
	valuePrint(chunk->consts.values[CONST]);
	printf("'\n");

	return offset + 1;
}

static size_t _const32Op(const char* NAME, Chunk* chunk, size_t offset) {
	size_t constant = chunk->code[++offset];
	constant |= chunk->code[++offset] << 8;
	constant |= chunk->code[++offset] << 16;

	printf("%-20s %4d '", NAME, constant);
	valuePrint(chunk->consts.values[constant]);
	printf("'\n");

	return offset + 1;
}

static size_t _global16Op(const char* NAME, Chunk* chunk, size_t offset) {
	const uint8_t CONST = chunk->code[++offset];

	printf("%-20s %4d '", NAME, CONST);
	valuePrint(vm.globalValues.values[CONST]);
	printf("'\n");

	return offset + 1;
}

static size_t _global32Op(const char* NAME, Chunk* chunk, size_t offset) {
	size_t constant = chunk->code[++offset];
	constant |= chunk->code[++offset] << 8;
	constant |= chunk->code[++offset] << 16;

	printf("%-20s %4d '", NAME, constant);
	valuePrint(vm.globalValues.values[constant]);
	printf("'\n");

	return offset + 1;
}

static size_t _local16Op(const char* NAME, Chunk* chunk, size_t offset) {
	uint8_t slot = chunk->code[++offset];
	printf("%-20s %4d\n", NAME, slot);
	return offset + 1;
}

static size_t _local32Op(const char* NAME, Chunk* chunk, size_t offset) {
	uint8_t slot = chunk->code[++offset];
	slot |= chunk->code[++offset] << 8;
	slot |= chunk->code[++offset] << 16;

	printf("%-20s %4d\n", NAME, slot);
	return offset + 1;
}

static size_t _jumpOp(const char* NAME, const int8_t SIGN, Chunk* chunk,
					  size_t offset) {
	uint16_t jump = (uint16_t)(chunk->code[++offset] << 8);
	jump |= chunk->code[++offset];
	printf("%-20s %4d -> %d\n", NAME, offset, offset + 1 + SIGN * jump);
	return offset + 1;
}

static size_t _16BitOp(const char* NAME, Chunk* chunk, size_t offset) {
	const uint8_t CONST = chunk->code[++offset];
	printf("%-20s %4d\n", NAME, CONST);

	return offset + 1;
}

static size_t _32BitOp(const char* NAME, Chunk* chunk, size_t offset) {
	uint8_t constant = chunk->code[++offset];
	constant |= chunk->code[++offset] << 8;
	constant |= chunk->code[++offset] << 16;

	printf("%-20s %4d\n", NAME, constant);

	return offset + 1;
}

static size_t _closureOp(const char* NAME, Chunk* chunk, size_t offset,
						 bool is24Bit) {
	offset++;

	size_t constant = chunk->code[offset++];
	if( is24Bit ) {
		constant |= chunk->code[offset++] << 8;
		constant |= chunk->code[offset++] << 16;
	}

	printf("%-20s %4d ", NAME, constant);
	valuePrint(chunk->consts.values[constant]);
	printf("\n");

	ObjFunction* function = AS_FUNCTION(chunk->consts.values[constant]);
	for( size_t j = 0; j < function->upvalueCount; ++j ) {
		uint8_t isLocal = chunk->code[offset++];

		int32_t index = chunk->code[offset++];
		index |= chunk->code[offset++] << 8;
		index |= chunk->code[offset++] << 16;

		printf("%04d    |                         > %s %d\n", offset - 2,
			   isLocal ? "local" : "upvalue", index);
	}

	return offset;
}
