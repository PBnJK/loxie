/**
 * @file gc.h
 * @author Pedro B.
 * @date 2024.04.13
 *
 * @brief Coletor de lixo
 */

#ifndef GUARD_LOXIE_GC_H
#define GUARD_LOXIE_GC_H

#include "common.h"
#include "value.h"

/**
 * @brief Roda o coletor de lixo
 */
void gcCollect(void);

/**
 * @brief Marca um valor para não ser liberado
 * @param[in] value Valor que será marcado
 */
void gcMarkValue(Value value);

/**
 * @brief Marca um objeto para não ser liberado
 * @param[in] object Objeto que será marcado
 */
void gcMarkObject(Obj *object);

#endif	// GUARD_LOXIE_GC_H
