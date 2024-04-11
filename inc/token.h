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

	TOKEN_AND = 28, /**< 'e' lógico */
	TOKEN_OR = 29,	/**< v'ou' lógico */

	TOKEN_TRUE = 30,  /**< 'verdadeiro' */
	TOKEN_FALSE = 31, /**< 'falso' */
	TOKEN_NIL = 32,	  /**< 'nulo' */

	TOKEN_FOR = 33,		 /**< 'para', loop 'para x em y faça' */
	TOKEN_WHILE = 34,	 /**< 'enquanto', loop 'enquanto x faça' */
	TOKEN_BREAK = 35,	 /**< 'saia', sai de um loop */
	TOKEN_CONTINUE = 36, /**< 'continue', volta pro inicio do loop
							  ("pula" um passo) */

	TOKEN_CLASS = 37, /**< 'classe', classe OOP */
	TOKEN_THIS = 38,  /**< 'isto', referência a própria classe */
	TOKEN_SUPER = 39, /**< 'super', super-classe */

	TOKEN_FUNC = 40,   /**< 'func', Declaração de função */
	TOKEN_RETURN = 41, /**< 'retorne', Retorna de uma função */

	TOKEN_IF = 42,		 /**< 'se', condicional */
	TOKEN_ELSE = 43,	 /**< 'senao', condicional 'caso contrário' */
	TOKEN_SWITCH = 44,	 /**< 'escola', condicional múltipla */
	TOKEN_CASE = 45,	 /**< 'caso', caso em uma condicional múltipla */
	TOKEN_DEFAULT = 46,	 /**< 'padrao', caso padrao da condicional múltipla */
	TOKEN_COLON = 47,	 /**< ':', dois pontos */
	TOKEN_QUESTION = 48, /**< '?', interrogação */

	TOKEN_PRINT = 49, /**< 'print', imprime uma variável pra tela */
	TOKEN_LET = 50,	  /**< 'var', declara uma nova variável */
	TOKEN_CONST = 51, /**< 'const', declara uma nova constante */

	TOKEN_ERROR = 52, /**< 'error', emitido quando um erro é detectado */
	TOKEN_EOF = 53,	  /**< 'eof', fim do arquivo */
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
