/**
 * @file value.h
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Representação de valores da linguagem
 */

#ifndef GUARD_NEAT_VALUE_H
#define GUARD_NEAT_VALUE_H

#include "common.h"

/** Forward-declaration do struct Obj */
typedef struct Obj Obj;

/** Forward-declaration do struct ObjString */
typedef struct ObjString ObjString;

/**
 * @brief Enum representando os tipos de valores que um Value pode representar
 */
typedef enum {
	VALUE_NIL = 0,	  /**< Valor nulo */
	VALUE_BOOL = 1,	  /**< Valor booleano */
	VALUE_NUMBER = 2, /**< Valor numérico */
	VALUE_OBJECT = 3, /**< Objeto */
} ValueType;

/**
 * @brief Struct representando um valor
 */
typedef struct Value {
	ValueType type; /**< O tipo atual sendo representado por este valor */

	union {
		bool vBool;			 /**< Valor booleano */
		NEAT_NUMBER vNumber; /**< Valor numérico */
		Obj *vObject;		 /**< Objeto */
	};
} Value;

/** Verifica se um valor é nulo */
#define IS_NIL(VALUE) ((VALUE).type == VALUE_NIL)

/** Verifica se um valor é numérico */
/** Verifica se um valor é booleano */
#define IS_BOOL(VALUE) ((VALUE).type == VALUE_BOOL)

/** Verifica se um valor é numérico */
#define IS_NUMBER(VALUE) ((VALUE).type == VALUE_NUMBER)

/** Verifica se um valor é um objeto */
#define IS_OBJECT(VALUE) ((VALUE).type == VALUE_OBJECT)

/** Trata um valor como um bool */
#define AS_BOOL(VALUE) ((VALUE).vBool)

/** Trata um valor como um NEAT_NUMBER */
#define AS_NUMBER(VALUE) ((VALUE).vNumber)

/** Trata um valor como um objeto */
#define AS_OBJECT(VALUE) ((VALUE).vObject)

/** Cria um valor nulo */
#define CREATE_NIL() ((Value){.type = VALUE_NIL, .vNumber = 0})

/**
 * @brief Cria um valor booleano
 * @param[in] VALUE Valor booleano inicial
 */
#define CREATE_BOOL(VALUE) ((Value){.type = VALUE_BOOL, .vBool = VALUE})

/**
 * @brief Cria um valor numérico
 * @param[in] VALUE Valor numérico inicial
 */
#define CREATE_NUMBER(VALUE) ((Value){.type = VALUE_NUMBER, .vNumber = VALUE})

/**
 * @brief Cria um objeto
 * @param[in] VALUE Objeto inicial
 */
#define CREATE_OBJECT(VALUE) \
	((Value){.type = VALUE_OBJECT, .vObject = (Obj *)VALUE})

/**
 * @brief Compara dois valores e retorna se são iguais
 *
 * @param[in] A Valor A
 * @param[in] B Valor B
 *
 * @return Se os valores são iguais
 */
bool valueEquals(const Value A, const Value B);

/**
 * @brief Imprime uma representação do valor dado na tela
 *
 * @param[in] value Valor que deve ser impresso
 */
void valuePrint(Value value);

#endif	// GUARD_NEAT_VALUE_H
