#include "btn_palettes_bank.h"

#include "btn_math.h"
#include "btn_span.h"
#include "btn_limits.h"
#include "btn_memory.h"
#include "btn_algorithm.h"

namespace btn
{

namespace
{
    [[maybe_unused]] bool _valid_colors_count(const span<const color>& colors_ref)
    {
        size_t colors_count = colors_ref.size();
        return colors_count >= hw::palettes::colors_per_palette() &&
                colors_count <= hw::palettes::colors() &&
                colors_count % hw::palettes::colors_per_palette() == 0;
    }
}

int palettes_bank::used_count() const
{
    int result = 0;

    for(const palette& pal : _palettes)
    {
        if(pal.usages)
        {
            result += pal.slots_count;
        }
    }

    return result;
}

optional<int> palettes_bank::find(const span<const color>& colors_ref, palette_bpp_mode bpp_mode)
{
    BTN_ASSERT(_valid_colors_count(colors_ref), "Invalid colors count: ", colors_ref.size());

    int slots_count = int(colors_ref.size() / hw::palettes::colors_per_palette());
    int bpp8_slots_count = _bpp8_slots_count();

    if(bpp_mode == palette_bpp_mode::BPP_4)
    {
        int inferior_index = _last_used_4bpp_index;
        int superior_index = inferior_index + 1;
        bool valid_inferior_index = inferior_index >= bpp8_slots_count;
        bool valid_superior_index = superior_index < hw::palettes::count();

        span<const color> visible_colors = colors_ref.subspan(1);

        while(valid_inferior_index || valid_superior_index)
        {
            if(valid_inferior_index)
            {
                palette& pal = _palettes[inferior_index];

                if(pal.usages && palette_bpp_mode(pal.bpp_mode) == palette_bpp_mode::BPP_4)
                {
                    if(visible_colors == pal.visible_colors_span())
                    {
                        ++pal.usages;
                        _last_used_4bpp_index = inferior_index;
                        return inferior_index;
                    }
                }

                --inferior_index;
                valid_inferior_index = inferior_index >= bpp8_slots_count;
            }

            if(valid_superior_index)
            {
                palette& pal = _palettes[superior_index];

                if(pal.usages)
                {
                    if(visible_colors == pal.visible_colors_span())
                    {
                        ++pal.usages;
                        _last_used_4bpp_index = superior_index;
                        return superior_index;
                    }
                }

                ++superior_index;
                valid_superior_index = superior_index < hw::palettes::count();
            }
        }
    }
    else
    {
        if(bpp8_slots_count >= slots_count)
        {
            ++_palettes[0].usages;
            return 0;
        }
    }

    return nullopt;
}

optional<int> palettes_bank::create(const span<const color>& colors_ref, palette_bpp_mode bpp_mode)
{
    BTN_ASSERT(_valid_colors_count(colors_ref), "Invalid colors count: ", colors_ref.size());

    int required_slots_count = int(colors_ref.size() / hw::palettes::colors_per_palette());
    int bpp8_slots_count = _bpp8_slots_count();

    if(bpp_mode == palette_bpp_mode::BPP_4)
    {
        int free_slots_count = 0;

        for(int index = hw::palettes::count() - 1; index >= bpp8_slots_count; --index)
        {
            palette& pal = _palettes[index];

            if(pal.usages)
            {
                free_slots_count = 0;
            }
            else
            {
                ++free_slots_count;

                if(free_slots_count == required_slots_count)
                {
                    pal = palette();
                    pal.usages = 1;
                    pal.bpp_mode = uint8_t(bpp_mode);
                    pal.slots_count = int8_t(required_slots_count);
                    set_colors_ref(index, colors_ref);
                    return index;
                }
            }
        }
    }
    else
    {
        palette& first_pal = _palettes[0];

        if(! first_pal.usages || palette_bpp_mode(first_pal.bpp_mode) == palette_bpp_mode::BPP_8)
        {
            if(bpp8_slots_count >= required_slots_count)
            {
                ++first_pal.usages;
                return 0;
            }

            if(required_slots_count <= _first_4bpp_palette_index())
            {
                if(first_pal.usages)
                {
                    ++first_pal.usages;
                }
                else
                {
                    first_pal = palette();
                    first_pal.usages = 1;
                    first_pal.bpp_mode = uint8_t(bpp_mode);
                }

                first_pal.slots_count = int8_t(required_slots_count);
                set_colors_ref(0, colors_ref);
                return 0;
            }
        }
    }

    return nullopt;
}

void palettes_bank::increase_usages(int id)
{
    palette& pal = _palettes[id];
    ++pal.usages;
}

void palettes_bank::decrease_usages(int id)
{
    palette& pal = _palettes[id];
    --pal.usages;

    if(! pal.usages)
    {
        pal = palette();
    }
}

void palettes_bank::set_colors_ref(int id, const span<const color>& colors_ref)
{
    BTN_ASSERT(int(colors_ref.size()) == colors_count(id), "Colors count mismatch: ",
               colors_ref.size(), " - ", colors_count(id));

    palette& pal = _palettes[id];
    pal.colors_ref = colors_ref.data();
    pal.update = true;
    _perform_update = true;

    if(palette_bpp_mode(pal.bpp_mode) == palette_bpp_mode::BPP_4)
    {
        _last_used_4bpp_index = id;
    }
}

void palettes_bank::reload_colors_ref(int id)
{
    palette& pal = _palettes[size_t(id)];
    pal.update = true;
    _perform_update = true;

    if(palette_bpp_mode(pal.bpp_mode) == palette_bpp_mode::BPP_4)
    {
        _last_used_4bpp_index = id;
    }
}

void palettes_bank::set_inverse_intensity(int id, fixed intensity)
{
    BTN_ASSERT(intensity >= 0 && intensity <= 1, "Invalid intensity: ", intensity);

    palette& pal = _palettes[size_t(id)];
    pal.inverse_intensity = intensity;
    pal.update = true;
    _perform_update = true;

    if(palette_bpp_mode(pal.bpp_mode) == palette_bpp_mode::BPP_4)
    {
        _last_used_4bpp_index = id;
    }
}

void palettes_bank::set_grayscale_intensity(int id, fixed intensity)
{
    BTN_ASSERT(intensity >= 0 && intensity <= 1, "Invalid intensity: ", intensity);

    palette& pal = _palettes[size_t(id)];
    pal.grayscale_intensity = intensity;
    pal.update = true;
    _perform_update = true;

    if(palette_bpp_mode(pal.bpp_mode) == palette_bpp_mode::BPP_4)
    {
        _last_used_4bpp_index = id;
    }
}

void palettes_bank::set_fade(int id, color color, fixed intensity)
{
    BTN_ASSERT(intensity >= 0 && intensity <= 1, "Invalid intensity: ", intensity);

    palette& pal = _palettes[size_t(id)];
    pal.fade_color = color;
    pal.fade_intensity = intensity;
    pal.update = true;
    _perform_update = true;

    if(palette_bpp_mode(pal.bpp_mode) == palette_bpp_mode::BPP_4)
    {
        _last_used_4bpp_index = id;
    }
}

void palettes_bank::set_rotate_count(int id, int count)
{
    BTN_ASSERT(abs(count) < colors_count(id), "Invalid count: ", count, " - ", colors_count(id));

    palette& pal = _palettes[size_t(id)];
    pal.rotate_count = int16_t(count);
    pal.update = true;
    _perform_update = true;

    if(palette_bpp_mode(pal.bpp_mode) == palette_bpp_mode::BPP_4)
    {
        _last_used_4bpp_index = id;
    }
}

void palettes_bank::set_transparent_color(const optional<color>& transparent_color)
{
    _transparent_color = transparent_color;
    _perform_update = true;
}

void palettes_bank::set_brightness(fixed brightness)
{
    BTN_ASSERT(brightness >= -1 && brightness <= 1, "Invalid brightness: ", brightness);

    _brightness = brightness;
    _perform_update = true;
}

void palettes_bank::set_contrast(fixed contrast)
{
    BTN_ASSERT(contrast >= -1 && contrast <= 1, "Invalid contrast: ", contrast);

    _contrast = contrast;
    _perform_update = true;
}

void palettes_bank::set_intensity(fixed intensity)
{
    BTN_ASSERT(intensity >= -1 && intensity <= 1, "Invalid intensity: ", intensity);

    _intensity = intensity;
    _perform_update = true;
}

void palettes_bank::set_inverse_intensity(fixed intensity)
{
    BTN_ASSERT(intensity >= 0 && intensity <= 1, "Invalid intensity: ", intensity);

    _inverse_intensity = intensity;
    _perform_update = true;
}

void palettes_bank::set_grayscale_intensity(fixed intensity)
{
    BTN_ASSERT(intensity >= 0 && intensity <= 1, "Invalid intensity: ", intensity);

    _grayscale_intensity = intensity;
    _perform_update = true;
}

void palettes_bank::set_fade(color color, fixed intensity)
{
    BTN_ASSERT(intensity >= 0 && intensity <= 1, "Invalid intensity: ", intensity);

    _fade_color = color;
    _fade_intensity = intensity;
    _perform_update = true;
}

void palettes_bank::update()
{
    int first_index = numeric_limits<int>::max();
    int last_index = 0;

    if(_perform_update)
    {
        _perform_update = false;

        int brightness = fixed_t<8>(_brightness).value();
        int contrast = fixed_t<8>(_contrast).value();
        int intensity = fixed_t<8>(_intensity).value();
        int inverse_intensity = fixed_t<5>(_inverse_intensity).value();
        int grayscale_intensity = fixed_t<5>(_grayscale_intensity).value();
        int fade_intensity = fixed_t<5>(_fade_intensity).value();
        bool update_all = brightness || contrast || intensity || inverse_intensity || grayscale_intensity ||
                fade_intensity;

        for(int index = 0, limit = hw::palettes::count(); index < limit; ++index)
        {
            palette& pal = _palettes[index];

            if(pal.update || (update_all && pal.usages))
            {
                int pal_colors_count = pal.slots_count * hw::palettes::colors_per_palette();
                pal.update = false;
                first_index = min(first_index, index);
                last_index = max(last_index, index);

                color& pal_colors_ref = _colors[index * hw::palettes::colors_per_palette()];
                memory::copy(*pal.colors_ref, pal_colors_count, pal_colors_ref);

                if(int pal_inverse_intensity = fixed_t<5>(pal.inverse_intensity).value())
                {
                    hw::palettes::inverse(pal_inverse_intensity, pal_colors_count, pal_colors_ref);
                }

                if(int pal_grayscale_intensity = fixed_t<5>(pal.grayscale_intensity).value())
                {
                    hw::palettes::grayscale(pal_grayscale_intensity, pal_colors_count, pal_colors_ref);
                }

                if(int pal_fade_intensity = fixed_t<5>(pal.fade_intensity).value())
                {
                    hw::palettes::fade(pal.fade_color, pal_fade_intensity, pal_colors_count, pal_colors_ref);
                }

                if(pal.rotate_count)
                {
                    hw::palettes::rotate(pal.rotate_count, pal_colors_count, pal_colors_ref);
                }
            }
        }

        if(_transparent_color)
        {
            _colors[0] = *_transparent_color;
            first_index = 0;
        }

        if(update_all && first_index != numeric_limits<int>::max())
        {
            color& all_colors_ref = _colors[first_index * hw::palettes::colors_per_palette()];
            int all_colors_count = (last_index - first_index + _palettes[last_index].slots_count) *
                    hw::palettes::colors_per_palette();

            if(brightness)
            {
                hw::palettes::brightness(brightness, all_colors_count, all_colors_ref);
            }

            if(contrast)
            {
                hw::palettes::contrast(contrast, all_colors_count, all_colors_ref);
            }

            if(intensity)
            {
                hw::palettes::intensity(intensity, all_colors_count, all_colors_ref);
            }

            if(inverse_intensity)
            {
                hw::palettes::inverse(inverse_intensity, all_colors_count, all_colors_ref);
            }

            if(grayscale_intensity)
            {
                hw::palettes::grayscale(grayscale_intensity, all_colors_count, all_colors_ref);
            }

            if(fade_intensity)
            {
                hw::palettes::fade(_fade_color, fade_intensity, all_colors_count, all_colors_ref);
            }
        }
    }

    if(first_index == numeric_limits<int>::max())
    {
        _first_index_to_commit.reset();
        _last_index_to_commit.reset();
    }
    else
    {
        _first_index_to_commit = first_index;
        _last_index_to_commit = last_index;
    }
}

optional<palettes_bank::commit_data> palettes_bank::retrieve_commit_data()
{
    optional<commit_data> result;

    if(_first_index_to_commit)
    {
        int first_index = *_first_index_to_commit;
        int last_index = *_last_index_to_commit;
        int colors_offset = first_index * hw::palettes::colors_per_palette();
        int colors_count = (last_index - first_index + _palettes[last_index].slots_count) *
                hw::palettes::colors_per_palette();
        result = commit_data{ _colors, colors_offset, colors_count };
        _first_index_to_commit.reset();
        _last_index_to_commit.reset();
    }

    return result;
}

int palettes_bank::_bpp8_slots_count() const
{
    const palette& first_pal = _palettes[0];

    if(first_pal.usages && palette_bpp_mode(first_pal.bpp_mode) == palette_bpp_mode::BPP_8)
    {
        return first_pal.slots_count;
    }

    return 0;
}

int palettes_bank::_first_4bpp_palette_index() const
{
    for(int index = 0; index < hw::palettes::count(); ++index)
    {
        const palette& pal = _palettes[index];

        if(pal.usages && palette_bpp_mode(pal.bpp_mode) == palette_bpp_mode::BPP_4)
        {
            return index;
        }
    }

    return hw::palettes::count();
}

}
