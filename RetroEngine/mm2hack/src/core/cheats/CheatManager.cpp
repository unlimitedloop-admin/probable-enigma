#include "pch.h"

#include "CheatManager.h"

#include "FreezePatchEffect.h"
#include "ICheatEffect.h"
#include "ICheatMemoryMap.h"
#include "ICodeParser.h"
#include "OnceWriteEffect.h"

namespace mm2hack::core::cheats
{
    CheatManager::CheatManager(ICheatMemoryMap& memory_map)
        : _memory_map(memory_map)
    {
    }

    void CheatManager::SetParsers(std::vector<std::unique_ptr<ICodeParser>> parsers)
    {
        _parsers = std::move(parsers);
    }

    bool CheatManager::AddFromString(const std::wstring& code, std::wstring& out_error)
    {
        std::vector<CheatPatch> patches;
        for (auto& p : _parsers)
        {
            if (p->TryParse(code, patches))
            {
                break;
            }
        }
        if (patches.empty())
        {
            out_error = L"Failed to parse cheat code.";
            return false;
        }

        for (auto& patch : patches)
        {
            ByteLocation loc{};
            bool ok = false;

            if (patch.symbolic_path.has_value())
            {
                ok = _memory_map.TryResolveSymbol(*patch.symbolic_path, loc);
            }
            else if (patch.address.has_value())
            {
                ok = _memory_map.TryResolve(patch.address.value(), loc);
            }

            if (!ok)
            {
                out_error = L"Unknown target for patch: " + (patch.symbolic_path ? *patch.symbolic_path : L"(addr)");
                return false;
            }

            std::unique_ptr<ICheatEffect> eff;
            if (patch.freeze)
            {
                eff = std::make_unique<FreezePatchEffect>(loc, patch.value, patch.compare, patch.label);
            }
            else
            {
                eff = std::make_unique<OnceWriteEffect>(loc, patch.value, patch.compare, patch.label);
            }

            CheatEntry entry{};
            entry.code_string = code;
            entry.effect = std::move(eff);
            entry.enabled = true;

            // Apply immediately
            entry.effect->ApplyOnce();
            _entries.emplace_back(std::move(entry));
        }
        return true;
    }

    void CheatManager::OnEnableAll()
    {
        for (auto& e : _entries)
        {
            if (e.enabled)
            {
                e.effect->ApplyOnce();
            }
        }
    }

    void CheatManager::OnDisableAll()
    {
        for (auto& e : _entries)
        {
            e.effect->Revert();
        }
    }

    void CheatManager::OnPreUpdate()
    {
        // TODO: queued one-shots can be executed here.
    }

    void CheatManager::OnLateUpdate()
    {
        for (auto& e : _entries)
        {
            if (e.enabled && e.effect->IsFreeze())
            {
                e.effect->ApplyFreeze();
            }
        }
    }

    const std::vector<CheatEntry>& CheatManager::GetEntries() const
    {
        return _entries;
    }
}