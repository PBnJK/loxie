/**
 * @file debug.h
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Funções para auxiliar o desenvolvimento
 */

#ifndef GUARD_NEAT_DEBUG_H
#define GUARD_NEAT_DEBUG_H

#include "chunk.h"
#include "common.h"

/**
 * @brief Desconstrói uma chunk
 *
 * Partindo de uma chunk, imprime uma representação legível do bytecode
 *
 * @param[in] chunk Ponteiro pra chunk que quer desconstruir
 * @param[in] NAME Nome da chunk
 */
void debugDisassembleChunk(Chunk *chunk, const char *NAME);

/**
 * @brief Desconstrói uma instrução
 *
 * Procura e imprime uma representação legível de um OpCode e seus possíveis
 * operandos
 *
 * @param[in] chunk Ponteiro pra chunk a qual pertence a instrução
 * @param[in] offset Índice da instrução no array de bytes
 *
 * @return Índice da próxima instrução
 */
size_t debugDisassembleInstruction(Chunk *chunk, size_t offset);

#endif	// GUARD_NEAT_DEBUG_H
