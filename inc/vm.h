/**
 * @file vm.h
 * @author Pedro B.
 * @date 2024.04.02
 *
 * @brief Implementa a máquina virtual (VM) que interpretará o nosso código
 */

#ifndef GUARD_LOXIE_VM_H
#define GUARD_LOXIE_VM_H

#include "chunk.h"
#include "common.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "value_array.h"

/** Número máximo de CallFrames */
#define FRAMES_MAX 64

/**
 * @brief Enum representando o resultado de uma operação da VM
 */
typedef enum {
	RESULT_OK,			   /**< Operação não teve erros */
	RESULT_COMPILER_ERROR, /**< Ocorreu um erro enquanto compilava */
	RESULT_RUNTIME_ERROR,  /**< Ocorreu um erro enquanto rodava o código */
} Result;

/**
 * @brief Struct representando uma "janela" na pilha
 *
 * @code{.unparsed}
 * Pilha:
 *    [a . b]
 * CallFrame:
 *    [a . b .| c . d |]
 * @endcode
 */
typedef struct CallFrame {
	ObjClosure *closure; /**< Função a qual pertence este CallFrame */
	uint8_t *fp;		 /**< Frame pointer */
	Value *slots;		 /**< Variáveis neste CallFrame */
} CallFrame;

/**
 * @brief Struct representando uma máquina virtual
 */
typedef struct VM {
	CallFrame frames[FRAMES_MAX]; /**< CallFrames atuais */
	int8_t frameCount;			  /**< Quantidade de CallFrames */

	Value *stackTop; /**< O espaço vazio logo após o último item na pilha */
	Value *stack;	 /**< A pilha de valores */
	uint16_t stackMax; /**< Valor máximo em que a pilha chegará */

	Table globalNames;		 /**< Hashmap com os nomes das variáveis globais */
	ValueArray globalValues; /**< Array com os valores das variáveis globais */

	Table strings;			  /**< Hashmap de strings */
	ObjUpvalue *openUpvalues; /**< Lista de upvalues abertos */

	size_t bytesAllocated; /**< Bytes de memória alocados */
	size_t nextGC;		   /**< Limite de bytes até o proximo GC */
	bool isLocked;		   /**< Se o GC está travado */
	Obj *objects;		   /**< Lista de objetos */

	size_t grayCount; /**< Quantidade de objetos na pilha de objetos marcados*/
	size_t graySize;  /**< Tamanho da pilha de objetos marcados*/
	Obj **grayStack;  /**< Pilha de objetos marcados */
} VM;

extern VM vm; /**< Instância global da VM, para acesso externo */

/**
 * @brief Inicializa a pilha
 */
void vmInitStack(void);

/**
 * @brief Inicializa a máquina virtual
 */
void vmInit(void);

/**
 * @brief Libera a máquina virtual da memória
 */
void vmFree(void);

/**
 * @brief Retorna a linha em que a VM está atualmente
 *
 * @param[in] FRAME_IDX Índice pro frame onde o código atual está
 * @return A linha atual
 */
size_t vmGetLine(const uint8_t FRAME_IDX);

/**
 * @brief Interpreta o código-fonte
 *
 * @param[in] SOURCE Código-fonte que será interpretado
 *
 * @return Enum indicando se a operação ocorreu com sucesso
 */
Result vmInterpret(const char *SOURCE);

/**
 * @brief Empurra um valor para a pilha
 *
 * @param[in] value Valor que será colocado na pilha
 */
void vmPush(Value value);

/**
 * @brief Retira e retorna o valor no topo da pilha
 *
 * @return O valor no topo da pilha
 */
Value vmPop(void);

#endif	// GUARD_LOXIE_VM_H
