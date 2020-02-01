#include "btn_bg_palette_ptr.h"

#include "btn_palettes_bank.h"
#include "btn_palettes_manager.h"

namespace btn
{

optional<bg_palette_ptr> bg_palette_ptr::find(const span<const color>& colors_ref, palette_bpp_mode bpp_mode)
{
    optional<bg_palette_ptr> result;

    if(optional<int> id = palettes_manager::bg_palettes_bank().find(colors_ref, bpp_mode))
    {
        result = bg_palette_ptr(*id);
    }

    return result;
}

bg_palette_ptr bg_palette_ptr::create(const span<const color>& colors_ref, palette_bpp_mode bpp_mode)
{
    optional<int> id = palettes_manager::bg_palettes_bank().create(colors_ref, bpp_mode);
    BTN_ASSERT(id, "Palette create failed");

    return bg_palette_ptr(*id);
}

bg_palette_ptr bg_palette_ptr::find_or_create(const span<const color>& colors_ref, palette_bpp_mode bpp_mode)
{
    palettes_bank& bg_palettes_bank = palettes_manager::bg_palettes_bank();
    optional<int> id = bg_palettes_bank.find(colors_ref, bpp_mode);

    if(! id)
    {
        id = bg_palettes_bank.create(colors_ref, bpp_mode);
        BTN_ASSERT(id, "Palette find or create failed");
    }

    return bg_palette_ptr(*id);
}

optional<bg_palette_ptr> bg_palette_ptr::optional_create(const span<const color>& colors_ref,
                                                         palette_bpp_mode bpp_mode)
{
    optional<bg_palette_ptr> result;

    if(optional<int> id = palettes_manager::bg_palettes_bank().create(colors_ref, bpp_mode))
    {
        result = bg_palette_ptr(*id);
    }

    return result;
}

optional<bg_palette_ptr> bg_palette_ptr::optional_find_or_create(const span<const color>& colors_ref,
                                                                 palette_bpp_mode bpp_mode)
{
    palettes_bank& bg_palettes_bank = palettes_manager::bg_palettes_bank();
    optional<bg_palette_ptr> result;

    if(optional<int> id = bg_palettes_bank.find(colors_ref, bpp_mode))
    {
        result = bg_palette_ptr(*id);
    }
    else if(optional<int> id = bg_palettes_bank.create(colors_ref, bpp_mode))
    {
        result = bg_palette_ptr(*id);
    }

    return result;
}

bg_palette_ptr::bg_palette_ptr(const bg_palette_ptr& other) :
    _id(other._id)
{
    palettes_manager::bg_palettes_bank().increase_usages(_id);
}

bg_palette_ptr& bg_palette_ptr::operator=(const bg_palette_ptr& other)
{
    if(_id != other._id)
    {
        if(_id >= 0)
        {
            palettes_manager::bg_palettes_bank().decrease_usages(_id);
        }

        _id = other._id;
        palettes_manager::bg_palettes_bank().increase_usages(_id);
    }

    return *this;
}

span<const color> bg_palette_ptr::colors_ref() const
{
    return palettes_manager::bg_palettes_bank().colors_ref(_id);
}

void bg_palette_ptr::set_colors_ref(const span<const color>& colors_ref)
{
    palettes_manager::bg_palettes_bank().set_colors_ref(_id, colors_ref);
}

void bg_palette_ptr::reload_colors_ref()
{
    palettes_manager::bg_palettes_bank().reload_colors_ref(_id);
}

int bg_palette_ptr::colors_count() const
{
    return palettes_manager::bg_palettes_bank().colors_count(_id);
}

palette_bpp_mode bg_palette_ptr::bpp_mode() const
{
    return palettes_manager::bg_palettes_bank().bpp_mode(_id);
}

fixed bg_palette_ptr::inverse_intensity() const
{
    return palettes_manager::bg_palettes_bank().inverse_intensity(_id);
}

void bg_palette_ptr::set_inverse_intensity(fixed intensity)
{
    palettes_manager::bg_palettes_bank().set_inverse_intensity(_id, intensity);
}

fixed bg_palette_ptr::grayscale_intensity() const
{
    return palettes_manager::bg_palettes_bank().grayscale_intensity(_id);
}

void bg_palette_ptr::set_grayscale_intensity(fixed intensity)
{
    palettes_manager::bg_palettes_bank().set_grayscale_intensity(_id, intensity);
}

color bg_palette_ptr::fade_color() const
{
    return palettes_manager::bg_palettes_bank().fade_color(_id);
}

fixed bg_palette_ptr::fade_intensity() const
{
    return palettes_manager::bg_palettes_bank().fade_intensity(_id);
}

void bg_palette_ptr::set_fade(color color, fixed intensity)
{
    palettes_manager::bg_palettes_bank().set_fade(_id, color, intensity);
}

int bg_palette_ptr::rotate_count() const
{
    return palettes_manager::bg_palettes_bank().rotate_count(_id);
}

void bg_palette_ptr::set_rotate_count(int count)
{
    palettes_manager::bg_palettes_bank().set_rotate_count(_id, count);
}

void bg_palette_ptr::_destroy()
{
    palettes_manager::bg_palettes_bank().decrease_usages(_id);
}

}
