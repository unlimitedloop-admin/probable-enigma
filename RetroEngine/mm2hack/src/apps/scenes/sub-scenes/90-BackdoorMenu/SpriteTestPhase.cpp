#include "pch.h"

#include "SpriteTestPhase.h"

#include <cstdint>
#include <fstream>
#include <iterator>

#include <nlohmann/json.hpp>

#include "apps/resources/ResourceManager.h"
#include "apps/runtime/GameContext.h"
#include "apps/scenes/PhaseFadeController.h"
#include "apps/world/entity/enemy/lists/EnemyLists.h"
#include "BackdoorMenuCatalog.h"
#include "BackdoorMenuPhase.h"
#include "config/GameAssets.h"
#include "core/assembly/StateProvider.h"
#include "input/Jpbtn.h"
#include "utils/string_converter.h"

using json = nlohmann::json;

namespace mm2hack::apps::scenes
{
    namespace BackdoorMenu_
    {
        using world::entity::enemy::animation::AnimationClip;
        using world::entity::enemy::animation::AnimationFrame;

        namespace
        {
            bool TryParseFrame(const json& source, AnimationFrame& out)
            {
                if (!source.is_object()) return false;

                const auto tile = source.find("tile");
                if (tile == source.end() || !tile->is_number_integer()) return false;
                out.tile = tile->get<int>();

                const auto wait = source.find("wait");
                if (wait == source.end()) return false;
                if (wait->is_string())
                {
                    if (wait->get<std::string>() != "hold") return false;
                    out.wait_frames = AnimationFrame::kHoldFrames;
                    return true;
                }
                if (!wait->is_number_integer()) return false;
                const std::int64_t parsed = wait->get<std::int64_t>();
                if (parsed < 1 || parsed > 3'600) return false;
                out.wait_frames = static_cast<int>(parsed);
                return true;
            }

            bool TryParsePattern(const json& source, SpriteTestPattern& out)
            {
                if (!source.is_object()) return false;

                const auto label = source.find("label");
                if (label == source.end() || !label->is_string()) return false;
                out.label = utils::utf8_to_wstring(label->get<std::string>());

                out.clip.loop = false;
                const auto loop = source.find("loop");
                if (loop != source.end())
                {
                    if (!loop->is_boolean()) return false;
                    out.clip.loop = loop->get<bool>();
                }

                const auto frames = source.find("frames");
                if (frames == source.end() || !frames->is_array() || frames->empty()) return false;
                for (const auto& frame_json : *frames)
                {
                    AnimationFrame frame{};
                    if (!TryParseFrame(frame_json, frame)) return false;
                    out.clip.frames.push_back(frame);
                }
                return true;
            }

            // Loads SPRITE TEST's own lightweight pattern-list JSON (distinct
            // from EnemyDefinitionLoader's game-logic animation state graph --
            // see SpriteTestPhase.h's header comment). Returns an empty list
            // (not a hard failure) so the screen still opens, just with
            // nothing to preview, if the file is missing/malformed -- this is
            // a debug tool, not gameplay-critical.
            std::vector<SpriteTestPattern> LoadPatterns(const std::wstring& filepath)
            {
                std::vector<SpriteTestPattern> result;
                try
                {
                    std::ifstream stream(utils::wstring_to_utf8(filepath), std::ios::binary);
                    if (!stream.is_open()) return result;

                    const std::string source{
                        std::istreambuf_iterator<char>(stream),
                        std::istreambuf_iterator<char>()
                    };
                    if (stream.bad()) return result;

                    const json document = json::parse(source.begin(), source.end());
                    const auto patterns = document.find("patterns");
                    if (patterns == document.end() || !patterns->is_array()) return result;

                    for (const auto& pattern_json : *patterns)
                    {
                        SpriteTestPattern pattern{};
                        if (TryParsePattern(pattern_json, pattern))
                        {
                            result.push_back(std::move(pattern));
                        }
                    }
                }
                catch (const std::exception&)
                {
                    return {};
                }
                return result;
            }

            std::wstring PatternJsonPathFor(SpriteTestCharacterId character)
            {
                switch (character)
                {
                case SpriteTestCharacterId::Rockman: return L"assets\\data\\sprite_test\\ROCKMAN.json";
                case SpriteTestCharacterId::Metall:  return L"assets\\data\\sprite_test\\METALL.json";
                }
                return L"";
            }

            std::wstring LabelFor(SpriteTestCharacterId character)
            {
                switch (character)
                {
                case SpriteTestCharacterId::Rockman: return L"ROCKMAN";
                case SpriteTestCharacterId::Metall:  return L"METALL";
                }
                return L"";
            }
        }

        SpriteTestPhase::SpriteTestPhase(BackdoorMenu& owner, SpriteTestCharacterId character)
            : owner(owner), character_(character), characterLabel_(LabelFor(character)),
              cursorAnim_(owner.Cursor())
        {
            auto& sprites = owner.Resource()->GetSpriteManager();
            switch (character_)
            {
            case SpriteTestCharacterId::Rockman:
                spriteId_ = sprites.Load(L"Player", MM2H_GRAPHICS(Player), MM2H_GRAPHPROPS(Player));
                break;
            case SpriteTestCharacterId::Metall:
                spriteId_ = sprites.Load(L"MetallArmy", MM2H_GRAPHICS(MetallArmy), MM2H_GRAPHPROPS(MetallArmy));
                break;
            }

            frameCount_ = sprites.FrameCountById(spriteId_);
            patterns_ = LoadPatterns(PatternJsonPathFor(character_));

            buildRows_();
        }

        void SpriteTestPhase::buildRows_()
        {
            rows_.clear();
            int y = 16;

            rows_.push_back({ L"PALETTE TABLE", RowKind::PaletteTable, -1, true, y }); y += 10;
            rows_.push_back({ L"ORIGINAL SHEET", RowKind::OriginalSheet, -1, true, y }); y += 10;
            y += 10; // Blank line, matches the target layout's gap before the pattern list.

            for (int i = 0; i < static_cast<int>(patterns_.size()); ++i)
            {
                rows_.push_back({ patterns_[static_cast<std::size_t>(i)].label, RowKind::Pattern, i, true, y });
                y += 10;
            }
            rows_.push_back({ L"BACK", RowKind::Back, -1, true, y });

            selectableRows_.clear();
            for (int i = 0; i < static_cast<int>(rows_.size()); ++i)
            {
                if (rows_[static_cast<std::size_t>(i)].selectable) selectableRows_.push_back(i);
            }

            cursorCtl_.SetItemCount(static_cast<int>(selectableRows_.size()));
            cursorCtl_.SetIndex(0);
        }

        void SpriteTestPhase::Update()
        {
            owner.StarField().UpdateStars();
            owner.Cursor().Update();

            if (!owner.Fader().InputEnabled()) return;

            auto& input = owner.Input();
            const bool verified = input->JustPressed(JPBTN::START) || input->JustPressed(JPBTN::A);
            const bool cancelled = input->JustPressed(JPBTN::B) || input->JustPressed(JPBTN::BACK);

            if (subState_ == SubState::Preview)
            {
                tickClip_();
                if (verified) { restartClip_(); }
                if (cancelled)
                {
                    owner.Resource()->GetAudioManager().PlaySe(L"plink_ring");
                    subState_ = SubState::PatternSelect;
                }
                return;
            }

            if (subState_ == SubState::TileBrowse)
            {
                // Reel-counter style: wraps at both ends instead of clamping.
                if (frameCount_ > 0)
                {
                    if (input->JustPressed(JPBTN::DOWN)) { tileBrowseIndex_ = (tileBrowseIndex_ + 1) % frameCount_; }
                    if (input->JustPressed(JPBTN::UP)) { tileBrowseIndex_ = (tileBrowseIndex_ - 1 + frameCount_) % frameCount_; }
                }
                if (cancelled)
                {
                    owner.Resource()->GetAudioManager().PlaySe(L"plink_ring");
                    subState_ = SubState::PatternSelect;
                }
                return;
            }

            if (subState_ == SubState::PaletteTable)
            {
                const int count = static_cast<int>(paletteLabels_.size());
                if (count > 0)
                {
                    if (input->JustPressed(JPBTN::RIGHT)) { paletteCursor_ = (paletteCursor_ + 1) % count; }
                    if (input->JustPressed(JPBTN::LEFT)) { paletteCursor_ = (paletteCursor_ - 1 + count) % count; }
                }
                if (verified && count > 0)
                {
                    // Commit: this palette becomes the sprite every other
                    // mode (ORIGINAL SHEET, pattern preview) draws from.
                    owner.Resource()->GetAudioManager().PlaySe(L"plink_ring");
                    spriteId_ = paletteSpriteIds_[static_cast<std::size_t>(paletteCursor_)];
                    subState_ = SubState::PatternSelect;
                    return;
                }
                if (cancelled)
                {
                    owner.Resource()->GetAudioManager().PlaySe(L"plink_ring");
                    subState_ = SubState::PatternSelect;
                }
                return;
            }

            // SubState::PatternSelect
            if (input->JustPressed(JPBTN::DOWN)) { cursorCtl_.Move(+1); }
            if (input->JustPressed(JPBTN::UP)) { cursorCtl_.Move(-1); }

            if (verified)
            {
                owner.Resource()->GetAudioManager().PlaySe(L"plink_ring");
                activateRow_(rows_[static_cast<std::size_t>(selectableRows_[static_cast<std::size_t>(cursorCtl_.Index())])]);
                return;
            }
            if (cancelled)
            {
                owner.Resource()->GetAudioManager().PlaySe(L"plink_ring");
                goBackToCharacterSelect_();
            }
        }

        void SpriteTestPhase::activateRow_(const MenuRow& row) noexcept
        {
            switch (row.kind)
            {
            case RowKind::PaletteTable:
                enterPaletteTable_();
                break;
            case RowKind::OriginalSheet:
                enterTileBrowse_();
                break;
            case RowKind::Pattern:
                startPreview_(row.patternIndex);
                break;
            case RowKind::Back:
                goBackToCharacterSelect_();
                break;
            }
        }

        void SpriteTestPhase::RenderWorld()
        {
            owner.StarField().DrawStars();

            auto& fonts = owner.Resource()->GetFontTileManager();
            for (const auto& row : rows_)
            {
                fonts.DrawTextImage(row.label.c_str(), 30, row.y);
            }

            if (subState_ == SubState::PatternSelect)
            {
                const int row = selectableRows_[static_cast<std::size_t>(cursorCtl_.Index())];
                owner.Cursor().DrawAt(16, rows_[static_cast<std::size_t>(row)].y);
            }

            SpriteId drawSpriteId = spriteId_;
            int tile = idleTile_();
            if (subState_ == SubState::Preview)
            {
                if (const auto* pattern = currentPattern_(); pattern != nullptr && !pattern->clip.frames.empty())
                {
                    tile = pattern->clip.frames[static_cast<std::size_t>(frameIndex_)].tile;
                }
            }
            else if (subState_ == SubState::TileBrowse)
            {
                tile = tileBrowseIndex_;
                std::wstring number = std::to_wstring(tileBrowseIndex_);
                if (number.size() < 2) { number = L"0" + number; }
                fonts.DrawTextImage(number.c_str(), 170, 80);
            }
            else if (subState_ == SubState::PaletteTable)
            {
                constexpr int kPaletteY = 110;
                constexpr int kPaletteStartX = 30;
                constexpr int kPaletteSpacingX = 20;
                for (std::size_t i = 0; i < paletteLabels_.size(); ++i)
                {
                    fonts.DrawTextImage(
                        paletteLabels_[i].c_str(), kPaletteStartX + static_cast<int>(i) * kPaletteSpacingX, kPaletteY);
                }
                if (!paletteSpriteIds_.empty())
                {
                    owner.Cursor().DrawAt(kPaletteStartX + paletteCursor_ * kPaletteSpacingX - 14, kPaletteY);
                    drawSpriteId = paletteSpriteIds_[static_cast<std::size_t>(paletteCursor_)];
                }
            }

            auto& sprites = owner.Resource()->GetSpriteManager();
            sprites.UseById(drawSpriteId, tile, 170, 100);
        }

        void SpriteTestPhase::RenderOverlay() { /* nothing */ }

        bool SpriteTestPhase::Save(core::save::StateWriter& writer) const
        {
            (void)writer;
            return false; // Debug-only screen; see the class comment in SpriteTestPhase.h.
        }

        bool SpriteTestPhase::Load(core::save::StateReader& reader)
        {
            (void)reader;
            return false; // Never reconstructed from a save (BackdoorMenu.cpp's Load() switch has no case for it).
        }

        void SpriteTestPhase::goBackToCharacterSelect_() noexcept
        {
            PhaseFadePlan next(
                5,   // preBlackHold
                20,  // fadeInFrames
                0,   // preFadeOutHold
                20,  // fadeOutFrames
                0,   // postBlackHold
                FadeLayerMask::All
            );
            constexpr int kSpriteTestTopItemIndex = 6;
            owner.QueuePhase(
                std::make_unique<InsideMenuPhase>(
                    owner,
                    MenuCursor{ {16, 16, 10}, static_cast<int>(kTopMenuTitles.size()) },
                    kSpriteTestTopItemIndex),
                next);
        }

        void SpriteTestPhase::startPreview_(int patternIndex) noexcept
        {
            previewingPatternIndex_ = patternIndex;
            subState_ = SubState::Preview;
            restartClip_();
        }

        void SpriteTestPhase::enterTileBrowse_() noexcept
        {
            subState_ = SubState::TileBrowse;
            tileBrowseIndex_ = 0;
        }

        void SpriteTestPhase::enterPaletteTable_() noexcept
        {
            if (!palettePresetsLoaded_)
            {
                loadPalettePresets_();
                palettePresetsLoaded_ = true;
            }
            subState_ = SubState::PaletteTable;
            paletteCursor_ = 0;
        }

        void SpriteTestPhase::loadPalettePresets_()
        {
            using world::entity::enemy::EnemyKind;
            using world::entity::enemy::animation::EnemyDefinitionCatalog;

            paletteLabels_.clear();
            paletteSpriteIds_.clear();

            auto& sprites = owner.Resource()->GetSpriteManager();

            if (character_ == SpriteTestCharacterId::Metall)
            {
                EnemyDefinitionCatalog& catalog =
                    runtime::GameContext::GetInstance().GetResourceManager().GetEnemyDefinitionCatalog();
                const auto* def = catalog.Find(EnemyKind::Met);
                if (def == nullptr)
                {
                    // Not loaded yet -- SPRITE TEST can be reached before any
                    // stage has touched the real gameplay catalog.
                    catalog.LoadForKind(EnemyKind::Met, L"assets\\data\\enemies\\METALL.json");
                    def = catalog.Find(EnemyKind::Met);
                }

                if (def != nullptr && !def->palette_presets.empty())
                {
                    for (std::size_t i = 0; i < def->palette_presets.size(); ++i)
                    {
                        const auto& preset = def->palette_presets[i];
                        paletteLabels_.push_back(std::to_wstring(i + 1));

                        if (preset.mappings.empty())
                        {
                            paletteSpriteIds_.push_back(spriteId_); // Preset 0: the sheet's original colors, no recolor needed.
                            continue;
                        }

                        bool created = false;
                        const std::wstring variantName =
                            L"MetallArmy_SpriteTest_" + utils::utf8_to_wstring(preset.id);
                        const SpriteId variantId = sprites.Load(
                            variantName, MM2H_GRAPHICS(MetallArmy), MM2H_GRAPHPROPS(MetallArmy), &created);

                        if (created)
                        {
                            // Only recolor a freshly-loaded copy -- Load() is
                            // idempotent by name, so re-entering this screen
                            // returns the already-baked Id instead, and
                            // re-applying the mapping on top of itself would
                            // recolor an already-recolored image.
                            std::vector<rendering::sprite::SpriteAtlas::PaletteColorMapping> mappings;
                            mappings.reserve(preset.mappings.size());
                            for (const auto& mapping : preset.mappings)
                            {
                                mappings.push_back({ mapping.source_index, mapping.target_index });
                            }
                            sprites.ReplacePixelColorsById(variantId, mappings);
                        }

                        paletteSpriteIds_.push_back(variantId);
                    }
                    return;
                }
            }

            // No presets defined yet for this character (e.g. ROCKMAN --
            // weapon-swap recoloring is planned but not built). Falls back to
            // a single "1" entry pointing at the plain sheet so PALETTE TABLE
            // still works instead of being a dead end, and starts behaving
            // like METALL's the moment real presets are added for it.
            paletteLabels_.push_back(L"1");
            paletteSpriteIds_.push_back(spriteId_);
        }

        void SpriteTestPhase::restartClip_() noexcept
        {
            frameIndex_ = 0;
            frameElapsed_ = 0;
        }

        void SpriteTestPhase::tickClip_() noexcept
        {
            const auto* pattern = currentPattern_();
            if (pattern == nullptr || pattern->clip.frames.empty()) return;

            const auto& frames = pattern->clip.frames;
            const auto& frame = frames[static_cast<std::size_t>(frameIndex_)];
            if (frame.wait_frames == AnimationFrame::kHoldFrames) return; // Sits here until a manual restart.

            ++frameElapsed_;
            if (frameElapsed_ < frame.wait_frames) return;
            frameElapsed_ = 0;

            const bool is_last_frame = frameIndex_ == static_cast<int>(frames.size()) - 1;
            if (!is_last_frame)
            {
                ++frameIndex_;
            }
            else if (pattern->clip.loop)
            {
                frameIndex_ = 0;
            }
            // Non-looping clip: stays on the last frame (matches AnimationStatePlayer's own behavior).
        }

        const SpriteTestPattern* SpriteTestPhase::currentPattern_() const noexcept
        {
            if (previewingPatternIndex_ < 0 || previewingPatternIndex_ >= static_cast<int>(patterns_.size()))
            {
                return nullptr;
            }
            return &patterns_[static_cast<std::size_t>(previewingPatternIndex_)];
        }

        int SpriteTestPhase::idleTile_() const noexcept
        {
            if (patterns_.empty() || patterns_.front().clip.frames.empty()) return 0;
            return patterns_.front().clip.frames.front().tile;
        }
    }
}
