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
	OP_GET_LOCAL_16 = 12,  /**< Pega uma variável local, com índice 8-bit */
	OP_GET_LOCAL_32 = 13,  /**< Pega uma variável local, com índice 24-bit */
	OP_GET_UPVALUE_16 = 14, /**< Pega uma local capturada, com índice 8-bit */
	OP_GET_UPVALUE_32 = 15, /**< Pega uma local capturada, com índice 24-bit */

	OP_SET_GLOBAL_16 = 16, /**< Muda uma variável global, con índice 8-bit */
	OP_SET_GLOBAL_32 = 17, /**< Muda uma variável global, com índice 24-bit */
	OP_SET_LOCAL_16 = 18,  /**< Muda uma variável local, com índice 8-bit */
	OP_SET_LOCAL_32 = 19,  /**< Muda uma variável local, com índice 24-bit */
	OP_SET_UPVALUE_16 = 20, /**< Muda uma local capturada, com índice 8-bit */
	OP_SET_UPVALUE_32 = 21, /**< Muda uma local capturada, com índice 24-bit */

	OP_EQUAL = 22,		   /**< Igual a */
	OP_GREATER = 23,	   /**< Maior que */
	OP_GREATER_EQUAL = 24, /**< Maior ou igual a */
	OP_LESS = 25,		   /**< Menor que */
	OP_LESS_EQUAL = 26,	   /**< Menor ou igual a */

	OP_ADD = 27, /**< Adiciona dois operandos */
	OP_SUB = 28, /**< Subtrai dois operandos */
	OP_MUL = 29, /**< Multiplica dois operandos */
	OP_DIV = 30, /**< Divide dois operandos */
	OP_MOD = 31, /**< Módulo de dois operandos (resto da divisão) */

	OP_NEGATE = 32, /**< Inverte o sinal de um número */
	OP_NOT = 33,	/**< Oposto de um booleano */

	OP_PRINT = 34, /**< Imprimir */

	OP_JUMP = 35,		   /**< Pulo */
	OP_JUMP_IF_FALSE = 36, /**< Pulo condicional */

	OP_LOOP = 37,  /**< Inicia um loop */
	OP_BREAK = 38, /**< Sai de um loop */

	OP_DUP = 39, /**< Duplica o item no topo da pilha */

	OP_CALL = 40,		   /**< Chama uma função */
	OP_CLOSURE_16 = 41,	   /**< Cria uma closure, com um índice 8-bit */
	OP_CLOSURE_32 = 42,	   /**< Cria uma closure, com um índice 24-bit */
	OP_CLOSE_UPVALUE = 43, /**< Fecha um upvalue */

	OP_CLASS_16 = 44, /**< Classe com índice 8-bit */
	OP_CLASS_32 = 45, /**< Classe com índice 24-bit */

	OP_SET_PROPERTY_16 =
		46, /**< Muda uma propriedade de uma classe, com índice 8-bit */
	OP_SET_PROPERTY_32 =
		47, /**< Muda uma propriedade de uma classe, com índice 24-bit */
	OP_GET_PROPERTY_16 =
		48, /**< Pega uma propriedade de uma classe, com índice 8-bit */
	OP_GET_PROPERTY_32 =
		49, /**< Pega uma propriedade de uma classe, com índice 24-bit */

	OP_METHOD_16 = 50, /**< Declara um método, com índice 8-bit */
	OP_METHOD_32 = 51, /**< Declara um método, com índice 24-bit */

	OP_INVOKE_16 = 52, /**< Invoca um método, com índice 8-bit */
	OP_INVOKE_32 = 53, /**< Invoca um método, com índice 24-bit */

	OP_INHERIT = 54, /**< Herda uma superclasse */

	OP_GET_SUPER_16 = 55, /**< Pega a superclasse, com índice 8-bit */
	OP_GET_SUPER_32 = 56, /**< Pega a superclasse, com índice 24-bit */

	OP_SUPER_INVOKE_16 = 57, /**< Invoca a superclasse, com índice 8-bit */
	OP_SUPER_INVOKE_32 = 58, /**< Invoca a superclasse, com índice 24-bit */

	OP_ARRAY = 59,		   /**< Inicializa um array */
	OP_PUSH_TO_ARRAY = 60, /**< Adiciona à um array */

	OP_TABLE = 61,		   /**< Inicializa um hashmap */
	OP_PUSH_TO_TABLE = 62, /**< Adiciona à um hashmap */

	OP_GET_SUBSCRIPT =
		63, /**< Pega um valor de um array/hashmap/string/etc... */
	OP_SET_SUBSCRIPT =
		64, /**< Muda um valor de um array/hashmap/string/etc... */

	OP_RETURN = 65, /**< Retorna de uma função */
} OpCode;

#endif	// GUARD_LOXIE_OPCODES_H
