/**
 * @file token.h
 * @author Pedro B.
 * @date 2024.04.03
 *
 * @brief Tokens da linguagem
 */

#ifndef GUARD_LOXIE_TOKENS_H
#define GUARD_LOXIE_TOKENS_H

#include "common.h"

/**
 * @brief Tipos de tokens
 */
typedef enum {
	TOKEN_LPAREN = 0,	/**< '(', parêntese esquerdo */
	TOKEN_RPAREN = 1,	/**< ')', parêntese direito */
	TOKEN_LBRACKET = 2, /**< '[', colchete esquerdo */
	TOKEN_RBRACKET = 3, /**< ']', colchete direito */
	TOKEN_LBRACE = 4,	/**< '{', chave esquerda */
	TOKEN_RBRACE = 5,	/**< '}', chave direita */

	TOKEN_DOLLAR = 6, /**< '$', dólar */
	TOKEN_HASH = 7,	  /**< '#', jogo da velha */

	TOKEN_COMMA = 8,	  /**< ',', vírgula */
	TOKEN_DOT = 9,		  /**< '.', ponto */
	TOKEN_SEMICOLON = 10, /**< ';', ponto e vírgula */

	TOKEN_PLUS = 11,	/**< '+', mais */
	TOKEN_MINUS = 12,	/**< '-', menos */
	TOKEN_STAR = 13,	/**< '*', asterisco */
	TOKEN_SLASH = 14,	/**< '/', barra */
	TOKEN_PERCENT = 15, /**< '%', porcentagem */

	TOKEN_BANG = 16,		  /**< '!', exclamação */
	TOKEN_BANG_EQUAL = 17,	  /**< '!=', não é igual a */
	TOKEN_EQUAL = 18,		  /**< '=', igual */
	TOKEN_EQUAL_EQUAL = 19,	  /**< '==', é igual a */
	TOKEN_LESS = 20,		  /**< '<', menor que */
	TOKEN_LESS_EQUAL = 21,	  /**< '<=', menor ou igual a */
	TOKEN_GREATER = 22,		  /**< '>', maior que */
	TOKEN_GREATER_EQUAL = 23, /**< '>=', maior ou igual a */

	TOKEN_IDENTIFIER = 24,	  /**< Identificador (variável, função, etc) */
	TOKEN_STRING = 25,		  /**< String */
	TOKEN_INTERPOLATION = 26, /**< String interpolada (f-strings) */
	TOKEN_NUMBER = 27,		  /**< Um número */

	TOKEN_AND = 28, /**< 'and', 'e' lógico */
	TOKEN_OR = 29,	/**< 'or', 'ou' lógico */

	TOKEN_TRUE = 30,  /**< 'true', valor verdadeiro */
	TOKEN_FALSE = 31, /**< 'falso', valor falso */
	TOKEN_NIL = 32,	  /**< 'nill', valor nulo */

	TOKEN_FOR = 33,	  /**< 'for', loop 'para x em y faça' */
	TOKEN_WHILE = 34, /**< 'while', loop 'enquanto x faça' */

	TOKEN_CLASS = 35, /**< 'class', classe OOP */
	TOKEN_THIS = 36,  /**< 'this', referência a própria classe */
	TOKEN_SUPER = 37, /**< 'super', super-classe */

	TOKEN_FN = 38,	   /**< 'func', Declaração de função */
	TOKEN_RETURN = 39, /**< 'return', Retorna de uma função */

	TOKEN_IF = 40,		 /**< 'if', condicional 'se' */
	TOKEN_ELSE = 41,	 /**< 'else', condicional 'caso contrário' */
	TOKEN_COLON = 42,	 /**< ':', dois pontos */
	TOKEN_QUESTION = 43, /**< '?', interrogação */

	TOKEN_PRINT = 44, /**< 'print', imprime uma variável pra tela */
	TOKEN_LET = 45,	  /**< 'var', declara uma nova variável */
	TOKEN_CONST = 46, /**< 'const', declara uma nova constante */

	TOKEN_ERROR = 47, /**< 'error', emitido quando um erro é detectado */
	TOKEN_EOF = 48,	  /**< 'eof', fim do arquivo */
} TokenType;

/**
 * @brief Struct representando um token da linguagem
 */
typedef struct Token {
	TokenType type;	   /**< Tipo do token */
	const char *START; /**< Ponteiro pro caractere inicial do token */

	size_t length; /**< Tamanho da string que compõe o token */
	size_t line;   /**< Linha onde o token está */
} Token;

#endif	// GUARD_LOXIE_TOKENS_H
