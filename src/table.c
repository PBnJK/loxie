/**
 * @file table.c
 * @author Pedro B.
 * @date 2024.04.06
 *
 * @brief Implementação de uma hash table
 */

#include "table.h"

#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"

/**
 * Define que a mesa só crescerá quand chegar a (TABLE_MAX_LOAD * 100)%
 * de sua capacidade total
 */
#define TABLE_MAX_LOAD 0.75

/** Determina se uma dada entrada não possui um valor associado a ela */
#define IS_ENTRY_EMPTY(ENTRY) (IS_EMPTY((ENTRY->key)))

/**
 * @brief Obtém a hash de um número
 *
 * @param[in] KEY Número a partir do qual a hash será gerada
 *
 * @return Hash do número @a KEY
 */
static uint32_t _hashNumber(const NEAT_NUMBER KEY) {
#ifdef NEAT_USE_32BIT_NUMBERS
	/* hash(x) = x é rápido! */
	return KEY;
#else
	/* Hash para doubles 64-bit, sugerida pelo livro:
	 * github.com/munificent/craftinginterpreters/blob/master/note/answers/chapter20_hash/1.md
	 */
	union BitCast {
		double value;
		uint32_t ints[2];
	};

	union BitCast cast;
	cast.value = (KEY) + 1.0;
	return cast.ints[0] + cast.ints[1];
#endif
}

/**
 * @brief Obtém a hash de um valor
 *
 * @param[in] VALUE Valor a partir do qual a hash será gerada
 *
 * @return Hash do valor @a VALUE
 */
static uint32_t _hashValue(const Value VALUE) {
	switch( GET_TYPE(VALUE) ) {
		case VALUE_BOOL:
			/* Mesma hash que o Java */
			return AS_BOOL(VALUE) ? 1231 : 1237;
		case VALUE_NIL:
			/* Número primo aleatório */
			return 1993;
		case VALUE_NUMBER:
			return _hashNumber(AS_NUMBER(VALUE));
		case VALUE_OBJECT:
			return AS_STRING(VALUE)->hash;
		default:
			return 0;
	}
}

static Entry *_findEntry(Entry *entries, const size_t SIZE, const Value KEY) {
	uint32_t index = _hashValue(KEY) % SIZE;
	Entry *tombstone = NULL;

	while( true ) {
		Entry *entry = &entries[index];
		if( IS_ENTRY_EMPTY(entry) ) {
			if( IS_NIL(entry->value) ) {
				return tombstone != NULL ? tombstone : entry;
			} else if( tombstone == NULL ) {
				tombstone = entry;
			}
		} else if( valueEquals(entry->key, KEY) ) {
			return entry;
		}

		/* Não achamos uma entrada
		 * Avançamos o índice e tentamos a próximo */
		index = (index + 1) % SIZE;
	}
}

static void _adjustSize(Table *table, const size_t SIZE) {
	Entry *entries = MEM_ALLOC(Entry, SIZE);
	for( size_t i = 0; i < SIZE; ++i ) {
		entries[i].key = CREATE_EMPTY();
		entries[i].value = CREATE_NIL();
	}

	table->count = 0;
	for( size_t i = 0; i < table->size; ++i ) {
		Entry *entry = &table->entries[i];
		if( IS_ENTRY_EMPTY(entry) ) continue;

		Entry *dest = _findEntry(entries, SIZE, entry->key);
		dest->key = entry->key;
		dest->value = entry->value;
		++table->count;
	}

	MEM_FREE_ARRAY(Entry, table->entries, table->size);
	table->entries = entries;
	table->size = SIZE;
}

void tableInit(Table *table) {
	table->count = 0;
	table->size = 0;
	table->entries = NULL;
}

void tableFree(Table *table) {
	MEM_FREE_ARRAY(Entry, table->entries, table->size);
	tableInit(table);
}

Value tableFindString(Table *table, const char *STR, const size_t LEN,
					  const uint32_t HASH) {
	if( table->count == 0 ) {
		return CREATE_EMPTY();
	}

	uint32_t index = HASH % table->size;
	while( true ) {
		Entry *entry = &table->entries[index];
		if( IS_ENTRY_EMPTY(entry) && IS_NIL(entry->value) ) {
			return CREATE_EMPTY();
		}

		ObjString *key = AS_STRING(entry->key);
		if( key->length == LEN && key->hash == HASH &&
			memcmp(key->str, STR, LEN) == 0 ) {
			return entry->key;
		}

		index = (index + 1) % table->size;
	}
}

bool tableGet(Table *table, const Value KEY, Value *value) {
	if( table->count == 0 ) {
		/* O hashmap está vazio, saímos cedo */
		return false;
	}

	Entry *entry = _findEntry(table->entries, table->size, KEY);
	if( IS_ENTRY_EMPTY(entry) ) {
		return false;
	}

	*value = entry->value;
	return true;
}

bool tableSet(Table *table, const Value KEY, const Value VALUE) {
	if( table->count + 1 > table->size * TABLE_MAX_LOAD ) {
		const size_t SIZE = MEM_GROW_SIZE(table->size);
		_adjustSize(table, SIZE);
	}

	Entry *entry = _findEntry(table->entries, table->size, KEY);

	const bool IS_NEW_KEY = IS_ENTRY_EMPTY(entry);
	if( IS_NEW_KEY && IS_NIL(entry->value) ) {
		++table->count;
	}

	entry->key = KEY;
	entry->value = VALUE;

	return IS_NEW_KEY;
}

bool tableDelete(Table *table, const Value KEY) {
	if( table->count == 0 ) {
		return false;
	}

	Entry *entry = _findEntry(table->entries, table->size, KEY);
	if( IS_ENTRY_EMPTY(entry) ) {
		return false;
	}

	entry->key = CREATE_EMPTY();
	entry->value = CREATE_BOOL(true);
	return true;
}

void tableCopyTo(Table *from, Table *to) {
	for( size_t i = 0; i < from->size; ++i ) {
		Entry *entry = &from->entries[i];
		if( IS_ENTRY_EMPTY(entry) ) {
			tableSet(to, entry->key, entry->value);
		}
	}
}
