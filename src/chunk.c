/**
 * @file chunk.c
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Definição de uma sequência de bytes (<i>bytecode</i>)
 */

#include "chunk.h"

#include "memory.h"
#include "opcodes.h"

void chunkInit(Chunk* chunk) {
	chunk->count = 0;
	chunk->size = 0;
	chunk->code = NULL;

	chunk->lineCount = 0;
	chunk->lineSize = 0;
	chunk->lines = NULL;

	valueArrayInit(&chunk->consts);
}

void chunkFree(Chunk* chunk) {
	MEM_FREE_ARRAY(uint8_t, chunk->code, chunk->size);
	MEM_FREE_ARRAY(LineStart, chunk->lines, chunk->lineSize);
	valueArrayFree(&chunk->consts);

	chunkInit(chunk);
}

void chunkWrite(Chunk* chunk, const uint8_t BYTE, const size_t LINE) {
	/* Aqui, vemos se o array passou do seu tamanho máximo
	 * Se sim, dobramos o seu tamanho
	 */
	if (chunk->size < chunk->count + 1) {
		const size_t OLD_SIZE = chunk->size;
		chunk->size = MEM_GROW_SIZE(OLD_SIZE);

		chunk->code =
			MEM_GROW_ARRAY(uint8_t, chunk->code, OLD_SIZE, chunk->size);
	}

	chunk->code[chunk->count++] = BYTE;

	/* Se o último byte estava na mesma linha que este, retornamos */
	if (chunk->lineCount > 0 &&
		chunk->lines[chunk->lineCount - 1].line == LINE) {
		return;
	}

	/* Caso contrário, expandimos o array (se necessário) e iniciamos
	 * um novo LineStart
	 */
	if (chunk->lineSize < chunk->lineCount + 1) {
		const size_t OLD_SIZE = chunk->lineSize;
		chunk->lineSize = MEM_GROW_SIZE(OLD_SIZE);

		chunk->lines =
			MEM_GROW_ARRAY(LineStart, chunk->lines, OLD_SIZE, chunk->lineSize);
	}

	LineStart* line = &chunk->lines[chunk->lineCount++];

	/* Offset = byte atual */
	line->offset = chunk->count - 1;
	line->line = LINE;
}

size_t chunkAddConst(Chunk* chunk, Value value) {
	valueArrayWrite(&chunk->consts, value);
	return chunk->consts.count - 1;
}

size_t chunkWriteConst(Chunk* chunk, Value value, const size_t LINE) {
	const size_t INDEX = chunkAddConst(chunk, value);

	/* Se o índice não cabe em um byte (> 255), guardamos em um número
	 * 24-bit (até 16.777.216 valores possíveis), guardado em 3 bytes separadas,
	 * o que é mais do que o bastante.
	 *
	 * O OpCode se chama OP_CONST_32 porque, junto ao OpCode em si, que toma
	 * 1 byte, a operação inteira ocupa 32 bits
	 */
	if (INDEX > UINT8_MAX) {
		chunkWrite(chunk, OP_CONST_32, LINE);
		chunkWrite(chunk, (uint8_t)(INDEX & 0xFF), LINE);
		chunkWrite(chunk, (uint8_t)((INDEX >> 8) & 0xFF), LINE);
		chunkWrite(chunk, (uint8_t)((INDEX >> 16) & 0xFF), LINE);
	} else {
		chunkWrite(chunk, OP_CONST_16, LINE);
		chunkWrite(chunk, (uint8_t)INDEX, LINE);
	}

	return INDEX;
}

size_t chunkGetLine(Chunk* chunk, const size_t OFFSET) {
	const size_t CURRENT_LINE = chunk->lineCount - 1;

	size_t start = 0;
	size_t end = CURRENT_LINE;

	while (true) {
		size_t middle = (start + end) / 2;
		const LineStart* LINE = &chunk->lines[middle];

		if (OFFSET < LINE->offset) {
			/* Linha está mais atrás no array */
			end = middle - 1;
		} else if (middle == CURRENT_LINE ||
				   OFFSET < chunk->lines[middle + 1].offset) {
			/* Achamos a linha! */
			return LINE->line;
		} else {
			/* Está mais a frente no array */
			start = middle + 1;
		}
	}
}
