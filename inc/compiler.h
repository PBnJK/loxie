/**
 * @file compiler.h
 * @author Pedro B.
 * @date 2024.04.02
 *
 * @brief Compila a linguagem NEAT
 */

#ifndef GUARD_NEAT_COMPILER_H
#define GUARD_NEAT_COMPILER_H

#include "common.h"

/**
 * @brief Compila o código-fonte para bytecode
 *
 * @param[in] SOURCE Código-fonte que será compilado
 */
void compCompile( const char *SOURCE );

#endif // GUARD_NEAT_COMPILER_H
