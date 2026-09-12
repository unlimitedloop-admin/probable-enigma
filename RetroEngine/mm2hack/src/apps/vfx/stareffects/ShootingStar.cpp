#include "pch.h"

#include "ShootingStar.h"

#include "Star.h"

namespace mm2hack::apps::vfx::stareffects
{
    ShootingStar::ShootingStar(int typeIndex, float x, float y, float vx, float vy) : Star(typeIndex, x, y, vx, vy)
    {
    }

    void ShootingStar::Update()
    {
        // Call the base class update to move the star
        Star::Update();
        // Additional behavior for shooting stars can be added here
        // For example, we could add a fading effect or change speed over time
    }
}