#include "pch.h"

#include "FontTileManager.h"

#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <string_view>
#include "config/GameAssets.h"
#include "utils/string_converter.h"

namespace
{
    struct RGBA8 { unsigned char r, g, b, a; };

    bool get_palette_256(int soft, std::array<RGBA8, 256>& out)
    {
        for (int i = 0; i < 256; ++i)
        {
            int r = 0, g = 0, b = 0, a = 255;
            if (::DxLib::GetPaletteSoftImage(soft, i, &r, &g, &b, &a) != 0) return false;
            out[(size_t)i] = RGBA8{ (unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a };
        }
        return true;
    }

    void set_palette_256(int soft, const std::array<RGBA8, 256>& pal)
    {
        for (int i = 0; i < 256; ++i)
        {
            const auto& c = pal[(size_t)i];
            ::DxLib::SetPaletteSoftImage(soft, i, c.r, c.g, c.b, c.a);
        }
    }

    void make_fade_variant_from_base(const std::array<RGBA8, 256>& base, int variantIndex, int variantCount, std::array<RGBA8, 256>& out)
    {
        // Only one variant (no fade)
        const int V = std::max(variantCount - 1, 1);
        // v=0 -> s=1.0 (brightest) / v=V -> s=0.0 (darkest = black)
        const float s = 1.0f - (std::clamp(variantIndex, 0, V) / float(V));

        for (size_t i = 0; i < 256; ++i)
        {
            const int r = int(std::lround(base[i].r * s));
            const int g = int(std::lround(base[i].g * s));
            const int b = int(std::lround(base[i].b * s));
            out[i] = RGBA8{ (unsigned char)std::clamp(r, 0, 255),
                            (unsigned char)std::clamp(g, 0, 255),
                            (unsigned char)std::clamp(b, 0, 255),
                            base[i].a };
        }
    }
}

namespace mm2hack::apps::graphics::fonts
{
    FontTileManager::~FontTileManager()
    {
        for (auto& [_, set] : _fontSets)
        {
            for (auto& gv : set.graphsByVariant)
                for (auto& [__, g] : gv) ::DxLib::DeleteGraph(g);
            if (set.softImage != -1) ::DxLib::DeleteSoftImage(set.softImage);
        }
    }

    void FontTileManager::Load(const std::wstring& name, std::wstring_view pngPath, std::wstring_view jsonPath)
    {
        Remove(name);
        ParsedMeta meta = ParseMeta_(std::wstring(jsonPath), name);
        if (!pngPath.empty()) meta.pngPath = std::wstring(pngPath);
        if (meta.pngPath.empty()) meta.pngPath = DerivePngFromJsonPath_(std::wstring(jsonPath));
        if (meta.pngPath.empty()) THROW_EXCEPTION(L"PNG path is not specified", kClassName);

        const int soft = ::DxLib::LoadSoftImage(meta.pngPath.c_str());
        if (soft == -1) THROW_EXCEPTION(L"LoadSoftImage failed", kClassName);

        CreateFontGraphs_(name, soft, meta.tile_w, meta.tile_h, meta.tiles_x, meta.tiles_y, meta.charToIndex, meta.variant_count, meta.nes_fade_step);
    }

    void FontTileManager::Load(const std::wstring& name, std::wstring_view jsonPath)
    {
        Load(name, std::wstring_view{}, jsonPath);
    }

    void FontTileManager::Remove(const std::wstring& name)
    {
        auto it = _fontSets.find(name);
        if (it == _fontSets.end()) return;
        auto& set = it->second;
        for (auto& gv : set.graphsByVariant)
            for (auto& [__, g] : gv) ::DxLib::DeleteGraph(g);
        if (set.softImage != -1) ::DxLib::DeleteSoftImage(set.softImage);
        _fontSets.erase(it);
    }

    void FontTileManager::DrawTextImage(const std::wstring& text, int x, int y, int spacing) const
    {
        int cursorX = x;
        for (wchar_t wc : text)
        {
            char ch = (wc >= 0 && wc <= 0x7F) ? static_cast<char>(wc) : '?';
            if (ch == ' ') { cursorX += spacing; continue; }

            for (const auto& [_, set] : _fontSets)
            {
                const int v = (_globalVariant < set.variantCount) ? _globalVariant : 0;
                if (v < 0 || v >= (int)set.graphsByVariant.size()) continue;
                const auto& gv = set.graphsByVariant[(size_t)v];

                // Leave the search order:
                auto it = gv.find(ch);
                // If not found, try uppercase
                if (it == gv.end())
                {
                    char up = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
                    if (up != ch) it = gv.find(up);
                }
                // Else try lowercase
                if (it == gv.end())
                {
                    char lo = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
                    if (lo != ch) it = gv.find(lo);
                }

                if (it != gv.end())
                {
                    ::DxLib::DrawGraph(cursorX, y, it->second, TRUE);
                    break;
                }
            }
            cursorX += spacing;
        }
    }

    void FontTileManager::ChangeColoredImage(const std::wstring& setName, char ch, uint8_t r, uint8_t g, uint8_t b)
    {
        auto itSet = _fontSets.find(setName);
        if (itSet == _fontSets.end()) return;
        auto& set = itSet->second;

        auto itIdx = set.charToIndex.find(ch); // Strict match
        if (itIdx == set.charToIndex.end())
        {
            const char up = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
            if (auto it2 = set.charToIndex.find(up); it2 != set.charToIndex.end())
            {
                ch = up; itIdx = it2;
            }
            else
            {
                const char lo = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
                if (auto it3 = set.charToIndex.find(lo); it3 != set.charToIndex.end())
                {
                    ch = lo; itIdx = it3;
                }
                else
                {
                    return;
                }
            }
        }
        const int idx = itIdx->second;

        constexpr int paletteIndex = 1;
        ::DxLib::SetPaletteSoftImage(set.softImage, paletteIndex, r, g, b, 255);

        const int sx = (idx % set.tiles_x) * set.tile_w;
        const int sy = (idx / set.tiles_x) * set.tile_h;
        const int newGraph = ::DxLib::CreateGraphFromRectSoftImage(set.softImage, sx, sy, set.tile_w, set.tile_h);
        if (newGraph != -1)
        {
            const int v = (_globalVariant < set.variantCount) ? _globalVariant : 0;
            auto& gv = set.graphsByVariant[(size_t)v];
            if (auto itG = gv.find(ch); itG != gv.end()) ::DxLib::DeleteGraph(itG->second);
            gv[ch] = newGraph;
        }
    }

    void FontTileManager::SetUp()
    {
        Load(L"alphabet", MM2H_GRAPHICS(Alphabet), MM2H_PROPERTIES(Alphabet));
        Load(L"numbers", MM2H_GRAPHICS(Numbers), MM2H_PROPERTIES(Numbers));
    }

    void FontTileManager::ShutDown()
    {
        Remove(L"alphabet");
        Remove(L"numbers");
    }

    // --- helpers ---
    FontTileManager::ParsedMeta FontTileManager::ParseMeta_(const std::wstring& jsonPath, const std::wstring& setName)
    {
        ParsedMeta out{};
        std::filesystem::path jpath(jsonPath);

        bool jsonLoaded = false;
        std::ifstream ifs(jpath);
        if (ifs)
        {
            nlohmann::json j; ifs >> j; jsonLoaded = true;
            const auto& root = j;

            const nlohmann::json* loader = nullptr;
            if (auto it = root.find("Loader"); it != root.end() && it->is_object()) loader = &(*it);

            auto geti = [&](const char* key, int& dst)
                {
                    if (loader && loader->contains(key)) dst = (*loader)[key].get<int>();
                    else if (root.contains(key)) dst = root[key].get<int>();
                };
            geti("tileWidth", out.tile_w);
            geti("tileHeight", out.tile_h);
            geti("tilesX", out.tiles_x);
            geti("tilesY", out.tiles_y);
            geti("paletteVariants", out.variant_count);
            geti("nesFadeStep", out.nes_fade_step);

            auto get_png = [&](const nlohmann::json& obj)->std::wstring
                {
                    if (auto it = obj.find("png"); it != obj.end() && it->is_string())
                        return utils::utf8_to_wstring(it->get<std::string>());
                    return L"";
                };
            std::wstring pngW;
            if (loader) pngW = get_png(*loader);
            if (pngW.empty()) pngW = get_png(root);
            if (!pngW.empty()) out.pngPath = ConcatPath_(jpath.parent_path(), pngW);

            if (auto it = root.find("map"); it != root.end() && it->is_object())
            {
                for (auto it2 = it->begin(); it2 != it->end(); ++it2)
                {
                    if (!it2.value().is_number_integer()) continue;
                    const std::string& k = it2.key();
                    if (k.empty()) continue;
                    const char ch = static_cast<char>(k[0]);
                    out.charToIndex[ch] = it2.value().get<int>();
                }
            }
        }

        if (out.charToIndex.empty())
        {
            // Probably so... (If you change the graphics, you will need to review this)
            if (setName == L"alphabet")
            {
                for (int i = 0; i < 26; ++i) out.charToIndex['A' + i] = i;
                out.charToIndex['r'] = 26; out.charToIndex['.'] = 27; out.charToIndex[','] = 28;
                out.charToIndex['\''] = 29; out.charToIndex['!'] = 30; out.charToIndex['?'] = 31;
                if (!jsonLoaded) { out.tile_w = 8; out.tile_h = 8; out.tiles_x = 16; out.tiles_y = 2; }
            }
            else if (setName == L"numbers")
            {
                for (int i = 0; i < 10; ++i) out.charToIndex['0' + i] = i;
                if (!jsonLoaded) { out.tile_w = 8; out.tile_h = 8; out.tiles_x = 10; out.tiles_y = 1; }
            }
        }

        if (out.pngPath.empty()) out.pngPath = DerivePngFromJsonPath_(jsonPath);
        if (out.variant_count <= 0) out.variant_count = 1;
        if (out.nes_fade_step < 0)  out.nes_fade_step = 0;
        return out;
    }

    std::wstring FontTileManager::DerivePngFromJsonPath_(const std::wstring& jsonPath)
    {
        std::filesystem::path p(jsonPath); p.replace_extension(L".png"); return p.wstring();
    }

    std::wstring FontTileManager::ConcatPath_(const std::filesystem::path& baseDir, const std::wstring& rel)
    {
        if (rel.empty()) return baseDir.wstring();
        std::filesystem::path r(rel); if (r.is_absolute()) return r.wstring();
        return (baseDir / r).wstring();
    }

    void FontTileManager::CreateFontGraphs_(const std::wstring& name, int softImage,
        int tile_w, int tile_h, int tiles_x, int tiles_y,
        const std::map<char, int>& charIndexMap,
        int variant_count, int fade_step)
    {
        FontSet set;
        set.softImage = softImage;
        set.tile_w = tile_w; set.tile_h = tile_h; set.tiles_x = tiles_x; set.tiles_y = tiles_y;
        set.variantCount = variant_count;
        set.charToIndex = charIndexMap;

        // Get palette (fallback if failed)
        std::array<RGBA8, 256> basePal{}, workPal{};
        const bool hasPal = get_palette_256(softImage, basePal);

        // First, generate variants for v=0..N-1
        set.graphsByVariant.resize((size_t)variant_count);
        for (int v = 0; v < variant_count; ++v)
        {
            std::vector<int> handles;

            if (hasPal)
            {
                // Apply palette -> batch split
                make_fade_variant_from_base(basePal, v, variant_count, workPal);
                set_palette_256(softImage, workPal);

                const int total = tiles_x * tiles_y;
                std::vector<int> tmp(total, -1);
                const int res = ::DxLib::CreateDivGraphFromSoftImage(
                    softImage, total, tiles_x, tiles_y, tile_w, tile_h, tmp.data());
                if (res != 0) THROW_EXCEPTION(L"CreateDivGraphFromSoftImage failed", kClassName);
                handles = std::move(tmp);
            }
            else
            {
                handles.assign(tiles_x * tiles_y, -1);
                for (const auto& [ch, idx] : charIndexMap)
                {
                    const int sx = (idx % tiles_x) * tile_w;
                    const int sy = (idx / tiles_x) * tile_h;
                    int h = ::DxLib::CreateGraphFromRectSoftImage(softImage, sx, sy, tile_w, tile_h);
                    if (h != -1 && variant_count > 1)
                    {
                        const int V = variant_count - 1;
                        // v=0 -> bri=0 (brightest) / v=V -> bri=-255 (darkest = black)
                        const int bri = (V > 0) ? -int(std::lround((v / float(V)) * 255.0f)) : 0;
                        if (bri != 0) ::DxLib::GraphFilter(h, DX_GRAPH_FILTER_HSB, 0, 0, 0, bri);
                    }
                    handles[(size_t)idx] = h;
                }
            }

            auto& gv = set.graphsByVariant[(size_t)v];
            for (const auto& [ch, idx] : charIndexMap)
            {
                const int h = (idx >= 0 && idx < (int)handles.size()) ? handles[(size_t)idx] : -1;
                if (h != -1) gv[ch] = h;
            }
        }

        // Restore palette
        if (hasPal) set_palette_256(softImage, basePal);

        _fontSets[name] = std::move(set);
    }

    int FontTileManager::MaxVariant() const noexcept
    {
        int result = -1;
        for (const auto& [_, set] : _fontSets)
        {
            const int mv = std::max(0, set.variantCount - 1);
            result = (result < 0) ? mv : std::min(result, mv);
        }
        return std::max(0, result);
    }

    void FontTileManager::SetGlobalVariantClamped(int v) noexcept
    {
        const int mv = MaxVariant();
        _globalVariant = std::max(0, std::min(v, mv));
    }

    int FontTileManager::VariantCountByName(const std::wstring& setName) const
    {
        auto it = _fontSets.find(setName);
        if (it == _fontSets.end()) return 0;
        return it->second.variantCount;
    }
}