#include "pch.h"

#include "StageDefinitionLoader.h"

#include <fstream>
#include <iterator>
#include <unordered_map>
#include <unordered_set>

#include <nlohmann/json.hpp>

#include "config/SystemConfig.h"
#include "utils/string_converter.h"

using json = nlohmann::json;

namespace mm2hack::apps::world::stage
{
    namespace
    {
        constexpr int kMaxRoomId = 0xFF;

        bool parse_respawn(const std::string& text, EnemyRespawnPolicy& out)
        {
            if (text == "always") { out = EnemyRespawnPolicy::Always; return true; }
            if (text == "until_defeated") { out = EnemyRespawnPolicy::UntilDefeated; return true; }
            if (text == "once") { out = EnemyRespawnPolicy::Once; return true; }
            return false;
        }

        bool parse_facing(const std::string& text, PlacementFacing& out)
        {
            if (text == "player") { out = PlacementFacing::TowardPlayer; return true; }
            if (text == "left") { out = PlacementFacing::Left; return true; }
            if (text == "right") { out = PlacementFacing::Right; return true; }
            return false;
        }

        bool read_room_id(const json& object, int& out)
        {
            const auto value = object.find("roomId");
            if (value == object.end() || !value->is_number_integer()) return false;
            const auto parsed = value->get<std::int64_t>();
            if (parsed < 0 || parsed > kMaxRoomId) return false;
            out = static_cast<int>(parsed);
            return true;
        }

        // An object's own room-local "x"/"y", inside one screen-sized room.
        bool read_xy(const json& object, foundation::math::Vec2& out)
        {
            const auto x = object.find("x");
            const auto y = object.find("y");
            if (x == object.end() || y == object.end() || !x->is_number() || !y->is_number()) return false;
            const double px = x->get<double>();
            const double py = y->get<double>();
            if (!(px >= 0.0 && px < config::SystemConfig::kScreenWidth &&
                  py >= 0.0 && py < config::SystemConfig::kScreenHeight))
            {
                return false;
            }
            out = { px, py };
            return true;
        }

        // Room-local { "x", "y" } nested under `key` (an entity's "position").
        bool read_local_pos(const json& object, const char* key, foundation::math::Vec2& out)
        {
            const auto value = object.find(key);
            return value != object.end() && value->is_object() && read_xy(*value, out);
        }

        bool parse_start(const json& stage, std::optional<PlayerStart>& out, std::string& error)
        {
            const auto start = stage.find("start");
            if (start == stage.end()) return true;

            PlayerStart parsed{};
            if (!start->is_object() || !read_room_id(*start, parsed.room_id))
            {
                error = "stage.start: needs an integer roomId (0-255)";
                return false;
            }
            // "start" carries x/y directly (not under "position") -- it's a point, not an entity.
            if (!read_xy(*start, parsed.local_pos))
            {
                error = "stage.start: x/y must be room-local pixels";
                return false;
            }
            out = parsed;
            return true;
        }

        bool parse_room_defaults(const json& root, std::unordered_map<int, EnemyRespawnPolicy>& out, std::string& error)
        {
            const auto nodes = root.find("nodes");
            if (nodes == root.end()) return true;
            if (!nodes->is_array())
            {
                error = "nodes: must be an array";
                return false;
            }

            for (const auto& node : *nodes)
            {
                const auto respawn = node.is_object() ? node.find("enemyRespawn") : node.end();
                if (!node.is_object() || respawn == node.end()) continue;

                int room_id = 0;
                EnemyRespawnPolicy policy{};
                if (!read_room_id(node, room_id) || !respawn->is_string() ||
                    !parse_respawn(respawn->get<std::string>(), policy))
                {
                    error = "nodes: enemyRespawn needs a roomId and one of always / until_defeated / once";
                    return false;
                }
                out[room_id] = policy;
            }
            return true;
        }

        bool parse_enemy(const json& entity, const std::unordered_map<int, EnemyRespawnPolicy>& room_defaults,
            EnemyPlacement& out, std::string& error)
        {
            const auto id = entity.find("id");
            if (id != entity.end() && id->is_string()) out.id = id->get<std::string>();
            else if (id != entity.end() && id->is_number_integer()) out.id = std::to_string(id->get<std::int64_t>());
            if (out.id.empty())
            {
                error = "entities: an enemy needs an id";
                return false;
            }

            const std::string where = "entities[" + out.id + "]: ";
            if (!read_room_id(entity, out.room_id))
            {
                error = where + "needs an integer roomId (0-255)";
                return false;
            }
            if (!read_local_pos(entity, "position", out.local_pos))
            {
                error = where + "position x/y must be room-local pixels";
                return false;
            }

            const auto properties = entity.find("properties");
            if (properties == entity.end() || !properties->is_object())
            {
                error = where + "needs a properties object carrying at least \"kind\"";
                return false;
            }

            const auto kind = properties->find("kind");
            if (kind == properties->end() || !kind->is_string() || kind->get<std::string>().empty())
            {
                error = where + "properties.kind must name an enemy definition id";
                return false;
            }
            out.kind = kind->get<std::string>();

            if (const auto palette = properties->find("palette"); palette != properties->end())
            {
                if (!palette->is_string())
                {
                    error = where + "properties.palette must be a string";
                    return false;
                }
                out.palette = palette->get<std::string>();
            }

            if (const auto facing = properties->find("facing"); facing != properties->end())
            {
                if (!facing->is_string() || !parse_facing(facing->get<std::string>(), out.facing))
                {
                    error = where + "properties.facing must be player / left / right";
                    return false;
                }
            }

            if (const auto respawn = properties->find("respawn"); respawn != properties->end())
            {
                if (!respawn->is_string() || !parse_respawn(respawn->get<std::string>(), out.respawn))
                {
                    error = where + "properties.respawn must be always / until_defeated / once";
                    return false;
                }
            }
            else if (const auto room = room_defaults.find(out.room_id); room != room_defaults.end())
            {
                out.respawn = room->second;
            }

            if (const auto despawn = properties->find("despawnOffscreen"); despawn != properties->end())
            {
                if (!despawn->is_boolean())
                {
                    error = where + "properties.despawnOffscreen must be true / false";
                    return false;
                }
                out.despawn_offscreen = despawn->get<bool>();
            }
            return true;
        }

        bool parse_definition(const json& root, StageDefinitionData& out, std::string& error)
        {
            if (!root.is_object())
            {
                error = "root must be an object";
                return false;
            }

            if (const auto stage = root.find("stage"); stage != root.end() && stage->is_object())
            {
                if (!parse_start(*stage, out.start, error)) return false;
            }

            std::unordered_map<int, EnemyRespawnPolicy> room_defaults;
            if (!parse_room_defaults(root, room_defaults, error)) return false;

            const auto entities = root.find("entities");
            if (entities == root.end()) return true;
            if (!entities->is_array())
            {
                error = "entities: must be an array";
                return false;
            }

            std::unordered_set<std::string> ids;
            for (const auto& entity : *entities)
            {
                if (!entity.is_object())
                {
                    error = "entities: every entry must be an object";
                    return false;
                }
                // Only enemies are placed by the game so far; other types
                // (item, object) are left in the file for later readers.
                if (entity.value("type", std::string{}) != "enemy") continue;

                EnemyPlacement placement{};
                if (!parse_enemy(entity, room_defaults, placement, error)) return false;
                if (!ids.insert(placement.id).second)
                {
                    error = "entities[" + placement.id + "]: id is used more than once";
                    return false;
                }
                out.enemies.push_back(std::move(placement));
            }
            return true;
        }
    }

    bool StageDefinitionLoader::LoadFromFile(const std::wstring& filepath, StageDefinitionData& out, std::string& error)
    {
        try
        {
            std::ifstream stream(utils::wstring_to_utf8(filepath), std::ios::binary);
            if (!stream.is_open())
            {
                error = "cannot open file";
                return false;
            }
            const std::string source{ std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>() };
            return LoadFromJson(source, out, error);
        }
        catch (const std::exception& e)
        {
            error = e.what();
            return false;
        }
    }

    bool StageDefinitionLoader::LoadFromJson(std::string_view source, StageDefinitionData& out, std::string& error)
    {
        try
        {
            StageDefinitionData parsed{};
            if (!parse_definition(json::parse(source.begin(), source.end()), parsed, error)) return false;
            out = std::move(parsed);
            return true;
        }
        catch (const std::exception& e)
        {
            error = e.what();
            return false;
        }
    }
}
