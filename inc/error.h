/**
 * @file error.h
 * @author Pedro B.
 * @date 2024.04.01
 *
 * @brief Helper para registro de erros
 */

#ifndef GUARD_LOXIE_ERROR_H
#define GUARD_LOXIE_ERROR_H

#include "common.h"

/**
 * @def COLOR_RED
 * @brief Torna a cor do texto do terminal vermelha
 */
#define COLOR_RED "\033[0;31m"

/**
 * @def COLOR_YELLOW
 * @brief Torna a cor do texto do terminal amarela
 */
#define COLOR_YELLOW "\033[0;33m"

/**
 * @def COLOR_GREEN
 * @brief Torna a cor do texto do terminal verde
 */
#define COLOR_GREEN "\033[0;32m"

/**
 * @def COLOR_RESET
 * @brief Retorna a cor do texto do terminal pra cor original
 */
#define COLOR_RESET "\033[0m"

/**
 * @brief Imprime a mensagem requisitada e sai do program com @a ERR_CODE
 *
 * @param[in] LINE Linha onde o erro ocorreu
 * @param[in] MSG Mensagem de erro que será posta na tela
 */
void errFatal(const size_t LINE, const char *MSG, ...);

/**
 * @brief Imprime a mensagem requisitada como um erro, sem sair do programa
 *
 * @param[in] LINE Linha onde o erro ocorreu
 * @param[in] MSG Mensagem de erro que será posta na tela
 */
void errWarn(const size_t LINE, const char *MSG, ...);

#endif	// GUARD_LOXIE_ERROR_H
