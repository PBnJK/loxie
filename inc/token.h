/**
 * @file token.h
 * @author Pedro B.
 * @date 2024.04.03
 *
 * @brief Tokens da linguagem
 */

#ifndef GUARD_NEAT_TOKENS_H
#define GUARD_NEAT_TOKENS_H

#include "common.h"

/**
 * @brief Tipos de tokens
 */
typedef enum {
	TOKEN_LPAREN        = 0,	/**< '(', parêntese esquerdo */
	TOKEN_RPAREN        = 1,	/**< ')', parêntese direito */
	TOKEN_LBRACKET      = 2,	/**< '[', colchete esquerdo */
	TOKEN_RBRACKET      = 3,	/**< ']', colchete direito */
	TOKEN_LBRACE        = 4,	/**< '{', chave esquerda */
	TOKEN_RBRACE        = 5,	/**< '}', chave direita */

	TOKEN_COMMA         = 6,	/**< ',', vírgula */
	TOKEN_DOT           = 7,	/**< '.', ponto */
	TOKEN_SEMICOLON     = 8,	/**< ';', ponto e vírgula */

	TOKEN_PLUS          = 9,	/**< '+', mais */
	TOKEN_MINUS         = 10,	/**< '-', menos */
	TOKEN_STAR          = 11,	/**< '*', asterisco */
	TOKEN_SLASH         = 12,	/**< '/', barra */
	TOKEN_PERCENT       = 13,	/**< '%', porcentagem */

	TOKEN_BANG          = 14,	/**< '!', exclamação */
	TOKEN_BANG_EQUAL    = 15,	/**< '!=', não é igual a */
	TOKEN_EQUAL         = 16,	/**< '=', igual */
	TOKEN_EQUAL_EQUAL   = 17,	/**< '==', é igual a */
	TOKEN_LESS          = 18,	/**< '<', menor que */
	TOKEN_LESS_EQUAL    = 19,	/**< '<=', menor ou igual a */
	TOKEN_GREATER       = 20,	/**< '>', maior que */
	TOKEN_GREATER_EQUAL = 21,	/**< '>=', maior ou igual a */

	TOKEN_IDENTIFIER    = 22,	/**< Identificador (variável, função, etc) */
	TOKEN_STRING        = 23,	/**< String */
	TOKEN_INTERPOLATION = 24,	/**< String interpolada (f-strings) */
	TOKEN_NUMBER        = 25,	/**< Um número */

	TOKEN_AND           = 26,	/**< 'and', 'e' booleano */
	TOKEN_OR            = 27,	/**< 'or', 'ou' booleano */

	TOKEN_TRUE          = 28,	/**< 'true', valor verdadeiro */
	TOKEN_FALSE         = 29,	/**< 'falso', valor falso */
	TOKEN_NIL           = 30,	/**< 'nill', valor nulo */

	TOKEN_FOR           = 31,	/**< 'for', loop 'para x em y faça' */
	TOKEN_WHILE         = 32,	/**< 'while', loop 'enquanto x faça' */

	TOKEN_CLASS         = 33,	/**< 'class', classe OOP */
	TOKEN_THIS          = 34,	/**< 'this', referência a própria classe */
	TOKEN_SUPER         = 35,	/**< 'super', super-classe */

	TOKEN_FN            = 36,	/**< 'fn', Declaração de função */
	TOKEN_RETURN        = 37,	/**< 'return', Retorna de uma função */

	TOKEN_IF            = 38,	/**< 'if', condicional 'se' */
	TOKEN_ELSE          = 39,	/**< 'else', condicional 'caso contrário' */

	TOKEN_PRINT         = 40,	/**< 'print', imprime uma variável pra tela */
	TOKEN_LET           = 41,	/**< 'let', declara uma nova variável */

	TOKEN_ERROR         = 42,	/**< 'error', emitido quando um erro é detectado */
	TOKEN_EOF           = 43,	/**< 'eof', fim do arquivo */
} TokenType;

/**
 * @brief Um token da linguagem
 */
typedef struct Token {
	TokenType type;		/**< Tipo do token */
	const char *START;	/**< Ponteiro pro caractere inicial do token */

	size_t length;		/**< Tamanho da string que compõe o token */
	size_t line;		/**< Linha onde o token está */
} Token;

#endif // GUARD_NEAT_TOKENS_H

