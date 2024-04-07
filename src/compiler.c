/**
 * @file compiler.c
 * @author Pedro B.
 * @date 2024.04.02
 *
 * @brief Compila a linguagem NEAT
 */

#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "object.h"
#include "opcodes.h"
#include "parser.h"
#include "scanner.h"
#include "token.h"
#include "vm.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

/**
 * @brief Struct representando uma variável local
 */
typedef struct Local {
	Token name;
	int16_t depth;
} Local;

/**
 * @brief Struct representando o compilador
 */
typedef struct Compiler {
	Local locals[UINT8_COUNT];
	int16_t localCount;
	int16_t scope;
} Compiler;

uint16_t stackMax = 0; /**< Quantidade máxima de valores que serão inseridos n
						 pilha */

Parser parser = {0};	  /**< Instância global do Parser */
Compiler* current = NULL; /**< Ponteiro pro compilador atual */

Chunk* currentChunk; /**< Chunk sendo usada atualmente */
static Chunk* _chunk() {
	return currentChunk;
}

static void _expression(void);
static void _declaration(void);
static void _statement(void);

static ParseRule* _getRule(const TokenType TYPE);
static void _precedence(const Precedence PRECEDENCE);

static void _errorAtCurr(const char* MSG);
static void _errorAtPrev(const char* MSG);

static void _increaseStackMax(void) {
	++stackMax;
	if( stackMax >= vm.stackMax ) {
		vm.stackMax = stackMax;
	}
}

/**
 * @brief Avança um token para frente
 */
static void _advance(void) {
	parser.previous = parser.current;

	while( true ) {
		parser.current = scanToken();
		if( parser.current.type != TOKEN_ERROR ) break;

		_errorAtCurr(parser.current.START);
	}
}

/**
 * @todo Documentar
 */
static bool _check(const TokenType TYPE) {
	return parser.current.type == TYPE;
}

/**
 * @todo Documentar
 */
static bool _match(const TokenType TYPE) {
	if( !_check(TYPE) ) {
		return false;
	}

	_advance();
	return true;
}

/**
 * @brief Consome o próximo token, dando erro se não for o esperado
 *
 * @param[in] TYPE Tipo de token esperado
 * @param[in] MSG Mensagem que deve ser mostrada caso o token não seja do tipo
 * esperado
 */
static void _consume(const TokenType TYPE, const char* MSG) {
	if( parser.current.type == TYPE ) {
		_advance();
		return;
	}

	_errorAtCurr(MSG);
}

static void _precedence(const Precedence PRECEDENCE) {
	_advance();

	ParseFn prefix = _getRule(parser.previous.type)->prefix;
	if( prefix == NULL ) {
		printf("%u\n", parser.previous.type);
		_errorAtPrev("Esperava expressao");
		return;
	}

	const bool CAN_ASSIGN = (PRECEDENCE <= PREC_ASSIGNMENT);
	prefix(CAN_ASSIGN);

	while( PRECEDENCE <= _getRule(parser.current.type)->precedence ) {
		_advance();
		ParseFn infix = _getRule(parser.previous.type)->infix;
		infix(CAN_ASSIGN);
	}
}

/**
 * @brief Coloca um byte na chunk
 *
 * @param[in] BYTE Byte que será escrito
 */
static void _emitByte(const uint8_t BYTE) {
	chunkWrite(_chunk(), BYTE, parser.previous.line);
}

/**
 * @brief Escreve dois bytes na chunk
 *
 * @param[in] BYTE1 Primeiro byte que será escrito
 * @param[in] BYTE2 Segundo byte que será escrito
 */
static void _emitBytes(const uint8_t BYTE1, const uint8_t BYTE2) {
	_emitByte(BYTE1);
	_emitByte(BYTE2);
}

/**
 * @brief Emite uma instrução pop
 */
static void _emitPop(void) {
	--stackMax;
	_emitByte(OP_POP);
}

/**
 * @brief Emite uma instrução de retorno
 */
static void _emitReturn(void) {
	_emitByte(OP_RETURN);
}

/**
 * @brief Insere uma constante no array de constantes, sem escrever um opcode na
 * chunk
 *
 * @param[in] value Valor constante que será criado
 * @return Índice do valor no array de constantes
 */
static size_t _makeConstant(Value value) {
	_increaseStackMax();
	return chunkAddConst(_chunk(), value);
}

/**
 * @brief Insere uma constante no array de constantes e escreve um opcode na
 * chunk
 *
 * @param[in] value Valor constante que será criado
 */
static void _emitConstant(Value value) {
	_increaseStackMax();
	chunkWriteConst(_chunk(), value, parser.previous.line);
}

static void _emitConstantWithOp(const OpCode TYPE_SHORT, const OpCode TYPE_LONG,
								const size_t INDEX) {
	if( INDEX > UINT8_MAX ) {
		_emitByte(TYPE_LONG);
		_emitByte((uint8_t)(INDEX & 0xFF));
		_emitByte((uint8_t)((INDEX >> 8) & 0xFF));
		_emitByte((uint8_t)((INDEX >> 16) & 0xFF));
		return;
	}

	_emitByte(TYPE_SHORT);
	_emitByte((uint8_t)INDEX);
}

static void _patchJump(const int32_t OFFSET) {
	const int32_t JUMP = _chunk()->count - OFFSET - 2;

	if( JUMP > UINT16_MAX ) {
		_errorAtPrev("Too much code to jump over.");
	}

	_chunk()->code[OFFSET] = (JUMP >> 8) & 0xff;
	_chunk()->code[OFFSET + 1] = JUMP & 0xff;
}

static int32_t _emitJump(const OpCode INSTRUCTION) {
	_emitByte(INSTRUCTION);
	_emitByte(0x7f);
	_emitByte(0x7f);

	return _chunk()->count - 2;
}

static void _initCompiler(Compiler* compiler) {
	compiler->localCount = 0;
	compiler->scope = 0;

	current = compiler;
}

static void _beginScope(void) {
	++current->scope;
}

static void _endScope(void) {
	--current->scope;

	while( current->localCount > 0 &&
		   current->locals[current->localCount - 1].depth > current->scope ) {
		_emitPop();
		--current->localCount;
	}
}

/**
 * @brief Compila uma expressão
 */
static void _expression(void) {
	_precedence(PREC_ASSIGNMENT);
}

static void _block(void) {
	while( !_check(TOKEN_RBRACE) && !_check(TOKEN_EOF) ) {
		_declaration();
	}

	_consume(TOKEN_RBRACE, "Esperava '{' depois de um bloco");
}

static void _expressionStatement(void) {
	_expression();
	_consume(TOKEN_SEMICOLON, "Esperava ';' depois do valor");
	_emitPop();
}

static void _printStatement(void) {
	_expression();
	_consume(TOKEN_SEMICOLON, "Esperava ';' depois do valor");
	_emitByte(OP_PRINT);
}

static void _ifStatement(void) {
	_consume(TOKEN_LPAREN, "Esperava '(' depois do 'if'.");
	_expression();
	_consume(TOKEN_RPAREN, "Esperava ')' depois da condicao.");

	const int32_t THEN_JUMP = _emitJump(OP_JUMP_IF_FALSE);
	_emitPop();
	_statement();

	const int32_t ELSE_JUMP = _emitJump(OP_JUMP);

	_patchJump(THEN_JUMP);
	_emitPop();

	if( _match(TOKEN_ELSE) ) {
		_statement();
	}

	_patchJump(ELSE_JUMP);
}

static void _synchronize(void) {
	parser.panicked = false;

	while( parser.current.type != TOKEN_EOF ) {
		if( parser.previous.type == TOKEN_SEMICOLON ) {
			return;
		}

		switch( parser.current.type ) {
			case TOKEN_CLASS:
			case TOKEN_FN:
			case TOKEN_LET:
			case TOKEN_CONST:
			case TOKEN_FOR:
			case TOKEN_IF:
			case TOKEN_WHILE:
			case TOKEN_PRINT:
			case TOKEN_RETURN:
				return;
			default:;
		}

		_advance();
	}
}

static size_t _identifierConstant(Token* name) {
	Value string = CREATE_OBJECT(objCopyString(name->START, name->length));
	Value index;
	if( tableGet(&vm.globalNames, string, &index) ) {
		return AS_NUMBER(index);
	}

	const size_t INDEX = vm.globalValues.count;

	valueArrayWrite(&vm.globalValues, CREATE_EMPTY());
	tableSet(&vm.globalNames, string, CREATE_NUMBER((double)INDEX));

	return INDEX;
}

static bool _identifiersEqual(Token* a, Token* b) {
	if( a->length != b->length ) {
		return false;
	}

	return memcmp(a->START, b->START, a->length) == 0;
}

static int16_t _resolveLocal(Compiler* compiler, Token* name) {
	for( int16_t i = compiler->localCount - 1; i >= 0; --i ) {
		Local* local = &compiler->locals[i];
		if( _identifiersEqual(name, &local->name) ) {
			if( local->depth == -1 ) {
				_errorAtPrev("Impossivel iniciar variavel consigo mesma");
			}

			return i;
		}
	}

	return -1;
}

static void _markInitialized() {
	current->locals[current->localCount - 1].depth = current->scope;
}

static void _addLocal(Token name) {
	if( current->localCount == UINT8_COUNT ) {
		_errorAtPrev("Variáveis locais de mais na função.");
		return;
	}

	Local* local = &current->locals[current->localCount++];
	local->name = name;
	local->depth = -1;
}

static void _declareVariable(void) {
	if( current->scope == 0 ) {
		return;
	}

	Token* name = &parser.previous;
	for( int16_t i = current->localCount - 1; i >= 0; --i ) {
		Local* local = &current->locals[i];
		if( local->depth != -1 && local->depth < current->scope ) {
			break;
		}

		if( _identifiersEqual(name, &local->name) ) {
			_errorAtPrev("Variavel com este nome ja existe nesse escopo");
		}
	}

	_addLocal(*name);
}

static size_t _parseVariable(const char* MSG) {
	_consume(TOKEN_IDENTIFIER, MSG);

	_declareVariable();
	if( current->scope > 0 ) {
		return 0;
	}

	return _identifierConstant(&parser.previous);
}

static void _defineVariable(const uint32_t GLOBAL) {
	if( current->scope > 0 ) {
		_markInitialized();
		return;
	}

	--stackMax;
	_emitConstantWithOp(OP_DEF_GLOBAL_16, OP_DEF_GLOBAL_32, GLOBAL);
}

static void _defineConst(const uint32_t GLOBAL) {
	if( current->scope > 0 ) {
		_markInitialized();
		return;
	}

	--stackMax;
	_emitConstantWithOp(OP_DEF_CONST_16, OP_DEF_CONST_32, GLOBAL);
}

static void _and(const bool CAN_ASSIGN) {
	const int32_t END_JUMP = _emitJump(OP_JUMP_IF_FALSE);

	_emitPop();
	_precedence(PREC_AND);

	_patchJump(END_JUMP);
}

static void _or(const bool CAN_ASSIGN) {
	const int32_t ELSE_JUMP = _emitJump(OP_JUMP_IF_FALSE);
	const int32_t END_JUMP = _emitJump(OP_JUMP);

	_patchJump(ELSE_JUMP);
	_emitPop();

	_precedence(PREC_OR);
	_patchJump(END_JUMP);
}

static void _varDeclaration(void) {
	const size_t GLOBAL = _parseVariable("Esperava o nome da variável.");

	if( _match(TOKEN_EQUAL) ) {
		_expression();
	} else {
		_emitByte(OP_NIL);
	}

	_consume(TOKEN_SEMICOLON, "Esperava ';' depois de declaração de variável.");

	_defineVariable(GLOBAL);
}

static void _constDeclaration(void) {
	const size_t GLOBAL = _parseVariable("Esperava o nome da variável.");

	if( _match(TOKEN_EQUAL) ) {
		_expression();
	} else {
		_errorAtPrev("Constantes precisam ser definidas imediatamente");
	}

	_consume(TOKEN_SEMICOLON, "Esperava ';' depois de declaração de variável.");

	_defineConst(GLOBAL);
}

/**
 * @brief Compila uma declaração
 */
static void _statement(void) {
	if( _match(TOKEN_PRINT) ) {
		_printStatement();
	} else if( _match(TOKEN_IF) ) {
		_ifStatement();
	} else if( _match(TOKEN_LBRACE) ) {
		_beginScope();
		_block();
		_endScope();
	} else {
		_expressionStatement();
	}
}

/**
 * @brief Compila uma declaração de nome
 */
static void _declaration(void) {
	if( _match(TOKEN_LET) ) {
		_varDeclaration();
	} else if( _match(TOKEN_CONST) ) {
		_constDeclaration();
	} else {
		_statement();
	}

	if( parser.panicked ) {
		_synchronize();
	}
}

/**
 * @brief Compila uma expressão de agrupamento
 */
static void _grouping(const bool CAN_ASSIGN) {
	_expression(); /* Chamada recursiva da função _expression()... */
	_consume(TOKEN_RPAREN, "Esperava um ')' depois da expressao");
}

/**
 * @brief Compila um número
 */
static void _number(const bool CAN_ASSIGN) {
	const NEAT_NUMBER NUMBER = (NEAT_NUMBER)strtod(parser.previous.START, NULL);
	_emitConstant(CREATE_NUMBER(NUMBER));
}

/**
 * @brief Compila uma string
 */
static void _string(const bool CAN_ASSIGN) {
	_emitConstant(CREATE_OBJECT(
		objCopyString(parser.previous.START + 1, parser.previous.length - 2)));
}

static void _namedVariable(Token name, const bool CAN_ASSIGN) {
	uint8_t getOp, setOp;
	int16_t arg = _resolveLocal(current, &name);

	if( arg != -1 ) {
		getOp = OP_GET_LOCAL_16;
		setOp = OP_SET_LOCAL_16;
	} else {
		arg = _identifierConstant(&name);
		getOp = OP_GET_GLOBAL_16;
		setOp = OP_SET_GLOBAL_16;
	}

	if( CAN_ASSIGN && _match(TOKEN_EQUAL) ) {
		_increaseStackMax();
		_expression();
		_emitConstantWithOp(setOp, setOp + 1, arg);
	} else {
		_emitConstantWithOp(getOp, getOp + 1, arg);
	}
}

static void _variable(const bool CAN_ASSIGN) {
	_namedVariable(parser.previous, CAN_ASSIGN);
}

/**
 * @brief Compila um literal (bool ou nil)
 */
static void _literal(const bool CAN_ASSIGN) {
	switch( parser.previous.type ) {
		case TOKEN_TRUE:
			_emitByte(OP_TRUE);
			break;
		case TOKEN_FALSE:
			_emitByte(OP_FALSE);
			break;
		case TOKEN_NIL:
			_emitByte(OP_NIL);
			break;
		default:
			return;
	}
}

/**
 * @brief Compila uma expressão unária ( -2, !variável, etc )
 */
static void _unary(const bool CAN_ASSIGN) {
	const TokenType OP_TYPE = parser.previous.type;

	/* Consumimos o operando... */
	_precedence(PREC_UNARY);

	/* ...e processamos o operador! */
	switch( OP_TYPE ) {
		case TOKEN_MINUS:
			_emitByte(OP_NEGATE);
			return;
		case TOKEN_BANG:
			_emitByte(OP_NOT);
			return;
		default:
			return;
	}
}

/**
 * @brief Compila uma expressão binária ( 2 + 3, a / 5, etc )
 */
static void _binary(const bool CAN_ASSIGN) {
	const TokenType OP_TYPE = parser.previous.type;
	ParseRule* rule = _getRule(OP_TYPE);
	_precedence((Precedence)(rule->precedence + 1));

	--stackMax;

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
		case TOKEN_BANG_EQUAL:
			_emitBytes(OP_EQUAL, OP_NOT);
			break;
		case TOKEN_EQUAL_EQUAL:
			_emitByte(OP_EQUAL);
			break;
		case TOKEN_GREATER:
			_emitByte(OP_GREATER);
			break;
		case TOKEN_GREATER_EQUAL:
			_emitByte(OP_GREATER_EQUAL);
			break;
		case TOKEN_LESS:
			_emitByte(OP_LESS);
			break;
		case TOKEN_LESS_EQUAL:
			_emitByte(OP_LESS_EQUAL);
			break;
		default:
			return;
	}
}

static void _conditional(const bool CAN_ASSIGN) {
	/*	_expression();
		_consume(TOKEN_RPAREN, "Esperava ')' depois da condicao.");

		const int32_t THEN_JUMP = _emitJump(OP_JUMP_IF_FALSE);
		_emitPop();
		_statement();

		const int32_t ELSE_JUMP = _emitJump(OP_JUMP);

		_patchJump(THEN_JUMP);
		_emitPop();

		if( _match(TOKEN_ELSE) ) {
			_statement();
		}

		_patchJump(ELSE_JUMP); */

	const int32_t THEN_JUMP = _emitJump(OP_JUMP_IF_FALSE);
	_emitPop();
	_expression();

	_consume(TOKEN_COLON, "Esperava ':'");
	const int32_t ELSE_JUMP = _emitJump(OP_JUMP);
	_patchJump(THEN_JUMP);

	_emitPop();
	_expression();

	_patchJump(ELSE_JUMP);
}

/**
 * @brief Array com as regras que serão usadas na compilação da linguagem
 */
ParseRule rules[] = {
	[TOKEN_LPAREN] = {_grouping, NULL, PREC_NONE},
	[TOKEN_RPAREN] = {NULL, NULL, PREC_NONE},
	[TOKEN_LBRACKET] = {NULL, NULL, PREC_NONE},
	[TOKEN_RBRACKET] = {NULL, NULL, PREC_NONE},
	[TOKEN_LBRACE] = {NULL, NULL, PREC_NONE},
	[TOKEN_RBRACE] = {NULL, NULL, PREC_NONE},

	[TOKEN_COMMA] = {NULL, NULL, PREC_NONE},
	[TOKEN_DOT] = {NULL, NULL, PREC_NONE},
	[TOKEN_SEMICOLON] = {NULL, NULL, PREC_NONE},

	[TOKEN_PLUS] = {NULL, _binary, PREC_TERM},
	[TOKEN_MINUS] = {_unary, _binary, PREC_TERM},
	[TOKEN_SLASH] = {NULL, _binary, PREC_FACTOR},
	[TOKEN_STAR] = {NULL, _binary, PREC_FACTOR},
	[TOKEN_PERCENT] = {NULL, _binary, PREC_FACTOR},
	[TOKEN_BANG] = {_unary, NULL, PREC_NONE},

	[TOKEN_BANG_EQUAL] = {NULL, _binary, PREC_EQUALITY},
	[TOKEN_EQUAL] = {NULL, _binary, PREC_EQUALITY},
	[TOKEN_EQUAL_EQUAL] = {NULL, _binary, PREC_COMPARISON},
	[TOKEN_LESS] = {NULL, _binary, PREC_COMPARISON},
	[TOKEN_LESS_EQUAL] = {NULL, _binary, PREC_COMPARISON},
	[TOKEN_GREATER] = {NULL, _binary, PREC_COMPARISON},
	[TOKEN_GREATER_EQUAL] = {NULL, _binary, PREC_COMPARISON},

	[TOKEN_IDENTIFIER] = {_variable, NULL, PREC_NONE},
	[TOKEN_STRING] = {_string, NULL, PREC_NONE},
	[TOKEN_INTERPOLATION] = {NULL, NULL, PREC_NONE},
	[TOKEN_NUMBER] = {_number, NULL, PREC_NONE},

	[TOKEN_AND] = {NULL, _and, PREC_AND},
	[TOKEN_OR] = {NULL, _or, PREC_OR},

	[TOKEN_TRUE] = {_literal, NULL, PREC_NONE},
	[TOKEN_FALSE] = {_literal, NULL, PREC_NONE},
	[TOKEN_NIL] = {_literal, NULL, PREC_NONE},

	[TOKEN_FOR] = {NULL, NULL, PREC_NONE},
	[TOKEN_WHILE] = {NULL, NULL, PREC_NONE},

	[TOKEN_CLASS] = {NULL, NULL, PREC_NONE},
	[TOKEN_THIS] = {NULL, NULL, PREC_NONE},
	[TOKEN_SUPER] = {NULL, NULL, PREC_NONE},

	[TOKEN_FN] = {NULL, NULL, PREC_NONE},
	[TOKEN_RETURN] = {NULL, NULL, PREC_NONE},

	[TOKEN_IF] = {NULL, NULL, PREC_NONE},
	[TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
	[TOKEN_QUESTION] = {NULL, _conditional, PREC_CONDITIONAL},
	//	[TOKEN_COLON] = {NULL, NULL, PREC_NONE},

	[TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
	[TOKEN_LET] = {NULL, NULL, PREC_NONE},
	[TOKEN_CONST] = {NULL, NULL, PREC_NONE},

	[TOKEN_ERROR] = {NULL, NULL, PREC_NONE},
	[TOKEN_EOF] = {NULL, NULL, PREC_NONE},
};

static ParseRule* _getRule(const TokenType TYPE) {
	return &rules[TYPE];
}

/**
 * @brief Encerra o compilador
 */
static void _end(void) {
	_emitReturn();
	vmInitStack();

#ifdef DEBUG_PRINT_CODE
	if( !parser.hadError ) {
		debugDisassembleChunk(_chunk(), "<script>");
	}
#endif
}

bool compCompile(const char* SOURCE, Chunk* chunk) {
	scannerInit(SOURCE);

	Compiler compiler;
	_initCompiler(&compiler);

	currentChunk = chunk;
	parser.hadError = parser.panicked = false;

	stackMax = 0;

	_advance();
	while( !_match(TOKEN_EOF) ) {
		_declaration();
	}

	_end();
	return !parser.hadError;
}

static void _errorAt(Token* token, const char* MSG) {
	if( parser.panicked ) {
		return;
	}

	parser.hadError = parser.panicked = true;

	if( token->type == TOKEN_EOF ) {
		errFatal(token->line, "%s\n\t~ no final da linha", MSG);
		return;
	} else if( token->type == TOKEN_ERROR ) {
		return;
	}

	errFatal(token->line, "%s\n\t~ no trecho '%.*s'", MSG, token->length,
			 token->START);
}

static void _errorAtCurr(const char* MSG) {
	_errorAt(&parser.current, MSG);
}

static void _errorAtPrev(const char* MSG) {
	_errorAt(&parser.previous, MSG);
}
