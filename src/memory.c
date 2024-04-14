/**
 * @file memory.c
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Lida com operações relacionadas à memória
 */

#include "memory.h"

#include <stdlib.h>

#include "error.h"
#include "gc.h"
#include "vm.h"

#ifdef DEBUG_LOG_GC
#include <stdio.h>
#endif

void *memRealloc(void *pointer, const size_t OLD_SIZE, const size_t NEW_SIZE) {
	vm.bytesAllocated += NEW_SIZE - OLD_SIZE;

	if( NEW_SIZE > OLD_SIZE ) {
#ifdef DEBUG_STRESS_GC
		if( !vm.isLocked ) {
			gcCollect();
		}
#else
		if( !vm.isLocked && vm.bytesAllocated > vm.nextGC ) {
			gcCollect();
		}
#endif
	}

	if( NEW_SIZE == 0 ) {
		/* Se o novo tamanho for 0, libere a memória, já que
		 * realloc com tamanho 0 é UB.
		 */
		free(pointer);
		return NULL;
	}

	void *result = realloc(pointer, NEW_SIZE);
	if( result == NULL ) {
		/* Um erro aconteceu quando tentamos alocar mais memória
		 * Ou a memória RAM acabou ( x _ x ) ...
		 * ... ou um erro bisonho aconteceu.
		 *
		 * De qualquer forma, sai com código ERR_UNAVAILABLE
		 */
		errFatal(vmGetLine(0), "Nao foi possivel alocar memoria!");
		exit(69);
	}

	return result;
}

void memFreeObject(Obj *object) {
#ifdef DEBUG_LOG_GC
	printf("%p | Liberando objeto ", (void *)object);
	valuePrint(CREATE_OBJECT(object));
	printf("\n");
#endif

	switch( object->type ) {
		case OBJ_STRING: {
			ObjString *obj = (ObjString *)object;
			obj = memRealloc(obj, sizeof(ObjString) + obj->length + 1, 0);
		} break;

		case OBJ_UPVALUE:
			MEM_FREE(ObjClosure, object);
			break;

		case OBJ_FUNCTION: {
			ObjFunction *function = (ObjFunction *)object;
			chunkFree(&function->chunk);
			MEM_FREE(ObjFunction, object);
		} break;

		case OBJ_NATIVE:
			MEM_FREE(ObjNative, object);
			break;

		case OBJ_CLOSURE: {
			ObjClosure *closure = (ObjClosure *)object;
			MEM_FREE_ARRAY(ObjUpvalue *, closure->upvalues,
						   closure->upvalueCount);
			MEM_FREE(ObjClosure, object);
		} break;

		case OBJ_CLASS: {
			ObjClass *klass = (ObjClass *)object;
			tableFree(&klass->methods);
			MEM_FREE(ObjClass, object);
		} break;

		case OBJ_INSTANCE: {
			ObjInstance *instance = (ObjInstance *)object;
			tableFree(&instance->fields);
			MEM_FREE(ObjInstance, object);
		} break;

		case OBJ_BOUND_METHOD:
			MEM_FREE(ObjBoundMethod, object);
			break;

		default:
			errFatal(vmGetLine(0),
					 "Tentou liberar um objeto de tipo desconhecido %u",
					 object->type);
	}
}

void memFreeObjects(void) {
	Obj *object = vm.objects;
	while( object != NULL ) {
		Obj *next = object->next;
		memFreeObject(object);
		object = next;
	}

	free(vm.grayStack);
}
