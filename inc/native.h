/**
 * @file native.h
 * @author Pedro B.
 * @date 2024.04.11
 *
 * @brief Funções para tratar de funções nativas
 */

#ifndef GUARD_LOXIE_NATIVE_H
#define GUARD_LOXIE_NATIVE_H

#include "common.h"
#include "object.h"
#include "value.h"

/** Erro retornado por uma função nativa */
#define ERROR_TYPE (CREATE_EMPTY())

/**
 * @brief Chama uma função nativa
 *
 * @param[in] native Função nativa sendo chamada
 * @param[in] ARG_COUNT Quantidade de argumentos passados
 *
 * @return Se a função rodou com sucesso
 */
bool nativeCall(NativeFn native, const uint8_t ARG_COUNT);

/**
 * @brief Define uma nova função nativa
 *
 * @param[in] native Função sendo definida
 * @param[in] NAME O nome da função
 * @param[in] ARGS Quantidade de argumentos
 */
void nativeDefine(NativeFn native, const char *NAME, const int16_t ARGS);

/**
 * @brief Define as funções nativas
 */
void nativeInit(void);

#endif	// GUARD_LOXIE_NATIVE_H
