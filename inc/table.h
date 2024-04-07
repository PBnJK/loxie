/**
 * @file table.h
 * @author Pedro B.
 * @date 2024.04.06
 *
 * @brief Implementação de um hashmap
 */

#ifndef GUARD_NEAT_TABLE_H
#define GUARD_NEAT_TABLE_H

#include "common.h"
#include "value.h"

/**
 * @brief Struct representando um par key-value em uma @ref Table
 */
typedef struct {
	Value key;	 /**< Chave que aponta pro valor */
	Value value; /**< Valor guardado no hasmap */
} Entry;

/**
 * @brief Struct representando um hashmap
 */
typedef struct {
	size_t count;	/**< Quantidade de itens no hashmap */
	size_t size;	/**< Tamanho reservado pelo hashmap */
	Entry *entries; /**< Array de itens no hashmap */
} Table;

void tableInit(Table *table);
void tableFree(Table *table);

Value tableFindString(Table *table, const char *STR, const size_t LEN,
					  const uint32_t HASH);

bool tableGet(Table *table, const Value KEY, Value *value);
bool tableSet(Table *table, const Value KEY, const Value VALUE);

bool tableDelete(Table *table, const Value KEY);

void tableCopyTo(Table *from, Table *to);

#endif	// GUARD_NEAT_TABLE_H
