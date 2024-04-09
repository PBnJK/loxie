/**
 * @file scanner.c
 * @author Pedro B.
 * @date 2024.04.03
 *
 * @brief Tokenizador de código da linguagem Loxie
 */

#include "scanner.h"

#include <stdio.h>
#include <string.h>

/**
 * @brief Struct representando o tokenizador
 */
typedef struct Scanner {
	const char* START;	 /**< Primeiro caractere do lexema atual */
	const char* CURRENT; /**< Caractere atual do lexema que atual*/

	size_t line; /**< Linha atual */
} Scanner;

Scanner scanner; /**< Instância global do tokenizador */

/**
 * @brief Constrói uma string
 * @return Token do tipo TOKEN_STRING
 */
static Token _string(void);

/**
 * @brief Constrói um número
 * @return Token do tipo TOKEN_NUMBER
 */
static Token _number(void);

/**
 * @brief Constrói um identificador (palavra-chave ou nome definido pelo
 * usuário)
 * @return TOKEN_n, onde n é uma palavra-chave, ou um token genérico
 * TOKEN_IDENTIFIER
 */
static Token _identifier(void);

/**
 * @brief Cria um token do dado tipo
 *
 * @param[in] TYPE Tipo de token que será criado
 *
 * @return Token do tipo @a type
 */
static Token _makeToken(const TokenType TYPE);

/**
 * @brief Cria um token especial que representa um erro de compilação
 *
 * @param[in] MSG Mensagem de erro
 *
 * @return Token do tipo TOKEN_ERROR
 */
static Token _errorToken(const char* MSG);

void scannerInit(const char* SOURCE) {
	scanner.START = scanner.CURRENT = SOURCE;
	scanner.line = 1;
}

/**
 * @brief Verifica se chegamos no fim do código
 * @return Verdadeiro se chegamos no final
 */
static bool _atEnd(void) {
	return *scanner.CURRENT == '\0';
}

/**
 * @brief Verifica se um dado caractere é numérico
 *
 * @param[in] CHAR Caractere que será verificado
 * @return Verdadeiro se o caractere for um número
 */
static bool _isDigit(const char CHAR) {
	return CHAR >= '0' && CHAR <= '9';
}

/**
 * @brief Verifica se um dado caractere é uma letra
 *
 * @param[in] CHAR Caractere que será verificado
 * @return Verdadeiro se o caractere for uma letra
 */
static bool _isAlpha(const char CHAR) {
	return (CHAR >= 'a' && CHAR <= 'z') || (CHAR >= 'A' && CHAR <= 'Z') ||
		   CHAR == '_';
}

/**
 * @brief Verifica se o caractere é alfanumérico
 *
 * @param[in] CHAR Caractere que será verificado
 * @return Verdadeiro se o caractere for um número ou uma letra
 */
static bool _isAlphanumeric(const char CHAR) {
	return _isAlpha(CHAR) || _isDigit(CHAR);
}

/**
 * @brief Avança o caractere atual
 * @return Caractere logo atrás do atual
 */
static char _advance(void) {
	return *(scanner.CURRENT++);
}

/**
 * @brief Vê o caractere atual, sem avançar
 * @return O caractere atual
 */
static char _peek(void) {
	return *scanner.CURRENT;
}

/**
 * @brief Vê o próximo caractere, sem avançar
 * @return O caractere a frente ou '\0', caso tenhamos chegado ao fim do código
 */
static char _peekNext(void) {
	if( _atEnd() ) {
		return '\0';
	}

	return scanner.CURRENT[1];
}

/**
 * @brief Verifica se o caractere atual é o esperado
 *
 * @param[in] EXPECTED Caractere esperado
 * @return Verdadeiro se o caractere atual for igual à @a EXPECTED
 */
static bool _match(const char EXPECTED) {
	if( _atEnd() || *scanner.CURRENT != EXPECTED ) {
		return false;
	}

	++scanner.CURRENT;
	return true;
}

/**
 * @brief Pula um comentário de múltiplas linhas estilo C
 */
static void _skipCommentBlock(void) {
	while( !_atEnd() ) {
		const char CHAR = _advance();

		if( CHAR == '*' && _peekNext() == '/' ) {
			return;
		}
	}
}

/**
 * @brief Pula espaços vazios/comentários
 */
static void _skipSpace(void) {
	while( true ) {
		const char CHAR = _peek();
		switch( CHAR ) {
			case ' ':
			case '\t':
			case '\r':
				_advance();
				break;
			case '\n':
				++scanner.line;
				_advance();
				break;
			case '/':
				if( _peekNext() == '/' ) {
					while( !_atEnd() && _advance() != '\n' )
						;
				} else if( _peekNext() == '*' ) {
					_skipCommentBlock();
				} else {
					return;
				}

				break;
			default:
				return;
		}
	}
}

/**
 * @brief Verifica se o lexema atual é igual à palavra-chave esperada
 *
 * @param[in] START Índice do caractere a partir do qual faremos a comparação
 * @param[in] LENGTH Tamanho da palavra-chave
 * @param[in] REST Resto da palavra-chave
 * @param[in] EXPECTED Tipo de Token à qual a palavra-chave esperada pertence
 *
 * @return Se o lexema bater com a palavra-chave esperada, retorna @a EXPECTED.
 * Caso contrário, retorna TOKEN_IDENTIFIER
 */
static TokenType _checkKeyword(const size_t START, const size_t LENGTH,
							   const char* REST, const TokenType EXPECTED) {
	/* Comparamos o tamanho das strings primeiro aqui para agilizar o processo
	 * Se não forem iguais, o memcmp (operação demorada...) é pulado e
	 * retornamos TOKEN_IDENTIFIER direto
	 */
	if( (size_t)(scanner.CURRENT - scanner.START) == (START + LENGTH) &&
		memcmp(scanner.START + START, REST, LENGTH) == 0 ) {
		return EXPECTED;
	}

	return TOKEN_IDENTIFIER;
}

Token scanToken(void) {
	_skipSpace();

	scanner.START = scanner.CURRENT;
	if( _atEnd() ) {
		return _makeToken(TOKEN_EOF);
	}

	const char CHAR = _advance();

	if( _isDigit(CHAR) ) {
		return _number();
	}

	if( _isAlpha(CHAR) ) {
		return _identifier();
	}

	switch( CHAR ) {
		case '(':
			return _makeToken(TOKEN_LPAREN);
		case ')':
			return _makeToken(TOKEN_RPAREN);
		case '[':
			return _makeToken(TOKEN_LBRACKET);
		case ']':
			return _makeToken(TOKEN_RBRACKET);
		case '{':
			return _makeToken(TOKEN_LBRACE);
		case '}':
			return _makeToken(TOKEN_RBRACE);
		case '$':
			return _makeToken(TOKEN_DOLLAR);
		case '#':
			return _makeToken(TOKEN_HASH);
		case ',':
			return _makeToken(TOKEN_COMMA);
		case '.':
			return _makeToken(TOKEN_DOT);
		case ';':
			return _makeToken(TOKEN_SEMICOLON);
		case '+':
			return _makeToken(TOKEN_PLUS);
		case '-':
			return _makeToken(TOKEN_MINUS);
		case '*':
			return _makeToken(TOKEN_STAR);
		case '/':
			return _makeToken(TOKEN_SLASH);
		case '%':
			return _makeToken(TOKEN_PERCENT);
		case '?':
			return _makeToken(TOKEN_QUESTION);
		case ':':
			return _makeToken(TOKEN_COLON);
		case '!':
			return _makeToken(_match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
		case '=':
			return _makeToken(_match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
		case '<':
			return _makeToken(_match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
		case '>':
			return _makeToken(_match('=') ? TOKEN_GREATER_EQUAL
										  : TOKEN_GREATER);
		case '"':
			return _string();
	}

	/* Nenhum caractere combinou, retornamos um erro */
	return _errorToken("Caractere inesperado encontrado");
}

static Token _string(void) {
	while( _peek() != '"' && !_atEnd() ) {
		if( _peek() == '\n' ) {
			++scanner.line;
		}

		_advance();
	}

	if( _atEnd() ) {
		return _errorToken("String sem aspas finais");
	}

	_advance();
	return _makeToken(TOKEN_STRING);
}

static Token _number(void) {
	while( _isDigit(_peek()) ) {
		_advance();
	}

	if( _peek() == '.' && _isDigit(_peekNext()) ) {
		_advance();
	}

	while( _isDigit(_peek()) ) {
		_advance();
	}

	return _makeToken(TOKEN_NUMBER);
}

static TokenType _getIdentifierType(void) {
	switch( *scanner.START ) {
		case 'c':
			if( scanner.CURRENT - scanner.START > 1 ) {
				switch( scanner.START[1] ) {
					case 'a':
						return _checkKeyword(2, 2, "so", TOKEN_CASE);
					case 'l':
						return _checkKeyword(2, 4, "asse", TOKEN_CLASS);
					case 'o': {
						if( _checkKeyword(2, 3, "nst", TOKEN_CONST) ==
							TOKEN_CONST ) {
							return TOKEN_CONST;
						}

						return _checkKeyword(2, 6, "ntinue", TOKEN_CONTINUE);
					}
				}
			}

			break;

		case 'e':
			if( scanner.CURRENT - scanner.START > 1 ) {
				switch( scanner.START[1] ) {
					case 'n':
						return _checkKeyword(2, 6, "quanto", TOKEN_WHILE);
					case 's':
						return _checkKeyword(2, 5, "colha", TOKEN_SWITCH);
				}
			}

			return TOKEN_AND;

		case 'f':
			if( scanner.CURRENT - scanner.START > 1 ) {
				switch( scanner.START[1] ) {
					case 'a':
						return _checkKeyword(2, 3, "lso", TOKEN_FALSE);
					case 'u':
						return _checkKeyword(2, 2, "nc", TOKEN_FN);
				}
			}

			break;

		case 'i':
			if( scanner.CURRENT - scanner.START > 1 ) {
				switch( scanner.START[1] ) {
					case 's':
						return _checkKeyword(2, 2, "to", TOKEN_THIS);
					case 'm':
						return _checkKeyword(2, 5, "prima", TOKEN_PRINT);
				}
			}

			break;

		case 'n':
			return _checkKeyword(1, 3, "ulo", TOKEN_NIL);

		case 'o':
			return _checkKeyword(1, 1, "u", TOKEN_OR);

		case 'p':
			if( scanner.CURRENT - scanner.START > 2 &&
				scanner.START[1] == 'a' ) {
				switch( scanner.START[2] ) {
					case 'd':
						return _checkKeyword(3, 3, "rao", TOKEN_DEFAULT);
					case 'r':
						return _checkKeyword(3, 1, "a", TOKEN_FOR);
				}
			}

			break;

		case 'r':
			return _checkKeyword(1, 6, "etorne", TOKEN_RETURN);

		case 's':
			if( scanner.CURRENT - scanner.START > 2 ) {
				switch( scanner.START[1] ) {
					case 'a':
						return _checkKeyword(2, 2, "ia", TOKEN_BREAK);
					case 'e':
						return _checkKeyword(2, 3, "nao", TOKEN_ELSE);
					case 'u':
						return _checkKeyword(2, 3, "per", TOKEN_SUPER);
				}
			}

			return _checkKeyword(1, 1, "e", TOKEN_IF);

		case 'v':
			if( scanner.CURRENT - scanner.START > 1 ) {
				switch( scanner.START[1] ) {
					case 'a':
						return _checkKeyword(2, 1, "r", TOKEN_LET);
					case 'e':
						return _checkKeyword(2, 8, "rdadeiro", TOKEN_TRUE);
				}
			}

			break;
	}

	return TOKEN_IDENTIFIER;
}

static Token _identifier(void) {
	while( _isAlphanumeric(_peek()) ) {
		_advance();
	}

	return _makeToken(_getIdentifierType());
}

static Token _makeToken(const TokenType TYPE) {
	return (Token){.type = TYPE,
				   .START = scanner.START,
				   .length = (size_t)(scanner.CURRENT - scanner.START),
				   .line = scanner.line};
}

static Token _errorToken(const char* MSG) {
	return (Token){.type = TOKEN_ERROR,
				   .START = MSG,
				   .length = (size_t)strlen(MSG),
				   .line = scanner.line};
}
