/**
 * @file common.h
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Cabeçalho com arquivos comuns a todo o projeto
 *
 * Inclue cabeçalhos da biblioteca padrão (stdint, stdbool, etc,)
 * além de definir algumas macros genéricas.
 */

#ifndef GUARD_LOXIE_COMMON_H
#define GUARD_LOXIE_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @def LOXIE_NUMBER
 * @brief Define o tipo usado para representar um número LOXIE
 *
 * Por padrão, usamos um double, que ocupa 64-bits
 * Compile o loxiec com -DLOXIE_USE_32BIT_NUMBER caso queira usar floats
 * 32-bits tradicionais
 */
#ifdef LOXIE_USE_32BIT_NUMBERS
#define LOXIE_NUMBER float
#else
#define LOXIE_NUMBER double
#endif

/** Quantidade de valores que podem ser representados por um número 8-bit */
#define UINT8_COUNT (UINT8_MAX + 1)

#endif	// GUARD_LOXIE_COMMON_H
