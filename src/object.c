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

ObjString *objCopyString(const char *STR, const size_t LEN) {
	ObjString *string = objMakeString(LEN);

	memcpy(string->str, STR, LEN);
	string->str[LEN] = '\0';

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
