#include "pch.h"

#include "DividedGraphLoader.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <limits>
#include <nlohmann/json.hpp>
#include <string_view>
#include <utility>

namespace
{
    using DivSettings = mm2hack::apps::rendering::common::DivSettings;

    struct RGBA8 final
    {
        unsigned char r{ 0 };
        unsigned char g{ 0 };
        unsigned char b{ 0 };
        unsigned char a{ 255 };
    };

    [[nodiscard]] int mul_safe(int a, int b) noexcept
    {
        const long long value = 1LL * a * b;
        if (value > std::numeric_limits<int>::max()) return std::numeric_limits<int>::max();
        if (value < 0) return 0;
        return static_cast<int>(value);
    }

    [[nodiscard]] bool get_palette_256(int soft_image, std::array<RGBA8, 256>& out) noexcept
    {
        for (int i = 0; i < 256; ++i)
        {
            int r = 0;
            int g = 0;
            int b = 0;
            int a = 255;
            if (::DxLib::GetPaletteSoftImage(soft_image, i, &r, &g, &b, &a) != 0) return false;
            out[static_cast<std::size_t>(i)] = RGBA8{
                static_cast<unsigned char>(r),
                static_cast<unsigned char>(g),
                static_cast<unsigned char>(b),
                static_cast<unsigned char>(a)
            };
        }
        return true;
    }

    void set_palette_256(int soft_image, const std::array<RGBA8, 256>& palette) noexcept
    {
        for (int i = 0; i < 256; ++i)
        {
            const auto& color = palette[static_cast<std::size_t>(i)];
            ::DxLib::SetPaletteSoftImage(soft_image, i, color.r, color.g, color.b, color.a);
        }
    }

    void make_fade_palette(
        const std::array<RGBA8, 256>& base,
        int variant,
        int variant_count,
        std::array<RGBA8, 256>& out) noexcept
    {
        const int max_variant = std::max(variant_count - 1, 1);
        const float scale = 1.0f - (std::clamp(variant, 0, max_variant) / static_cast<float>(max_variant));
        for (std::size_t i = 0; i < base.size(); ++i)
        {
            const int r = static_cast<int>(std::lround(base[i].r * scale));
            const int g = static_cast<int>(std::lround(base[i].g * scale));
            const int b = static_cast<int>(std::lround(base[i].b * scale));
            out[i] = RGBA8{
                static_cast<unsigned char>(std::clamp(r, 0, 255)),
                static_cast<unsigned char>(std::clamp(g, 0, 255)),
                static_cast<unsigned char>(std::clamp(b, 0, 255)),
                base[i].a
            };
            if (i == 0) out[i].a = 0;
        }
    }

    void create_divided_graphs(
        int soft_image,
        const DivSettings& div,
        std::vector<int>& frames) noexcept
    {
        const int created = ::DxLib::CreateDivGraphFromSoftImage(
            soft_image,
            div.tiles_x * div.tiles_y,
            div.tiles_x,
            div.tiles_y,
            div.tile_w,
            div.tile_h,
            frames.data());
        if (created != -1) return;

        for (int ty = 0; ty < div.tiles_y; ++ty)
        {
            for (int tx = 0; tx < div.tiles_x; ++tx)
            {
                const int index = ty * div.tiles_x + tx;
                frames[static_cast<std::size_t>(index)] = ::DxLib::CreateGraphFromRectSoftImage(
                    soft_image,
                    tx * div.tile_w,
                    ty * div.tile_h,
                    div.tile_w,
                    div.tile_h);
            }
        }
    }

    [[nodiscard]] std::pair<DivSettings, int> load_settings(
        const std::wstring& json_path,
        std::wstring_view asset_label,
        const std::wstring& owner_name)
    {
        DivSettings div{};
        int variant_count = 1;

        if (!json_path.empty())
        {
            std::ifstream input(json_path);
            if (input)
            {
                nlohmann::json json;
                input >> json;
                if (json.is_null())
                {
                    THROW_EXCEPTION(L"JSON parse failed", owner_name);
                }
                if (json.contains("Loader"))
                {
                    const auto& loader = json["Loader"];
                    div.tile_w = loader.value("tileWidth", div.tile_w);
                    div.tile_h = loader.value("tileHeight", div.tile_h);
                    div.tiles_x = loader.value("tilesX", div.tiles_x);
                    div.tiles_y = loader.value("tilesY", div.tiles_y);
                    variant_count = loader.value("paletteVariants", variant_count);
                }
            }
        }

        variant_count = std::max(variant_count, 1);
        if (div.tile_w <= 0 || div.tile_h <= 0 || div.tiles_x <= 0 || div.tiles_y <= 0)
        {
            THROW_EXCEPTION(
                L"Invalid " + std::wstring(asset_label) + L" div settings in json: " + json_path,
                owner_name);
        }
        return { div, variant_count };
    }
}

namespace mm2hack::apps::rendering::common
{
    DividedGraphData load_divided_graph(
        const std::wstring& png_path,
        const std::wstring& json_path,
        std::wstring_view asset_label,
        const std::wstring& owner_name)
    {
        const auto [div, variant_count] = load_settings(json_path, asset_label, owner_name);

        if (png_path.empty())
        {
            THROW_EXCEPTION(L"The PNG file path is empty: " + png_path, owner_name);
        }
        std::ifstream png_input(png_path);
        if (!png_input)
        {
            THROW_EXCEPTION(L"File not found: " + png_path, owner_name);
        }

        const int soft_image = ::DxLib::LoadSoftImage(png_path.c_str());
        if (soft_image == -1)
        {
            THROW_EXCEPTION(L"LoadSoftImage failed for " + std::wstring(asset_label) + L": " + png_path, owner_name);
        }

        const int frame_count = mul_safe(div.tiles_x, div.tiles_y);
        std::vector<std::vector<int>> graphs_by_variant;
        graphs_by_variant.reserve(static_cast<std::size_t>(variant_count));

        std::array<RGBA8, 256> base_palette{};
        std::array<RGBA8, 256> work_palette{};
        const bool has_palette = get_palette_256(soft_image, base_palette);

        for (int variant = 0; variant < variant_count; ++variant)
        {
            std::vector<int> frames(static_cast<std::size_t>(frame_count), -1);
            if (has_palette)
            {
                make_fade_palette(base_palette, variant, variant_count, work_palette);
                set_palette_256(soft_image, work_palette);
            }

            create_divided_graphs(soft_image, div, frames);

            if (!has_palette && variant_count > 1)
            {
                const int max_variant = variant_count - 1;
                const int brightness = max_variant > 0
                    ? -static_cast<int>(std::lround((variant / static_cast<float>(max_variant)) * 255.0f))
                    : 0;
                if (brightness != 0)
                {
                    for (const int handle : frames)
                    {
                        if (handle != -1)
                        {
                            ::DxLib::GraphFilter(handle, DX_GRAPH_FILTER_HSB, 0, 0, 0, brightness);
                        }
                    }
                }
            }

            graphs_by_variant.emplace_back(std::move(frames));
        }

        if (has_palette) set_palette_256(soft_image, base_palette);
        return DividedGraphData{ div, soft_image, std::move(graphs_by_variant) };
    }
}
