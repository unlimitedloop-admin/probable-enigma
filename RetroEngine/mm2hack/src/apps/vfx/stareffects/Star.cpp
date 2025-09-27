#include "pch.h"

#include "Star.h"

#include "apps/deal/GameContext.h"
#include "StarState.h"

namespace mm2hack::apps::vfx::stareffects
{
    Star::Star(int typeIndex, float x, float y, float vx, float vy)
        : _typeIndex(typeIndex), _x(x), _y(y), _vx(vx), _vy(vy)
    {
    }

    void Star::Update()
    {
        // Update the star's position based on its speed
        _x += _vx;
        _y += _vy;
    }

    void Star::Draw()
    {
        auto& context = deal::GameContext::GetInstance();
        auto& sprites = context.GetResourceManager().GetSpriteManager();
        sprites.Use(std::wstring(kStarSpriteName), _typeIndex, static_cast<int>(_x), static_cast<int>(_y));
    }

    bool Star::IsOffScreen() const
    {
        return _x < -8 || _y >(config::SystemConfig::kScreenHeight + 8);   // Off-screen threshold
    }

    StarState Star::ToState() const
    {
        return StarState{ _typeIndex, _x, _y, _vx, _vy };
    }
}