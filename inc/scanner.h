/**
 * @file scanner.h
 * @author Pedro B.
 * @date 2024.04.03
 *
 * @brief Tokenizador de código da linguagem LOXIE
 */

#ifndef GUARD_LOXIE_SCANNER_H
#define GUARD_LOXIE_SCANNER_H

#include "common.h"
#include "token.h"

/**
 * @brief Inicializa o tokenizador
 *
 * @param[in] SOURCE Código-fonte que será tokenizado
 */
void scannerInit(const char *SOURCE);

/**
 * @brief Scans a token
 *
 * @return Scanned token
 */
Token scanToken(void);

#endif	// GUARD_LOXIE_SCANNER_H
