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
#include "value.h"

/**
 * @brief Lê um byte e avança o ponteiro
 */
#define READ_8() (*vm.pc++)

/**
 * @brief Lê dois bytes e avança o ponteiro
 */
#define READ_16() (vm.pc += 2, (uint16_t)((vm.pc[-2] << 8) | (vm.pc[-1])))

/**
 * @brief Lê três bytes e avança o ponteiro
 */
#define READ_24() \
	(vm.pc += 3, (uint32_t)((vm.pc[-3] << 16) | (vm.pc[-2] << 8) | (vm.pc[-1])))

/**
 * @brief Lê uma constante
 */
#define READ_CONST_16() (vm.chunk->consts.values[READ_8()])

/**
 * @brief Lê uma constante longa
 */
#define READ_CONST_32() (vm.chunk->consts.values[READ_24()])

/**
 * @brief Realiza um operação binária com os dois itens no topo da pilha
 *
 * @param OPERATOR Operação (uma de +, -, *, /) que será realizada
 */
#define BINARY_OP(OPERATOR)   \
	do {                      \
		double b = vmPop();   \
		double a = vmPop();   \
		vmPush(a OPERATOR b); \
	} while (false)

/**
 * Tamanho máximo da pilha
 */
#define STACK_MAX 256

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
	Value stack[STACK_MAX]; /**< A pilha de valores */
} VM;

extern VM vm; /**< Instância global da VM, para acesso externo */

/**
 * @brief Inicializa a máquina virtual
 */
void vmInit(void);

/**
 * @brief Libera a máquina virtual da memória
 */
void vmFree(void);

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
