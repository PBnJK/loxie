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

ObjString *objMakeString(const size_t LEN) {
	ObjString *string =
		(ObjString *)_allocObject(sizeof(ObjString) + LEN + 1, OBJ_STRING);

	string->length = LEN;

	return string;
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
		default:
			errFatal(vmGetLine(),
					 "Tentou imprimir objeto de tipo desconhecido %u",
					 OBJECT_TYPE(VALUE));
	}
}
