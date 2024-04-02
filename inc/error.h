/**
 * @file error.h
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Helper para registro de erros
 */

#ifndef GUARD_NEAT_ERROR_H
#define GUARD_NEAT_ERROR_H

#include "common.h"

/**
 * @brief Imprime a mensagem requisitada e sai do program com @a ERR_CODE
 *
 * @note Use somente quando o erro for inrecuperável, como um malloc retornar
 * NULL ou um overflow da pilha
 *
 * @param[in] MSG Mensagem de erro que será posta na tela
 * @param[in] ERR_CODE Código de erro (veja https://man.freebsd.org/cgi/man.cgi?query=sysexits&manpath=FreeBSD+4.3-RELEASE)
 */
void errFatal( const char *MSG, const uint8_t ERR_CODE );

#endif // GUARD_NEAT_ERROR_H

