/**
 * @file object.c
 * @author Pedro B.
 * @date 2024.04.05
 *
 * @brief Representa um objeto que mora na heap
 */

#include "object.h"

#include <stdio.h>
#include <string.h>

#include "error.h"
#include "memory.h"
#include "table.h"
#include "value.h"
#include "vm.h"

/** Macro de conveniência para alocar um novo objeto */
#define ALLOC_OBJECT(TYPE, OBJ_TYPE) \
	(TYPE *)_allocObject(sizeof(TYPE), OBJ_TYPE)

static Obj *_allocObject(const size_t SIZE, const ObjType TYPE) {
	Obj *newObject = memRealloc(NULL, 0, SIZE);

	newObject->type = TYPE;
	newObject->isMarked = false;

	newObject->next = vm.objects;
	vm.objects = newObject;

#ifdef DEBUG_LOG_GC
	printf("%p | Alocou %u bytes para obj. tipo %d\n", (void *)newObject, SIZE,
		   TYPE);
#endif

	return newObject;
}

static void _printFunction(ObjFunction *function) {
	if( function->name == NULL ) {
		/* Função não tem nome, logo é o script em si */
		printf("<script>");
		return;
	}

	printf("<func %s>", function->name->str);
}

static void _printArray(ValueArray *array) {
	size_t idx = 0;

	printf("[");
	while( idx < array->count ) {
		valuePrint(array->values[idx]);

		if( (++idx) != array->count ) {
			printf(", ");
		}
	}

	printf("]");
}

static void _printTable(Table *table) {
	size_t idx = 0;
	size_t items = 0;

	printf("{");
	while( idx < table->size ) {
		if( IS_EMPTY(table->entries[idx].key) ) {
			++idx;
			continue;
		}

		valuePrint(table->entries[idx].key);
		printf(": ");
		valuePrint(table->entries[idx].value);

		if( (++items) == table->count ) {
			break;
		}

		++idx;
		printf(", ");
	}

	printf("}");
}
ObjString *objMakeString(const size_t LEN) {
	ObjString *string =
		(ObjString *)_allocObject(sizeof(ObjString) + LEN + 1, OBJ_STRING);

	string->length = LEN;

	return string;
}

ObjUpvalue *objMakeUpvalue(Value *slot) {
	ObjUpvalue *upvalue = ALLOC_OBJECT(ObjUpvalue, OBJ_UPVALUE);

	upvalue->location = slot;
	upvalue->closed = CREATE_NIL();
	upvalue->next = NULL;

	return upvalue;
}

ObjFunction *objMakeFunction(void) {
	ObjFunction *function = ALLOC_OBJECT(ObjFunction, OBJ_FUNCTION);

	function->arity = 0;
	function->upvalueSize = 0;
	function->upvalueCount = 0;
	function->name = NULL;
	chunkInit(&function->chunk);

	return function;
}

ObjNative *objMakeNative(NativeFn function, const uint16_t ARGS) {
	ObjNative *native = ALLOC_OBJECT(ObjNative, OBJ_NATIVE);
	native->function = function;
	native->argCount = ARGS;

	return native;
}

ObjClosure *objMakeClosure(ObjFunction *function) {
	ObjUpvalue **upvalues = MEM_ALLOC(ObjUpvalue *, function->upvalueCount);

	for( size_t i = 0; i < function->upvalueCount; ++i ) {
		upvalues[i] = NULL;
	}

	ObjClosure *closure = ALLOC_OBJECT(ObjClosure, OBJ_CLOSURE);
	closure->function = function;

	closure->upvalues = upvalues;
	closure->upvalueCount = function->upvalueCount;
	closure->upvalueSize = function->upvalueSize;

	return closure;
}

ObjClass *objMakeClass(ObjString *name) {
	ObjClass *klass = ALLOC_OBJECT(ObjClass, OBJ_CLASS);

	klass->name = name;
	klass->constructor = CREATE_NIL();
	tableInit(&klass->methods);

	return klass;
}

ObjInstance *objMakeInstance(ObjClass *klass) {
	ObjInstance *instance = ALLOC_OBJECT(ObjInstance, OBJ_INSTANCE);

	instance->klass = klass;
	tableInit(&instance->fields);

	return instance;
}

ObjBoundMethod *objMakeBoundMethod(Value receiver, ObjClosure *method) {
	ObjBoundMethod *bound = ALLOC_OBJECT(ObjBoundMethod, OBJ_BOUND_METHOD);

	bound->receiver = receiver;
	bound->method = method;

	return bound;
}

ObjRange *objMakeRange(Value start, Value end) {
	ObjRange *range = ALLOC_OBJECT(ObjRange, OBJ_RANGE);

	if( AS_NUMBER(start) < AS_NUMBER(end) ) {
		range->start = end;
		range->end = start;
	} else {
		range->start = start;
		range->end = end;
	}

	return range;
}

ObjArray *objMakeArray(void) {
	ObjArray *array = ALLOC_OBJECT(ObjArray, OBJ_ARRAY);
	valueArrayInit(&array->array);

	return array;
}

ObjTable *objMakeTable(void) {
	ObjTable *table = ALLOC_OBJECT(ObjTable, OBJ_TABLE);
	tableInit(&table->table);

	return table;
}

uint32_t hashString(const char *KEY, const size_t LENGTH) {
	uint32_t hash = 2166136261u;
	for( size_t i = 0; i < LENGTH; ++i ) {
		hash ^= (uint8_t)KEY[i];
		hash *= 16777619;
	}

	return hash;
}

ObjString *objCopyString(const char *STR, const size_t LEN) {
	uint32_t hash = hashString(STR, LEN);

	Value interned = tableFindString(&vm.strings, STR, LEN, hash);
	if( !IS_EMPTY(interned) ) {
		return AS_STRING(interned);
	}

	ObjString *string = objMakeString(LEN);

	memcpy(string->str, STR, LEN);
	string->str[LEN] = '\0';

	string->hash = hashString(string->str, LEN);

	vm.isLocked = true;
	tableSet(&vm.strings, CREATE_OBJECT(string), CREATE_NIL());
	vm.isLocked = false;

	return string;
}

void objPrint(const Value VALUE) {
	switch( OBJECT_TYPE(VALUE) ) {
		case OBJ_STRING:
			printf("%s", AS_CSTRING(VALUE));
			break;

		case OBJ_UPVALUE:
			printf("upvalue");
			break;

		case OBJ_FUNCTION:
			_printFunction(AS_FUNCTION(VALUE));
			break;

		case OBJ_NATIVE:
			printf("<fn nativa>");
			break;

		case OBJ_CLOSURE:
			_printFunction(AS_CLOSURE_FN(VALUE));
			break;

		case OBJ_CLASS:
			printf("%s", AS_CLASS(VALUE)->name->str);
			break;

		case OBJ_INSTANCE:
			printf("instancia de %s", AS_INSTANCE(VALUE)->klass->name->str);
			break;

		case OBJ_BOUND_METHOD:
			_printFunction(AS_BOUND_METHOD(VALUE)->method->function);
			break;

		case OBJ_RANGE:
			printf("range");
			break;

		case OBJ_ARRAY:
			_printArray(&AS_ARRAY(VALUE)->array);
			break;

		case OBJ_TABLE:
			_printTable(&AS_TABLE(VALUE)->table);
			break;

		default:
			errFatal(vmGetLine(0),
					 "Tentou imprimir objeto de tipo desconhecido %u",
					 OBJECT_TYPE(VALUE));
	}
}

static bool _arrayEquals(const ValueArray *A, const ValueArray *B) {
	if( A->count != B->count ) {
		return false;
	}

	for( size_t idx = 0; idx < A->count; ++idx ) {
		if( !valueEquals(A->values[idx], A->values[idx]) ) {
			return false;
		}
	}

	return true;
}

static bool _tableEquals(const Table *A, const Table *B) {
	if( A->count != B->count ) {
		return false;
	}

	for( size_t idx = 0; idx < A->size; ++idx ) {
		if( !valueEquals(A->entries[idx].key, B->entries[idx].key) ||
			!valueEquals(A->entries[idx].value, B->entries[idx].value) ) {
			return false;
		}
	}

	return true;
}

bool objEquals(const Value A, const Value B) {
	switch( OBJECT_TYPE(A) ) {
		case OBJ_STRING:
		case OBJ_UPVALUE:
		case OBJ_FUNCTION:
		case OBJ_NATIVE:
		case OBJ_CLOSURE:
		case OBJ_CLASS:
		case OBJ_INSTANCE:
		case OBJ_BOUND_METHOD:
			return AS_OBJECT(A) == AS_OBJECT(B);

		case OBJ_RANGE:
			return false;

		case OBJ_ARRAY:
			return _arrayEquals(&AS_ARRAY(A)->array, &AS_ARRAY(B)->array);

		case OBJ_TABLE:
			return _tableEquals(&AS_TABLE(A)->table, &AS_TABLE(B)->table);

		default:
			errFatal(vmGetLine(0),
					 "Tentou comparar objetos de tipo desconhecido %u",
					 OBJECT_TYPE(A));
			return false;
	}
}
