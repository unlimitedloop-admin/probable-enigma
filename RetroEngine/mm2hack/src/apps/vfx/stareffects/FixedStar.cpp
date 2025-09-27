#include "pch.h"

#include "FixedStar.h"

#include "apps/deal/GameContext.h"
#include "Star.h"
#include "StarState.h"

namespace mm2hack::apps::vfx::stareffects
{
    FixedStar::FixedStar(int tileIndex, float x, float y) : _tileIndex(tileIndex), _x(x), _y(y)
    {
    }

    FixedStar::FixedStar(const FixedStarState& s) : _tileIndex(s.tileIndex), _x(s.x), _y(s.y)
    {
    }

    void FixedStar::Draw() const
    {
        auto& context = deal::GameContext::GetInstance();
        auto& sprites = context.GetResourceManager().GetSpriteManager();
        sprites.Use(std::wstring(kStarSpriteName), _tileIndex, static_cast<int>(_x), static_cast<int>(_y));
    }

    FixedStarState FixedStar::ToState() const
    {
        return FixedStarState{ _tileIndex, _x, _y };
    }
}