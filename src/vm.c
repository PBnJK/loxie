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
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "debug.h"
#include "error.h"
#include "memory.h"
#include "native.h"
#include "object.h"
#include "opcodes.h"
#include "value.h"

/**
 * @brief Lê um byte e avança o ponteiro
 */
#define READ_8() (*fp++)

/**
 * @brief Lê dois bytes e avança o ponteiro
 */
#define READ_16() (fp += 2, (uint16_t)(fp[-2] << 8) | fp[-1])

/**
 * @brief Lê três bytes e avança o ponteiro
 */
#define READ_24() (fp += 3, (uint32_t)(fp[-3] | (fp[-2] << 8) | (fp[-1] << 16)))

/**
 * @brief Lê uma constante
 */
#define READ_CONST_16() \
	(frame->closure->function->chunk.consts.values[READ_8()])

/**
 * @brief Lê uma constante longa
 */
#define READ_CONST_32() \
	(frame->closure->function->chunk.consts.values[READ_24()])

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
#define RUNTIME_ERROR(FMT, ...)                     \
	do {                                            \
		frame->fp = fp;                             \
		errFatal(vmGetLine(0), FMT, ##__VA_ARGS__); \
		_runtimeError();                            \
	} while( false )

/**
 * @brief Levanta um erro durante a interpretação (sem salvar o frame)
 */
#define RUNTIME_ERROR_F(FMT, ...)                   \
	do {                                            \
		errFatal(vmGetLine(0), FMT, ##__VA_ARGS__); \
		_runtimeError();                            \
	} while( false )

VM vm = {0}; /**< Instância global da máquina virtual */

/**
 * @brief Esvazia a pilha
 */
static void _resetStack(void) {
	vm.stackTop = vm.stack;
	vm.frameCount = 0;
	vm.openUpvalues = NULL;
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
	vm.isLocked = true;

	ObjString *strB = AS_STRING(vmPop());
	ObjString *strA = AS_STRING(vmPop());

	const size_t LENGTH = strA->length + strB->length;

	ObjString *result = objMakeString(LENGTH);

	memcpy(result->str, strA->str, strA->length);
	memcpy(result->str + strA->length, strB->str, strB->length);

	result->str[LENGTH] = '\0';
	result->hash = hashString(result->str, LENGTH);

	tableSet(&vm.strings, CREATE_OBJECT(result), CREATE_NIL());

	vm.isLocked = false;

	vmPush(CREATE_OBJECT(result));
}

static void _runtimeError(void) {
	if( vm.stackTop > &vm.stack[vm.stackMax] ) {
		fprintf(stderr, COLOR_RED "\nSTACK OVERFLOW!" COLOR_RESET
								  "Variaveis de mais. Funcao recursiva?");
		_resetStack();
		return;
	}

	if( vm.frameCount - 2 <= 0 ) {
		_resetStack();
		return;
	}

	fprintf(stderr, COLOR_YELLOW "\nStack trace " COLOR_RESET
								 "(ultima chamada primeiro):");

	for( int16_t i = vm.frameCount - 2; i >= 0; --i ) {
		CallFrame *frame = &vm.frames[i];
		ObjFunction *function = frame->closure->function;
		const size_t INSTR = frame->fp - function->chunk.code - 1;
		const size_t LINE = chunkGetLine(&function->chunk, INSTR);

		fprintf(stderr, "\n  L%-4d: ", LINE);

		if( function->name == NULL ) {
			fprintf(stderr, "no script");
		} else {
			fprintf(stderr, "na funcao %s();", function->name->str);
		}
	}

	_resetStack();
}

static bool _call(ObjClosure *closure, const uint8_t ARG_COUNT) {
	CallFrame *frame = &vm.frames[vm.frameCount++];

	if( ARG_COUNT != closure->function->arity ) {
		RUNTIME_ERROR_F("Esperava %d argumentos mas recebeu %d",
						closure->function->arity, ARG_COUNT);
		return false;
	}

	if( vm.frameCount == FRAMES_MAX ) {
		RUNTIME_ERROR_F("CallFrames demais (funcao recursiva demais?)");
		return false;
	}

	frame->closure = closure;
	frame->fp = closure->function->chunk.code;
	frame->slots = vm.stackTop - ARG_COUNT - 1;

	return true;
}

static bool _callValue(Value callee, const uint8_t ARG_COUNT) {
	if( IS_OBJECT(callee) ) {
		switch( OBJECT_TYPE(callee) ) {
			case OBJ_CLOSURE:
				return _call(AS_CLOSURE(callee), ARG_COUNT);

			case OBJ_NATIVE: {
				ObjNative *n = AS_NATIVE(callee);
				if( n->argCount != ARG_COUNT && n->argCount != -1 ) {
					RUNTIME_ERROR_F("Esperava %d argumentos, mas recebeu %u",
									n->argCount, ARG_COUNT);
					return false;
				}

				return nativeCall(AS_NATIVE_FN(callee), ARG_COUNT);
			}

			case OBJ_CLASS: {
				ObjClass *klass = AS_CLASS(callee);
				vm.stackTop[-ARG_COUNT - 1] =
					CREATE_OBJECT(objMakeInstance(klass));

				if( !IS_NIL(klass->constructor) ) {
					return _call(AS_CLOSURE(klass->constructor), ARG_COUNT);
				} else if( ARG_COUNT != 0 ) {
					RUNTIME_ERROR_F(
						"O construtor 0 argumentos mas recebeu %d. Esqueceu de "
						"definir um construtor?",
						ARG_COUNT);
					return false;
				}

				return true;
			}

			case OBJ_BOUND_METHOD: {
				ObjBoundMethod *bound = AS_BOUND_METHOD(callee);
				vm.stackTop[-ARG_COUNT - 1] = bound->receiver;

				return _call(bound->method, ARG_COUNT);
			}

			default:
				break;	// Objeto inchamável
		}
	}

	RUNTIME_ERROR_F("So e possivel chamar funcoes e classes");

	return false;
}

static ObjUpvalue *_captureUpvalue(Value *local) {
	ObjUpvalue *prevUpvalue = NULL;
	ObjUpvalue *upvalue = vm.openUpvalues;

	while( upvalue != NULL && upvalue->location > local ) {
		prevUpvalue = upvalue;
		upvalue = upvalue->next;
	}

	if( upvalue != NULL && upvalue->location == local ) {
		return upvalue;
	}

	ObjUpvalue *createdUpvalue = objMakeUpvalue(local);
	createdUpvalue->next = upvalue;

	if( prevUpvalue == NULL ) {
		vm.openUpvalues = createdUpvalue;
	} else {
		prevUpvalue->next = createdUpvalue;
	}

	return createdUpvalue;
}

static void _closeUpvalues(Value *last) {
	while( vm.openUpvalues != NULL && vm.openUpvalues->location >= last ) {
		ObjUpvalue *upvalue = vm.openUpvalues;
		upvalue->closed = *upvalue->location;
		upvalue->location = &upvalue->closed;

		vm.openUpvalues = upvalue->next;
	}
}

static void _defineMethod(ObjString *name) {
	Value method = _peek(0);
	ObjClass *klass = AS_CLASS(_peek(1));
	tableSet(&klass->methods, CREATE_OBJECT(name), method);

	if( name == klass->name ) {
		klass->constructor = method;
	}

	vmPop();
}

static bool _bindMethod(ObjClass *klass, ObjString *name) {
	Value method;
	if( !tableGet(&klass->methods, CREATE_OBJECT(name), &method) ) {
		RUNTIME_ERROR_F("Propriedade indefinida '%s'.", name->str);
		return false;
	}

	ObjBoundMethod *bound = objMakeBoundMethod(_peek(0), AS_CLOSURE(method));
	vmPop();
	vmPush(CREATE_OBJECT(bound));
	return true;
}

static bool _invokeFromClass(ObjClass *klass, ObjString *name,
							 const uint8_t ARG_COUNT) {
	Value method;
	if( !tableGet(&klass->methods, CREATE_OBJECT(name), &method) ) {
		RUNTIME_ERROR_F("Propriedade indefinida '%s'.", name->str);
		return false;
	}

	return _call(AS_CLOSURE(method), ARG_COUNT);
}

static bool _invoke(ObjString *method, const uint8_t ARG_COUNT) {
	Value receiver = _peek(ARG_COUNT);
	if( !IS_INSTANCE(receiver) ) {
		RUNTIME_ERROR_F("So instancias possuem metodos");
		return false;
	}

	ObjInstance *instance = AS_INSTANCE(receiver);
	Value value;
	if( tableGet(&instance->fields, CREATE_OBJECT(method), &value) ) {
		vm.stackTop[-ARG_COUNT - 1] = value;
		return _callValue(value, ARG_COUNT);
	}

	return _invokeFromClass(instance->klass, method, ARG_COUNT);
}

static bool _getArrayValue(ValueArray *array, const int64_t INDEX) {
	if( INDEX < 0 ) {
		if( array->count + INDEX < 0 ) {
			RUNTIME_ERROR_F("Tentou acessar indice fora do array");
			return false;
		}

		vmPush(array->values[array->count + INDEX]);
	} else {
		if( INDEX > array->count - 1 ) {
			RUNTIME_ERROR_F("Tentou acessar indice fora do array");
			return false;
		}

		vmPush(array->values[INDEX]);
	}

	return true;
}

static bool _setArrayValue(ValueArray *array, const int64_t INDEX, Value new) {
	if( INDEX < 0 ) {
		if( array->count + INDEX < 0 ) {
			RUNTIME_ERROR_F("Tentou acessar indice fora do array");
			return false;
		}

		array->values[array->count + INDEX] = new;
	} else {
		if( INDEX > array->count - 1 ) {
			RUNTIME_ERROR_F("Tentou acessar indice fora do array");
			return false;
		}

		array->values[INDEX] = new;
	}

	return true;
}

static bool _getCharAt(ObjString *str, const int64_t INDEX) {
	if( INDEX < 0 ) {
		if( str->length + INDEX < 0 ) {
			RUNTIME_ERROR_F("Tentou acessar indice fora da string");
			return false;
		}

		vmPush(CREATE_OBJECT(objCopyString(&str->str[str->length + INDEX], 1)));
	} else {
		if( INDEX > str->length - 1 ) {
			RUNTIME_ERROR_F("Tentou acessar indice fora da string");
			return false;
		}

		vmPush(CREATE_OBJECT(objCopyString(&str->str[INDEX], 1)));
	}

	return true;
}

static void _vmTempInitStack(void) {
	vm.stack = memRealloc(vm.stack, 0, 32);
	_resetStack();
}

void vmInitStack(void) {
	vm.stack = MEM_GROW_ARRAY(Value, vm.stack, (size_t)(vm.stackTop - vm.stack),
							  vm.stackMax);

	_resetStack();
}

void vmInit(void) {
	vm.objects = NULL;
	vm.stackMax = 0;

	vm.grayCount = 0;
	vm.graySize = 0;
	vm.grayStack = NULL;

	vm.bytesAllocated = 0;
	vm.nextGC = 1024 * 1024;
	vm.isLocked = false;

	_vmTempInitStack();

	tableInit(&vm.globalNames);
	valueArrayInit(&vm.globalValues);

	tableInit(&vm.strings);
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

size_t vmGetLine(const uint8_t FRAME_IDX) {
	INTENTIONALLY_UNUSED(FRAME_IDX);

	CallFrame *frame = &vm.frames[vm.frameCount - 1];
	const size_t OFFSET = frame->fp - frame->closure->function->chunk.code - 1;

	return chunkGetLine(&frame->closure->function->chunk, OFFSET);
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
	CallFrame *frame = &vm.frames[vm.frameCount - 1];
	register uint8_t *fp = frame->fp;

	while( true ) {
#ifdef DEBUG_TRACE_EXECUTION
		_printStack();
		debugDisassembleInstruction(
			&frame->closure->function->chunk,
			(size_t)(fp - frame->closure->function->chunk.code));
#endif
		const OpCode OP = READ_8();
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
				SET_TO_CONSTANT(vm.globalValues.values[INDEX]);
			} break;

			case OP_DEF_CONST_32: {
				const size_t INDEX = READ_24();

				vm.globalValues.values[INDEX] = vmPop();
				SET_TO_CONSTANT(vm.globalValues.values[INDEX]);
			} break;

			case OP_GET_GLOBAL_16: {
				Value value = READ_GLOBAL_16();
				if( IS_EMPTY(value) ) {
					RUNTIME_ERROR("Variavel indefinida");
					return RESULT_RUNTIME_ERROR;
				}

				vmPush(value);
				break;
			}

			case OP_GET_GLOBAL_32: {
				Value value = READ_GLOBAL_32();
				if( IS_EMPTY(value) ) {
					RUNTIME_ERROR("Variavel indefinida");
					return RESULT_RUNTIME_ERROR;
				}

				vmPush(value);
				break;
			}

			case OP_GET_LOCAL_16:
				vmPush(frame->slots[READ_8()]);
				break;

			case OP_GET_LOCAL_32:
				vmPush(frame->slots[READ_24()]);
				break;

			case OP_GET_UPVALUE_16:
				vmPush(*frame->closure->upvalues[READ_8()]->location);
				break;

			case OP_GET_UPVALUE_32:
				vmPush(*frame->closure->upvalues[READ_24()]->location);
				break;

			case OP_SET_GLOBAL_16: {
				const uint8_t index = READ_8();
				Value value = vm.globalValues.values[index];

				if( IS_EMPTY(value) ) {
					RUNTIME_ERROR("Variavel indefinida");
					return RESULT_RUNTIME_ERROR;
				}

				vm.globalValues.values[index] = _peek(0);
			} break;

			case OP_SET_GLOBAL_32: {
				const uint32_t INDEX = READ_24();
				Value value = vm.globalValues.values[INDEX];

				if( IS_EMPTY(value) ) {
					RUNTIME_ERROR("Variavel indefinida");
					return RESULT_RUNTIME_ERROR;
				}

				vm.globalValues.values[INDEX] = _peek(0);
			} break;

			case OP_SET_LOCAL_16:
				frame->slots[READ_8()] = _peek(0);
				break;

			case OP_SET_LOCAL_32:
				frame->slots[READ_24()] = _peek(0);
				break;

			case OP_SET_UPVALUE_16:
				*frame->closure->upvalues[READ_8()]->location = _peek(0);
				break;

			case OP_SET_UPVALUE_32:
				*frame->closure->upvalues[READ_24()]->location = _peek(0);
				break;

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
						"Operandos devem ser dois numeros ou duas "
						"strings");
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

#ifdef NAN_BOXING
				vmPush(-vmPop());
#else
				(vm.stackTop - 1)->vNumber = -(vm.stackTop - 1)->vNumber;
#endif
			} break;

			case OP_NOT:
				vmPush(CREATE_BOOL(_isFalsey(vmPop())));
				break;

			case OP_PRINT:
				valuePrint(vmPop());
				printf("\n");
				break;

			case OP_JUMP: {
				const uint16_t OFFSET = READ_16();
				fp += OFFSET;
			} break;

			case OP_JUMP_IF_FALSE: {
				const uint16_t OFFSET = READ_16();
				if( _isFalsey(_peek(0)) ) {
					fp += OFFSET;
				}
			} break;

			case OP_LOOP: {
				const uint16_t OFFSET = READ_16();
				fp -= OFFSET;
			} break;

			case OP_DUP:
				vmPush(_peek(0));
				break;

			case OP_CALL: {
				const uint8_t ARG_COUNT = READ_8();

				if( vm.stackTop + ARG_COUNT > &vm.stack[vm.stackMax] ) {
					RUNTIME_ERROR("Overflow da pilha");
					return RESULT_RUNTIME_ERROR;
				}

				frame->fp = fp;

				if( !_callValue(_peek(ARG_COUNT), ARG_COUNT) ) {
					return RESULT_RUNTIME_ERROR;
				}

				frame = &vm.frames[vm.frameCount - 1];
				fp = frame->fp;
			} break;

			case OP_CLOSURE_16: {
				ObjFunction *function = AS_FUNCTION(READ_CONST_16());
				ObjClosure *closure = objMakeClosure(function);
				vmPush(CREATE_OBJECT(closure));

				for( size_t i = 0; i < closure->upvalueCount; ++i ) {
					uint8_t isLocal = READ_8();
					uint32_t index = READ_24();
					if( isLocal ) {
						closure->upvalues[i] =
							_captureUpvalue(frame->slots + index);
					} else {
						closure->upvalues[i] = frame->closure->upvalues[index];
					}
				}
			} break;

			case OP_CLOSURE_32: {
				ObjFunction *function = AS_FUNCTION(READ_CONST_32());
				ObjClosure *closure = objMakeClosure(function);
				vmPush(CREATE_OBJECT(closure));

				for( size_t i = 0; i < closure->upvalueCount; ++i ) {
					uint8_t isLocal = READ_8();
					uint32_t index = READ_24();
					if( isLocal ) {
						closure->upvalues[i] =
							_captureUpvalue(frame->slots + index);
					} else {
						closure->upvalues[i] = frame->closure->upvalues[index];
					}
				}
			} break;

			case OP_CLOSE_UPVALUE:
				_closeUpvalues(vm.stackTop - 1);
				vmPop();
				break;

			case OP_CLASS_16:
				vmPush(CREATE_OBJECT(objMakeClass(READ_STRING_16())));
				break;

			case OP_CLASS_32:
				vmPush(CREATE_OBJECT(objMakeClass(READ_STRING_32())));
				break;

			case OP_GET_PROPERTY_16: {
				if( !IS_INSTANCE(_peek(0)) ) {
					RUNTIME_ERROR(
						"So e possivel acessar as propriedades de uma "
						"instancia");
					return RESULT_RUNTIME_ERROR;
				}

				ObjInstance *instance = AS_INSTANCE(_peek(0));
				ObjString *name = READ_STRING_16();

				Value value;
				if( tableGet(&instance->fields, CREATE_OBJECT(name), &value) ) {
					vmPop();
					vmPush(value);
					break;
				}

				frame->fp = fp;
				if( !_bindMethod(instance->klass, name) ) {
					return RESULT_RUNTIME_ERROR;
				}
			} break;

			case OP_GET_PROPERTY_32: {
				if( !IS_INSTANCE(_peek(0)) ) {
					RUNTIME_ERROR(
						"So e possivel acessar as propriedades de uma "
						"instancia");
					return RESULT_RUNTIME_ERROR;
				}

				ObjInstance *instance = AS_INSTANCE(_peek(0));
				ObjString *name = READ_STRING_32();

				Value value;
				if( tableGet(&instance->fields, CREATE_OBJECT(name), &value) ) {
					vmPop();
					vmPush(value);
					break;
				}

				frame->fp = fp;
				if( !_bindMethod(instance->klass, name) ) {
					return RESULT_RUNTIME_ERROR;
				}
			} break;

			case OP_SET_PROPERTY_16: {
				if( !IS_INSTANCE(_peek(1)) ) {
					RUNTIME_ERROR(
						"So e possivel mudar as propriedades de uma "
						"instancia");
					return RESULT_RUNTIME_ERROR;
				}

				ObjInstance *instance = AS_INSTANCE(_peek(1));
				tableSet(&instance->fields, CREATE_OBJECT(READ_STRING_16()),
						 _peek(0));
				Value value = vmPop();
				vmPop();
				vmPush(value);
			} break;

			case OP_SET_PROPERTY_32: {
				if( !IS_INSTANCE(_peek(1)) ) {
					RUNTIME_ERROR(
						"So e possivel mudar as propriedades de uma "
						"instancia");
					return RESULT_RUNTIME_ERROR;
				}

				ObjInstance *instance = AS_INSTANCE(_peek(1));
				tableSet(&instance->fields, CREATE_OBJECT(READ_STRING_32()),
						 _peek(0));
				Value value = vmPop();
				vmPop();
				vmPush(value);
			} break;

			case OP_METHOD_16:
				_defineMethod(READ_STRING_16());
				break;

			case OP_METHOD_32:
				_defineMethod(READ_STRING_32());
				break;

			case OP_INVOKE_16: {
				ObjString *method = READ_STRING_16();
				const uint8_t ARG_COUNT = READ_8();

				frame->fp = fp;
				if( !_invoke(method, ARG_COUNT) ) {
					return RESULT_RUNTIME_ERROR;
				}

				frame = &vm.frames[vm.frameCount - 1];
				fp = frame->fp;
			} break;

			case OP_INVOKE_32: {
				ObjString *method = READ_STRING_32();
				const uint8_t ARG_COUNT = READ_8();

				frame->fp = fp;
				if( !_invoke(method, ARG_COUNT) ) {
					return RESULT_RUNTIME_ERROR;
				}

				frame = &vm.frames[vm.frameCount - 1];
				fp = frame->fp;
			} break;

			case OP_INHERIT: {
				Value superclass = _peek(1);
				if( !IS_CLASS(superclass) ) {
					RUNTIME_ERROR("So e possivel herdar classes");
					return RESULT_RUNTIME_ERROR;
				}

				ObjClass *subclass = AS_CLASS(_peek(0));
				tableCopyTo(&AS_CLASS(superclass)->methods, &subclass->methods);
				vmPop();
			} break;

			case OP_GET_SUPER_16: {
				ObjString *name = READ_STRING_16();
				ObjClass *superclass = AS_CLASS(vmPop());

				if( !_bindMethod(superclass, name) ) {
					return RESULT_RUNTIME_ERROR;
				}
			} break;

			case OP_GET_SUPER_32: {
				ObjString *name = READ_STRING_32();
				ObjClass *superclass = AS_CLASS(vmPop());

				if( !_bindMethod(superclass, name) ) {
					return RESULT_RUNTIME_ERROR;
				}
			} break;

			case OP_SUPER_INVOKE_16: {
				ObjString *method = READ_STRING_16();
				const uint8_t ARG_COUNT = READ_8();

				ObjClass *superclass = AS_CLASS(vmPop());
				frame->fp = fp;
				if( !_invokeFromClass(superclass, method, ARG_COUNT) ) {
					return RESULT_RUNTIME_ERROR;
				}

				frame = &vm.frames[vm.frameCount - 1];
				fp = frame->fp;
			} break;

			case OP_SUPER_INVOKE_32: {
				ObjString *method = READ_STRING_32();
				const uint8_t ARG_COUNT = READ_8();

				ObjClass *superclass = AS_CLASS(vmPop());
				frame->fp = fp;
				if( !_invokeFromClass(superclass, method, ARG_COUNT) ) {
					return RESULT_RUNTIME_ERROR;
				}

				frame = &vm.frames[vm.frameCount - 1];
				fp = frame->fp;
			} break;

			case OP_ARRAY:
				vmPush(CREATE_OBJECT(objMakeArray()));
				break;

			case OP_PUSH_TO_ARRAY: {
				ObjArray *array = AS_ARRAY(_peek(1));
				valueArrayWrite(&array->array, vmPop());
			} break;

			case OP_TABLE:
				vmPush(CREATE_OBJECT(objMakeTable()));
				break;

			case OP_PUSH_TO_TABLE: {
				ObjTable *table = AS_TABLE(_peek(2));
				tableSet(&table->table, vmPop(), vmPop());
			} break;

			case OP_GET_SUBSCRIPT: {
				if( IS_ARRAY(_peek(1)) ) {
					if( !IS_NUMBER(_peek(0)) ) {
						RUNTIME_ERROR("Indice do array deve ser um numero");
						return RESULT_RUNTIME_ERROR;
					}

					int64_t index = (int64_t)AS_NUMBER(vmPop());
					ValueArray array = AS_ARRAY(vmPop())->array;

					frame->fp = fp;
					if( !_getArrayValue(&array, index) ) {
						return RESULT_RUNTIME_ERROR;
					}
				} else if( IS_TABLE(_peek(1)) ) {
					Value key = vmPop();
					if( !IS_STRING(key) ) {
						RUNTIME_ERROR(
							"Valores chave em um hasmap so podem ser"
							" numeros, strings, bools ou nulo");
						return RESULT_RUNTIME_ERROR;
					}

					Table table = AS_TABLE(vmPop())->table;
					Value value;

					if( !tableGet(&table, key, &value) ) {
						RUNTIME_ERROR("Esta chave nao existe no hashmap");
						return RESULT_RUNTIME_ERROR;
					}

					vmPush(value);
				} else if( IS_STRING(_peek(1)) ) {
					if( !IS_NUMBER(_peek(0)) ) {
						RUNTIME_ERROR("Indice do array deve ser um numero");
						return RESULT_RUNTIME_ERROR;
					}

					int64_t index = (int64_t)AS_NUMBER(vmPop());
					ObjString *str = AS_STRING(vmPop());

					frame->fp = fp;
					if( !_getCharAt(str, index) ) {
						return RESULT_RUNTIME_ERROR;
					}
				} else {
					RUNTIME_ERROR("So arrays, hashmaps e strings podem ter seus itens acessados por indice");
					return RESULT_RUNTIME_ERROR;
				}
			} break;

			case OP_SET_SUBSCRIPT: {
				if( IS_ARRAY(_peek(2)) ) {
					if( !IS_NUMBER(_peek(1)) ) {
						RUNTIME_ERROR("Indice do array deve ser um numero");
						return RESULT_RUNTIME_ERROR;
					}

					Value value = vmPop();
					int64_t index = (int64_t)AS_NUMBER(vmPop());

					frame->fp = fp;
					if( !_setArrayValue(&AS_ARRAY(_peek(0))->array, index,
										value) ) {
						return RESULT_RUNTIME_ERROR;
					}
				} else if( IS_TABLE(_peek(2)) ) {
					if( !IS_STRING(_peek(1)) ) {
						RUNTIME_ERROR(
							"Valores chave em um hasmap so podem ser"
							" numeros, strings, bools ou nulo");
						return RESULT_RUNTIME_ERROR;
					}

					Value value = vmPop();
					Value key = vmPop();

					tableSet(&AS_TABLE(_peek(0))->table, key, value);
				} else {
					RUNTIME_ERROR("So arrays e hashmap podem ter seus valores mudados por acesso de indice");
					return RESULT_RUNTIME_ERROR;
				}
			} break;

			case OP_RETURN: {
				Value result = vmPop();
				frame->fp = fp;

				_closeUpvalues(frame->slots);
				--vm.frameCount;
				if( vm.frameCount == 0 ) {
					vmPop();
					return RESULT_OK;
				}

				vm.stackTop = frame->slots;
				vmPush(result);

				frame = &vm.frames[vm.frameCount - 1];
				fp = frame->fp;
			} break;

			default:
				errWarn(
					chunkGetLine(&frame->closure->function->chunk, *(fp - 1)),
					"OPCODE desconhecido encontrado! -> ");
				printf("%02x\n", OP);
		}
	}
}

Result vmInterpret(const char *SOURCE) {
	ObjFunction *function = compCompile(SOURCE);

	if( function == NULL ) {
		return RESULT_COMPILER_ERROR;
	}

	vm.isLocked = true;
	ObjClosure *closure = objMakeClosure(function);
	vm.isLocked = false;

	vmPush(CREATE_OBJECT(closure));
	_call(closure, 0);

	return _run();
}

#undef READ_8
#undef READ_16
#undef READ_24

#undef READ_CONST_16
#undef READ_CONST_32

#undef READ_STRING_16
#undef READ_STRING_32

#undef BINARY_OP
