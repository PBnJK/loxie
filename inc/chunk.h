/**
 * @file chunk.h
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Definição de uma sequência de bytes (<i>bytecode</i>)
 */

#ifndef GUARD_NEAT_CHUNK_H
#define GUARD_NEAT_CHUNK_H

#include "common.h"
#include "value_array.h"

/**
 * @brief Enum representando todos os OpCodes possíveis
 *
 * @note OpCodes não são tokens. Enquanto tokens representam uma unidade
 * do código fonte, um OpCode é uma operação explícita realizada pela VM
 */
typedef enum {
	OP_RETURN = 1,
} OpCode;

/**
 * @brief Struct representando uma sequência de bytecodes
 */
typedef struct Chunk {
	size_t count;		/**< Ocupação atual do array de bytes */
	size_t size;		/**< Tamanho do array de bytes */
	uint8_t *code;		/**< Array de bytes */
} Chunk;

/**
 * @brief Inicializa uma chunk
 *
 * @param[out] chunk Ponteiro pra chunk que quer inicializar
 */
void chunkInit( Chunk *chunk );

/**
 * @brief Libera uma chunk da memória
 *
 * @param[in] chunk Ponteiro pra chunk que quer liberar
 */
void chunkFree( Chunk *chunk );

/**
 * @brief Coloca um byte na chunk
 *
 * @param[out] chunk Ponteiro pra chunk alvo
 * @param[in] BYTE Byte que será colocado na chunk
 */
void chunkWrite( Chunk *chunk, const uint8_t BYTE );

#endif // GUARD_NEAT_CHUNK_H

