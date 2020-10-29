/*
 * Copyright (c) 2020 Gustavo Valiente gustavo.valiente@protonmail.com
 * zlib License, see LICENSE file.
 */

#ifndef BTN_SPRITE_DOUBLE_SIZE_MODE_H
#define BTN_SPRITE_DOUBLE_SIZE_MODE_H

#include "btn_common.h"

namespace btn
{

/**
 * @brief Specifies the area a sprite uses to be drawn when it has an attached sprite_affine_mat_ptr.
 *
 * @ingroup sprite
 * @ingroup affine_mat
 */
enum class sprite_double_size_mode
{
    AUTO, //!< The sprite uses 2x the area specified by its shape and size only when its sprite_affine_mat_ptr requires it.
    ENABLED, //!< The sprite always uses 2x the area specified by its shape and size.
    DISABLED //!< The sprite never uses more pixels than the area specified by its shape and size.
};

}

#endif
