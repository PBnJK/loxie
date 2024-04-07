/**
 * @file opcodes.h
 * @author Pedro B.
 * @date 2024.04.02
 *
 * @brief Cabeçalho com as operações que são executadas pela VM
 */

#ifndef GUARD_NEAT_OPCODES_H
#define GUARD_NEAT_OPCODES_H

/**
 * @brief Enum representando todos os OpCodes possíveis
 *
 * @note OpCodes não são tokens. Enquanto tokens representam uma unidade
 * do código fonte, um OpCode é uma operação explícita realizada pela VM
 */
typedef enum {
	OP_CONST_16 = 0, /**< Pega uma constante, com um índice 8-bit */
	OP_CONST_32 = 1, /**< Pega uma constante, com um índice 24-bit */

	OP_TRUE = 2,  /**< Valor verdadeiro */
	OP_FALSE = 3, /**< Valor falso */
	OP_NIL = 4,	  /**< Valor nulo */

	OP_POP = 5,			  /**< Retira um valor da pilha */
	OP_DEF_GLOBAL_16 = 6, /**< Define uma variável global, con índice 8-bit */
	OP_DEF_GLOBAL_32 = 7, /**< Define uma variável global, com índice 24-bit */
	OP_GET_GLOBAL_16 = 8,  /**< Pega uma variável global, con índice 8-bit */
	OP_GET_GLOBAL_32 = 9,  /**< Pega uma variável global, com índice 24-bit */
	OP_SET_GLOBAL_16 = 10, /**< Pega uma variável global, con índice 8-bit */
	OP_SET_GLOBAL_32 = 11, /**< Pega uma variável global, com índice 24-bit */

	OP_EQUAL = 12,		   /**< Igual a */
	OP_GREATER = 13,	   /**< Maior que */
	OP_GREATER_EQUAL = 14, /**< Maior ou igual a */
	OP_LESS = 15,		   /**< Menor que */
	OP_LESS_EQUAL = 16,	   /**< Menor ou igual a */

	OP_ADD = 17, /**< Adiciona dois operandos */
	OP_SUB = 18, /**< Subtrai dois operandos */
	OP_MUL = 19, /**< Multiplica dois operandos */
	OP_DIV = 20, /**< Divide dois operandos */
	OP_MOD = 21, /**< Módulo de dois operandos (resto da divisão) */

	OP_NEGATE = 22, /**< Inverte o sinal de um número */
	OP_NOT = 23,	/**< Oposto de um booleano */

	OP_PRINT = 24, /**< Imprimir */

	OP_RETURN = 25, /**< Retorna de uma função */
} OpCode;

#endif	// GUARD_NEAT_OPCODES_H
