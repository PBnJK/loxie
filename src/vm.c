/**
 * @file vm.c
 * @author Pedro B.
 * @date 2024.04.02
 *
 * @brief Implementa a máquina virtual (VM) que interpretará o nosso código
 */

#include "vm.h"

#include <math.h>
#include <stdio.h>

#include "compiler.h"
#include "debug.h"
#include "error.h"
#include "opcodes.h"
#include "value.h"

VM vm = {0}; /**< Instância global da máquina virtual */

/**
 * @brief Esvazia a pilha
 */
static void _resetStack(void) { vm.stackTop = vm.stack; }

void vmInit(void) { _resetStack(); }

void vmFree(void) {}

void vmPush(Value value) { *(vm.stackTop++) = value; }

Value vmPop(void) {
	--vm.stackTop;
	return *vm.stackTop;
}

#ifdef DEBUG_TRACE_EXECUTION

/**
 * @brief Imprime o estado atual da pilha
 */
static void _printStack(void) {
	for (Value* slot = vm.stack; slot < vm.stackTop; ++slot) {
		printf("[");
		valuePrint(*slot);
		printf("]");
	}
	printf("\n");
}
#endif

static Result _run(void) {
	while (true) {
#ifdef DEBUG_TRACE_EXECUTION
		_printStack();
		debugDisassembleInstruction(vm.chunk, (size_t)(vm.pc - vm.chunk->code));
#endif
		const uint8_t OP = READ_8();
		switch (OP) {
			case OP_CONST_16: {
				Value constant = READ_CONST_16();
				vmPush(constant);
				printf("\n");
			} break;

			case OP_CONST_32: {
				Value constant = READ_CONST_32();
				vmPush(constant);
				printf("\n");
			} break;

			case OP_ADD:
				BINARY_OP(+);
				break;

			case OP_SUB:
				BINARY_OP(-);
				break;

			case OP_MUL:
				BINARY_OP(*);
				break;

			case OP_DIV:
				BINARY_OP(/);
				break;

			case OP_MOD: {
				double b = vmPop();
				double a = vmPop();
				vmPush(fmod(a, b));
			} break;

			case OP_NEGATE:
				*(vm.stackTop - 1) = -(*(vm.stackTop - 1));
				break;

			case OP_RETURN:
				valuePrint(vmPop());
				printf("\n");
				return RESULT_OK;

			default:
				errWarn(chunkGetLine(vm.chunk, *(vm.pc - 1)),
						"OPCODE desconhecido encontrado! -> ");
				printf("%02x\n", OP);
		}
	}
}

Result vmInterpret(const char* SOURCE) {
	Chunk chunk;
	chunkInit(&chunk);

	if (!compCompile(SOURCE, &chunk)) {
		chunkFree(&chunk);
		return RESULT_COMPILER_ERROR;
	}

	vm.chunk = &chunk;
	vm.pc = vm.chunk->code;

	const Result RESULT = _run();

	chunkFree(&chunk);
	return RESULT;
}
