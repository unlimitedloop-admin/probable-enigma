#include "pch.h"

#include "LadderingState.h"

#include <cmath>
#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/systems/physics/ILadderService.h"
#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/systems/physics/Probes.h"
#include "apps/systems/physics/TileAttribute.h"
#include "apps/systems/scrolling/atomic/ScrollTypes.h"
#include "apps/world/entity/avatar/abilities/AnimationAbilities.h"
#include "apps/world/entity/avatar/abilities/MovementAbilities.h"
#include "apps/world/entity/avatar/AvatarStatus.h"
#include "apps/world/entity/avatar/PlayerContext.h"
#include "apps/world/entity/avatar/PlayerParams.h"
#include "core/assembly/StateProvider.h"
#include "input/Jpbtn.h"


namespace mm2hack::apps::world::entity::avatar::states
{
    using systems::physics::TileAttribute;
    using systems::physics::LadderEntryKind;

    AvatarStatus LadderingState::Id() const noexcept { return AvatarStatus::Laddering; }

    void LadderingState::OnEnter(PlayerContext& cx, StateProvider* in, const PlayerTuning& t)
    {
        cx.onGround = false;
        cx.justLanded = false;

        cx.vel.x = 0.0;
        if (cx.ladder != nullptr && cx.ladder->getEntryKind() == LadderEntryKind::FromTopDown)
        {
            cx.vel.y = 0x0F.00p0;   // Start climbing down when entering from the top.
        }
        else
        {
            cx.vel.y = 0.0;         // No vertical movement on normal entry (from sides)
        }
        snapToLadderCenter_(cx, t); // Align avatar's X-position to ladder center on entry.

        cx.texture = static_cast<int>(STile::LadderingA);
        cx.animeStepper.reset();
        cx.ladder->setEntryKind(LadderEntryKind::None);
    }

    void LadderingState::OnExit(PlayerContext& cx, StateProvider* in, const PlayerTuning& t)
    {
        cx.animeStepper.reset();
        cx.vel.x = 0.0;
    }

    AvatarStatus LadderingState::Update(PlayerContext& cx, StateProvider* in, const PlayerTuning& t, double /*dt*/)
    {
        using namespace abilities;
        using namespace systems::scrolling::atomic;
        // Climb movement
        const bool up = in->IsPressed(JPBTN::UP);
        const bool down = in->IsPressed(JPBTN::DOWN);

        const auto fixedScrollLk = [&]()
            {
                using conf = config::SystemConfig;
                const double actualDy = cx.vel.y;
                if (cx.pendingFixedScroll.available && actualDy != 0.0)
                {
                    const int page_w = conf::kTileCountX * conf::kTileSize;
                    const int page_h = conf::kTileCountY * conf::kTileSize;

                    // World Y of the probes before and after movement.
                    const double probePrevWorldY = cx.prelimProbes.behindGround.middlePoint.y;
                    const double probeCurrWorldY = cx.probes.behindGround.middlePoint.y + actualDy;

                    const auto origin = cx.pageOriginPx;
                    const double prevLocalY = probePrevWorldY - origin.y;
                    const double currLocalY = probeCurrWorldY - origin.y;

                    if (actualDy < 0.0)
                    {
                        // Up: cross local 0
                        if (prevLocalY >= 0.0 && currLocalY < 0.0)
                        {
                            FixedScrollRequest req{};
                            req.dir = PageScroll::Dir::Up;
                            req.carryTotalPx = 0x06.00p0;
                            cx.pendingFixedScroll = req;
                        }
                    }
                    else if (actualDy > 0.0)
                    {
                        // Down: cross local 240
                        if (prevLocalY < static_cast<double>(page_h) && currLocalY >= static_cast<double>(page_h))
                        {
                            FixedScrollRequest req{};
                            req.dir = PageScroll::Dir::Down;
                            req.carryTotalPx = 0x01.00p0;
                            cx.pendingFixedScroll = req;
                        }
                    }
                }
            };

        // Always snap X to ladder center (warp is allowed)
        snapToLadderCenter_(cx, t);

        // If ladder lost: fall
        if (!isOnLadder_(cx, t))
        {
            cx.onGround = false;
            return AvatarStatus::Hovering;
        }

        cx.vel.x = 0.0;
        cx.vel.y = 0.0;

        if (cx.lockClimbMove)
        {
            cx.vel.y = 0.0;
        }
        else if (up && !down)
        {
            cx.vel.y = -t.climbSpeed;
            auto vHit = cx.terrain->SweepVertical(cx.probes, cx.vel);
            if (vHit.hit && vHit.kind == systems::physics::VHitKind::Ceiling)
            {
                // Stop at ceiling. 
                cx.vel.y = 0.0;
            }
            else
            {
                fixedScrollLk();    // Handle fixed scrolling when climbing up
            }
        }
        else if (down && !up)
        {
            cx.vel.y = +t.climbSpeed;
            auto vHit = cx.terrain->SweepVertical(cx.probes, cx.vel);
            if (vHit.hit && vHit.kind == systems::physics::VHitKind::Floor)
            {
                // Stop at floor
                cx.vel.y = vHit.maxDistanceY;

                // If you want: transition to Standing when trying to go down but floor blocks
                cx.onGround = true;
                cx.justLanded = true;
                return AvatarStatus::Standing;
            }
            else
            {
                fixedScrollLk();    // Handle fixed scrolling when climbing down
            }
        }
        // !up && !down -> vel.y = 0.0, and 
        else
        {
            cx.vel.y = 0.0;
            // Jump -> Hovering (use existing DoJump)
            if (in->JustPressed(JPBTN::A))
            {
                UpdateVerticalVelocity(cx, t, false);   // Start falling
                return AvatarStatus::Hovering;
            }
        }

        // Rising to ground at ladder top (original-like rule)
        if (!cx.lockClimbMove && up && shouldRisingToGround_(cx))
        {
            doRisingToGround_(cx);
            return AvatarStatus::Standing;
        }

        auto [input, isTopAttrEmpty] = computeInputAndTopEmpty_(cx, in);
        LadderingAnim(cx, input, isTopAttrEmpty);
        return AvatarStatus::Laddering;
    }

    void LadderingState::TickAnimationOnly(AnimeContext& ax, StateProvider* in, const PlayerTuning& t, double dt)
    {
        using namespace abilities;

        LadderingAnim(ax);  // Not move on its own.
    }

    bool LadderingState::isOnLadder_(const PlayerContext& cx, const PlayerTuning& t) const noexcept
    {
        if (cx.ladder == nullptr)
        {
            return false;
        }

        Vec2 candidates[9]{};
        buildGrabCandidates_(candidates, cx, t);

        for (const auto& p : candidates)
        {
            if (cx.ladder->CanGrabAt(p))
            {
                return true;
            }
        }

        return false;
    }

    void LadderingState::snapToLadderCenter_(PlayerContext& cx, const PlayerTuning& t) const noexcept
    {
        if (cx.ladder == nullptr)
        {
            return;
        }

        Vec2 candidates[9]{};
        buildGrabCandidates_(candidates, cx, t);

        for (const auto& p : candidates)
        {
            const auto centerPos = cx.ladder->TryGetCenterXAt(p);
            if (centerPos.has_value())
            {
                cx.pos.x = centerPos->x;
                return;
            }
        }
    }

    bool LadderingState::shouldRisingToGround_(const PlayerContext& cx) const
    {
        using namespace systems::physics;
        // Condition:
        // behind.middle == Empty AND behind.bottom == Laddering
        const auto& b = cx.probes.behindGround;

        const auto midAttr = cx.terrain->AttributeAt(b.middlePoint);
        const auto btmAttr = cx.terrain->AttributeAt(b.bottomPoint);

        return (Has(midAttr, TileAttribute::Empty) && (Has(btmAttr, TileAttribute::Ladder)));
    }

    void LadderingState::doRisingToGround_(PlayerContext& cx) const
    {
        using namespace abilities;
        const double y = cx.pos.y;

        const double moveY =
            std::floor((y + 9.0) / config::SystemConfig::kTileSize + 1.0) * config::SystemConfig::kTileSize - 8.0;

        // Directly set position to avoid overshooting.
        cx.pos.y = moveY - 17.0;

        cx.vel.x = 0.0;
        cx.vel.y = 0.0;

        cx.onGround = true;
        cx.justLanded = true;

        cx.facingLR = OppositeFacingDirection(cx.facingLR);
        cx.texture = static_cast<int>(STile::StandingA);
    }

    void LadderingState::buildGrabCandidates_(Vec2 out[9], const PlayerContext& cx, const PlayerTuning& t) const noexcept
    {
        const auto& b = cx.probes.behindGround;

        // Testing check minimum epsilon.
        const double eps = kSnapEpsBase;

        out[0] = b.topPoint;
        out[1] = b.middlePoint;
        out[2] = b.bottomPoint;

        out[3] = Vec2{ b.topPoint.x,    b.topPoint.y + eps };
        out[4] = Vec2{ b.middlePoint.x, b.middlePoint.y + eps };
        out[5] = Vec2{ b.bottomPoint.x, b.bottomPoint.y + eps };

        out[6] = Vec2{ b.topPoint.x,    b.topPoint.y - eps };
        out[7] = Vec2{ b.middlePoint.x, b.middlePoint.y - eps };
        out[8] = Vec2{ b.bottomPoint.x, b.bottomPoint.y - eps };
    }

    std::pair<int, bool> LadderingState::computeInputAndTopEmpty_(const PlayerContext& cx, StateProvider* in) const noexcept
    {
        const bool up = in->IsPressed(JPBTN::UP);
        const bool down = in->IsPressed(JPBTN::DOWN);

        auto topAttr = cx.terrain->AttributeAt(cx.probes.behindGround.topPoint);
        const bool isTopAttrEmpty = Has(topAttr, TileAttribute::Empty);
        const int input = (up && !down) ? -1 : ((down && !up) ? +1 : 0);

        return { input, isTopAttrEmpty };
    }
}