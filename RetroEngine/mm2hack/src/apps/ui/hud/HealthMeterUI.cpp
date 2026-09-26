#include "pch.h"

#include "HealthMeterUI.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iterator>

#include <nlohmann/json.hpp>

#include "apps/foundation/NES/NESPalette.h"
#include "apps/runtime/GameContext.h"
#include "utils/hex_parser.h"
#include "utils/string_converter.h"

using json = nlohmann::json;

namespace mm2hack::apps::ui::hud
{
    namespace
    {
        // Integer written as a plain number or a "0x.." string; missing keeps `out`.
        bool read_int(const json& object, const char* key, int minimum, int maximum, int& out)
        {
            const auto value = object.find(key);
            if (value == object.end()) return true;

            std::int64_t parsed = 0;
            if (value->is_number_integer())
            {
                parsed = value->get<std::int64_t>();
            }
            else if (!value->is_string() || !utils::try_parse_hex(value->get<std::string>(), parsed))
            {
                return false;
            }
            if (parsed < minimum || parsed > maximum) return false;
            out = static_cast<int>(parsed);
            return true;
        }

        bool read_nes_color(const json& object, const char* key, int& out)
        {
            return read_int(object, key, 0x00, 0x3F, out);
        }

        // Optional { "x": .., "y": .. }; missing keeps both.
        bool read_point(const json& object, const char* key, int& x, int& y)
        {
            const auto value = object.find(key);
            if (value == object.end()) return true;
            return value->is_object() &&
                read_int(*value, "x", -1'024, 1'024, x) &&
                read_int(*value, "y", -1'024, 1'024, y);
        }

        bool parse_row_role(const std::string& text, HealthMeterRowRole& out)
        {
            if (text == "light") { out = HealthMeterRowRole::Light; return true; }
            if (text == "highlight") { out = HealthMeterRowRole::Highlight; return true; }
            if (text == "main") { out = HealthMeterRowRole::Main; return true; }
            if (text == "dark") { out = HealthMeterRowRole::Dark; return true; }
            return false;
        }

        bool parse_style(const json& source, HealthMeterStyle& out)
        {
            if (!source.is_object()) return false;

            if (!read_point(source, "position", out.x, out.y) ||
                !read_point(source, "initials", out.initials_x, out.initials_y) ||
                !read_point(source, "lives", out.lives_x, out.lives_y))
            {
                return false;
            }

            if (const auto bar = source.find("bar"); bar != source.end())
            {
                if (!bar->is_object() ||
                    !read_int(*bar, "x", 0, 1'024, out.bar_x) ||
                    !read_int(*bar, "y", 0, 1'024, out.bar_y) ||
                    !read_int(*bar, "length_px", 1, 1'024, out.bar_length_px) ||
                    !read_int(*bar, "slant_px", 0, 16, out.bar_slant_px))
                {
                    return false;
                }

                if (const auto roles = bar->find("row_roles"); roles != bar->end())
                {
                    if (!roles->is_array() || roles->empty() || roles->size() > 64) return false;
                    std::vector<HealthMeterRowRole> parsed;
                    for (const auto& role : *roles)
                    {
                        HealthMeterRowRole value{};
                        if (!role.is_string() || !parse_row_role(role.get<std::string>(), value)) return false;
                        parsed.push_back(value);
                    }
                    out.row_roles = std::move(parsed);
                    out.bar_rows = static_cast<int>(out.row_roles.size());
                }
            }

            if (const auto tones = source.find("tones"); tones != source.end())
            {
                if (!tones->is_array() || tones->empty()) return false;
                std::vector<HealthMeterTone> parsed;
                for (const auto& tone_json : *tones)
                {
                    HealthMeterTone tone{};
                    if (!tone_json.is_object() ||
                        !read_int(tone_json, "above_percent", 0, 100, tone.above_percent) ||
                        !read_nes_color(tone_json, "light", tone.light) ||
                        !read_nes_color(tone_json, "main", tone.main) ||
                        !read_nes_color(tone_json, "dark", tone.dark))
                    {
                        return false;
                    }
                    parsed.push_back(tone);
                }
                out.tones = std::move(parsed);
            }

            return
                read_nes_color(source, "highlight", out.highlight) &&
                read_nes_color(source, "chip_flash", out.chip_flash) &&
                read_nes_color(source, "chip", out.chip) &&
                read_int(source, "chip_hold_frames", 0, 600, out.chip_hold_frames) &&
                read_int(source, "chip_flash_frames", 0, 600, out.chip_flash_frames) &&
                read_int(source, "chip_drain_interval", 1, 60, out.chip_drain_interval) &&
                read_int(source, "refill_interval", 1, 60, out.refill_interval) &&
                read_int(source, "low_hp_threshold", 0, 1'000, out.low_hp_threshold) &&
                read_nes_color(source, "low_hp_flash", out.low_hp_flash) &&
                read_int(source, "low_hp_blink_frames", 1, 120, out.low_hp_blink_frames);
        }

        unsigned int to_dx_color(int nes_index, double brightness)
        {
            const auto& rgb = foundation::NES::NESPalette::GetColor(static_cast<std::size_t>(nes_index));
            const auto scaled = [brightness](int channel)
            {
                return std::clamp(static_cast<int>(std::lround(channel * brightness)), 0, 255);
            };
            return ::DxLib::GetColor(scaled(rgb.red), scaled(rgb.green), scaled(rgb.blue));
        }
    }

    bool HealthMeterUI::LoadStyle(const std::wstring& filepath, HealthMeterStyle& out)
    {
        try
        {
            std::ifstream stream(utils::wstring_to_utf8(filepath), std::ios::binary);
            if (!stream.is_open()) return false;
            const std::string source{ std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>() };

            HealthMeterStyle parsed = out;
            if (!parse_style(json::parse(source), parsed)) return false;
            out = std::move(parsed);
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    void HealthMeterUI::Reset(int hp, int max_hp) noexcept
    {
        _max_hp = std::max(1, max_hp);
        _target_hp = std::clamp(hp, 0, _max_hp);
        _shown_hp = _target_hp;
        _chip_hp = _target_hp;
        _chip_hold = 0;
    }

    void HealthMeterUI::SetTarget(int hp, int max_hp) noexcept
    {
        _max_hp = std::max(1, max_hp);
        const int clamped = std::clamp(hp, 0, _max_hp);
        if (clamped < _target_hp)
        {
            // Damage lands at once; what was lost lingers as the chip trail.
            _chip_hp = std::max(_chip_hp, _shown_hp);
            _shown_hp = clamped;
            _chip_hold = _style.chip_hold_frames;
        }
        _target_hp = clamped;
    }

    void HealthMeterUI::Tick() noexcept
    {
        ++_frame;

        // A heal fills back up gradually, like the classic energy refill.
        if (_shown_hp < _target_hp && _frame % _style.refill_interval == 0)
        {
            ++_shown_hp;
        }

        if (_chip_hp <= _shown_hp)
        {
            _chip_hp = _shown_hp;
        }
        else if (_chip_hold > 0)
        {
            --_chip_hold;
        }
        else if (_frame % _style.chip_drain_interval == 0)
        {
            --_chip_hp;
        }
    }

    const HealthMeterTone& HealthMeterUI::currentTone_() const noexcept
    {
        const int percent = (_shown_hp * 100) / _max_hp;
        for (const auto& tone : _style.tones)
        {
            if (percent > tone.above_percent)
            {
                return tone;
            }
        }
        return _style.tones.back();
    }

    int HealthMeterUI::toFillPx_(int hp) const noexcept
    {
        return (hp * _style.bar_length_px + _max_hp / 2) / _max_hp;
    }

    void HealthMeterUI::Render(rendering::sprite::SpriteManager::Id frame_sprite, std::wstring_view initials, int lives) const
    {
        auto& resources = runtime::GameContext::GetInstance().GetResourceManager();
        auto& sprites = resources.GetSpriteManager();

        if (frame_sprite != static_cast<rendering::sprite::SpriteManager::Id>(-1))
        {
            sprites.UseById(frame_sprite, 0, _style.x, _style.y);
        }

        // The fill isn't a sprite, so darken it by hand to the same fade step
        // the frame sprite is drawn at (same ratio as make_fade_palette()).
        const int max_variant = sprites.MaxVariant();
        const int variant = std::clamp(sprites.GlobalVariant(), 0, std::max(0, max_variant));
        const double brightness = max_variant > 0 ? 1.0 - static_cast<double>(variant) / max_variant : 1.0;

        const HealthMeterTone& tone = currentTone_();
        const bool low_hp_blink = _shown_hp > 0 && _shown_hp <= _style.low_hp_threshold &&
            (_frame / _style.low_hp_blink_frames) % 2 == 1;
        const bool chip_flashing = _chip_hold > _style.chip_hold_frames - _style.chip_flash_frames;
        const unsigned int chip_color = to_dx_color(chip_flashing ? _style.chip_flash : _style.chip, brightness);

        const int fill_px = toFillPx_(_shown_hp);
        const int chip_px = toFillPx_(_chip_hp);
        const int left = _style.x + _style.bar_x;

        for (int row = 0; row < _style.bar_rows; ++row)
        {
            // Row `row` reaches (bar_rows - 1 - row) * slant px further right,
            // and so does the fill front, keeping it parallel to the frame's end.
            const int slant = (_style.bar_rows - 1 - row) * _style.bar_slant_px;
            const int row_end = left + _style.bar_length_px + slant;
            const int fill_end = fill_px > 0 ? std::min(row_end, left + fill_px + slant) : left;
            const int chip_end = chip_px > 0 ? std::min(row_end, left + chip_px + slant) : left;
            const int y = _style.y + _style.bar_y + row;

            if (fill_end > left)
            {
                int nes = tone.main;
                if (low_hp_blink)
                {
                    nes = _style.low_hp_flash;
                }
                else
                {
                    switch (_style.row_roles[static_cast<std::size_t>(row)])
                    {
                    case HealthMeterRowRole::Light:     nes = tone.light; break;
                    case HealthMeterRowRole::Highlight: nes = _style.highlight; break;
                    case HealthMeterRowRole::Main:      nes = tone.main; break;
                    case HealthMeterRowRole::Dark:      nes = tone.dark; break;
                    }
                }
                ::DxLib::DrawBox(left, y, fill_end, y + 1, to_dx_color(nes, brightness), TRUE);
            }
            if (chip_end > fill_end)
            {
                ::DxLib::DrawBox(std::max(left, fill_end), y, chip_end, y + 1, chip_color, TRUE);
            }
            // Past the trail the frame art's own (empty) interior shows through.
        }

        auto& fonts = resources.GetFontTileManager();
        fonts.DrawTextImage(std::wstring(initials), _style.x + _style.initials_x, _style.y + _style.initials_y);

        const int shown_lives = std::clamp(lives, 0, 99);
        const wchar_t digits[3]{ static_cast<wchar_t>(L'0' + shown_lives / 10), static_cast<wchar_t>(L'0' + shown_lives % 10), L'\0' };
        fonts.DrawTextImage(digits, _style.x + _style.lives_x, _style.y + _style.lives_y);
    }
}
