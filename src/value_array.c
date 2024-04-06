/**
 * @file value_array.c
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Array de valores
 */

#include "value_array.h"

#include "memory.h"

void valueArrayInit(ValueArray* array) {
	array->count = 0;
	array->size = 0;
	array->values = NULL;
}

void valueArrayFree(ValueArray* array) {
	MEM_FREE_ARRAY(Value, array->values, array->size);
	valueArrayInit(array);
}

void valueArrayWrite(ValueArray* array, Value value) {
	if( array->size < array->count + 1 ) {
		const size_t OLD_SIZE = array->size;
		array->size = MEM_GROW_SIZE(OLD_SIZE);

		array->values =
			MEM_GROW_ARRAY(Value, array->values, OLD_SIZE, array->size);
	}

	array->values[array->count] = value;
	++array->count;
}
