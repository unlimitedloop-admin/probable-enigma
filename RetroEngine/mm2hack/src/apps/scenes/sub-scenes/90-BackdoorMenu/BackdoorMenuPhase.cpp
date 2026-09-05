#include "pch.h"

#include "BackdoorMenuPhase.h"

#include <span>
#include "apps/resources/parameters/Parameters.h"
#include "apps/scenes/PhaseFadeController.h"
#include "apps/ui/controls/MenuCursorController.h"
#include "BackdoorMenu.h"
#include "BackdoorMenuCatalog.h"
#include "core/assembly/StateProvider.h"
#include "core/save/StateIO.h"
#include "input/Jpbtn.h"

namespace mm2hack::apps::scenes
{
    namespace BackdoorMenu_
    {
        //==============================================================================
        //
        //  CreditPhase
        //
        //==============================================================================
        void CreditPhase::Update()
        {
            if (!owner.Fader().InputEnabled()) return;

            auto& input = owner.Input();
            auto verified = input->JustPressed(JPBTN::START) || input->JustPressed(JPBTN::A);

            if (verified)
            {
                auto& audio = owner.Resource()->GetAudioManager();
                audio.PlaySe(L"enter_ring");

                PhaseFadePlan next(
                    5,   // preBlackHold
                    20,  // fadeInFrames
                    0,   // preFadeOutHold
                    20,  // fadeOutFrames
                    0,   // postBlackHold
                    FadeLayerMask::All // layers
                );
                owner.QueuePhase(std::make_unique<TopMenuPhase>(owner), next);
            }
        }

        void CreditPhase::RenderWorld()
        {
            auto& fonts = owner.Resource()->GetFontTileManager();
            fonts.DrawTextImage(L"   MM2HACK DEMO   ", 60, 46);
            fonts.DrawTextImage(L"2024-2026 SIRIUS X", 60, 66);
            fonts.DrawTextImage(L"  BACKDOOR MENU   ", 60, 106);
            fonts.DrawTextImage(L" PRESS START KEY  ", 60, 136);
        }

        void CreditPhase::RenderOverlay()
        {
            // Nothing to render in overlay for this phase.
        }

        BackdoorMenuPhaseId CreditPhase::Id() const noexcept
        {
            return BackdoorMenuPhaseId::Credit;
        }

        bool CreditPhase::Save(core::save::StateWriter& writer) const
        {
            return writer.Good();
        }

        bool CreditPhase::Load(core::save::StateReader& reader)
        {
            return reader.Good();
        }

        //==============================================================================
        //
        //  TopMenuPhase
        //
        //==============================================================================
        void TopMenuPhase::Update()
        {
            owner.StarField().UpdateStars();
            owner.Cursor().Update();

            if (!owner.Fader().InputEnabled()) return;

            auto& input = owner.Input();
            auto verified = input->JustPressed(JPBTN::START) || input->JustPressed(JPBTN::A);
            if (input->JustPressed(JPBTN::DOWN)) { cursorCtl_.Move(+1); }
            if (input->JustPressed(JPBTN::UP)) { cursorCtl_.Move(-1); }
            cursorPos_ = cursorCtl_.Index();

            if (verified)
            {
                auto& audio = owner.Resource()->GetAudioManager();
                audio.PlaySe(L"plink_ring");

                PhaseFadePlan next(
                    5,   // preBlackHold
                    20,  // fadeInFrames
                    0,   // preFadeOutHold
                    20,  // fadeOutFrames
                    0,   // postBlackHold
                    FadeLayerMask::All // layers
                );
                owner.QueuePhase(std::make_unique<InsideMenuPhase>(owner, cursorCtl_, cursorPos_), next);
            }
        }

        void TopMenuPhase::RenderWorld()
        {
            owner.StarField().DrawStars();
            owner.Cursor().DrawAt(cursorCtl_.CursorX(), cursorCtl_.CursorY());
            DrawMenuItems();
        }

        void TopMenuPhase::RenderOverlay()
        {
            // Nothing to render in overlay for this phase.
        }

        BackdoorMenuPhaseId TopMenuPhase::Id() const noexcept
        {
            return BackdoorMenuPhaseId::TopMenu;
        }

        bool TopMenuPhase::Save(core::save::StateWriter& writer) const
        {
            return writer.WriteI32(cursorCtl_.Index());
        }

        bool TopMenuPhase::Load(core::save::StateReader& reader)
        {
            std::int32_t cursor{};
            if (!reader.ReadI32(cursor) || cursor < 0 ||
                cursor >= static_cast<std::int32_t>(kTopMenuTitles.size()))
            {
                return false;
            }
            cursorCtl_.SetIndex(cursor);
            cursorPos_ = cursorCtl_.Index();
            return true;
        }

        void TopMenuPhase::DrawMenuItems() const
        {
            auto& fonts = owner.Resource()->GetFontTileManager();
            int y = 16;
            for (const auto& item : kTopMenuTitles)
            {
                auto labels = std::wstring(item);
                fonts.DrawTextImage(labels, 30, y);
                y += 10;
            }
        }

        //==============================================================================
        //
        //  InsideMenuPhase
        //
        //==============================================================================
        InsideMenuPhase::InsideMenuPhase(BackdoorMenu& owner)
            : InsideMenuPhase(owner, MenuCursor{ {16, 16, 10}, static_cast<int>(kTopMenuTitles.size()) }, 0)
        {
        }

        InsideMenuPhase::InsideMenuPhase(BackdoorMenu& owner, MenuCursor cursorCtl, int topItemIndex)
            : owner(owner), cursorCtl_(cursorCtl), cursorAnim_(owner.Cursor()), topItemIndex_(topItemIndex)
        {
            BuildPageModel_();
            ApplyPageLayout_();
        }

        void InsideMenuPhase::Update()
        {
            owner.StarField().UpdateStars();
            owner.Cursor().Update();

            if (!owner.Fader().InputEnabled()) return;

            auto& input = owner.Input();
            const bool verified = input->JustPressed(JPBTN::START) || input->JustPressed(JPBTN::A);
            const bool cancelled = input->JustPressed(JPBTN::B) || input->JustPressed(JPBTN::BACK);

            if (roomEdit_.active)
            {
                UpdateRoomEdit_();
                if (verified)
                {
                    // Commit changes
                    auto& audio = owner.Resource()->GetAudioManager();
                    audio.PlaySe(L"plink_ring");
                    roomEdit_.active = false;
                }
                else if (cancelled)
                {
                    // Cancel changes
                    roomNo_ = roomEdit_.snapshot;
                    auto& audio = owner.Resource()->GetAudioManager();
                    audio.PlaySe(L"plink_ring");
                    roomEdit_.active = false;
                }
                return;
            }

            if (input->JustPressed(JPBTN::DOWN)) { cursorCtl_.Move(+1); }
            if (input->JustPressed(JPBTN::UP)) { cursorCtl_.Move(-1); }
            cursorPos_ = cursorCtl_.Index();

            if (verified) { ActivateCurrent_(); return; }
            if (cancelled) { NavigateBack_(defaultBack_); return; }
        }

        void InsideMenuPhase::RenderWorld()
        {
            owner.StarField().DrawStars();

            // Draw text contents (all entries)
            auto& fonts = owner.Resource()->GetFontTileManager();
            for (size_t i = 0; i < page_.entries.size(); ++i)
            {
                fonts.DrawTextImage(page_.entries[i].label.data(), 30, page_.rowYs[i]);
            }

            // Cursor: Reference the "visual row" of the selectable row
            const int selIdx = cursorCtl_.Index();
            const int row = page_.selectableRows[selIdx];
            owner.Cursor().DrawAt(page_.cursorX, page_.rowYs[row]);

            if (topItemIndex_ == 3)
            {
                const int depth = static_cast<int>(insideStack_.size());
                if (depth == 1)
                {
                    DrawRoomLineOverlay_(); // Update "ROOM NO." overlay
                }
            }

            // Additional drawing (only for necessary pages)
            if (topItemIndex_ >= 0 && topItemIndex_ < static_cast<int>(kDrawHandlers.size()))
            {
                const auto fn = kDrawHandlers[static_cast<std::size_t>(topItemIndex_)];
                (this->*fn)();
            }
        }

        void InsideMenuPhase::RenderOverlay() { /* nothing */ }

        BackdoorMenuPhaseId InsideMenuPhase::Id() const noexcept { return BackdoorMenuPhaseId::InsideMenu; }

        bool InsideMenuPhase::Save(core::save::StateWriter& writer) const
        {
            constexpr std::uint32_t kMaximumCrumbs = 1;
            if (insideStack_.size() > kMaximumCrumbs) return false;

            if (!writer.WriteI32(topItemIndex_) ||
                !writer.WriteI32(cursorCtl_.Index()) ||
                !writer.WriteU32(static_cast<std::uint32_t>(defaultBack_)) ||
                !writer.WriteI32(roomNo_) ||
                !writer.WriteU32(roomEdit_.active ? 1U : 0U) ||
                !writer.WriteI32(roomEdit_.digit) ||
                !writer.WriteI32(roomEdit_.blink) ||
                !writer.WriteI32(roomEdit_.snapshot) ||
                !writer.WriteU32(static_cast<std::uint32_t>(insideStack_.size())))
            {
                return false;
            }

            for (const auto& crumb : insideStack_)
            {
                if (!writer.WriteI32(crumb.subId) || !writer.WriteI32(crumb.cursor)) return false;
            }
            return true;
        }

        bool InsideMenuPhase::Load(core::save::StateReader& reader)
        {
            constexpr std::uint32_t kMaximumCrumbs = 1;
            std::int32_t top_item{};
            std::int32_t cursor{};
            std::uint32_t back_behavior{};
            std::int32_t room_no{};
            std::uint32_t room_edit_active{};
            std::int32_t room_edit_digit{};
            std::int32_t room_edit_blink{};
            std::int32_t room_edit_snapshot{};
            std::uint32_t crumb_count{};

            if (!reader.ReadI32(top_item) || !reader.ReadI32(cursor) ||
                !reader.ReadU32(back_behavior) || !reader.ReadI32(room_no) ||
                !reader.ReadU32(room_edit_active) || !reader.ReadI32(room_edit_digit) ||
                !reader.ReadI32(room_edit_blink) || !reader.ReadI32(room_edit_snapshot) ||
                !reader.ReadU32(crumb_count) ||
                top_item < 0 || top_item >= static_cast<std::int32_t>(kTopMenuTitles.size()) ||
                cursor < 0 || back_behavior > static_cast<std::uint32_t>(BackBehavior::ToTop) ||
                room_no < 0 || room_no > 99 || room_edit_active > 1 ||
                room_edit_digit < 0 || room_edit_digit > 1 || room_edit_blink < 0 ||
                room_edit_snapshot < 0 || room_edit_snapshot > 99 ||
                crumb_count > kMaximumCrumbs)
            {
                return false;
            }

            std::vector<Crumb> crumbs;
            crumbs.reserve(crumb_count);
            for (std::uint32_t i = 0; i < crumb_count; ++i)
            {
                std::int32_t sub_id{};
                std::int32_t crumb_cursor{};
                if (!reader.ReadI32(sub_id) || !reader.ReadI32(crumb_cursor) ||
                    sub_id < 0 || sub_id > 1 || crumb_cursor < 0 || crumb_cursor > 1)
                {
                    return false;
                }
                crumbs.push_back(Crumb{ sub_id, crumb_cursor });
            }

            if ((!crumbs.empty() && top_item != 3) ||
                (room_edit_active != 0 && (top_item != 3 || crumbs.size() != 1)))
            {
                return false;
            }

            topItemIndex_ = top_item;
            defaultBack_ = static_cast<BackBehavior>(back_behavior);
            roomNo_ = room_no;
            roomEdit_ = RoomEditState{
                room_edit_active != 0,
                room_edit_digit,
                room_edit_blink,
                room_edit_snapshot
            };
            insideStack_ = std::move(crumbs);
            BuildPageModel_();
            ApplyPageLayout_();
            if (cursor >= cursorCtl_.ItemCount()) return false;
            cursorCtl_.SetIndex(cursor);
            cursorPos_ = cursorCtl_.Index();
            return true;
        }

        void InsideMenuPhase::BuildPageModel_()
        {
            page_ = {};
            switch (topItemIndex_)
            {
            case 0: // COMPLETE ARSENAL
                page_.cursorX = 16; page_.firstY = 16; page_.lineH = 10;
                page_.entries.clear();
                AppendEntriesFrom_(kInsideMenu_CompleteArsenal);
                break;

            case 3: // STAGES
            {
                const int depth = static_cast<int>(insideStack_.size());
                if (depth == 0)
                {
                    page_.cursorX = 16; page_.firstY = 16; page_.lineH = 10;
                    page_.entries.clear();
                    AppendEntriesFrom_(kInsideMenu_Stages);
                }
                else if (depth == 1)
                {
                    page_.cursorX = 16; page_.firstY = 16; page_.lineH = 10;
                    page_.entries.clear();
                    AppendEntriesFrom_(kInsideMenu_StageEdit);
                }
                break;
            }

            // HACK: Add other inside menu pages

            case 7: // RESET PARAMETER
                page_.cursorX = 16; page_.firstY = 16; page_.lineH = 10;
                page_.entries.clear();
                AppendEntriesFrom_(kInsideMenu_ResetParameter);
                break;

            default:
                page_.cursorX = 16; page_.firstY = 16; page_.lineH = 10;
                page_.entries = {
                    { L"BACK", true, &InsideMenuPhase::GoBackToTop_, 1 },
                };
                break;
            }

            // Build selectableRows.
            page_.selectableRows.clear();
            for (int i = 0; i < static_cast<int>(page_.entries.size()); ++i)
                if (page_.entries[i].selectable) page_.selectableRows.push_back(i);

            // If no selectable rows, add a BACK entry.
            if (page_.selectableRows.empty())
            {
                page_.entries.push_back({ L"BACK", true, &InsideMenuPhase::GoBackToTop_, 1 });
                page_.selectableRows.push_back(static_cast<int>(page_.entries.size()) - 1);
            }

            // Accumulate Y positions for each entry.
            page_.rowYs.clear();
            int y = page_.firstY;
            for (const auto& e : page_.entries)
            {
                page_.rowYs.push_back(y);
                y += page_.lineH * e.advanceLines;
            }
        }

        void InsideMenuPhase::ApplyPageLayout_()
        {
            // Configure cursor controller
            cursorCtl_.SetLayout({ page_.cursorX, page_.firstY, page_.lineH });
            cursorCtl_.SetItemCount(static_cast<int>(page_.selectableRows.size()));
            cursorCtl_.SetIndex(0);
            cursorPos_ = 0;
        }

        void InsideMenuPhase::ActivateCurrent_() noexcept
        {
            auto& audio = owner.Resource()->GetAudioManager();
            audio.PlaySe(L"plink_ring");

            const int selIdx = cursorCtl_.Index();
            const int row = page_.selectableRows[selIdx];
            const auto& ent = page_.entries[row];

            if (ent.enterSubId.has_value())
            {
                NavigateInto_(*ent.enterSubId);
                return;
            }
            if (const auto h = ent.onActivate)
            {
                (this->*h)();
                return;
            }
        }

        void InsideMenuPhase::GoBackToTop_() noexcept
        {
            auto& audio = owner.Resource()->GetAudioManager();
            audio.PlaySe(L"plink_ring");

            PhaseFadePlan next(
                5,   // preBlackHold
                20,  // fadeInFrames
                0,   // preFadeOutHold
                20,  // fadeOutFrames
                0,   // postBlackHold
                FadeLayerMask::All
            );
            owner.QueuePhase(std::make_unique<TopMenuPhase>(owner, topItemIndex_), next);
        }

        // Extra display handlers for each inside menu page.
        void InsideMenuPhase::CompleteArsenalDisplay_() const {}
        void InsideMenuPhase::ParameterConfigurationDisplay_() const {}
        void InsideMenuPhase::ViewerModeDisplay_() const {}
        void InsideMenuPhase::StagesDisplay_() const {}
        void InsideMenuPhase::RegularBootDisplay_() const {}
        void InsideMenuPhase::SoundTestModeDisplay_() const {}
        void InsideMenuPhase::SpriteTestDisplay_() const {}
        void InsideMenuPhase::ResetParameterDisplay_() const {}

        InsideMenuPhase::ActHandler InsideMenuPhase::ResolveAction_(Action a) noexcept
        {
            using A = Action;
            switch (a)
            {
            case A::Back: return &InsideMenuPhase::BackOne_;
            case A::Top: return &InsideMenuPhase::BackToTop_;
            case A::Enter: return nullptr; // Handled via enterSubId
            case A::NextScene: return &InsideMenuPhase::JumpToScene_;
            case A::EditRoomNo: return &InsideMenuPhase::EnterRoomEdit_;
            case A::None:   // Fallthrough
            default: return nullptr;
            }
        }

        void InsideMenuPhase::AppendEntriesFrom_(std::span<const InsideMenuItemDesc> src)
        {
            for (const auto& d : src)
            {
                page_.entries.push_back(MenuEntry{ d.label, d.selectable, ResolveAction_(d.action), d.advanceLines, d.subId });
            }
        }

        void InsideMenuPhase::NavigateInto_(int subId)
        {
            insideStack_.push_back(Crumb{ subId, cursorCtl_.Index() });
            BuildPageModel_();
            ApplyPageLayout_();
        }

        void InsideMenuPhase::NavigateBack_(BackBehavior mode) noexcept
        {
            if (mode == BackBehavior::ToTop || insideStack_.empty())
            {
                GoBackToTop_();
                return;
            }
            insideStack_.pop_back();
            BuildPageModel_();
            ApplyPageLayout_();
        }

        void InsideMenuPhase::BackOne_() noexcept
        {
            auto& audio = owner.Resource()->GetAudioManager();
            audio.PlaySe(L"plink_ring");
            NavigateBack_(BackBehavior::Step);
        }

        void InsideMenuPhase::BackToTop_() noexcept
        {
            auto& audio = owner.Resource()->GetAudioManager();
            audio.PlaySe(L"plink_ring");
            NavigateBack_(BackBehavior::ToTop);
        }

        void InsideMenuPhase::JumpToScene_() noexcept
        {
            auto& audio = owner.Resource()->GetAudioManager();
            audio.PlaySe(L"plink_ring");

            // Determine which scene to jump to based on the current menu context.
            if (topItemIndex_ == 3)     // STAGES
            {
                const int depth = static_cast<int>(insideStack_.size());
                if (depth == 1)
                {
                    if (!ValidateRoom_(roomNo_))
                    {
                        roomNo_ = 0;
                        audio.PlaySe(L"plink_ring");
                        return;
                    }

                    resources::parameters::Parameters p;
                    p = p.With<int>(L"RoomNo", roomNo_);
                    p = p.With<std::wstring>(L"bgm_key", L"demo_stage_2_bgm");  // HACK: Select BGM for demo stage
                    owner.SetNextScene(GetSceneIDForJumpParameter(insideStack_[0].subId), p);
                }
            }

            owner.MarkLeaving();    // Mark as leaving to prevent further input.
            owner.QueuePhase(nullptr, PhaseFadePlan{
                5,   // preBlackHold
                20,  // fadeInFrames
                0,   // preFadeOutHold
                20,  // fadeOutFrames
                0,   // postBlackHold
                FadeLayerMask::All
                });
        }

        void InsideMenuPhase::EnterRoomEdit_() noexcept
        {
            auto& audio = owner.Resource()->GetAudioManager();
            audio.PlaySe(L"plink_ring");

            roomEdit_.active = true;
            roomEdit_.digit = 1;    // Begin with ones place
            roomEdit_.blink = 0;
            roomEdit_.snapshot = roomNo_;   // Hold current room number
        }

        void InsideMenuPhase::UpdateRoomEdit_() noexcept
        {
            auto& in = owner.Input();

            // Shift digit selection : LEFT/RIGHT
            if (in->JustPressed(JPBTN::LEFT)) { roomEdit_.digit = 0; }
            if (in->JustPressed(JPBTN::RIGHT)) { roomEdit_.digit = 1; }

            int tens = roomNo_ / 10;
            int ones = roomNo_ % 10;

            // For increment/decrement : UP/DOWN
            if (in->JustPressed(JPBTN::UP))
            {
                if (roomEdit_.digit == 0) { tens = (tens + 1) % 10; }
                else { ones = (ones + 1) % 10; }
            }
            if (in->JustPressed(JPBTN::DOWN))
            {
                if (roomEdit_.digit == 0) { tens = (tens + 9) % 10; }
                else { ones = (ones + 9) % 10; }
            }

            roomNo_ = tens * 10 + ones;
            roomEdit_.blink++;
        }

        bool InsideMenuPhase::ValidateRoom_(int no) const noexcept
        {
            // TODO: Check against actual valid room numbers.
            // For now, assume valid room numbers are 0-99.
            return (0 <= no && no <= 99);
        }

        void InsideMenuPhase::DrawRoomLineOverlay_() const
        {
            // 1) Get the Y coordinate of the "ROOM" line
            //    Here, we assume "Stage Edit page's 0th row is ROOM"
            const int visibleRow = 0;
            const int y = page_.rowYs[visibleRow];

            // 2) Just after "ROOM NO. ", draw the room number
            //    Room number starts at character index 9 (0-based)
            constexpr int kTextStartX = 30;
            constexpr int kDigitsOffsetChars = 9;
            const int digitsX = kTextStartX + kDigitsOffsetChars * 8;

            // 3) Split roomNo_ into tens and ones
            const wchar_t tens = L'0' + (roomNo_ / 10);
            const wchar_t ones = L'0' + (roomNo_ % 10);

            // 4) Flashing logic
            const bool blinkOn = roomEdit_.active && ((roomEdit_.blink % 3) == 0);

            auto& fonts = owner.Resource()->GetFontTileManager();

            // tens digit
            if (roomEdit_.active && roomEdit_.digit == 0 && blinkOn)
            {
                fonts.DrawFilledRect(digitsX, y, 8, 8);
            }
            else
            {
                wchar_t s[2] = { tens, 0 };
                fonts.DrawTextImage(s, digitsX, y);
            }

            // ones digit
            const int onesX = digitsX + 8;
            if (roomEdit_.active && roomEdit_.digit == 1 && blinkOn)
            {
                fonts.DrawFilledRect(onesX, y, 8, 8);
            }
            else
            {
                wchar_t s[2] = { ones, 0 };
                fonts.DrawTextImage(s, onesX, y);
            }
        }
    }
}
