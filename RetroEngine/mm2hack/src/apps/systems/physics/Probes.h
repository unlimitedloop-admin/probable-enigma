//==============================================================================
// 
//  Project: mm2hack
//  Probes.h
// 
//  It has points for detecting contact with objects.
// 
//==============================================================================
#pragma once

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"

namespace mm2hack::apps::systems::physics
{
    struct EnvironmentProbe
    {
        foundation::math::Vec2 centerPoint{ 0.0, 0.0 };
    };

    struct BehindGroundProbe
    {
        foundation::math::Vec2 topPoint{ 0.0, 0.0 };
        foundation::math::Vec2 middlePoint{ 0.0, 0.0 };
        foundation::math::Vec2 bottomPoint{ 0.0, 0.0 };
    };

    struct FrontLineProbe
    {
        foundation::math::Vec2 topPoint{ 0.0, 0.0 };
        foundation::math::Vec2 middlePoint{ 0.0, 0.0 };
        foundation::math::Vec2 bottomPoint{ 0.0, 0.0 };
        foundation::math::Vec2 bottomPoint2{ 0.0, 0.0 };
    };

    struct RearLineProbe
    {
        foundation::math::Vec2 topPoint{ 0.0, 0.0 };
        foundation::math::Vec2 middlePoint{ 0.0, 0.0 };
        foundation::math::Vec2 bottomPoint{ 0.0, 0.0 };
        foundation::math::Vec2 bottomPoint2{ 0.0, 0.0 };
    };

    struct TopLineProbe
    {
        foundation::math::Vec2 frontPoint{ 0.0, 0.0 };
        foundation::math::Vec2 middlePoint{ 0.0, 0.0 };
        foundation::math::Vec2 behindPoint{ 0.0, 0.0 };
    };

    struct BottomLineProbe
    {
        foundation::math::Vec2 frontPoint{ 0.0, 0.0 };
        foundation::math::Vec2 middlePoint{ 0.0, 0.0 };
        foundation::math::Vec2 behindPoint{ 0.0, 0.0 };
    };

    // The probes container for character physics
    struct Probes
    {
        BehindGroundProbe behindGround{};
        FrontLineProbe    frontLine{};
        RearLineProbe     rearLine{};
        TopLineProbe      topLine{};
        BottomLineProbe   bottomLine{};
        EnvironmentProbe  environment{};

        foundation::math::Vec2 half{};

        Probes(foundation::math::Vec2 _half) noexcept
            : half(_half)
        {
        }

        // Reset all probes to zero
        void reset() noexcept
        {
            behindGround = {};
            frontLine    = {};
            rearLine     = {};
            topLine      = {};
            bottomLine   = {};
            environment  = {};
        }

        // Refresh all probe positions based on player context and probe offsets
        void refreshAll(const world::entity::avatar::PlayerContext& cx, const world::entity::avatar::PlayerProbes& p)
        {
            // Update probes based on current position and bounding box.
            double currentPosX;
            double currentPosY;
            // Explicitly register the base point of the character.
            double basePosX = cx.pos.x - half.x;
            double basePosY = cx.pos.y - half.y;

            // Front collision probe (ahead of the player)
            {
                currentPosX = basePosX + half.x + (static_cast<int>(cx.facingLR) * p.frontLineOffsetX);
                currentPosY = basePosY + half.y;
                frontLine.topPoint     = { currentPosX, currentPosY + p.verticalTopOffsetY };
                frontLine.middlePoint  = { currentPosX, currentPosY + p.verticalMidOffsetY };
                frontLine.bottomPoint  = { currentPosX, currentPosY + p.verticalBtmOffsetY };
                frontLine.bottomPoint2 = { currentPosX, currentPosY + p.verticalBtmOffsetY2 };
            }
            // Rear collision probe (behind the player)
            {
                currentPosX = basePosX + half.x + (static_cast<int>(cx.facingLR) * p.rearLineOffsetX);
                currentPosY = basePosY + half.y;
                rearLine.topPoint     = { currentPosX, currentPosY + p.verticalTopOffsetY };
                rearLine.middlePoint  = { currentPosX, currentPosY + p.verticalMidOffsetY };
                rearLine.bottomPoint  = { currentPosX, currentPosY + p.verticalBtmOffsetY };
                rearLine.bottomPoint2 = { currentPosX, currentPosY + p.verticalBtmOffsetY2 };
            }
            // Check on ground probe (below the player)
            {
                currentPosX = basePosX + half.x;
                currentPosY = basePosY + half.y + p.groundLineOffsetY;
                bottomLine.frontPoint   = { currentPosX + p.horizonFrontOffsetX,   currentPosY };
                bottomLine.middlePoint  = { currentPosX + p.horizonMidOffsetX,     currentPosY };
                bottomLine.behindPoint  = { currentPosX + p.horizonBehindOffsetX,  currentPosY };
            }
            // Overhead probe (above the player)
            {
                currentPosX = basePosX + half.x;
                currentPosY = basePosY + half.y + p.overheadLineOffsetY;
                topLine.frontPoint  = { currentPosX + p.horizonFrontOffsetX,  currentPosY };
                topLine.middlePoint = { currentPosX + p.horizonMidOffsetX,    currentPosY };
                topLine.behindPoint = { currentPosX + p.horizonBehindOffsetX, currentPosY };
            }
            // Check behind BG tile probe
            {
                currentPosX = basePosX + half.x + (static_cast<int>(cx.facingLR) * 1);
                currentPosY = basePosY + half.y;
                behindGround.topPoint    = { currentPosX, currentPosY + p.verticalTopOffsetY };
                behindGround.middlePoint = { currentPosX, currentPosY + p.verticalMidOffsetY };
                behindGround.bottomPoint = { currentPosX, currentPosY + p.verticalBtmOffsetY };
            }
            // Environment probe at the center of the player.
            {
                environment.centerPoint = cx.pos;
            }
        }

        void swapFrontLR(const world::entity::avatar::PlayerContext& cx, const world::entity::avatar::PlayerProbes& p)
        {
            // Update probes based on current position and bounding box.
            double currentPosX;
            double currentPosY;
            // Explicitly register the base point of the character.
            double basePosX = cx.pos.x - half.x;
            double basePosY = cx.pos.y - half.y;

            // Front collision probe (ahead of the player)
            {
                currentPosX = basePosX + half.x + (static_cast<int>(cx.facingLR) * p.frontLineOffsetX);
                currentPosY = basePosY + half.y;
                frontLine.topPoint     = { currentPosX, currentPosY + p.verticalTopOffsetY };
                frontLine.middlePoint  = { currentPosX, currentPosY + p.verticalMidOffsetY };
                frontLine.bottomPoint  = { currentPosX, currentPosY + p.verticalBtmOffsetY };
                frontLine.bottomPoint2 = { currentPosX, currentPosY + p.verticalBtmOffsetY2 };
            }
            // Rear collision probe (behind the player)
            {
                currentPosX = basePosX + half.x + (static_cast<int>(cx.facingLR) * p.rearLineOffsetX);
                currentPosY = basePosY + half.y;
                rearLine.topPoint     = { currentPosX, currentPosY + p.verticalTopOffsetY };
                rearLine.middlePoint  = { currentPosX, currentPosY + p.verticalMidOffsetY };
                rearLine.bottomPoint  = { currentPosX, currentPosY + p.verticalBtmOffsetY };
                rearLine.bottomPoint2 = { currentPosX, currentPosY + p.verticalBtmOffsetY2 };
            }
        }
    };
}