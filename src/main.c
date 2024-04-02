/**
 * @file main.c
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Ponto de entrada do programa
 */

#include "common.h"
#include "chunk.h"
#include "debug.h"

/**
 * @brief Ponto de entrada da linguagem
 *
 * Decide se a NEAT funcionará em modo REPL ou se interpretará um dado arquivo
 *
 * @param[in] argc Quantidade de argumentos
 * @param[in] argv Array de argumentos
 *
 * @return Código de erro do programa
 */
int main( int argc, const char *argv[] ) {
	Chunk chunk;
	chunkInit(&chunk);
	chunkWrite(&chunk, OP_RETURN);
	debugDisassembleChunk(&chunk, "TESTE");
	chunkFree(&chunk);

	return 0;
}
