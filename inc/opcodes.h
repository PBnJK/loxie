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

	OP_EQUAL = 5,		  /**< Igual a */
	OP_GREATER = 6,		  /**< Maior que */
	OP_GREATER_EQUAL = 7, /**< Maior ou igual a */
	OP_LESS = 8,		  /**< Menor que */
	OP_LESS_EQUAL = 9,	  /**< Menor ou igual a */

	OP_ADD = 10, /**< Adiciona dois operandos */
	OP_SUB = 11, /**< Subtrai dois operandos */
	OP_MUL = 12, /**< Multiplica dois operandos */
	OP_DIV = 13, /**< Divide dois operandos */
	OP_MOD = 14, /**< Módulo de dois operandos (resto da divisão) */

	OP_NEGATE = 15, /**< Inverte o sinal de um número */
	OP_NOT = 16,	/**< Oposto de um booleano */

	OP_RETURN = 17, /**< Retorna de uma função */
} OpCode;

#endif	// GUARD_NEAT_OPCODES_H
