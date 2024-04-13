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

	newObject->next = vm.objects;
	vm.objects = newObject;
	return newObject;
}

static void _printFunction(ObjFunction *function) {
	if( function->name == NULL ) {
		/* Função não tem nome
		 * Ou o usuário fez algo esquisito, ou este é o script em si
		 * Vamos apostar na última hipótese...
		 */
		printf("<script>");
		return;
	}

	printf("<func %s>", function->name->str);
}

ObjString *objMakeString(const size_t LEN) {
	ObjString *string =
		(ObjString *)_allocObject(sizeof(ObjString) + LEN + 1, OBJ_STRING);

	string->length = LEN;

	return string;
}

ObjFunction *objMakeFunction(void) {
	ObjFunction *function = ALLOC_OBJECT(ObjFunction, OBJ_FUNCTION);

	function->arity = 0;
	function->name = NULL;
	chunkInit(&function->chunk);

	return function;
}

ObjNative *objMakeNative(NativeFn function) {
	ObjNative *native = ALLOC_OBJECT(ObjNative, OBJ_NATIVE);
	native->function = function;

	return native;
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
	tableSet(&vm.strings, CREATE_OBJECT(string), CREATE_NIL());

	return string;
}

void objPrint(const Value VALUE) {
	switch( OBJECT_TYPE(VALUE) ) {
		case OBJ_STRING:
			printf("%s", AS_CSTRING(VALUE));
			break;

		case OBJ_FUNCTION:
			_printFunction(AS_FUNCTION(VALUE));
			break;

		case OBJ_NATIVE:
			printf("<native fn>");
			break;

		default:
			errFatal(vmGetLine(0),
					 "Tentou imprimir objeto de tipo desconhecido %u",
					 OBJECT_TYPE(VALUE));
	}
}
