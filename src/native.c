/**
 * @file native.c
 * @author Pedro B.
 * @date 2024.04.11
 *
 * @brief Funções para tratar de funções nativas
 */

#include "native.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "vm.h"

static Value _nativeClock(const uint8_t ARG_COUNT, Value *args) {
	return CREATE_NUMBER((LOXIE_NUMBER)clock() / CLOCKS_PER_SEC);
}

static Value _nativeType(const uint8_t ARG_COUNT, Value *args) {
	return CREATE_NUMBER((LOXIE_NUMBER)clock() / CLOCKS_PER_SEC);
}

void nativeInit(void) {
	nativeDefine(_nativeClock, "cronometro", 0);
	nativeDefine(_nativeType, "tipo", 1);
}

bool nativeCall(NativeFn native, const uint8_t ARG_COUNT) {
	Value result = native(ARG_COUNT, vm.stackTop - ARG_COUNT);
	vm.stackTop -= ARG_COUNT + 1;
	vmPush(result);

	return GET_TYPE(result) != VALUE_EMPTY;
}

void nativeDefine(NativeFn native, const char *NAME, const int16_t ARGS) {
	vmPush(CREATE_OBJECT(objCopyString(NAME, (size_t)strlen(NAME))));
	vmPush(CREATE_OBJECT(objMakeNative(native, ARGS)));

	const Value INDEX = CREATE_NUMBER((double)vm.globalValues.count);

	tableSet(&vm.globalNames, vm.stack[0], INDEX);
	valueArrayWrite(&vm.globalValues, vm.stack[1]);

	vmPop();
	vmPop();
}
