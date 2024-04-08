/**
 * @file object.c
 * @author Pedro B.
 * @date 2024.04.05
 *
 * @brief Representa um objeto que mora na heap
 */

#ifndef GUARD_LOXIE_OBJECT_H
#define GUARD_LOXIE_OBJECT_H

#include "common.h"
#include "value.h"

/**
 * @brief Enum representando todos os tipos de objetos que existem
 */
typedef enum {
	OBJ_STRING = 0,
} ObjType;

/**
 * @brief Struct representando um objeto que vive na heap
 *
 * Este struct age como um "struct base" para os outros tipos de objeto
 * Quando um novo objeto é criado, seu primeiro membro precisa ser um Obj
 */
struct Obj {
	ObjType type;	  /**< O tipo deste objeto */
	struct Obj *next; /**< Próximo objeto (em uma lista linkada de objetos) */
};

/**
 * @brief Struct representando uma string
 */
struct ObjString {
	Obj obj;	   /**< Objeto base */
	uint32_t hash; /**< Hash da string (valor numérico que a representa */
	size_t length; /**< Tamanho da string */
	char str[];	   /**< Caracteres que compõe a string */
};

/** Retorna o valor de um objeto */
#define OBJECT_TYPE(VALUE) (AS_OBJECT(VALUE)->type)

/** Verifica se um objeto é do tipo string */
#define IS_STRING(VALUE) _isObjectOfType(VALUE, OBJ_STRING)

/** Trata um objeto como sendo do tipo ObjString */
#define AS_STRING(VALUE) ((ObjString *)AS_OBJECT(VALUE))

/** Trata um objeto como uma string C */
#define AS_CSTRING(VALUE) ((AS_STRING(VALUE))->str)

/**
 * @brief Verifica se um objeto é de um dado tipo
 *
 * @param[in] VALUE Valor sendo verificado
 * @param[in] TYPE Tipo esperado
 *
 * @return Verdadeiro se o valor @a VALUE pertençe ao tipo @a TYPE
 */
static inline bool _isObjectOfType(const Value VALUE, const ObjType TYPE) {
	return IS_OBJECT(VALUE) && OBJECT_TYPE(VALUE) == TYPE;
}

/**
 * @brief Cria um ObjString diretamente da string @a STR
 *
 * @param[in] LEN Tamanho da string
 *
 * @return A string criada
 */
ObjString *objMakeString(const size_t LEN);

/**
 * @brief Obtém a hash de uma string
 *
 * Usa o algoritmo FNV-1a
 *
 * @param[in] KEY String a partir da qual a hash será gerada
 * @param[in] LENGTH Tamanho da string
 *
 * @return Hash da string @a KEY
 */
uint32_t hashString(const char *KEY, const size_t LENGTH);

/**
 * @brief Cria um ObjString a partir de uma cópia da string @a STR
 *
 * @param[in] STR Caracteres que formarão a string
 * @param[in] LEN Tamanho da string
 *
 * @return A string criada
 */
ObjString *objCopyString(const char *STR, const size_t LEN);

/**
 * @brief Imprime um objeto
 *
 * @param[in] VALUE Valor que será impresso
 */
void objPrint(const Value VALUE);

#endif	// GUARD_LOXIE_OBJECT_H
