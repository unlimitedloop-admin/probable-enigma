//==============================================================================
// 
//  Project: mm2hack
//  LadderService.h
// 
//  Laddering service module implementation.
// 
//==============================================================================
#pragma once

#include "ILadderService.h"

#include <optional>
#include "ITerrainProbe.h"

namespace mm2hack::apps::systems::physics
{
    struct Vec2;

    class LadderService final : public ILadderService
    {
    public:
        explicit LadderService(const ITerrainProbe& terrain);

        bool CanGrabAt(const Vec2& worldPos) const override;
        std::optional<Vec2> TryGetCenterXAt(const Vec2& worldPos) const override;

        void setEntryKind(LadderEntryKind v) noexcept { _entry = v; }
        LadderEntryKind getEntryKind() const noexcept { return _entry; }

    private:
        static int toTileIndex_(double v);
        static double toTileCenter_(int tileIndex);

    private:
        const ITerrainProbe& _terrain;
        LadderEntryKind _entry{ LadderEntryKind::None };
    };
}