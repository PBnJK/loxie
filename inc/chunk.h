/**
 * @file chunk.h
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Definição de uma sequência de bytes (<i>bytecode</i>)
 */

#ifndef GUARD_LOXIE_CHUNK_H
#define GUARD_LOXIE_CHUNK_H

#include "common.h"
#include "value.h"
#include "value_array.h"

/**
 * @brief Struct guardando informações sobre uma linha de código
 *
 * Este struct é usado para armazenar a linha de código de onde um bytecode
 * veio, para que possamos mostrar ao programador a linha relevante quando
 * encontramos um erro
 *
 * É implementado atráves da compressão <a
 * href="https://pt.wikipedia.org/wiki/Codifica%C3%A7%C3%A3o_run-length">RLE</a>,
 * evitando a repetição de linhas iguais
 */
typedef struct LineStart {
	size_t offset; /**< Offset no código para onde esta linha aponta */
	size_t line;   /**< Número da linha */
} LineStart;

/**
 * @brief Struct representando uma sequência de bytecodes
 */
typedef struct Chunk {
	size_t count;  /**< Ocupação atual do array de bytes */
	size_t size;   /**< Tamanho do array de bytes */
	uint8_t *code; /**< Array de bytes */

	ValueArray consts; /**< Array de valores constantes */

	size_t lineCount; /**< Ocupação atual do array de linhas */
	size_t lineSize;  /**< Tamanho do array de linhas */
	LineStart *lines; /**< Array de linhas */
} Chunk;

/**
 * @brief Inicializa uma chunk
 *
 * @param[out] chunk Ponteiro pra chunk que quer inicializar
 */
void chunkInit(Chunk *chunk);

/**
 * @brief Libera uma chunk da memória
 *
 * @param[out] chunk Ponteiro pra chunk que quer liberar
 */
void chunkFree(Chunk *chunk);

/**
 * @brief Coloca um byte na chunk
 *
 * @param[out] chunk Ponteiro pra chunk alvo
 * @param[in] BYTE Byte que será colocado na chunk
 * @param[in] LINE Linha de código de onde este byte vem
 */
void chunkWrite(Chunk *chunk, const uint8_t BYTE, const size_t LINE);

/**
 * @brief Adiciona um valor constante ao array de valores da chunk
 *
 * Este valor permanecerá no array até ser coletado pelo GC, e guarda
 * números, strings, objetos e outros valores usados no código
 *
 * @param[out] chunk Ponteiro pra chunk alvo
 * @param[in] value Valor que será adicionado
 *
 * @return Índice do valor no array de itens
 */
size_t chunkAddConst(Chunk *chunk, Value value);

/**
 * @brief Adiciona uma instrução OP_CONST_* ao código e adiciona um valor
 * constante ao array de valores, selecionando entre as versões 16- e 32-bit
 * de acordo com a necessidade
 *
 * @param[out] chunk Ponteiro pra chunk alvo
 * @param[in] value Valor que será adicionado
 * @param[in] LINE Linha de código onde este valor foi declarado
 *
 * @return Índice do valor no array de itens
 */
size_t chunkWriteConst(Chunk *chunk, Value value, const size_t LINE);

/**
 * @brief Acha a linha onde um dado offset está localizado no código-fonte
 *
 * Faz uma pesquisa binária do array de linhas numa chunk
 *
 * @param[in] chunk Ponteiro pra chunk alvo
 * @param[in] OFFSET Offset do byte que desejamos encontrar
 *
 * @return Linha onde o offset se encontra
 */
size_t chunkGetLine(Chunk *chunk, const size_t OFFSET);

#endif	// GUARD_LOXIE_CHUNK_H
