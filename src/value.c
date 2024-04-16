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
#ifdef NAN_BOXING
	if( IS_NUMBER(A) && IS_NUMBER(B) ) {
		return AS_NUMBER(A) == AS_NUMBER(B);
	}

	return A == B;
#else
	if( GET_TYPE(A) != GET_TYPE(B) ) {
		return false;
	}

	switch( GET_TYPE(A) ) {
		case VALUE_NIL:
		case VALUE_EMPTY:
			return true;
		case VALUE_BOOL:
			return AS_BOOL(A) == AS_BOOL(B);
		case VALUE_NUMBER:
			return AS_NUMBER(A) == AS_NUMBER(B);
		case VALUE_OBJECT:
			return objEquals(A, B);
		default:
			errFatal(vmGetLine(0), "Tentou comparar %u com %u", GET_TYPE(A),
					 GET_TYPE(B));
	}

	return false;
#endif
}

void valuePrint(Value value) {
#ifdef NAN_BOXING
	if( IS_BOOL(value) ) {
		printf(AS_BOOL(value) ? "true" : "false");
	} else if( IS_NIL(value) ) {
		printf("nil");
	} else if( IS_NUMBER(value) ) {
		printf("%g", AS_NUMBER(value));
	} else if( IS_OBJECT(value) ) {
		objPrint(value);
	} else {
		errFatal(vmGetLine(0), "Valor desconhecido %064x", value);
	}
#else
	switch( GET_TYPE(value) ) {
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
		case VALUE_EMPTY:
			printf("empty");
			break;
		default:
			errFatal(vmGetLine(0), "Valor desconhecido %u", GET_TYPE(value));
	}
#endif
}
