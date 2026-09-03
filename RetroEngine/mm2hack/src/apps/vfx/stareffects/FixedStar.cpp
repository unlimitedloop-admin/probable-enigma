#include "pch.h"

#include "FixedStar.h"

#include "StarState.h"

namespace mm2hack::apps::vfx::stareffects
{
    FixedStar::FixedStar(int tileIndex, float x, float y) : _tileIndex(tileIndex), _x(x), _y(y)
    {
    }

    FixedStar::FixedStar(const FixedStarState& s) : _tileIndex(s.tileIndex), _x(s.x), _y(s.y)
    {
    }

    void FixedStar::Draw(const rendering::sprite::SpriteManager& sprites,
                         rendering::sprite::SpriteManager::Id sprite_id) const
    {
        sprites.UseById(sprite_id, _tileIndex, static_cast<int>(_x), static_cast<int>(_y));
    }

    FixedStarState FixedStar::ToState() const
    {
        return FixedStarState{ _tileIndex, _x, _y };
    }
}
