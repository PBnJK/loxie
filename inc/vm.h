/**
 * @file vm.h
 * @author Pedro B.
 * @date 2024.04.02
 *
 * @brief Implementa a máquina virtual (VM) que interpretará o nosso código
 */

#ifndef GUARD_NEAT_VM_H
#define GUARD_NEAT_VM_H

#include "chunk.h"
#include "common.h"
#include "table.h"
#include "value.h"
#include "value_array.h"

/**
 * @brief Enum representando o resultado de uma operação da VM
 */
typedef enum {
	RESULT_OK,			   /**< Operação não teve erros */
	RESULT_COMPILER_ERROR, /**< Ocorreu um erro enquanto compilava */
	RESULT_RUNTIME_ERROR,  /**< Ocorreu um erro enquanto rodava o código */
} Result;

/**
 * @brief Struct representando uma máquina virtual
 */
typedef struct VM {
	Chunk *chunk; /**< Chunk sendo interpretada pela máquina virtual */
	uint8_t *pc;  /**< "Program Counter". Indica o byte sendo interpretado */

	Value *stackTop; /**< O espaço vazio logo após o último item na pilha */
	Value *stack;	 /**< A pilha de valores */
	uint16_t stackMax; /**< Valor máximo em que a pilha chegará */

	Table globalNames;		 /**< Hashmap com os nomes das variáveis globais */
	ValueArray globalValues; /**< Array com os valores das variáveis globais */

	Table strings; /**< Hashmap de strings */
	Obj *objects;  /**< Lista de objetos */
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
 * @return A linha atual
 */
size_t vmGetLine(void);

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

#endif	// GUARD_NEAT_VM_H
