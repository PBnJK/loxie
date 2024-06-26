/**
 * @file value.h
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Representação de valores da linguagem
 */

#ifndef GUARD_LOXIE_VALUE_H
#define GUARD_LOXIE_VALUE_H

#include <string.h>

#include "common.h"

/** Forward-declaration do struct Obj */
typedef struct Obj Obj;

/** Forward-declaration do struct ObjString */
typedef struct ObjString ObjString;

#ifdef NAN_BOXING

/** Bit do sinal */
#define SIGN_BIT ((uint64_t)0x8000000000000000)

/** Quiet NaN */
#define QNAN ((uint64_t)0x7ffc000000000000)

/** Valor constante */
#define TAG_CONST ((uint64_t)0x4000000000000)

/** Valor verdadeiro */
#define TAG_TRUE 3

/** Valor falso */
#define TAG_FALSE 2

/** Valor nulo */
#define TAG_NIL 1

/** Valor vazio */
#define TAG_EMPTY 4

typedef uint64_t Value;

#define CREATE_NUMBER(NUMBER) (_numToValue(NUMBER))
#define CREATE_TRUE() ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define CREATE_FALSE() ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define CREATE_NIL() ((Value)(uint64_t)(QNAN | TAG_NIL))
#define CREATE_EMPTY() ((Value)(uint64_t)(QNAN | TAG_EMPTY))
#define CREATE_BOOL(BOOL) ((BOOL) ? CREATE_TRUE() : CREATE_FALSE())
#define CREATE_OBJECT(OBJECT) \
	(Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(OBJECT))

#define AS_NUMBER(VALUE) (_valueToNum(VALUE))
#define AS_BOOL(VALUE) ((VALUE) == CREATE_TRUE())
#define AS_OBJECT(VALUE) ((Obj *)(uintptr_t)((VALUE) & ~(SIGN_BIT | QNAN)))

#define IS_NUMBER(VALUE) (((VALUE) & QNAN) != QNAN)
#define IS_BOOL(VALUE) (((VALUE) | 1) == CREATE_TRUE())
#define IS_NIL(VALUE) ((VALUE) == TAG_NIL)
#define IS_EMPTY(VALUE) ((VALUE) == TAG_EMPTY)
#define IS_OBJECT(VALUE) (((VALUE) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))
#define IS_CONSTANT(VALUE) (((VALUE) & TAG_CONST) == TAG_CONST)

#define SET_TO_CONSTANT(VALUE) (VALUE |= TAG_CONST)

static inline Value _numToValue(const double NUMBER) {
	Value value;
	memcpy(&value, &NUMBER, sizeof(double));
	return value;
}

static inline double _valueToNum(const Value VALUE) {
	double number;
	memcpy(&number, &VALUE, sizeof(Value));
	return number;
}

#else

/**
 * @brief Enum representando os tipos de valores que um Value pode representar
 */
typedef enum {
	VALUE_NIL = 0,	  /**< Valor nulo */
	VALUE_BOOL = 1,	  /**< Valor booleano */
	VALUE_NUMBER = 2, /**< Valor numérico */
	VALUE_OBJECT = 3, /**< Objeto */
	VALUE_EMPTY = 4, /**< Valor vazio, usado para representar uma entry vazia em
					   um hasmap */
} ValueType;
/**
 * @brief Struct representando um valor
 */
typedef struct Value {
	uint8_t props; /**< Propriedades do valor:
					* 7 6 5 4 3 2 1 0
					*         | |   |
					*         | ValueType do valor
					*         Se este valor é constante
					*/
	union {
		bool vBool;			  /**< Valor booleano */
		LOXIE_NUMBER vNumber; /**< Valor numérico */
		Obj *vObject;		  /**< Objeto */
	};
} Value;

/** Retorna o tipo do valor */
#define GET_TYPE(VALUE) ((VALUE).props & 7)

/** Retorna se o valor é constante */
#define IS_CONSTANT(VALUE) (((VALUE).props >> 3) & 1)

/** Verifica se um valor é nulo */
#define IS_NIL(VALUE) (GET_TYPE(VALUE) == VALUE_NIL)

/** Verifica se um valor é booleano */
#define IS_BOOL(VALUE) (GET_TYPE(VALUE) == VALUE_BOOL)

/** Verifica se um valor é numérico */
#define IS_NUMBER(VALUE) (GET_TYPE(VALUE) == VALUE_NUMBER)

/** Verifica se um valor é um objeto */
#define IS_OBJECT(VALUE) (GET_TYPE(VALUE) == VALUE_OBJECT)

/** Verifica se um valor é vazio */
#define IS_EMPTY(VALUE) (GET_TYPE(VALUE) == VALUE_EMPTY)

/** Trata um valor como um bool */
#define AS_BOOL(VALUE) ((VALUE).vBool)

/** Trata um valor como um LOXIE_NUMBER */
#define AS_NUMBER(VALUE) ((VALUE).vNumber)

/** Trata um valor como um objeto */
#define AS_OBJECT(VALUE) ((VALUE).vObject)

/** Cria um valor nulo */
#define CREATE_NIL() ((Value){.props = VALUE_NIL, .vNumber = 0})

/**
 * @brief Cria um valor booleano
 * @param[in] VALUE Valor booleano inicial
 */
#define CREATE_BOOL(VALUE) ((Value){.props = VALUE_BOOL, .vBool = VALUE})

/**
 * @brief Cria um valor numérico
 * @param[in] VALUE Valor numérico inicial
 */
#define CREATE_NUMBER(VALUE) ((Value){.props = VALUE_NUMBER, .vNumber = VALUE})

/**
 * @brief Cria um objeto
 * @param[in] VALUE Objeto inicial
 */
#define CREATE_OBJECT(VALUE) \
	((Value){.props = VALUE_OBJECT, .vObject = (Obj *)VALUE})

/** Cria um valor vazio */
#define CREATE_EMPTY() ((Value){.props = VALUE_EMPTY, .vNumber = 0})

#define SET_TO_CONSTANT(VALUE) (VALUE.props |= 0x08)

#endif

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

#endif	// GUARD_LOXIE_VALUE_H
