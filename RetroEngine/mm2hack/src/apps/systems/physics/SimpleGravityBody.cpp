#include "pch.h"

#include "SimpleGravityBody.h"

#include <cmath>

#include "config/SystemConfig.h"
#include "ITerrainProbe.h"
#include "TileAttribute.h"

namespace mm2hack::apps::systems::physics
{
    bool SimpleGravityBodyState::Save(core::save::StateWriter& writer) const
    {
        return IsValid() &&
            writer.WriteF64(vertical_velocity) &&
            writer.WriteBool(on_ground);
    }

    bool SimpleGravityBodyState::Load(core::save::StateReader& reader)
    {
        SimpleGravityBodyState loaded{};
        if (!reader.ReadF64(loaded.vertical_velocity) || !reader.ReadBool(loaded.on_ground))
        {
            return false;
        }
        if (!loaded.IsValid())
        {
            return false;
        }
        *this = loaded;
        return true;
    }

    bool SimpleGravityBodyState::IsValid() const noexcept
    {
        constexpr double kMaximumVelocity = 1'000.0;
        return std::isfinite(vertical_velocity) && std::abs(vertical_velocity) <= kMaximumVelocity;
    }

    SimpleGravityBody::SimpleGravityBody(double gravity_per_frame, double terminal_velocity_per_frame) noexcept
        : _gravity(gravity_per_frame), _terminal_velocity(terminal_velocity_per_frame)
    {
    }

    double SimpleGravityBody::Tick(
        const ITerrainProbe* terrain, double foot_x, double current_bottom_y) noexcept
    {
        if (terrain == nullptr)
        {
            return current_bottom_y;
        }

        _vel_y += _gravity;
        if (_vel_y > _terminal_velocity)
        {
            _vel_y = _terminal_velocity;
        }

        const double next_bottom_y = current_bottom_y + _vel_y;

        // Only fall-through-and-land, not "push up out of a ceiling" -- these
        // entities don't jump, so upward velocity never happens today, but the
        // guard keeps the ground check meaningful if that ever changes.
        if (_vel_y >= 0.0 && Has(terrain->AttributeAt(foot_x, next_bottom_y), TileAttribute::Solid))
        {
            constexpr double kTileSize = static_cast<double>(config::SystemConfig::kTileSize);
            const double tile_row = std::floor(next_bottom_y / kTileSize);
            const double floor_top_y = tile_row * kTileSize;

            _vel_y = 0.0;
            _on_ground = true;
            return floor_top_y;
        }

        _on_ground = false;
        return next_bottom_y;
    }
}
