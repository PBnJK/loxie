/**
 * @file compiler.c
 * @author Pedro B.
 * @date 2024.04.02
 *
 * @brief Compila a linguagem Loxie
 */

#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "memory.h"
#include "object.h"
#include "opcodes.h"
#include "parser.h"
#include "scanner.h"
#include "token.h"
#include "vm.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

/** Número máximo de casos em um escolha-caso */
#define MAX_CASES 256

/**
 * @brief Struct representando uma variável local
 */
typedef struct Local {
	Token name;		 /**< Token representando a variável */
	int16_t depth;	 /**< Escopo da variável */
	bool isCaptured; /**< Se esta variável foi capturada */
} Local;

/**
 * @brief Struct representando uma variável local capturada
 */
typedef struct Upvalue {
	ssize_t index; /**< Índice do upvalue no array de upvalues */
	bool isLocal;  /**< Se a variável sendo capturada é local */
} Upvalue;

/**
 * @brief Enum representando os tipos de função existentes
 */
typedef enum {
	TYPE_FUNCTION = 0, /**< Função declarada pelo usuário */
	TYPE_SCRIPT = 1,   /**< O script em si */
} FunctionType;

/**
 * @brief Struct representando o compilador
 */
typedef struct Compiler {
	struct Compiler* enclosing; /**< Compilador que contém este */

	ObjFunction* function; /**< Função sendo compilada */
	FunctionType type;	   /**< Tipo de função sendo compilada */

	Local* locals;		/**< Array com variáveis locais */
	int32_t localSize;	/**< Tamanho do array de variáveis locais */
	int32_t localCount; /**< Quantidade de variáveis locais */

	Upvalue* upvalues;	/**< Upvalues */
	size_t upvalueSize; /**< Tamanho do array de upvalues */

	int16_t scope; /**< Escopo das variáveis */
} Compiler;

uint16_t stackMax = 0;		 /**< Máximo de valores que estarão na pilha */
int16_t innerLoopStart = -1; /**< Começo do loop mais interno */
int16_t innerLoopScope = 0;	 /**< Profundidade do loop mais interno */

Parser parser = {0};	  /**< Instância global do Parser */
Compiler* current = NULL; /**< Ponteiro pro compilador atual */

static Chunk* _chunk() {
	return &current->function->chunk;
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

static void _doubleStackMax(void) {
	stackMax *= 2;
	if( stackMax >= vm.stackMax ) {
		vm.stackMax = stackMax;
	}
}

static void _decreaseStackMax(void) {
	if( stackMax > 0 ) {
		--stackMax;
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
	_decreaseStackMax();
	_emitByte(OP_POP);
}

/**
 * @brief Emite uma instrução de retorno
 */
static void _emitReturn(void) {
	_increaseStackMax();
	_emitBytes(OP_NIL, OP_RETURN);
}

/**
 * @brief Encerra o compilador
 */
static ObjFunction* _end(void) {
	_emitReturn();
	ObjFunction* function = current->function;

#ifdef DEBUG_PRINT_CODE
	if( !parser.hadError ) {
		debugDisassembleChunk(_chunk(), function->name != NULL
											? function->name->str
											: "<script>");
	}
#endif

	vmInitStack();

	current = current->enclosing;
	return function;
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

static void _emitLoop(const int32_t LOOP_START) {
	_emitByte(OP_LOOP);

	const int32_t OFFSET = _chunk()->count - LOOP_START + 2;
	if( OFFSET > UINT16_MAX ) {
		_errorAtPrev("Loop grande demais");
	}

	_emitByte((OFFSET >> 8) & 0xff);
	_emitByte(OFFSET & 0xff);
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

static void _initCompiler(Compiler* compiler, const FunctionType TYPE) {
	compiler->enclosing = current;

	compiler->function = NULL;
	compiler->type = TYPE;

	compiler->locals = malloc(sizeof(Local) * 16);
	compiler->localCount = 0;
	compiler->localSize = 16;

	compiler->upvalues = malloc(sizeof(Upvalue) * 16);
	compiler->upvalueSize = 16;

	compiler->scope = 0;

	compiler->function = objMakeFunction();

	current = compiler;
	if( TYPE != TYPE_SCRIPT ) {
		current->function->name =
			objCopyString(parser.previous.START, parser.previous.length);
	}

	/* Dedicamos o primeiro slot do array de variáveis locais
	 * para uso pessoal do compilador
	 */
	Local* local = &current->locals[current->localCount++];
	local->depth = 0;
	local->isCaptured = false;
	local->name.START = "";
	local->name.length = 0;
	local->name.type = TOKEN_NIL;
}

static void _beginScope(void) {
	++current->scope;
}

static void _endScope(void) {
	--current->scope;

	while( current->localCount > 0 &&
		   current->locals[current->localCount - 1].depth > current->scope ) {
		if( current->locals[current->localCount - 1].isCaptured ) {
			_emitByte(OP_CLOSE_UPVALUE);
		} else {
			_emitPop();
		}
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

static void _synchronize(void) {
	parser.panicked = false;

	while( parser.current.type != TOKEN_EOF ) {
		if( parser.previous.type == TOKEN_SEMICOLON ) {
			return;
		}

		switch( parser.current.type ) {
			case TOKEN_CLASS:
			case TOKEN_FUNC:
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

static void _markInitialized() {
	if( current->scope == 0 ) {
		return;
	}

	current->locals[current->localCount - 1].depth = current->scope;
}

static void _addLocal(Token name) {
	if( current->localSize < current->localCount + 1 ) {
		const size_t OLD_SIZE = current->localSize;
		current->localSize = MEM_GROW_SIZE(OLD_SIZE);

		current->locals = MEM_GROW_ARRAY(Local, current->locals, OLD_SIZE,
										 current->localSize);
	}

	Local* local = &current->locals[current->localCount++];
	local->name = name;
	local->depth = -1;
	local->isCaptured = false;
}

static ssize_t _resolveLocal(Compiler* compiler, Token* name) {
	for( ssize_t i = compiler->localCount - 1; i >= 0; --i ) {
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

static ssize_t _addUpvalue(Compiler* compiler, ssize_t index, bool isLocal) {
	size_t upvalueCount = compiler->function->upvalueCount;

	for( size_t i = 0; i < upvalueCount; ++i ) {
		Upvalue* upvalue = &compiler->upvalues[i];
		if( upvalue->index == index && upvalue->isLocal == isLocal ) {
			return i;
		}
	}

	if( compiler->upvalueSize < upvalueCount + 1 ) {
		const size_t OLD_SIZE = compiler->upvalueSize;
		compiler->upvalueSize = MEM_GROW_SIZE(OLD_SIZE);

		compiler->upvalues = MEM_GROW_ARRAY(Upvalue, compiler->upvalues,
											OLD_SIZE, compiler->upvalueSize);
		return 0;
	}

	compiler->upvalues[upvalueCount].isLocal = isLocal;
	compiler->upvalues[upvalueCount].index = index;
	return compiler->function->upvalueCount++;
}

static ssize_t _resolveUpvalue(Compiler* compiler, Token* name) {
	if( compiler->enclosing == NULL ) {
		return -1;
	}

	ssize_t local = _resolveLocal(compiler->enclosing, name);
	if( local != -1 ) {
		compiler->enclosing->locals[local].isCaptured = true;
		return _addUpvalue(compiler, local, true);
	}

	ssize_t upvalue = _resolveUpvalue(compiler->enclosing, name);
	if( upvalue != -1 ) {
		return _addUpvalue(compiler, upvalue, false);
	}

	return -1;
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

	_decreaseStackMax();
	_emitConstantWithOp(OP_DEF_GLOBAL_16, OP_DEF_GLOBAL_32, GLOBAL);
}

static void _defineConst(const uint32_t GLOBAL) {
	if( current->scope > 0 ) {
		_markInitialized();
		return;
	}

	_decreaseStackMax();
	_emitConstantWithOp(OP_DEF_CONST_16, OP_DEF_CONST_32, GLOBAL);
}

static void _and(const bool CAN_ASSIGN) {
	INTENTIONALLY_UNUSED(CAN_ASSIGN);

	const int32_t END_JUMP = _emitJump(OP_JUMP_IF_FALSE);

	_emitPop();
	_precedence(PREC_AND);

	_patchJump(END_JUMP);
}

static void _or(const bool CAN_ASSIGN) {
	INTENTIONALLY_UNUSED(CAN_ASSIGN);

	const int32_t ELSE_JUMP = _emitJump(OP_JUMP_IF_FALSE);
	const int32_t END_JUMP = _emitJump(OP_JUMP);

	_patchJump(ELSE_JUMP);
	_emitPop();

	_precedence(PREC_OR);
	_patchJump(END_JUMP);
}

static void _function(const FunctionType TYPE) {
	Compiler compiler;
	_initCompiler(&compiler, TYPE);
	_beginScope();

	_consume(TOKEN_LPAREN, "Esperava '(' depois do nome da funcao");
	if( !_check(TOKEN_RPAREN) ) {
		do {
			if( (++current->function->arity) == 0 ) {
				_errorAtCurr(
					"Nao e possivel ter uma funcao com >255 parametros.");
			}

			const size_t CONST = _parseVariable("Esperava parametro");
			_defineVariable(CONST);
		} while( _match(TOKEN_COMMA) );
	}

	_consume(TOKEN_RPAREN, "Esperava ')' depois dos parametros ");
	_consume(TOKEN_LBRACE, "Esperava '{' antes do corpo da funcao");
	_block();

	ObjFunction* function = _end();
	_emitConstantWithOp(OP_CLOSURE_16, OP_CLOSURE_32,
						_makeConstant(CREATE_OBJECT(function)));

	for( size_t i = 0; i < function->upvalueCount; ++i ) {
		Upvalue upvalue = compiler.upvalues[i];

		_emitByte(upvalue.isLocal ? 1 : 0);
		_emitByte((uint8_t)(upvalue.index & 0xFF));
		_emitByte((uint8_t)((upvalue.index >> 8) & 0xFF));
		_emitByte((uint8_t)((upvalue.index >> 16) & 0xFF));
	}
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

static void _funcDeclaration() {
	size_t global = _parseVariable("Esperava o nome da função");
	_markInitialized();

	_function(TYPE_FUNCTION);

	_defineVariable(global);
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

static void _switchStatement(void) {
	_consume(TOKEN_LPAREN, "Esperava '(' depois do 'escolha'.");
	_expression();
	_consume(TOKEN_RPAREN, "Esperava ')' depois do valor.");

	_consume(TOKEN_LBRACE, "Esperava '{' depois da condicao.");

	bool hasAnyCase = false;	 /* Se você já adicionou um caso */
	bool hasDefaultCase = false; /* Se você já adicionou o caso padrão */

	int16_t cases[MAX_CASES];  /* Onde os casos acabam */
	int16_t caseCount = 0;	   /* Quantidade de casos */
	int32_t skipPrevCase = -1; /* Posição onde acabou o último caso */

	while( !_match(TOKEN_RBRACE) && !_check(TOKEN_EOF) ) {
		if( _match(TOKEN_CASE) || _match(TOKEN_DEFAULT) ) {
			if( caseCount == MAX_CASES ) {
				_errorAtPrev("So e possivel ter 256 casos em um escolha-caso");
			}

			const TokenType TYPE = parser.previous.type;

			if( hasDefaultCase ) {
				_errorAtPrev(
					"Nao e possivel ter outro caso apos o caso padrao");
			} else if( hasAnyCase ) {
				cases[caseCount++] = _emitJump(OP_JUMP);

				_patchJump(skipPrevCase);
				_emitPop();
			}

			if( TYPE == TOKEN_CASE ) {
				hasAnyCase = true;

				/* Duplicamos o valor... */
				_emitByte(OP_DUP);

				/* ...consumimos o caso ... */
				_expression();
				_consume(TOKEN_COLON, "Esperava ':' depois do caso");

				/* ...e comparamos com o valor
				 * Se for falso, pulamos pro próximo
				 */
				_emitByte(OP_EQUAL);
				skipPrevCase = _emitJump(OP_JUMP_IF_FALSE);

				_emitPop();
			} else { /* Caso padrão */
				hasDefaultCase = true;

				_consume(TOKEN_COLON, "Esperava ':' depois do caso padrao");
				skipPrevCase = -1;
			}
		} else {
			if( !hasAnyCase ) {
				/* Nenhum caso, logo damos erro! */
				_errorAtPrev("Esperava um caso");
			}

			/* Estamos dentro de um caso,
			 * então processamos a declaração
			 */
			_statement();
		}
	}

	if( !hasDefaultCase ) {
		/* Sem caso padrão */
		_patchJump(skipPrevCase);
		_emitPop();
	}

	for( int8_t curCase = 0; curCase < caseCount; ++curCase ) {
		_patchJump(cases[curCase]);
	}

	_emitPop();
}

static void _returnStatement() {
	if( current->type == TYPE_SCRIPT ) {
		_errorAtPrev("'retorne' so pode ser usado dentro de uma funcao");
	}

	if( _match(TOKEN_SEMICOLON) ) {
		_emitReturn();
	} else {
		_expression();
		_consume(TOKEN_SEMICOLON, "Esperava ';' depois do valor de retorno");
		_increaseStackMax();
		_emitByte(OP_RETURN);
	}
}

static void _fixUpBreaks(void) {
	size_t offset = (size_t)innerLoopStart;
	Chunk* chunk = _chunk();

	while( offset < chunk->count ) {
		if( chunk->code[offset] == OP_BREAK ) {
			chunk->code[offset] = OP_JUMP;
			_patchJump(offset + 1);
		} else {
			++offset;
		}
	}
}

static void _whileStatement(void) {
	/* Variáveis necessárias pro 'continue' e 'saia' */
	int16_t topLoopStart = innerLoopStart;
	int16_t topLoopScope = innerLoopScope;
	innerLoopStart = _chunk()->count;
	innerLoopScope = current->scope;

	_consume(TOKEN_LPAREN, "Esperava '(' depois do 'while'");
	_expression();
	_consume(TOKEN_RPAREN, "Esperava ')' depois da condicao");

	int32_t loopEnd = _emitJump(OP_JUMP_IF_FALSE);

	_emitPop();
	_statement();
	_emitLoop(innerLoopStart);

	_patchJump(loopEnd);
	_emitPop();

	_fixUpBreaks();

	innerLoopStart = topLoopStart;
	innerLoopScope = topLoopScope;
}

static void _forStatement(void) {
	_beginScope();

	_consume(TOKEN_LPAREN, "Esperava '(' depois do 'for'.");
	if( _match(TOKEN_LET) ) {
		_varDeclaration();
	} else if( _match(TOKEN_SEMICOLON) ) {
		/* Sem inicializador */
	} else {
		_expressionStatement();
	}

	/* Variáveis necessárias pro 'continue' e 'saia' */
	int16_t topLoopStart = innerLoopStart;
	int16_t topLoopScope = innerLoopScope;
	innerLoopStart = _chunk()->count;
	innerLoopScope = current->scope;

	int32_t loopEnd = -1;
	if( !_match(TOKEN_SEMICOLON) ) {
		_expression();
		_consume(TOKEN_SEMICOLON, "Esperava ';' depois da condicao");

		loopEnd = _emitJump(OP_JUMP_IF_FALSE);
		_emitByte(OP_POP); /* Condicão */
	}

	if( !_match(TOKEN_RPAREN) ) {
		const int32_t BODY_JUMP = _emitJump(OP_JUMP);
		const int32_t INCREMENT = _chunk()->count;

		_expression();
		_emitByte(OP_POP);
		_consume(TOKEN_RPAREN, "Esperava ')' depois das clausulas.");

		_emitLoop(innerLoopStart);
		innerLoopStart = INCREMENT;
		_patchJump(BODY_JUMP);
	}

	_statement();
	_emitLoop(innerLoopStart);

	if( loopEnd != -1 ) {
		_patchJump(loopEnd);
		_emitByte(OP_POP); /* Condicao */
		_fixUpBreaks();
	}

	innerLoopStart = topLoopStart;
	innerLoopScope = topLoopScope;

	_endScope();
}

static void _discardLocals(void) {
	for( int16_t i = current->localCount - 1;
		 i >= 0 && current->locals[i].depth > innerLoopScope; --i ) {
		_emitPop();
	}
}

static void _breakStatement(void) {
	if( innerLoopStart == -1 ) {
		_errorAtPrev("Nao e possivel usar o 'saia' fora de um loop");
	}

	_consume(TOKEN_SEMICOLON, "Esperava ';' depois do 'saia'");

	_discardLocals();

	_emitJump(OP_BREAK);
}

static void _continueStatement(void) {
	if( innerLoopStart == -1 ) {
		_errorAtPrev("Nao e possivel usar o 'continue' fora de um loop");
	}

	_consume(TOKEN_SEMICOLON, "Esperava ';' depois do 'continue'");

	_discardLocals();

	/* Retornamos pro início do loop */
	_emitLoop(innerLoopStart);
}

/**
 * @brief Compila uma declaração
 */
static void _statement(void) {
	if( _match(TOKEN_PRINT) ) {
		_printStatement();
	} else if( _match(TOKEN_IF) ) {
		_ifStatement();
	} else if( _match(TOKEN_SWITCH) ) {
		_switchStatement();
	} else if( _match(TOKEN_RETURN) ) {
		_returnStatement();
	} else if( _match(TOKEN_WHILE) ) {
		_whileStatement();
	} else if( _match(TOKEN_FOR) ) {
		_forStatement();
	} else if( _match(TOKEN_BREAK) ) {
		_breakStatement();
	} else if( _match(TOKEN_CONTINUE) ) {
		_continueStatement();
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
	if( _match(TOKEN_FUNC) ) {
		_funcDeclaration();
	} else if( _match(TOKEN_LET) ) {
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
	INTENTIONALLY_UNUSED(CAN_ASSIGN);

	_expression(); /* Chamada recursiva da função _expression()... */
	_consume(TOKEN_RPAREN, "Esperava um ')' depois da expressao");
}

/**
 * @brief Compila um número
 */
static void _number(const bool CAN_ASSIGN) {
	INTENTIONALLY_UNUSED(CAN_ASSIGN);

	const LOXIE_NUMBER NUMBER =
		(LOXIE_NUMBER)strtod(parser.previous.START, NULL);
	_emitConstant(CREATE_NUMBER(NUMBER));
}

/**
 * @brief Analisa uma string
 *
 * @param[out] size Tamanho da string construída
 * @return String construída
 */
static char* _parseString(size_t* size) {
	const size_t MAX_SIZE = parser.previous.length - 2;

	char* newString = MEM_ALLOC(char, MAX_SIZE);
	ssize_t j = -1;

	for( size_t i = 0; i < MAX_SIZE; ++i ) {
		++j;
		const char* CUR_CHAR = (parser.previous.START + 1) + i;

		if( *CUR_CHAR == '\0' ) {
			newString[j] = '\0';
			break;
		}

		if( *CUR_CHAR == '\\' ) {
			switch( *(CUR_CHAR + 1) ) {
				case 'r':
					newString[j] = '\r';
					++i;
					break;
				case 'n':
					newString[j] = '\n';
					++i;
					break;
				case 't':
					newString[j] = '\t';
					++i;
					break;
				case '"':
					newString[j] = '"';
					++i;
					break;
				case '\\':
					newString[j] = '\\';
					++i;
					break;
				default:
					_errorAtPrev("Escape sequence invalida");
					return newString;
			}
		} else {
			newString[j] = *CUR_CHAR;
		}
	}

	*size = j + 1;
	return newString;
}

/**
 * @brief Compila uma string
 */
static void _string(const bool CAN_ASSIGN) {
	INTENTIONALLY_UNUSED(CAN_ASSIGN);

	size_t strSize = 0;
	char* string = _parseString(&strSize);

	_emitConstant(CREATE_OBJECT(objCopyString(string, strSize)));
}

static void _namedVariable(Token name, const bool CAN_ASSIGN) {
	uint8_t getOp, setOp;
	int16_t arg = _resolveLocal(current, &name);

	if( arg != -1 ) {
		getOp = OP_GET_LOCAL_16;
		setOp = OP_SET_LOCAL_16;
	} else if( (arg = _resolveUpvalue(current, &name)) != -1 ) {
		getOp = OP_GET_UPVALUE_16;
		setOp = OP_SET_UPVALUE_16;
	} else {
		arg = _identifierConstant(&name);
		getOp = OP_GET_GLOBAL_16;
		setOp = OP_SET_GLOBAL_16;
	}

	if( CAN_ASSIGN && _match(TOKEN_EQUAL) ) {
		_expression();
		_emitConstantWithOp(setOp, setOp + 1, arg);
	} else {
		_increaseStackMax();
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
	INTENTIONALLY_UNUSED(CAN_ASSIGN);

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
	INTENTIONALLY_UNUSED(CAN_ASSIGN);

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
	INTENTIONALLY_UNUSED(CAN_ASSIGN);

	const TokenType OP_TYPE = parser.previous.type;
	ParseRule* rule = _getRule(OP_TYPE);
	_precedence((Precedence)(rule->precedence + 1));

	_decreaseStackMax();

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

static uint8_t _argumentList(void) {
	uint8_t argCount = 0;

	if( !_check(TOKEN_RPAREN) ) {
		do {
			_expression();
			++argCount;
			if( argCount == 0 ) {
				_errorAtPrev(
					"Nao e possivel ter uma funcao com >255 parametros.");
			}
		} while( _match(TOKEN_COMMA) );
	}

	_consume(TOKEN_RPAREN, "Esperava ')' depois dos parametros.");
	return argCount;
}

static void _call(const bool CAN_ASSIGN) {
	INTENTIONALLY_UNUSED(CAN_ASSIGN);

	_doubleStackMax();

	const uint8_t ARG_COUNT = _argumentList();
	_emitBytes(OP_CALL, ARG_COUNT);
}

static void _conditional(const bool CAN_ASSIGN) {
	INTENTIONALLY_UNUSED(CAN_ASSIGN);

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
	[TOKEN_LPAREN] = {_grouping, _call, PREC_CALL},
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
	[TOKEN_EQUAL] = {NULL, _binary, PREC_NONE},
	[TOKEN_EQUAL_EQUAL] = {NULL, _binary, PREC_EQUALITY},
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

	[TOKEN_FUNC] = {NULL, NULL, PREC_NONE},
	[TOKEN_RETURN] = {NULL, NULL, PREC_NONE},

	[TOKEN_IF] = {NULL, NULL, PREC_NONE},
	[TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
	[TOKEN_QUESTION] = {NULL, _conditional, PREC_CONDITIONAL},
	[TOKEN_COLON] = {NULL, NULL, PREC_NONE},

	[TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
	[TOKEN_LET] = {NULL, NULL, PREC_NONE},
	[TOKEN_CONST] = {NULL, NULL, PREC_NONE},

	[TOKEN_ERROR] = {NULL, NULL, PREC_NONE},
	[TOKEN_EOF] = {NULL, NULL, PREC_NONE},
};

static ParseRule* _getRule(const TokenType TYPE) {
	return &rules[TYPE];
}

ObjFunction* compCompile(const char* SOURCE) {
	scannerInit(SOURCE);

	Compiler compiler;
	_initCompiler(&compiler, TYPE_SCRIPT);

	parser.hadError = parser.panicked = false;

	stackMax = 1; /* Local reservada */

	_advance();
	while( !_match(TOKEN_EOF) ) {
		_declaration();
	}

	ObjFunction* func = _end();
	if( parser.hadError ) {
		return NULL;
	}

	return func;
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
