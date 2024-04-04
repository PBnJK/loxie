/**
 * @file compiler.c
 * @author Pedro B.
 * @date 2024.04.02
 *
 * @brief Compila a linguagem NEAT
 */

#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "parser.h"
#include "scanner.h"
#include "token.h"
#include "opcodes.h"
#include "error.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

Parser parser = {0};		/**< Instância global do Parser */

Chunk *currentChunk;
static Chunk *_chunk() {
	return currentChunk;
}

static void _expression( void );
static ParseRule *_getRule( const TokenType TYPE );
static void _precedence( const Precedence PRECEDENCE );

static void _errorAtCurr( const char *MSG );
static void _errorAtPrev( const char *MSG );

/**
 * @brief Avança um token para frente
 */
static void _advance( void ) {
	parser.previous = parser.current;

	while( true ) {
		parser.current = scanToken();
		if( parser.current.type != TOKEN_ERROR )
			break;

		_errorAtCurr(parser.current.START);
	}
}

/**
 * @brief Consome o próximo token, dando erro se não for o esperado
 *
 * @param[in] TYPE Tipo de token esperado
 * @param[in] MSG Mensagem que deve ser mostrada caso o token não seja do tipo
 * esperado
 */
static void _consume( const TokenType TYPE, const char *MSG  ) {
if( parser.current.type == TYPE ) {
		_advance();
		return;
	}

	_errorAtCurr(MSG);
}

static void _precedence( const Precedence PRECEDENCE ) {
	_advance();

	ParseFn prefix = _getRule(parser.previous.type)->prefix;
	if( prefix == NULL ) {
		_errorAtPrev("Esperava expressao");
		return;
	}

	prefix();

	while( PRECEDENCE <= _getRule(parser.current.type)->precedence ) {
		_advance();
		ParseFn infix = _getRule(parser.previous.type)->infix;
		infix();
	}
}

/**
 * @brief Coloca um byte na chunk
 *
 * @param[in] BYTE Byte que será escrito
 */
static void _emitByte( const uint8_t BYTE ) {
	chunkWrite( _chunk(), BYTE, parser.previous.line );
}

/**
 * @brief Escreve dois bytes na chunk
 *
 * @param[in] BYTE1 Primeiro byte que será escrito
 * @param[in] BYTE2 Segundo byte que será escrito
 */
static void _emitBytes( const uint8_t BYTE1, const uint8_t BYTE2 ) {
	_emitByte(BYTE1);
	_emitByte(BYTE2);
}

/**
 * @brief Emite uma instrução de retorno
 */
static void _emitReturn( void ) {
	_emitByte(OP_RETURN);
}

/**
 * @brief Insere uma constante no array de constantes, sem escrever um opcode na chunk
 *
 * @param[in] value Valor constante que será criado
 * @return Índice do valor no array de constantes
 */
static size_t _makeConstant( Value value ) {
	return chunkAddConst( _chunk(), value );
}

/**
 * @brief Insere uma constante no array de constantes e escreve um opcode na chunk
 *
 * @param[in] value Valor constante que será criado
 */
static void _emitConstant( Value value ) {
	chunkWriteConst( _chunk(), value, parser.previous.line );
}

/**
 * @brief Compila uma expressão
 */
static void _expression( void ) {
	_precedence(PREC_ASSIGNMENT);
}

/**
 * @brief Compila uma expressão de agrupamento
 */
static void _grouping( void ) {
	_expression();	/* Chamada recursiva da função _expression()... */
	_consume(TOKEN_RPAREN, "Esperava um ')' depois da expressao");
}

/**
 * @brief Compila um número, inserindo seu valor na chunk
 */
static void _number( void ) {
	Value value = strtod(parser.previous.START, NULL);
	_emitConstant(value);
}

/**
 * @brief Compila uma expressão unária ( -2, !variável, etc )
 */
static void _unary( void ) {
	const TokenType OP_TYPE = parser.previous.type;

	/* Consumimos o operando... */
	_precedence(PREC_UNARY);

	/* ...e processamos o operador! */
	switch( OP_TYPE ) {
		case TOKEN_MINUS:
			_emitByte(OP_NEGATE);
			return;
		default:
			return;
	}
}

/**
 * @brief Compila uma expressão binária ( 2 + 3, a / 5, etc )
 */
static void _binary( void ) {
	const TokenType OP_TYPE = parser.previous.type;
	ParseRule *rule = _getRule(OP_TYPE);
	_precedence( (Precedence)(rule->precedence + 1) );

	switch( OP_TYPE ) {
		case TOKEN_PLUS:
			_emitByte(OP_ADD);
			break;
		case TOKEN_MINUS:
			_emitByte(OP_SUB);
			break;
		case TOKEN_STAR:
			_emitByte(OP_MUL);
			break;
		case TOKEN_SLASH:
			_emitByte(OP_DIV);
			break;
		case TOKEN_PERCENT:
			_emitByte(OP_MOD);
			break;
		default:
			return;
	}
}

/**
 * @brief Array com as regras que serão usadas na compilação da linguagem
 */
ParseRule rules[] = {
	[TOKEN_LPAREN]        = { _grouping, NULL,    PREC_NONE   },
	[TOKEN_RPAREN]        = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_LBRACKET]      = { NULL,      NULL,    PREC_NONE   }, 
	[TOKEN_RBRACKET]      = { NULL,      NULL,    PREC_NONE   }, 
	[TOKEN_LBRACE]        = { NULL,      NULL,    PREC_NONE   }, 
	[TOKEN_RBRACE]        = { NULL,      NULL,    PREC_NONE   },

	[TOKEN_COMMA]         = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_DOT]           = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_SEMICOLON]     = { NULL,      NULL,    PREC_NONE   },

	[TOKEN_PLUS]          = { NULL,      _binary, PREC_TERM   },
	[TOKEN_MINUS]         = { _unary,    _binary, PREC_TERM   },
	[TOKEN_SLASH]         = { NULL,      _binary, PREC_FACTOR },
	[TOKEN_STAR]          = { NULL,      _binary, PREC_FACTOR },
	[TOKEN_PERCENT]       = { NULL,      _binary, PREC_FACTOR },
	[TOKEN_BANG]          = { NULL,      NULL,    PREC_NONE   },

	[TOKEN_BANG_EQUAL]    = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_EQUAL]         = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_EQUAL_EQUAL]   = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_LESS]          = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_LESS_EQUAL]    = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_GREATER]       = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_GREATER_EQUAL] = { NULL,      NULL,    PREC_NONE   },

	[TOKEN_IDENTIFIER]    = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_STRING]        = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_INTERPOLATION] = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_NUMBER]        = { _number,   NULL,    PREC_NONE   },

	[TOKEN_AND]           = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_OR]            = { NULL,      NULL,    PREC_NONE   },

	[TOKEN_TRUE]          = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_FALSE]         = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_NIL]           = { NULL,      NULL,    PREC_NONE   },

	[TOKEN_FOR]           = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_WHILE]         = { NULL,      NULL,    PREC_NONE   },

	[TOKEN_CLASS]         = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_THIS]          = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_SUPER]         = { NULL,      NULL,    PREC_NONE   },
	
	[TOKEN_FN]            = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_RETURN]        = { NULL,      NULL,    PREC_NONE   },
	
	[TOKEN_IF]            = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_ELSE]          = { NULL,      NULL,    PREC_NONE   },

	[TOKEN_PRINT]         = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_LET]           = { NULL,      NULL,    PREC_NONE   },

	[TOKEN_ERROR]         = { NULL,      NULL,    PREC_NONE   },
	[TOKEN_EOF]           = { NULL,      NULL,    PREC_NONE   },
};

static ParseRule *_getRule( const TokenType TYPE ) {
	return &rules[TYPE];
}

/**
 * @brief Encerra o compilador
 */
static void _end( void ) {
	_emitReturn();

#ifdef DEBUG_PRINT_CODE
	if( !parser.hadError ) {
		debugDisassembleChunk( _chunk(), "<script>" );
	}
#endif
}

bool compCompile( const char *SOURCE, Chunk *chunk ) {
	scannerInit(SOURCE);

	currentChunk = chunk;
	parser.hadError = parser.panicked = false;

	_advance();
	_expression();
	_consume(TOKEN_EOF, "Esperava fim da expressao");

	_end();
	return !parser.hadError;
}

static void _errorAt( Token *token, const char *MSG ) {
	if( parser.panicked ) {
		return;
	}

	parser.hadError = parser.panicked = true;

	errFatal( token->line, MSG );

	if( token->type == TOKEN_EOF ) {
		errFatal(token->line, "%s\n\t~ no final da linha", MSG);
		return;
	} else if( token->type == TOKEN_ERROR ) {
		return;
	}

	errFatal( token->line, "%s\n\t~ no trecho '%.*s'", token->length, token->START );
}

static void _errorAtCurr( const char *MSG ) {
	_errorAt( &parser.current, MSG );
}

static void _errorAtPrev( const char *MSG ) {
	_errorAt( &parser.previous, MSG );
}

