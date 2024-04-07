/**
 * @file vm.c
 * @author Pedro B.
 * @date 2024.04.02
 *
 * @brief Implementa a máquina virtual (VM) que interpretará o nosso código
 */

#include "vm.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "compiler.h"
#include "debug.h"
#include "error.h"
#include "memory.h"
#include "object.h"
#include "opcodes.h"
#include "value.h"

/**
 * @brief Lê um byte e avança o ponteiro
 */
#define READ_8() (*vm.pc++)

/**
 * @brief Lê dois bytes e avança o ponteiro
 */
#define READ_16() (vm.pc += 2, (uint16_t)(vm.pc[-2] << 8) | vm.pc[-1])

/**
 * @brief Lê três bytes e avança o ponteiro
 */
#define READ_24() \
	(vm.pc += 3, (uint32_t)(vm.pc[-3] | (vm.pc[-2] << 8) | (vm.pc[-1] << 16)))

/**
 * @brief Lê uma constante
 */
#define READ_CONST_16() (vm.chunk->consts.values[READ_8()])

/**
 * @brief Lê uma constante longa
 */
#define READ_CONST_32() (vm.chunk->consts.values[READ_24()])

/**
 * @brief Lê uma global
 */
#define READ_GLOBAL_16() (vm.globalValues.values[READ_8()])

/**
 * @brief Lê uma global longa
 */
#define READ_GLOBAL_32() (vm.globalValues.values[READ_24()])

/**
 * @brief Interpreta a próxima constante 8-bit como uma string
 */
#define READ_STRING_16() (AS_STRING(READ_CONST_16()))

/**
 * @brief Interpreta a próxima constante 24-bit como uma string
 */
#define READ_STRING_32() (AS_STRING(READ_CONST_32()))

/**
 * @brief Realiza um operação binária com os dois itens no topo da pilha
 *
 * @param TYPE Tipo de valor usado na operação
 * @param OPERATOR Operação (uma de +, -, *, /) que será realizada
 */
#define BINARY_OP(TYPE, OPERATOR)                                  \
	do {                                                           \
		if( !IS_NUMBER(_peek(0)) || !IS_NUMBER(_peek(1)) ) {       \
			RUNTIME_ERROR("Ambos os operandos devem ser numeros"); \
			return RESULT_RUNTIME_ERROR;                           \
		}                                                          \
		double b = AS_NUMBER(vmPop());                             \
		double a = AS_NUMBER(vmPop());                             \
		vmPush(TYPE(a OPERATOR b));                                \
	} while( false )

/**
 * @brief Levanta um erro durante a interpretação
 */
#define RUNTIME_ERROR(FMT, ...)                    \
	do {                                           \
		errFatal(vmGetLine(), FMT, ##__VA_ARGS__); \
		_resetStack();                             \
	} while( false )

VM vm = {0}; /**< Instância global da máquina virtual */

/**
 * @brief Esvazia a pilha
 */

static void _resetStack(void) {
	vm.stackTop = vm.stack;
}

/**
 * @brief Vê um valor @a DIST valores a frente na pilha
 *
 * @param[in] DIST Distância a frente que deve olhar
 * @return Valor no local indicado da pilha
 */
static Value _peek(const size_t DIST) {
	return vm.stackTop[-1 - DIST];
}

static bool _isFalsey(Value value) {
	return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void _concatenate(void) {
	ObjString *strB = AS_STRING(vmPop());
	ObjString *strA = AS_STRING(vmPop());

	const size_t LENGTH = strA->length + strB->length;

	ObjString *result = objMakeString(LENGTH);

	memcpy(result->str, strA->str, strA->length);
	memcpy(result->str + strA->length, strB->str, strB->length);

	result->str[LENGTH] = '\0';
	result->hash = hashString(result->str, LENGTH);

	tableSet(&vm.strings, CREATE_OBJECT(result), CREATE_NIL());

	vmPush(CREATE_OBJECT(result));
}

void vmInitStack(void) {
	vm.stack = MEM_GROW_ARRAY(Value, vm.stack, (size_t)(vm.stackTop - vm.stack),
							  vm.stackMax);

	_resetStack();
}

void vmInit(void) {
	vm.objects = NULL;
	vm.stackMax = 0;

	tableInit(&vm.globalNames);
	valueArrayInit(&vm.globalValues);

	tableInit(&vm.strings);

	_resetStack();
}

void vmFree(void) {
	tableFree(&vm.globalNames);
	valueArrayFree(&vm.globalValues);

	tableFree(&vm.strings);
	memFreeObjects();
}

void vmPush(Value value) {
	*vm.stackTop = value;
	++vm.stackTop;
}

Value vmPop(void) {
	--vm.stackTop;
	return *vm.stackTop;
}

size_t vmGetLine(void) {
	return chunkGetLine(vm.chunk, vm.pc - vm.chunk->code - 1);
}

#ifdef DEBUG_TRACE_EXECUTION
/**
 * @brief Imprime o estado atual da pilha
 */
static void _printStack(void) {
	printf("(%u/%u)", vm.stackTop - vm.stack, vm.stackMax);
	for( Value *slot = vm.stack; slot < vm.stackTop; ++slot ) {
		printf("[");
		valuePrint(*slot);
		printf("]");
	}
	printf("\n");
}
#endif

static Result _run(void) {
	while( true ) {
#ifdef DEBUG_TRACE_EXECUTION
		_printStack();
		debugDisassembleInstruction(vm.chunk, (size_t)(vm.pc - vm.chunk->code));
#endif
		const uint8_t OP = READ_8();
		switch( OP ) {
			case OP_CONST_16: {
				Value constant = READ_CONST_16();
				vmPush(constant);
			} break;

			case OP_CONST_32: {
				Value constant = READ_CONST_32();
				vmPush(constant);
			} break;

			case OP_TRUE:
				vmPush(CREATE_BOOL(true));
				break;

			case OP_FALSE:
				vmPush(CREATE_BOOL(false));
				break;

			case OP_NIL:
				vmPush(CREATE_NIL());
				break;

			case OP_POP:
				vmPop();
				break;

			case OP_DEF_GLOBAL_16:
				READ_GLOBAL_16() = vmPop();
				break;

			case OP_DEF_GLOBAL_32:
				READ_GLOBAL_32() = vmPop();
				break;

			case OP_DEF_CONST_16: {
				const uint8_t INDEX = READ_8();
				vm.globalValues.values[INDEX] = vmPop();
				vm.globalValues.values[INDEX].props |= 0x08;
			} break;

			case OP_DEF_CONST_32: {
				const size_t INDEX = READ_24();
				vm.globalValues.values[INDEX] = vmPop();
				vm.globalValues.values[INDEX].props |= 0x08;
			} break;

			case OP_GET_GLOBAL_16: {
				Value value = READ_GLOBAL_16();
				if( IS_EMPTY(value) ) {
					RUNTIME_ERROR("Variavel indefinida '%s'.",
								  AS_STRING(value)->str);
					return RESULT_RUNTIME_ERROR;
				}

				vmPush(value);
				break;
			}

			case OP_GET_LOCAL_16: {
				uint8_t slot = READ_8();
				vmPush(vm.stack[slot]);
			} break;

			case OP_GET_LOCAL_32: {
				uint8_t slot = READ_24();
				vmPush(vm.stack[slot]);
			} break;

			case OP_GET_GLOBAL_32: {
				Value value = READ_GLOBAL_32();
				if( IS_EMPTY(value) ) {
					RUNTIME_ERROR("Variavel indefinida '%s'.",
								  AS_STRING(value)->str);
					return RESULT_RUNTIME_ERROR;
				}

				vmPush(value);
				break;
			}

			case OP_SET_GLOBAL_16: {
				uint8_t index = READ_8();
				Value value = vm.globalValues.values[index];
				if( IS_EMPTY(value) ) {
					RUNTIME_ERROR("Variavel indefinida");
					return RESULT_RUNTIME_ERROR;
				} else if( IS_CONSTANT(value) ) {
					RUNTIME_ERROR("Tentou redefinir um valor constante");
					return RESULT_RUNTIME_ERROR;
				}

				vm.globalValues.values[index] = _peek(0);
			} break;

			case OP_SET_GLOBAL_32: {
				uint32_t index = READ_24();
				Value value = vm.globalValues.values[index];
				if( IS_EMPTY(value) ) {
					RUNTIME_ERROR("Variavel indefinida");
					return RESULT_RUNTIME_ERROR;
				} else if( IS_CONSTANT(value) ) {
					RUNTIME_ERROR("Tentou redefinir um valor constante");
					return RESULT_RUNTIME_ERROR;
				}

				vm.globalValues.values[index] = _peek(0);
			} break;

			case OP_SET_LOCAL_16: {
				uint8_t slot = READ_8();
				vm.stack[slot] = _peek(0);
			} break;

			case OP_SET_LOCAL_32: {
				uint8_t slot = READ_24();
				vm.stack[slot] = _peek(0);
			} break;

			case OP_EQUAL: {
				Value a = vmPop();
				Value b = vmPop();
				vmPush(CREATE_BOOL(valueEquals(a, b)));
			} break;

			case OP_GREATER:
				BINARY_OP(CREATE_BOOL, >);
				break;

			case OP_GREATER_EQUAL:
				BINARY_OP(CREATE_BOOL, >=);
				break;

			case OP_LESS:
				BINARY_OP(CREATE_BOOL, <);
				break;

			case OP_LESS_EQUAL:
				BINARY_OP(CREATE_BOOL, <=);
				break;

			case OP_ADD: {
				if( IS_STRING(_peek(0)) && IS_STRING(_peek(1)) ) {
					_concatenate();
				} else if( IS_NUMBER(_peek(0)) && IS_NUMBER(_peek(1)) ) {
					const double B = AS_NUMBER(vmPop());
					const double A = AS_NUMBER(vmPop());

					vmPush(CREATE_NUMBER(A + B));
				} else {
					RUNTIME_ERROR(
						"Operandos devem ser dois numeros ou duas strings");
					return RESULT_RUNTIME_ERROR;
				}
			} break;

			case OP_SUB:
				BINARY_OP(CREATE_NUMBER, -);
				break;

			case OP_MUL:
				BINARY_OP(CREATE_NUMBER, *);
				break;

			case OP_DIV:
				BINARY_OP(CREATE_NUMBER, /);
				break;

			case OP_MOD: {
				if( !IS_NUMBER(_peek(0)) || !IS_NUMBER(_peek(1)) ) {
					RUNTIME_ERROR("Ambos os operandos devem ser numeros");
					return RESULT_RUNTIME_ERROR;
				}

				double b = AS_NUMBER(vmPop());
				double a = AS_NUMBER(vmPop());
				vmPush(CREATE_NUMBER(fmod(a, b)));
			} break;

			case OP_NEGATE: {
				if( !IS_NUMBER(_peek(0)) ) {
					RUNTIME_ERROR("Impossivel negar algo que nao e um numero");
					return RESULT_RUNTIME_ERROR;
				}

				(vm.stackTop - 1)->vNumber = -(vm.stackTop - 1)->vNumber;
			} break;

			case OP_NOT:
				vmPush(CREATE_BOOL(_isFalsey(vmPop())));
				break;

			case OP_PRINT:
				valuePrint(vmPop());
				printf("\n");
				return RESULT_OK;

			case OP_JUMP: {
				const uint16_t OFFSET = READ_16();
				vm.pc += OFFSET;
			} break;

			case OP_JUMP_IF_FALSE: {
				const uint16_t OFFSET = READ_16();
				if( _isFalsey(_peek(0)) ) {
					vm.pc += OFFSET;
				}
			} break;

			case OP_RETURN:
				return RESULT_OK;

			default:
				errWarn(chunkGetLine(vm.chunk, *(vm.pc - 1)),
						"OPCODE desconhecido encontrado! -> ");
				printf("%02x\n", OP);
		}
	}
}

Result vmInterpret(const char *SOURCE) {
	Chunk chunk;
	chunkInit(&chunk);

	if( !compCompile(SOURCE, &chunk) ) {
		chunkFree(&chunk);
		return RESULT_COMPILER_ERROR;
	}

	vm.chunk = &chunk;
	vm.pc = vm.chunk->code;

	const Result RESULT = _run();

	chunkFree(&chunk);
	return RESULT;
}

#undef READ_8
#undef READ_16
#undef READ_24

#undef READ_CONST_16
#undef READ_CONST_32

#undef READ_STRING_16
#undef READ_STRING_32

#undef BINARY_OP
