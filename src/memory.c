/**
 * @file memory.c
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Lida com operações relacionadas à memória
 */

#include "memory.h"

#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "vm.h"

static void _freeObject(Obj *object) {
	switch( object->type ) {
		case OBJ_STRING: {
			ObjString *obj = (ObjString *)object;
			obj = memRealloc(obj, sizeof(ObjString) + obj->length + 1, 0);
		} break;

		case OBJ_FUNCTION: {
			ObjFunction *obj = (ObjFunction *)object;
			chunkFree(&obj->chunk);
			MEM_FREE(ObjFunction, obj);
		} break;

		case OBJ_NATIVE:
			MEM_FREE(ObjNative, object);
			break;

		default:
			errFatal(vmGetLine(0),
					 "Tentou liberar um objeto de tipo desconhecido %u",
					 object->type);
	}
}

void *memRealloc(void *pointer, const size_t OLD_SIZE, const size_t NEW_SIZE) {
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

void memFreeObjects(void) {
	Obj *object = vm.objects;
	while( object != NULL ) {
		Obj *next = object->next;
		_freeObject(object);
		object = next;
	}
}
