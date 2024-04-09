/**
 * @file opcodes.h
 * @author Pedro B.
 * @date 2024.04.02
 *
 * @brief Cabeçalho com as operações que são executadas pela VM
 */

#ifndef GUARD_LOXIE_OPCODES_H
#define GUARD_LOXIE_OPCODES_H

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

	OP_POP = 5, /**< Retira um valor da pilha */

	OP_DEF_GLOBAL_16 = 6, /**< Define uma variável, con índice 8-bit */
	OP_DEF_GLOBAL_32 = 7, /**< Define uma variável, com índice 24-bit */
	OP_DEF_CONST_16 = 8,  /**< Define uma constante, com índice 8-bit */
	OP_DEF_CONST_32 = 9,  /**< Define uma constante, com índice 24-bit */

	OP_GET_GLOBAL_16 = 10, /**< Pega uma variável global, con índice 8-bit */
	OP_GET_GLOBAL_32 = 11, /**< Pega uma variável global, com índice 24-bit */
	OP_GET_LOCAL_16 = 12,  /**< Pega uma variável global, con índice 8-bit */
	OP_GET_LOCAL_32 = 13,  /**< Pega uma variável global, con índice 8-bit */

	OP_SET_GLOBAL_16 = 14, /**< Pega uma variável global, con índice 8-bit */
	OP_SET_GLOBAL_32 = 15, /**< Pega uma variável global, com índice 24-bit */
	OP_SET_LOCAL_16 = 16,  /**< Pega uma variável global, com índice 24-bit */
	OP_SET_LOCAL_32 = 17,  /**< Pega uma variável global, com índice 24-bit */

	OP_EQUAL = 18,		   /**< Igual a */
	OP_GREATER = 19,	   /**< Maior que */
	OP_GREATER_EQUAL = 20, /**< Maior ou igual a */
	OP_LESS = 21,		   /**< Menor que */
	OP_LESS_EQUAL = 22,	   /**< Menor ou igual a */

	OP_ADD = 23, /**< Adiciona dois operandos */
	OP_SUB = 24, /**< Subtrai dois operandos */
	OP_MUL = 25, /**< Multiplica dois operandos */
	OP_DIV = 26, /**< Divide dois operandos */
	OP_MOD = 27, /**< Módulo de dois operandos (resto da divisão) */

	OP_NEGATE = 28, /**< Inverte o sinal de um número */
	OP_NOT = 29,	/**< Oposto de um booleano */

	OP_PRINT = 30, /**< Imprimir */

	OP_JUMP = 31,		   /**< Pulo */
	OP_JUMP_IF_FALSE = 32, /**< Pulo condicional */

	OP_LOOP = 33,  /**< Inicia um loop */
	OP_BREAK = 34, /**< Sai de um loop */

	OP_DUP = 35, /**< Duplica o item no topo da pilha */

	OP_RETURN = 36, /**< Retorna de uma função */
} OpCode;

#endif	// GUARD_LOXIE_OPCODES_H
