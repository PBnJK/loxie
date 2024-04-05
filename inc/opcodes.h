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

	OP_NEGATE = 2, /**< Inverte o sinal de um número */
	OP_ADD = 3,	   /**< Adiciona dois operandos */
	OP_SUB = 4,	   /**< Subtrai dois operandos */
	OP_MUL = 5,	   /**< Multiplica dois operandos */
	OP_DIV = 6,	   /**< Divide dois operandos */
	OP_MOD = 7,	   /**< Módulo de dois operandos (resto da divisão) */

	OP_RETURN = 8, /**< Retorna de uma função */
} OpCode;

#endif	// GUARD_NEAT_OPCODES_H
