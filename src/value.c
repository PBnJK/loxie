/**
 * @file value.c
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Representação de valores da linguagem
 */

#include "value.h"

#include <stdio.h>
#include <string.h>

#include "error.h"
#include "object.h"
#include "vm.h"

bool valueEquals(const Value A, const Value B) {
	if( A.type != B.type ) {
		return false;
	}

	switch( A.type ) {
		case VALUE_NIL:
			return true;
		case VALUE_BOOL:
			return AS_BOOL(A) == AS_BOOL(B);
		case VALUE_NUMBER:
			return AS_NUMBER(A) == AS_NUMBER(B);
		case VALUE_OBJECT: {
			ObjString *strA = AS_STRING(A);
			ObjString *strB = AS_STRING(B);

			return strA->length == strB->length &&
				   memcmp(strA->str, strB->str, strA->length) == 0;
		}
		default:
			errFatal(vmGetLine(), "Tentou comparar %u com %u", A.type, B.type);
	}

	return false;
}

void valuePrint(Value value) {
	switch( value.type ) {
		case VALUE_NIL:
			printf("nil");
			break;
		case VALUE_BOOL:
			printf("%s", AS_BOOL(value) ? "true" : "false");
			break;
		case VALUE_NUMBER:
			printf("%g", AS_NUMBER(value));
			break;
		case VALUE_OBJECT:
			objPrint(value);
			break;
		default:
			errFatal(vmGetLine(), "Valor desconhecido %u", value.type);
	}
}
