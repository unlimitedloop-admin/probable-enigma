//==============================================================================
// 
//  Project: mm2hack
//  MenuCursorController.h
// 
//  Manages the cursor position in a menu with a given layout and item count.
// 
//==============================================================================
#pragma once

#include <algorithm>

namespace mm2hack::apps::ui::controls
{
    // A simple controller for menu cursor position management
    class MenuCursorController final
    {
    public:
        struct Layout { int originX; int originY; int lineHeight; };

    public:
        explicit MenuCursorController(Layout layout, int itemCount) noexcept
            : _layout(layout), _count(itemCount)
        {
        }

        // Set the number of items in the menu (clamped to 0 or more)
        void SetItemCount(int itemCount) noexcept { _count = std::max(0, itemCount); Clamp_(); }
        void SetLayout(Layout layout) noexcept { _layout = layout; }

        // Move index by +-delta (clamped)
        void Move(int delta) noexcept { _index += delta; Clamp_(); }

        // Set index directly (clamped)
        void SetIndex(int idx) noexcept { _index = idx; Clamp_(); }

        // Get menu command count, current index and cursor position
        [[nodiscard]] int ItemCount() const noexcept { return _count; }
        [[nodiscard]] int Index() const noexcept { return _index; }
        [[nodiscard]] int CursorX() const noexcept { return _layout.originX; }
        [[nodiscard]] int CursorY() const noexcept { return _layout.originY + _index * _layout.lineHeight; }

    private:
        // Clamp index to valid range
        void Clamp_() noexcept
        {
            // The cursor will not go out of bounds.
            if (_count <= 0) { _index = 0; return; }
            if (_index < 0) { _index = 0; }
            const int last = _count - 1;
            if (_index > last) { _index = last; }
        }

    private:
        Layout _layout{ 16, 16, 10 };   // Default layout for menu list of development mode
        int    _count{ 0 };
        int    _index{ 0 };
    };
}