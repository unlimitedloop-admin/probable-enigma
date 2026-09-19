#include "pch.h"

#include "EnemyDefinitionLoader.h"

#include <cmath>
#include <fstream>
#include <iterator>
#include <unordered_set>

#include <nlohmann/json.hpp>

#include "utils/string_converter.h"

using json = nlohmann::json;

namespace mm2hack::apps::world::entity::enemy::animation
{
    namespace
    {
        bool try_read_string(const json& object, const char* key, std::string& out)
        {
            const auto value = object.find(key);
            if (value == object.end() || !value->is_string()) return false;
            out = value->get<std::string>();
            return !out.empty();
        }

        bool try_read_bool(const json& object, const char* key, bool default_value, bool& out)
        {
            const auto value = object.find(key);
            if (value == object.end())
            {
                out = default_value;
                return true;
            }
            if (!value->is_boolean()) return false;
            out = value->get<bool>();
            return true;
        }

        bool try_read_int(const json& object, const char* key, int minimum, int maximum, int& out)
        {
            const auto value = object.find(key);
            if (value == object.end() || !value->is_number_integer()) return false;
            const std::int64_t parsed = value->get<std::int64_t>();
            if (parsed < minimum || parsed > maximum) return false;
            out = static_cast<int>(parsed);
            return true;
        }

        bool try_read_nonnegative_double(const json& object, const char* key, double default_value, double& out)
        {
            const auto value = object.find(key);
            if (value == object.end())
            {
                out = default_value;
                return true;
            }
            if (!value->is_number()) return false;
            const double parsed = value->get<double>();
            if (!std::isfinite(parsed) || parsed < 0.0) return false;
            out = parsed;
            return true;
        }

        // Like try_read_nonnegative_double, but any finite sign (e.g. a jump
        // impulse is conventionally negative -- upward).
        bool try_read_double(const json& object, const char* key, double default_value, double& out)
        {
            const auto value = object.find(key);
            if (value == object.end())
            {
                out = default_value;
                return true;
            }
            if (!value->is_number()) return false;
            const double parsed = value->get<double>();
            if (!std::isfinite(parsed)) return false;
            out = parsed;
            return true;
        }

        bool try_parse_frame(const json& source, AnimationFrame& out)
        {
            if (!source.is_object()) return false;
            if (!try_read_int(source, "tile", 0, 65'535, out.tile)) return false;

            const auto wait = source.find("wait");
            if (wait == source.end()) return false;
            if (wait->is_string())
            {
                if (wait->get<std::string>() != "hold") return false;
                out.wait_frames = AnimationFrame::kHoldFrames;
                return true;
            }
            if (!wait->is_number_integer()) return false;
            const std::int64_t parsed = wait->get<std::int64_t>();
            if (parsed < 1 || parsed > 3'600) return false;
            out.wait_frames = static_cast<int>(parsed);
            return true;
        }

        bool try_parse_projectile_spawn(const json& source, ProjectileSpawnSpec& out)
        {
            if (!source.is_object()) return false;
            return try_read_double(source, "angle_deg", 0.0, out.angle_deg) &&
                out.angle_deg >= -180.0 && out.angle_deg <= 180.0 &&
                try_read_nonnegative_double(source, "speed_px_per_frame", 0.0, out.speed_px_per_frame) &&
                out.speed_px_per_frame > 0.0 && out.speed_px_per_frame <= 100.0;
        }

        bool try_parse_transition(const json& source, AnimationTransition& out)
        {
            if (!source.is_object()) return false;

            std::string when;
            if (!try_read_string(source, "when", when) || !try_read_string(source, "to", out.to_state) ||
                !try_read_double(source, "jump_impulse", 0.0, out.jump_impulse))
            {
                return false;
            }

            const auto spawns = source.find("projectile_spawns");
            if (spawns != source.end())
            {
                if (!spawns->is_array()) return false;
                for (const auto& spawn_json : *spawns)
                {
                    ProjectileSpawnSpec spawn{};
                    if (!try_parse_projectile_spawn(spawn_json, spawn)) return false;
                    out.projectile_spawns.push_back(spawn);
                }
            }

            if (when == "timer")
            {
                out.condition = AnimationCondition::Timer;
                return try_read_int(source, "frames", 1, 36'000, out.param_frames);
            }
            if (when == "clip_finished")
            {
                out.condition = AnimationCondition::ClipFinished;
                return true;
            }
            if (when == "player_near")
            {
                out.condition = AnimationCondition::PlayerNear;
                return try_read_nonnegative_double(source, "x", 0.0, out.param_x) &&
                    try_read_nonnegative_double(source, "y", 0.0, out.param_y);
            }
            if (when == "grounded")
            {
                out.condition = AnimationCondition::Grounded;
                return true;
            }
            if (when == "airborne")
            {
                out.condition = AnimationCondition::Airborne;
                return true;
            }
            return false;
        }

        bool try_parse_state(const json& source, AnimationState& out)
        {
            if (!source.is_object() || !try_read_string(source, "id", out.id)) return false;

            if (!try_read_bool(source, "loop", false, out.clip.loop)) return false;
            if (!try_read_bool(source, "allow_movement", true, out.allow_movement)) return false;
            if (!try_read_nonnegative_double(source, "move_speed_multiplier", 1.0, out.move_speed_multiplier)) return false;

            const auto frames = source.find("frames");
            if (frames == source.end() || !frames->is_array() || frames->empty()) return false;
            for (const auto& frame_json : *frames)
            {
                AnimationFrame frame{};
                if (!try_parse_frame(frame_json, frame)) return false;
                out.clip.frames.push_back(frame);
            }

            const auto transitions = source.find("transitions");
            if (transitions != source.end())
            {
                if (!transitions->is_array()) return false;
                for (const auto& transition_json : *transitions)
                {
                    AnimationTransition transition{};
                    if (!try_parse_transition(transition_json, transition)) return false;
                    out.transitions.push_back(std::move(transition));
                }
            }
            return true;
        }

        bool try_parse_palette_mapping(const json& source, EnemyPaletteMapping& out)
        {
            if (!source.is_object()) return false;
            return try_read_int(source, "source", 0, 63, out.source_index) &&
                try_read_int(source, "target", 0, 63, out.target_index);
        }

        bool try_parse_palette_preset(const json& source, EnemyPalettePreset& out)
        {
            if (!source.is_object() || !try_read_string(source, "id", out.id)) return false;

            const auto mappings = source.find("mappings");
            if (mappings == source.end()) return true;
            if (!mappings->is_array()) return false;
            for (const auto& mapping_json : *mappings)
            {
                EnemyPaletteMapping mapping{};
                if (!try_parse_palette_mapping(mapping_json, mapping)) return false;
                out.mappings.push_back(mapping);
            }
            return true;
        }

        // Cross-references between states (initial_state, every transition's
        // `to`) can only be checked once every state id is known -- do that
        // here, after the whole graph has been parsed.
        bool validate_state_graph(const EnemyAnimationDef& animation)
        {
            std::unordered_set<std::string> ids;
            for (const auto& state : animation.states)
            {
                if (!ids.insert(state.id).second) return false; // duplicate id
            }
            if (!ids.contains(animation.initial_state)) return false;
            for (const auto& state : animation.states)
            {
                for (const auto& transition : state.transitions)
                {
                    if (!ids.contains(transition.to_state)) return false;
                }
            }
            return true;
        }

        bool validate_palette_presets(const std::vector<EnemyPalettePreset>& presets)
        {
            std::unordered_set<std::string> ids;
            for (const auto& preset : presets)
            {
                if (!ids.insert(preset.id).second) return false; // duplicate id
            }
            return true;
        }

        bool try_parse_definition(const json& source, EnemyDefinition& out)
        {
            if (!source.is_object() ||
                !try_read_string(source, "id", out.id) ||
                !try_read_string(source, "enemy_type", out.enemy_type) ||
                !try_read_string(source, "name", out.name) ||
                !try_read_string(source, "initial_state", out.animation.initial_state))
            {
                return false;
            }

            const auto states = source.find("states");
            if (states == source.end() || !states->is_array() || states->empty()) return false;
            for (const auto& state_json : *states)
            {
                AnimationState state{};
                if (!try_parse_state(state_json, state)) return false;
                out.animation.states.push_back(std::move(state));
            }
            if (!validate_state_graph(out.animation)) return false;

            const auto presets = source.find("palette_presets");
            if (presets != source.end())
            {
                if (!presets->is_array()) return false;
                for (const auto& preset_json : *presets)
                {
                    EnemyPalettePreset preset{};
                    if (!try_parse_palette_preset(preset_json, preset)) return false;
                    out.palette_presets.push_back(std::move(preset));
                }
                if (!validate_palette_presets(out.palette_presets)) return false;
            }

            return true;
        }
    }

    bool EnemyDefinitionLoader::LoadFromFile(const std::wstring& filepath, EnemyDefinition& out)
    {
        try
        {
            std::ifstream stream(utils::wstring_to_utf8(filepath), std::ios::binary);
            if (!stream.is_open()) return false;

            const std::string source{
                std::istreambuf_iterator<char>(stream),
                std::istreambuf_iterator<char>()
            };
            if (stream.bad()) return false;
            return LoadFromJson(source, out);
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    bool EnemyDefinitionLoader::LoadFromJson(std::string_view source, EnemyDefinition& out)
    {
        try
        {
            const json document = json::parse(source.begin(), source.end());
            EnemyDefinition parsed{};
            if (!try_parse_definition(document, parsed)) return false;

            out = std::move(parsed);
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }
}
