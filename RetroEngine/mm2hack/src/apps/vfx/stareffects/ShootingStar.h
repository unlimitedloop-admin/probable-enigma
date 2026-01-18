//==============================================================================
// 
//  Project: mm2hack
//  ShootingStar.h
// 
//  BG material that expresses the shooting star effect.
// 
//==============================================================================
#pragma once

#include "Star.h"

#include <string>

namespace mm2hack::apps::vfx::stareffects
{
    // Shooting star effect class using the Star base class
    class ShootingStar : public Star
    {
    public:
        ShootingStar(int typeIndex, float x, float y, float vx, float vy);

        void Update() override;

    private:
        const std::wstring kClassName{ L"ShootingStar" };
    };
}