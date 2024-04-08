/**
 * @file table.h
 * @author Pedro B.
 * @date 2024.04.06
 *
 * @brief Implementação de um hashmap
 */

#ifndef GUARD_LOXIE_TABLE_H
#define GUARD_LOXIE_TABLE_H

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

/**
 * @brief Inicializa um hashmap
 * @param[in] table Ponteiro pro hashmap
 */
void tableInit(Table *table);

/**
 * @brief Libera um hashmap da memória
 * @param[in] table Ponteiro pro hashmap
 */
void tableFree(Table *table);

/**
 * @brief Procura por uma string em um hashmap
 *
 * @param[in] table Ponteiro pro hashmap
 * @param[in] STR String sendo procurada
 * @param[in] LEN Tamanho da string
 * @param[in] HASH Hash da string
 *
 * @return A string se for enconrada, ou um valor do tipo EMPTY caso contrário
 */
Value tableFindString(Table *table, const char *STR, const size_t LEN,
					  const uint32_t HASH);

/**
 * @brief Procura um valor em um hashmap
 *
 * @param[in] table Ponteiro pro hashmap
 * @param[in] KEY Chave que aponta pro valor que está sendo buscado
 * @param[out] value Valor que foi encontrado
 *
 * @return Se o valor foi encontrado
 */
bool tableGet(Table *table, const Value KEY, Value *value);

/**
 * @brief Insere um valor em um hashmap
 *
 * @param[in] table Ponteiro pro hashmap
 * @param[in] KEY Chave que apontará pro valor sendo inserido
 * @param[out] VALUE Valor que será inserido
 *
 * @return Se a inserção foi bem sucedida
 */
bool tableSet(Table *table, const Value KEY, const Value VALUE);

/**
 * @brief Remove um valor de um hashmap
 *
 * @param[in] table Ponteiro pro hashmap
 * @param[in] KEY Chave que aponta pro valor sendo removido
 *
 * @return Se a remoção foi bem sucedida
 */
bool tableDelete(Table *table, const Value KEY);

/**
 * @brief Copia as entradas de um hashmap para outro
 *
 * @param[in] from Ponteiro pro hashmap fonte
 * @param[out] to Ponteiro pro hashmap alvo
 */
void tableCopyTo(Table *from, Table *to);

#endif	// GUARD_LOXIE_TABLE_H
