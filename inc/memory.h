/**
 * @file memory.h
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Lida com operações relacionadas à memória
 */

#ifndef GUARD_NEAT_MEMORY_H
#define GUARD_NEAT_MEMORY_H

#include "common.h"

/**
 * @brief Cresce a capacidade de um array, dobrando-a
 *
 * @param[in] OLD Tamanho velho do array
 */
#define MEM_GROW_SIZE(OLD) \
	((OLD) < 8 ? 8 : (OLD) * 2)

/**
 * @brief Cresce o array em si (use em conjunto com @ref MEM_GROW_SIZE)
 *
 * @param[in] TYPE Tipo dos itens no array
 * @param[in] ARR Ponteiro pro array em si
 * @param[in] OLD Tamanho velho do array
 * @param[in] NEW Tamanho novo do array
 */
#define MEM_GROW_ARRAY(TYPE, ARR, OLD, NEW) \
	memRealloc(ARR, sizeof(TYPE) * OLD, sizeof(TYPE) * NEW)

/**
 * @brief Libera um array da memória
 *
 * @param[in] TYPE Tipo dos itens no array
 * @param[in] ARR Array que será liberado
 * @param[in] OLD Tamanho velho do array
 */
#define MEM_FREE_ARRAY(TYPE, ARR, OLD) \
	memRealloc(ARR, sizeof(TYPE) * OLD, 0)

/**
 * @brief Realoca uma quantidade de memória
 *
 * Wrapper para realloc e free
 *
 * @param[in] pointer Ponteiro pro bloco de memória que será realocado
 * @param[in] OLD_SIZE Tamanho velho do bloco de memória
 * @param[in] NEW_SIZE Tamanho novo do bloco de memória
 *
 * @return Novo bloco de memória
 */
void *memRealloc( void *pointer, const size_t OLD_SIZE, const size_t NEW_SIZE );

#endif // GUARD_NEAT_MEMORY_H

