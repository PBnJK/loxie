/**
 * @file compiler.h
 * @author Pedro B.
 * @date 2024.04.02
 *
 * @brief Compila a linguagem LOXIE
 */

#ifndef GUARD_LOXIE_COMPILER_H
#define GUARD_LOXIE_COMPILER_H

#include "chunk.h"
#include "common.h"
#include "object.h"

/**
 * @brief Compila o código-fonte para bytecode
 *
 * @param[in] SOURCE Código-fonte que será compilado
 * @return Função representando o script em si (NULL caso ocorra erro)
 */
ObjFunction *compCompile(const char *SOURCE);

/**
 * @brief Marca as partes do compilador que não serão coletadas pelo GC
 */
void compMarkRoots(void);

#endif	// GUARD_LOXIE_COMPILER_H
