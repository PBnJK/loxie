/**
 * @file object.c
 * @author Pedro B.
 * @date 2024.04.05
 *
 * @brief Representa um objeto que mora na heap
 */

#ifndef GUARD_LOXIE_OBJECT_H
#define GUARD_LOXIE_OBJECT_H

#include "chunk.h"
#include "common.h"
#include "table.h"
#include "value.h"

/**
 * @brief Enum representando todos os tipos de objetos que existem
 */
typedef enum {
	OBJ_STRING = 0,	  /**< Objeto representando uma string */
	OBJ_UPVALUE = 1,  /**< Objeto representando um upvalue */
	OBJ_FUNCTION = 2, /**< Objeto representando uma função */
	OBJ_NATIVE = 3,	  /**< Objeto representando uma função nativa */
	OBJ_CLOSURE = 4,  /**< Objeto representando uma closure */
	OBJ_CLASS = 5,	  /**< Objeto representando uma classe */
	OBJ_INSTANCE = 6, /**< Objeto representando uma instância de uma classe */
	OBJ_BOUND_METHOD = 7, /**< Objeto representando um método capturado */
	OBJ_RANGE = 8,		  /**< Objeto representando uma faixa de valores */
	OBJ_ARRAY = 9,		  /**< Objeto representando um array */
	OBJ_TABLE = 10,		  /**< Objeto representando um hashmap */
} ObjType;

/**
 * @brief Struct representando um objeto que vive na heap
 *
 * Este struct age como um "struct base" para os outros tipos de objeto
 * Quando um novo objeto é criado, seu primeiro membro precisa ser um Obj
 */
struct Obj {
	ObjType type;	  /**< O tipo deste objeto */
	bool isMarked;	  /**< Se este objeto foi marcado pelo GC */
	struct Obj *next; /**< Próximo objeto (em uma lista linkada de objetos) */
};

/**
 * @brief Struct representando uma string
 */
struct ObjString {
	Obj obj;	   /**< Objeto base */
	uint32_t hash; /**< Hash da string (valor numérico que a representa */
	size_t length; /**< Tamanho da string */
	char str[];	   /**< Caracteres que compõe a string */
};

/**
 * @brief Struct representando um upvalue
 */
typedef struct ObjUpvalue {
	Obj obj;				 /**< Objeto base */
	Value *location;		 /**< Ponteiro a variável local capturada */
	Value closed;			 /**< Valor da variável fechada */
	struct ObjUpvalue *next; /**< Próximo Upvalue na lista */
} ObjUpvalue;

/**
 * @brief Struct representando uma função
 */
typedef struct ObjFunction {
	Obj obj;	   /**< Objeto base */
	uint8_t arity; /**< Quantidade de argumentos que a função recebe */

	size_t upvalueCount; /**< Quantidade de upvalues */
	size_t upvalueSize;	 /**< Tamanho do array de upvalues */

	Chunk chunk;	 /**< Chunk de código dentro da função */
	ObjString *name; /**< O nome da função */
} ObjFunction;

/** Typedef para uma função nativa */
typedef Value (*NativeFn)(const uint8_t ARG_COUNT, Value *args);

/**
 * @brief Struct representando uma função nativa
 */
typedef struct ObjNative {
	Obj obj;		   /**< Objeto base */
	NativeFn function; /**< Função */
	int16_t argCount;  /**< Quantidade de argumentos (-1 para variádica) */
} ObjNative;

/**
 * @brief Struct representando uma closure
 */
typedef struct ObjClosure {
	Obj obj;			   /**< Objeto base */
	ObjFunction *function; /**< Função interna */
	ObjUpvalue **upvalues; /**< Upvalues */
	size_t upvalueCount;   /**< Quantidade de upvalues (redundante, pro GC) */
	size_t upvalueSize;	   /**< Tamanho do array de upvalues (pro GC) */
} ObjClosure;

/**
 * @brief Struct representando uma classe
 */
typedef struct ObjClass {
	Obj obj;		   /**< Objeto base */
	ObjString *name;   /**< Nome da classe */
	Value constructor; /**< Método construtor da classe */
	Table methods;	   /**< Métodos da classe */
} ObjClass;

/**
 * @brief Struct representando uma instância de uma classe
 */
typedef struct ObjInstance {
	Obj obj;		 /**< Objeto base */
	ObjClass *klass; /**< Classe sendo instanciada */
	Table fields;	 /**< Campos da instância */
} ObjInstance;

/**
 * @brief Struct representando uma closure de um método
 */
typedef struct ObjBoundMethod {
	Obj obj;			/**< Objeto base */
	Value receiver;		/**< Variável que segura este método */
	ObjClosure *method; /**< Método origem */
} ObjBoundMethod;

/**
 * @brief Struct representando uma faixa de valores
 */
typedef struct ObjRange {
	Obj obj;	 /**< Objeto base */
	Value start; /**< Inicio da faixa (inclusivo) */
	Value end;	 /**< Fim da faixa (exclusivo) */
} ObjRange;

/**
 * @brief Struct representando um array
 */
typedef struct ObjArray {
	Obj obj;		  /**< Objeto base */
	ValueArray array; /**< Array */
} ObjArray;

/**
 * @brief Struct representando um hashmap
 */
typedef struct ObjTable {
	Obj obj;	 /**< Objeto base */
	Table table; /**< Hashmap */
} ObjTable;

/** Retorna o valor de um objeto */
#define OBJECT_TYPE(VALUE) (AS_OBJECT(VALUE)->type)

/** Verifica se um objeto é uma string */
#define IS_STRING(VALUE) _isObjectOfType(VALUE, OBJ_STRING)

/** Verifica se um objeto é um upvalue */
#define IS_UPVALUE(VALUE) _isObjectOfType(VALUE, OBJ_UPVALUE)

/** Verifica se um objeto é uma função */
#define IS_FUNCTION(VALUE) _isObjectOfType(VALUE, OBJ_FUNCTION)

/** Verifica se um objeto é uma função nativa */
#define IS_NATIVE(VALUE) _isObjectOfType(VALUE, OBJ_NATIVE)

/** Verifica se um objeto é uma closure */
#define IS_CLOSURE(VALUE) _isObjectOfType(VALUE, OBJ_CLOSURE)

/** Verifica se um objeto é uma classe */
#define IS_CLASS(VALUE) _isObjectOfType(VALUE, OBJ_CLASS)

/** Verifica se um objeto é uma instância de classe */
#define IS_INSTANCE(VALUE) _isObjectOfType(VALUE, OBJ_INSTANCE)

/** Verifica se um objeto é um método capturado */
#define IS_BOUND_METHOD(VALUE) _isObjectOfType(VALUE, OBJ_BOUND_METHOD)

/** Verifica se um objeto é uma faixa numérica */
#define IS_RANGE(VALUE) _isObjectOfType(VALUE, OBJ_RANGE)

/** Verifica se um objeto é um array */
#define IS_ARRAY(VALUE) _isObjectOfType(VALUE, OBJ_ARRAY)

/** Verifica se um objeto é um hashmap */
#define IS_TABLE(VALUE) _isObjectOfType(VALUE, OBJ_TABLE)

/** Trata um objeto como sendo do tipo ObjString */
#define AS_STRING(VALUE) ((ObjString *)AS_OBJECT(VALUE))

/** Trata um objeto como uma string C */
#define AS_CSTRING(VALUE) ((AS_STRING(VALUE))->str)

/** Trata um objeto como sendo do tipo ObjUpvalue */
#define AS_UPVALUE(VALUE) ((ObjUpvalue *)AS_OBJECT(VALUE))

/** Trata um objeto como sendo do tipo ObjFunction */
#define AS_FUNCTION(VALUE) ((ObjFunction *)AS_OBJECT(VALUE))

/** Trata um objeto como sendo do tipo ObjNative */
#define AS_NATIVE(VALUE) ((ObjNative *)AS_OBJECT(VALUE))

/** Pega a função de um objeto ObjNative */
#define AS_NATIVE_FN(VALUE) (AS_NATIVE(VALUE)->function)

/** Trata um objeto como sendo do tipo ObjClosure */
#define AS_CLOSURE(VALUE) ((ObjClosure *)AS_OBJECT(VALUE))

/** Pega a função de um objeto ObjClosure */
#define AS_CLOSURE_FN(VALUE) (AS_CLOSURE(VALUE)->function)

/** Trata um objeto como sendo do tipo ObjClass */
#define AS_CLASS(VALUE) ((ObjClass *)AS_OBJECT(VALUE))

/** Trata um objeto como sendo do tipo ObjInstance */
#define AS_INSTANCE(VALUE) ((ObjInstance *)AS_OBJECT(VALUE))

/** Trata um objeto como sendo do tipo ObjBoundMethod */
#define AS_BOUND_METHOD(VALUE) ((ObjBoundMethod *)AS_OBJECT(VALUE))

/** Trata um objeto como sendo do tipo ObjRange */
#define AS_RANGE(VALUE) ((ObjRange *)AS_OBJECT(VALUE))

/** Trata um objeto como sendo do tipo ObjArray */
#define AS_ARRAY(VALUE) ((ObjArray *)AS_OBJECT(VALUE))

/** Trata um objeto como sendo do tipo ObjTable */
#define AS_TABLE(VALUE) ((ObjTable *)AS_OBJECT(VALUE))

/**
 * @brief Verifica se um objeto é de um dado tipo
 *
 * @param[in] VALUE Valor sendo verificado
 * @param[in] TYPE Tipo esperado
 *
 * @return Verdadeiro se o valor @a VALUE pertençe ao tipo @a TYPE
 */
static inline bool _isObjectOfType(const Value VALUE, const ObjType TYPE) {
	return IS_OBJECT(VALUE) && OBJECT_TYPE(VALUE) == TYPE;
}

/**
 * @brief Cria uma string
 *
 * @param[in] LEN Tamanho da string
 * @return A string criada
 */
ObjString *objMakeString(const size_t LEN);

/**
 * @brief Cria um upvalue
 *
 * @param[in] slot todo
 * @return O upvalue criado
 */
ObjUpvalue *objMakeUpvalue(Value *slot);

/**
 * @brief Cria uma função
 * @return A função criada
 */
ObjFunction *objMakeFunction(void);

/**
 * @brief Cria uma função nativa
 *
 * @param[in] function Ponteiro pra função
 * @param[in] ARGS Quantidade de argumentos
 *
 * @return A função nativacriada
 */
ObjNative *objMakeNative(NativeFn function, const uint16_t ARGS);

/**
 * @brief Cria uma closure
 *
 * @param[in] function Função a partir da qual a closure será criada
 * @return A closure criada
 */
ObjClosure *objMakeClosure(ObjFunction *function);

/**
 * @brief Cria uma classe
 *
 * @param[in] name Nome da função
 * @return A classe criada
 */
ObjClass *objMakeClass(ObjString *name);

/**
 * @brief Instancia uma classe
 *
 * @param[in] klass Classe sendo instanciada
 * @return Instância criada
 */
ObjInstance *objMakeInstance(ObjClass *klass);

/**
 * @brief Cria um método capturado
 *
 * @param[in] receiver Classe sendo instanciada
 * @param[in] method Classe sendo instanciada
 *
 * @return O método capturado
 */
ObjBoundMethod *objMakeBoundMethod(Value receiver, ObjClosure *method);

/**
 * @brief Cria uma faixa numérica
 *
 * @param[in] start Começo da faixa numérica
 * @param[in] end Fim da faixa numérica
 *
 * @return A faixa numérica capturada
 */
ObjRange *objMakeRange(Value start, Value end);

/**
 * @brief Cria um array
 * @return O array criado
 */
ObjArray *objMakeArray(void);

/**
 * @brief Cria um hashmap
 * @return O hashmap criado
 */
ObjTable *objMakeTable(void);

/**
 * @brief Obtém a hash de uma string
 *
 * Usa o algoritmo FNV-1a
 *
 * @param[in] KEY String a partir da qual a hash será gerada
 * @param[in] LENGTH Tamanho da string
 *
 * @return Hash da string @a KEY
 */
uint32_t hashString(const char *KEY, const size_t LENGTH);

/**
 * @brief Cria um ObjString a partir de uma cópia da string @a STR
 *
 * @param[in] STR Caracteres que formarão a string
 * @param[in] LEN Tamanho da string
 *
 * @return A string criada
 */
ObjString *objCopyString(const char *STR, const size_t LEN);

/**
 * @brief Imprime um objeto
 * @param[in] VALUE Valor que será impresso
 */
void objPrint(const Value VALUE);

/**
 * @brief Compara dois objetos de tipo igual
 *
 * @param[in] A Valor A
 * @param[in] B Valor B
 *
 * @return Se os dois objetos são iguais
 */
bool objEquals(const Value A, const Value B);

#endif	// GUARD_LOXIE_OBJECT_H
