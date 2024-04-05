/**
 * @file compiler.h
 * @author Pedro B.
 * @date 2024.04.02
 *
 * @brief Compila a linguagem NEAT
 */

#ifndef GUARD_NEAT_COMPILER_H
#define GUARD_NEAT_COMPILER_H

#include "chunk.h"
#include "common.h"

/**
 * @brief Compila o código-fonte para bytecode
 *
 * @param[in] SOURCE Código-fonte que será compilado
 * @param[in] chunk todo
 *
 * @return Verdadeiro se a compilação ocorreu sem erros
 */
bool compCompile(const char *SOURCE, Chunk *chunk);

#endif	// GUARD_NEAT_COMPILER_H
