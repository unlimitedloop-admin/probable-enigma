//==============================================================================
// 
//  Project: mm2hack
//  FreeScrollPolicy.h
// 
//  Adapts free scrolling policy.
// 
//==============================================================================
#pragma once

#include "ScrollController.h"

#include <string>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/view/ViewState.h"

namespace mm2hack::apps
{
    namespace resources::bg
    {
        class RoomGraphAdapter;
    }
}

namespace mm2hack::apps::systems::scrolling::atomic
{
    // Free scrolling policy with deadzone and page transitions
    class FreeScrollPolicy final : public IScrollPolicy
    {
        using RoomGraphAdapter = apps::resources::bg::RoomGraphAdapter;
        using Scalar = systems::view::Scalar;
        using RectF = foundation::math::RectF;

    public:
        FreeScrollPolicy(RoomGraphAdapter& g, int tilePx,
            int deadzoneW = 64, int deadzoneH = 64)
            : _g(g), _ts(tilePx), _dzW(deadzoneW), _dzH(deadzoneH)
        {
        }

        // Returns true if page transition occurred
        bool Update(const RectF& p, Camera& cam, size_t& pageIndex, double /*dt*/) override;

    private:
        const std::wstring kClassName = L"FreeScrollPolicy";

        RoomGraphAdapter& _g;   // Reference to room graph
        int _ts;                // Tile size in pixels
        int _dzW, _dzH;         // Deadzone width and height in pixels
    };
}