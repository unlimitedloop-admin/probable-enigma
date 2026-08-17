//==============================================================================
//
//  Project: mm2hack
//  BGTileAnimator.h
//
//  Resolves animated BG tile indices from frame-based animation definitions.
//
//==============================================================================
#pragma once

#include <cstdint>
#include <span>

namespace mm2hack::apps::rendering::bg
{
    // Single frame of a BG tile animation
    struct BGTileAnimationFrame
    {
        std::uint8_t tileId{ 0 };
        std::uint16_t durationFrames{ 1 };
    };

    // Animation definition associated with one logical source tile
    struct BGTileAnimation
    {
        std::uint8_t sourceTile{ 0 };
        std::span<const BGTileAnimationFrame> frames{};
    };

    // Resolves logical BG tiles to animated drawing tiles
    class BGTileAnimator
    {
    public:
        BGTileAnimator() noexcept = default;

        // Sets animation definitions
        void SetAnimations(std::span<const BGTileAnimation> animations) noexcept
        {
            _animations = animations;
            _frame_counter = 0;
        }

        // Advances animation by one game frame
        void Update() noexcept
        {
            ++_frame_counter;
        }

        // Resolves a logical source tile into the current drawing tile [e.g., animated tile]
        [[nodiscard]] std::uint8_t ResolveTile(std::uint8_t source_tile) const noexcept
        {
            for (const auto& animation : _animations)
            {
                if (animation.sourceTile != source_tile ||
                    animation.frames.empty())
                {
                    continue;
                }

                std::uint32_t total_frames = 0;

                for (const auto& frame : animation.frames)
                {
                    total_frames += frame.durationFrames;
                }

                if (total_frames == 0)
                {
                    return source_tile;
                }

                const std::uint32_t local_frame =
                    _frame_counter % total_frames;

                std::uint32_t accumulated = 0;

                for (const auto& frame : animation.frames)
                {
                    accumulated += frame.durationFrames;

                    if (local_frame < accumulated)
                    {
                        return frame.tileId;
                    }
                }
            }

            return source_tile;
        }

        // Resets animation timing
        void Reset() noexcept
        {
            _frame_counter = 0;
        }

    private:
        std::span<const BGTileAnimation> _animations{};
        std::uint32_t _frame_counter{ 0 };
    };
}