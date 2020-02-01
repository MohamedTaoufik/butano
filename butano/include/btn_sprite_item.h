#ifndef BTN_SPRITE_ITEM_H
#define BTN_SPRITE_ITEM_H

#include "btn_sprite_shape_size.h"
#include "btn_sprite_tiles_item.h"
#include "btn_sprite_palette_item.h"

namespace btn
{

class sprite_item
{

public:
    constexpr sprite_item(sprite_shape shape, sprite_size size, const span<const tile>& tiles,
                          const span<const color>& palette, palette_bpp_mode bpp_mode, int graphics_count) :
        sprite_item(shape, size, sprite_tiles_item(tiles, graphics_count), sprite_palette_item(palette, bpp_mode))
    {
    }

    constexpr sprite_item(sprite_shape shape, sprite_size size, const sprite_tiles_item& tiles_item,
                          const sprite_palette_item& palette_item) :
        _shape(shape),
        _size(size),
        _tiles_item(tiles_item),
        _palette_item(palette_item)
    {
        BTN_CONSTEXPR_ASSERT(int(tiles_item.tiles_ref().size()) ==
                             shape_size().tiles_count(palette_item.bpp_mode()) * tiles_item.graphics_count(),
                             "Invalid shape or size");
    }

    [[nodiscard]] constexpr sprite_shape shape() const
    {
        return _shape;
    }

    [[nodiscard]] constexpr sprite_size size() const
    {
        return _size;
    }

    [[nodiscard]] constexpr sprite_shape_size shape_size() const
    {
        return sprite_shape_size(_shape, _size);
    }

    [[nodiscard]] constexpr const sprite_tiles_item& tiles_item() const
    {
        return _tiles_item;
    }

    [[nodiscard]] constexpr const sprite_palette_item& palette_item() const
    {
        return _palette_item;
    }

    [[nodiscard]] constexpr friend bool operator==(const sprite_item& a, const sprite_item& b)
    {
        return a._shape == b._shape && a._size == b._size && a._tiles_item == b._tiles_item &&
                a._palette_item == b._palette_item;
    }

    [[nodiscard]] constexpr friend bool operator!=(const sprite_item& a, const sprite_item& b)
    {
        return ! (a == b);
    }

private:
    sprite_shape _shape;
    sprite_size _size;
    sprite_tiles_item _tiles_item;
    sprite_palette_item _palette_item;
};

}

#endif

