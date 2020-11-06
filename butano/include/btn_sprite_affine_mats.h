/*
 * Copyright (c) 2020 Gustavo Valiente gustavo.valiente@protonmail.com
 * zlib License, see LICENSE file.
 */

#ifndef BTN_SPRITE_AFFINE_MATS_H
#define BTN_SPRITE_AFFINE_MATS_H

#include "btn_common.h"

/**
 * @brief Sprite affine transformation matrices related functions.
 *
 * @ingroup sprite
 * @ingroup affine_mat
 */
namespace btn::sprite_affine_mats
{
    /**
     * @brief Returns the number of used sprite affine transformation matrices
     * managed with sprite_affine_mat_ptr objects.
     */
    [[nodiscard]] int used_count();

    /**
     * @brief Returns the number of available sprite affine transformation matrices
     * that can be managed with sprite_affine_mat_ptr objects.
     */
    [[nodiscard]] int available_count();
}

#endif
