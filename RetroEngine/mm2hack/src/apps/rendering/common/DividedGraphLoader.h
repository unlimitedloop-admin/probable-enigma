//==============================================================================
//
//  Project: mm2hack
//  DividedGraphLoader.h
//
//  Shared loader for divided sprite and background graph atlases.
//
//==============================================================================
#pragma once

#include <array>
#include <string>
#include <string_view>
#include <vector>

namespace mm2hack::apps::rendering::common
{
    struct RGBA8 final
    {
        unsigned char r{ 0 };
        unsigned char g{ 0 };
        unsigned char b{ 0 };
        unsigned char a{ 255 };
    };

    // The full 256-entry palette of an indexed (8-bit) soft image.
    using Palette256 = std::array<RGBA8, 256>;

    // Read / write a soft image's whole palette. get_palette_256() returns
    // false if the image has no palette (i.e. isn't an indexed PNG).
    [[nodiscard]] bool get_palette_256(int soft_image, Palette256& out) noexcept;
    void set_palette_256(int soft_image, const Palette256& palette) noexcept;
    // Builds the palette for fade step `variant` of `variant_count`: 0 is
    // `base` unchanged, variant_count-1 is fully black. Index 0 is always
    // made transparent (the atlases' shared transparent-color convention).
    void make_fade_palette(
        const Palette256& base,
        int variant,
        int variant_count,
        Palette256& out) noexcept;

    struct DivSettings final
    {
        int tile_w{ 0 };
        int tile_h{ 0 };
        int tiles_x{ 0 };
        int tiles_y{ 0 };
    };

    struct DividedGraphData final
    {
        DivSettings div{};
        int soft_image{ -1 };
        std::vector<std::vector<int>> graphs_by_variant{};
    };

    [[nodiscard]] DividedGraphData load_divided_graph(
        const std::wstring& png_path,
        const std::wstring& json_path,
        std::wstring_view asset_label,
        const std::wstring& owner_name);
}
