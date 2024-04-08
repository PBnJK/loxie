/**
 * @file value_array.h
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Array de valores
 */

#ifndef GUARD_LOXIE_VALUE_ARRAY_H
#define GUARD_LOXIE_VALUE_ARRAY_H

#include "common.h"
#include "value.h"

/**
 * @brief Struct representando um array de valores
 */
typedef struct ValueArray {
	size_t count;  /**< Ocupação atual do array de valores */
	size_t size;   /**< Tamanho atual do array de valores */
	Value *values; /**< Array de valores */
} ValueArray;

/**
 * @brief Inicializa um ValueArray
 *
 * @param[out] array Ponteiro pro array que quer inicializar
 */
void valueArrayInit(ValueArray *array);

/**
 * @brief Libera um ValueArray da memória
 *
 * @param[in] array Ponteiro pro array que quer liberar
 */
void valueArrayFree(ValueArray *array);

/**
 * @brief Coloca um valor no array
 *
 * @param[out] array Ponteiro pro ValueArray alvo
 * @param[in] value Novo valor que será colocado no array
 */
void valueArrayWrite(ValueArray *array, Value value);

#endif	// GUARD_LOXIE_VALUE_ARRAY_H
