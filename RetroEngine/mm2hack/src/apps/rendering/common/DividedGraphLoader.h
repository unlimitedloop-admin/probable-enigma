//==============================================================================
//
//  Project: mm2hack
//  DividedGraphLoader.h
//
//  Shared loader for divided sprite and background graph atlases.
//
//==============================================================================
#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace mm2hack::apps::rendering::common
{
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
