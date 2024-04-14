/**
 * @file gc.c
 * @author Pedro B.
 * @date 2024.04.13
 *
 * @brief Coletor de lixo
 */

#include "gc.h"

#include <stdlib.h>

#include "compiler.h"
#include "error.h"
#include "memory.h"
#include "table.h"
#include "vm.h"

/** Taxa na qual o GC coleta */
#define GC_HEAP_GROW_FACTOR 2

#include <stdio.h>
#ifdef DEBUG_LOG_GC

#include "debug.h"
#endif

static void _markTable(Table *table) {
	for( size_t i = 0; i < table->size; ++i ) {
		Entry *entry = &table->entries[i];
		if( !IS_EMPTY(entry->key) ) {
			gcMarkValue(entry->key);
			gcMarkValue(entry->value);
		}
	}
}

static void _markArray(ValueArray *array) {
	for( size_t i = 0; i < array->count; ++i ) {
		gcMarkValue(array->values[i]);
	}
}

static void _markRoots(void) {
	/* Marcamos os valores na pilha */
	for( Value *slot = vm.stack; slot < vm.stackTop; ++slot ) {
		gcMarkValue(*slot);
	}

	/* Marcamos as closures */
	for( int8_t i = 0; i < vm.frameCount; ++i ) {
		gcMarkObject((Obj *)vm.frames[i].closure);
	}

	/* Marcamos os upvalues */
	for( ObjUpvalue *upvalue = vm.openUpvalues; upvalue != NULL;
		 upvalue = upvalue->next ) {
		gcMarkObject((Obj *)upvalue);
	}

	/* Marcamos os valores globais */
	_markTable(&vm.globalNames);
	for( size_t i = 0; i < vm.globalValues.count; ++i ) {
		gcMarkValue(vm.globalValues.values[i]);
	}

	/* Marcamos os valores no compilador */
	compMarkRoots();
}

static void _blackenObject(Obj *object) {
#ifdef DEBUG_LOG_GC
	printf("%p | Escureceu objeto ", (void *)object);
	valuePrint(CREATE_OBJECT(object));
	printf("\n");
#endif

	switch( object->type ) {
		case OBJ_UPVALUE:
			gcMarkValue(((ObjUpvalue *)object)->closed);
			break;

		case OBJ_FUNCTION: {
			ObjFunction *function = (ObjFunction *)object;
			gcMarkObject((Obj *)function->name);
			_markArray(&function->chunk.consts);
		} break;

		case OBJ_CLOSURE: {
			ObjClosure *closure = (ObjClosure *)object;
			gcMarkObject((Obj *)closure->function);

			for( size_t i = 0; i < closure->upvalueCount; ++i ) {
				gcMarkObject((Obj *)closure->upvalues[i]);
			}
		} break;

		case OBJ_CLASS: {
			ObjClass *klass = (ObjClass *)object;
			gcMarkObject((Obj *)klass->name);
			_markTable(&klass->methods);
			break;
		}

		case OBJ_INSTANCE: {
			ObjInstance *instance = (ObjInstance *)object;
			gcMarkObject((Obj *)instance->klass);
			_markTable(&instance->fields);
		} break;

		case OBJ_BOUND_METHOD: {
			ObjBoundMethod *bound = (ObjBoundMethod *)object;
			gcMarkValue(bound->receiver);
			gcMarkObject((Obj *)bound->method);
		} break;

		case OBJ_RANGE: {
			ObjRange *range = (ObjRange *)object;
			gcMarkValue(range->start);
			gcMarkValue(range->end);
		} break;

		case OBJ_ARRAY: {
			ObjArray *array = (ObjArray *)object;
			_markArray(&array->array);
		} break;

		case OBJ_TABLE: {
			ObjTable *table = (ObjTable *)object;
			_markTable(&table->table);
		} break;

		case OBJ_NATIVE:
		case OBJ_STRING:
			/* Não possuem referências a outros objetos. Ignoramos */
			break;
	}
}

static void _traceRefs(void) {
	while( vm.grayCount > 0 ) {
		Obj *object = vm.grayStack[--vm.grayCount];
		_blackenObject(object);
	}
}

static void _sweep(void) {
	Obj *previous = NULL;
	Obj *object = vm.objects;

	while( object != NULL ) {
		if( object->isMarked ) {
			object->isMarked = false;
			previous = object;
			object = object->next;
		} else {
			Obj *unreached = object;
			object = object->next;
			if( previous != NULL ) {
				previous->next = object;
			} else {
				vm.objects = object;
			}

			memFreeObject(unreached);
		}
	}
}

void gcCollect(void) {
#ifdef DEBUG_LOG_GC
	printf("-- gc begin\n");
	const size_t BEFORE = vm.bytesAllocated;
#endif

	_markRoots();
	_traceRefs();
	tableRemoveWhite(&vm.strings);
	_sweep();

	vm.nextGC = vm.bytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
	printf("-- gc end\n");
	printf("   Coletou %u bytes (de %u para %u), proxima em %u\n",
		   BEFORE - vm.bytesAllocated, BEFORE, vm.bytesAllocated, vm.nextGC);
#endif
}

void gcMarkValue(Value value) {
	if( IS_OBJECT(value) ) {
		gcMarkObject(AS_OBJECT(value));
	}
}

void gcMarkObject(Obj *object) {
	if( object == NULL || object->isMarked == true ) {
		return;
	}

#ifdef DEBUG_LOG_GC
	printf("%p | Marcou o objeto ", (void *)object);
	valuePrint(CREATE_OBJECT(object));
	printf("\n");
#endif

	object->isMarked = true;

	if( vm.graySize < vm.grayCount + 1 ) {
		vm.graySize = MEM_GROW_SIZE(vm.graySize);
		vm.grayStack =
			(Obj **)realloc(vm.grayStack, sizeof(Obj *) * vm.graySize);

		if( vm.grayStack == NULL ) {
			errFatal(0,
					 "Sem memoria o bastante para alocar a pilha de objetos");
			exit(1);
		}
	}

	vm.grayStack[vm.grayCount++] = object;
}
