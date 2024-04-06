/**
 * @file main.c
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Ponto de entrada do programa
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "error.h"
#include "vm.h"

/**
 * @brief Tamanho máximo de um comando durante a sessão REPL
 */
#define REPL_BUFFER 1024

static void _runREPL(void);
static void _runFile(const char* PATH);

/**
 * @brief Ponto de entrada da linguagem
 *
 * @param[in] argc Quantidade de argumentos
 * @param[in] argv Array de argumentos
 *
 * @return Código de erro do programa
 */
int main(int argc, const char* argv[]) {
	if( argc == 1 ) {
		/* Nenhum argumento foi dado.
		 * Iniciamos uma sessão interativa
		 */
		_runREPL();
	} else if( argc == 2 ) {
		/* Um argumento foi dado.
		 * assumimos que é uma arquivo e tentamos interpretá-lo
		 */
		_runFile(argv[1]);
	} else {
		/* Uma quantidade inválida de argumentos foi dada.
		 * Sai com erro
		 */
		errFatal(
			0, "Invocacao invalida. Utilize assim:\n\t~> neatc.exe [arquivo]");
		exit(64);
	}

	vmFree();

	return 0;
}

static void _runREPL(void) {
	char buffer[REPL_BUFFER];

	while( true ) {
		printf("> ");

		if( !fgets(buffer, REPL_BUFFER, stdin) ) {
			printf("\n");
			break;
		}

		if( *buffer == '\n' ) {
			break;
		}

		vmInterpret(buffer);
	}
}

static char* _readFile(const char* PATH) {
	FILE* file = fopen(PATH, "rb");
	if( file == NULL ) {
		errFatal(0, "Nao foi possivel abrir o arquivo '%s'", PATH);
		exit(74);
	}

	fseek(file, 0L, SEEK_END);
	const size_t FILE_SIZE = ftell(file);
	rewind(file);

	char* buffer = (char*)malloc(FILE_SIZE + 1);
	if( buffer == NULL ) {
		errFatal(0, "Sem memoria o bastante para ler o arquivo '%s'", PATH);
		exit(74);
	}

	const size_t BYTES_READ = fread(buffer, sizeof(char), FILE_SIZE, file);
	if( BYTES_READ < FILE_SIZE ) {
		errFatal(0, "Sem memoria o bastante para ler o arquivo '%s'", PATH);
		exit(74);
	}

	buffer[BYTES_READ] = '\0';

	fclose(file);
	return buffer;
}

static void _runFile(const char* PATH) {
	char* source = _readFile(PATH);
	Result result = vmInterpret(source);
	free(source);

	if( result == RESULT_COMPILER_ERROR ) {
		exit(65);
	}

	if( result == RESULT_RUNTIME_ERROR ) {
		exit(64);
	}
}
