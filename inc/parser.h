/**
 * @file parser.h
 * @author Pedro B.
 * @date 2024.04.02
 *
 * @brief Interpretador de tokens
 */

#ifndef GUARD_NEAT_PARSER_H
#define GUARD_NEAT_PARSER_H

#include "common.h"
#include "token.h"

/**
 * @brief Enum representando o nível de precedência de cada operação. Valores
 * maiores tem maior precedência (são computados primeiro)
 */
typedef enum {
	PREC_NONE = 0,		  /**< Sem precedência */
	PREC_ASSIGNMENT = 1,  /**< Definição de valor */
	PREC_CONDITIONAL = 2, /**< Operador condiciona (?:) */
	PREC_OR = 3,		  /**< Ou lógico */
	PREC_AND = 4,		  /**< E lógico */
	PREC_EQUALITY = 5,	  /**< Comparadores de igualidade '!= e '==' */
	PREC_COMPARISON = 6,  /**< Comparadores '<', '<=', '>' e '>=' */
	PREC_TERM = 7,		  /**< Adição e subtração */
	PREC_FACTOR = 8,	  /**< Multiplicação e divisão */
	PREC_UNARY = 9,		  /**< Operadores unários '!' e '-' */
	PREC_CALL = 10,		  /**< '.' e '()' */
	PREC_PRIMARY = 11	  /**< Precedência máxima */
} Precedence;

typedef void (*ParseFn)(void); /**< Função de parsing */

/**
 * @brief Struct representando uma regra através da qual uma expressão e
 * compilda
 */
typedef struct ParseRule {
	ParseFn prefix;
	ParseFn infix;
	Precedence precedence;
} ParseRule;

/**
 * @brief Struct representando um interpretador de tokens
 */
typedef struct Parser {
	Token previous; /**< Token prévio */
	Token current;	/**< Token atual */
	bool hadError;	/**< Ocorreu um erro durante a compilação? */
	bool panicked;	/**< Parser está no "modo pânico"? */
} Parser;

#endif	// GUARD_NEAT_PARSER_H
