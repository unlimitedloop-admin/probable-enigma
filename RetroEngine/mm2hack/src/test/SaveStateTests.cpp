#include "pch.h"

#include "SaveStateTests.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "apps/foundation/math/CoordinateTypes.h"
#include "apps/scenes/IBaseScene.h"
#include "apps/scenes/IStageAssetProvider.h"
#include "apps/scenes/phases/AbstractActionPhase.h"
#include "apps/scenes/SceneManager.h"
#include "apps/systems/audio/ApuVoice.h"
#include "apps/systems/audio/ApuVoiceArbiter.h"
#include "apps/systems/audio/AudioConfigLoader.h"
#include "apps/systems/audio/BgmManager.h"
#include "apps/systems/audio/BgmTransportState.h"
#include "apps/systems/audio/ChannelManager.h"
#include "apps/systems/audio/ISoundChannel.h"
#include "apps/systems/audio/SeManager.h"
#include "apps/systems/audio/SePriority.h"
#include "apps/systems/audio/SeRestorePolicy.h"
#include "apps/systems/audio/SeTransportState.h"
#include "apps/systems/physics/ILadderService.h"
#include "apps/systems/physics/ITerrainProbe.h"
#include "apps/systems/physics/Probes.h"
#include "apps/systems/physics/TileAttribute.h"
#include "apps/systems/scrolling/atomic/IScrollRuleProvider.h"
#include "apps/systems/scrolling/atomic/ScrollTypes.h"
#include "apps/world/entity/avatar/PlayerEntity.h"
#include "apps/world/entity/avatar/PlayerEntityState.h"
#include "apps/world/entity/enemy/lists/EnemyLists.h"
#include "apps/world/entity/EntityManager.h"
#include "apps/world/entity/EntityStateFactory.h"
#include "core/assembly/InputTypes.h"
#include "core/assembly/StateProvider.h"
#include "core/save/SaveData.h"
#include "core/save/SaveSystem.h"
#include "core/save/StateIO.h"

namespace mm2hack::test
{
    namespace
    {
        using apps::foundation::math::Vec2;
        using apps::scenes::SceneID;
        using apps::scenes::SpriteManagerId;
        using apps::systems::audio::ApuVoice;
        using apps::systems::audio::ApuVoiceArbiter;
        using apps::systems::audio::ApuVoiceClaim;
        using apps::systems::audio::AudioConfigLoader;
        using apps::systems::audio::BgmManager;
        using apps::systems::audio::BgmPlaybackStatus;
        using apps::systems::audio::BgmTransportState;
        using apps::systems::audio::ChannelManager;
        using apps::systems::audio::ContinuousSeTransportState;
        using apps::systems::audio::ISoundChannel;
        using apps::systems::audio::SeManager;
        using apps::systems::audio::SePlaybackStatus;
        using apps::systems::audio::SePriority;
        using apps::systems::audio::SeRestorePolicy;
        using apps::systems::audio::SeTransportState;
        using apps::systems::physics::AvatarDirection;
        using apps::systems::physics::ILadderService;
        using apps::systems::physics::ITerrainProbe;
        using apps::systems::physics::LadderEntryKind;
        using apps::systems::physics::OverlapXFix;
        using apps::systems::physics::Probes;
        using apps::systems::physics::SweepHHit;
        using apps::systems::physics::SweepVHit;
        using apps::systems::physics::TileAttribute;
        using apps::systems::scrolling::atomic::IScrollRuleProvider;
        using apps::systems::scrolling::atomic::ScrollKind;
        using apps::world::entity::EntityManager;
        using apps::world::entity::EntityManagerState;
        using apps::world::entity::EntityStateFactory;
        using apps::world::entity::avatar::PlayerEntity;
        using apps::world::entity::avatar::PlayerEntityState;
        using core::assembly::InputSnapshot;
        using core::assembly::Key16;
        using core::assembly::KeyFrameState;
        using core::assembly::LogicalBinding;
        using core::assembly::StateProvider;
        using core::save::SaveData;
        using core::save::SaveSystem;
        using core::save::StateReader;
        using core::save::StateWriter;

        constexpr std::uint32_t kDemoStage2StateVersion = 3;
        constexpr std::uint8_t kAbstractActionPhaseType = 1;

        class TestRunner final
        {
        public:
            void Check(bool passed, std::wstring_view name) noexcept
            {
                ++_total;
                if (passed)
                {
                    return;
                }

                ++_failed;
                std::fwprintf(stderr, L"[FAIL] %.*s\n",
                    static_cast<int>(name.size()), name.data());
            }

            [[nodiscard]] int Result() const noexcept
            {
                std::fwprintf(stderr, L"Save-state tests: %d passed, %d failed.\n",
                    _total - _failed, _failed);
                return _failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
            }

        private:
            int _total{};
            int _failed{};
        };

        class EmptyInput final : public StateProvider
        {
        public:
            void BeginTick(std::uint64_t tick) noexcept override { _snapshot.tick = tick; }
            void EndTick() noexcept override {}
            bool UpdateJoystick() noexcept override { return true; }
            [[nodiscard]] InputSnapshot GetSnapshot() const override { return _snapshot; }
            [[nodiscard]] const KeyFrameState& Get(Key16 key) const noexcept override
            {
                return _snapshot.keys[static_cast<std::size_t>(key)];
            }
            [[nodiscard]] bool IsPressed(Key16 key) const noexcept override { return Get(key).pressed; }
            [[nodiscard]] std::int32_t Frames(Key16 key) const noexcept override { return Get(key).frames; }
            void SetBindings(const std::vector<LogicalBinding>& bindings) override { (void)bindings; }

        private:
            InputSnapshot _snapshot{};
        };

        class FakeSoundChannel final : public ISoundChannel
        {
        public:
            bool Load(const std::wstring& filepath) override
            {
                _loaded = !filepath.empty();
                _playing = false;
                _paused = false;
                _position_milliseconds = 0;
                return _loaded;
            }
            void Play(bool loop) override
            {
                (void)loop;
                if (!_loaded) return;
                _position_milliseconds = 0;
                _playing = true;
                _paused = false;
            }
            void Stop() override
            {
                _playing = false;
                _paused = false;
                _position_milliseconds = 0;
            }
            void Pause() override
            {
                if (!_playing) return;
                _playing = false;
                _paused = true;
            }
            void Resume(bool loop) override
            {
                (void)loop;
                if (!_paused) return;
                _playing = true;
                _paused = false;
            }
            void SetVolume(int volume) override { _volume = volume; }
            int GetVolume() const override { return _volume; }
            bool IsPlaying() const override { return _playing; }
            std::int64_t GetPositionMilliseconds() const override
            {
                return _position_milliseconds;
            }
            void SetPositionMilliseconds(std::int64_t position) override
            {
                _position_milliseconds = position;
            }
            void StartFade(int targetVolume, int durationFrames) override
            {
                (void)durationFrames;
                _volume = targetVolume;
            }
            void Update() override {}
            int GetNativeHandle() const override { return -1; }

        private:
            bool _loaded = false;
            bool _playing = false;
            bool _paused = false;
            int _volume = 0;
            std::int64_t _position_milliseconds = 0;
        };

        class TemporarySaveFile final
        {
        public:
            TemporarySaveFile()
            {
                std::error_code error;
                const std::filesystem::path temporary_root =
                    std::filesystem::temp_directory_path(error);
                if (error) return;

                const auto nonce = std::chrono::steady_clock::now()
                    .time_since_epoch().count();
                const std::wstring prefix = L"mm2hack_save_state_tests_" +
                    std::to_wstring(nonce) + L"_";
                for (int attempt = 0; attempt < 100; ++attempt)
                {
                    _directory = temporary_root / (prefix + std::to_wstring(attempt));
                    if (std::filesystem::create_directory(_directory, error))
                    {
                        _path = _directory / L"state.sav";
                        _ready = true;
                        return;
                    }
                    if (error)
                    {
                        error.clear();
                    }
                }
            }

            ~TemporarySaveFile()
            {
                if (!_ready) return;
                std::error_code ignored;
                std::filesystem::remove(_path, ignored);
                std::filesystem::path temporary_path = _path;
                temporary_path += L".tmp";
                std::filesystem::remove(temporary_path, ignored);
                std::filesystem::remove(_directory, ignored);
            }

            [[nodiscard]] bool IsReady() const noexcept { return _ready; }
            [[nodiscard]] std::wstring Path() const { return _path.wstring(); }

        private:
            std::filesystem::path _directory{};
            std::filesystem::path _path{};
            bool _ready = false;
        };

        class EmptyTerrain final : public ITerrainProbe
        {
        public:
            void SetCurrentPage(std::size_t page_index) noexcept override { _page_index = page_index; }
            SweepHHit SweepHorizontal(const Probes& probes, double dx, bool air_flag) const override
            {
                (void)probes;
                (void)dx;
                (void)air_flag;
                return {};
            }
            SweepVHit SweepVertical(const Probes& probes, Vec2 velocity) const override
            {
                (void)probes;
                (void)velocity;
                return {};
            }
            bool IsGroundLike(AvatarDirection direction, const Probes& probes, double dy) const override
            {
                (void)direction;
                (void)probes;
                (void)dy;
                return false;
            }
            OverlapXFix ResolveOverlapX(const Probes& probes, double parity) const override
            {
                (void)probes;
                (void)parity;
                return {};
            }
            TileAttribute AttributeAt(Vec2 position) const override
            {
                (void)position;
                return TileAttribute::None;
            }
            TileAttribute AttributeAt(double world_x, double world_y) const override
            {
                (void)world_x;
                (void)world_y;
                return TileAttribute::None;
            }

        private:
            std::size_t _page_index{};
        };

        class EmptyLadder final : public ILadderService
        {
        public:
            bool CanGrabAt(const Vec2& world_position) const override
            {
                (void)world_position;
                return false;
            }
            std::optional<Vec2> TryGetCenterXAt(const Vec2& world_position) const override
            {
                (void)world_position;
                return std::nullopt;
            }
            void setEntryKind(LadderEntryKind value) noexcept override { _entry_kind = value; }
            LadderEntryKind getEntryKind() const noexcept override { return _entry_kind; }

        private:
            LadderEntryKind _entry_kind{ LadderEntryKind::None };
        };

        class EmptyScrollRules final : public IScrollRuleProvider
        {
        public:
            ScrollKind RightType(std::size_t page_index) const override { return Type(page_index); }
            ScrollKind LeftType(std::size_t page_index) const override { return Type(page_index); }
            ScrollKind UpType(std::size_t page_index) const override { return Type(page_index); }
            ScrollKind DownType(std::size_t page_index) const override { return Type(page_index); }
            std::int16_t RightRoom(std::size_t page_index) const override { return Room(page_index); }
            std::int16_t LeftRoom(std::size_t page_index) const override { return Room(page_index); }
            std::int16_t UpRoom(std::size_t page_index) const override { return Room(page_index); }
            std::int16_t DownRoom(std::size_t page_index) const override { return Room(page_index); }
            int ToPageIndex(std::uint8_t room) const override
            {
                (void)room;
                return -1;
            }
            Vec2 PageOriginPx(std::size_t page_index, int page_width, int page_height) const override
            {
                (void)page_index;
                (void)page_width;
                (void)page_height;
                return {};
            }

        private:
            static ScrollKind Type(std::size_t page_index) noexcept
            {
                (void)page_index;
                return ScrollKind::None;
            }
            static std::int16_t Room(std::size_t page_index) noexcept
            {
                (void)page_index;
                return -1;
            }
        };

        class TestAssets final : public apps::scenes::IStageAssetProvider
        {
        public:
            SpriteManagerId PlayerSprite() const noexcept override { return 1; }
            SpriteManagerId PlayerChargeLevel1Sprite() const noexcept override { return 2; }
            SpriteManagerId PlayerChargeLevel2Sprite() const noexcept override { return 3; }
            SpriteManagerId PlayerAttackSprite() const noexcept override { return 4; }
            SpriteManagerId EffectsSprite() const noexcept override { return 5; }
            SpriteManagerId SlidingDustEffectSprite() const noexcept override { return 6; }
            SpriteManagerId ChargeEffectSprite() const noexcept override { return 7; }
            bool TryEnemySprite(
                apps::world::entity::enemy::EnemyKind kind,
                SpriteManagerId& out) const noexcept override
            {
                (void)kind;
                (void)out;
                return false;
            }
        };

        std::vector<std::uint8_t> ToBytes(const std::string& value)
        {
            std::vector<std::uint8_t> bytes{};
            bytes.reserve(value.size());
            for (const char byte : value)
            {
                bytes.emplace_back(static_cast<std::uint8_t>(byte));
            }
            return bytes;
        }

        std::string ToString(const std::vector<std::uint8_t>& bytes)
        {
            std::string value{};
            value.reserve(bytes.size());
            for (const std::uint8_t byte : bytes)
            {
                value.push_back(static_cast<char>(byte));
            }
            return value;
        }

        bool EqualSaveData(const SaveData& left, const SaveData& right)
        {
            return left.sequenceID == right.sequenceID &&
                left.sceneID == right.sceneID &&
                left.scenePayload == right.scenePayload;
        }

        bool ReadFileBytes(const std::wstring& path, std::vector<std::uint8_t>& bytes)
        {
            std::ifstream stream(path, std::ios::in | std::ios::binary);
            if (!stream) return false;

            const std::string source{
                std::istreambuf_iterator<char>(stream),
                std::istreambuf_iterator<char>()
            };
            if (stream.bad()) return false;
            bytes = ToBytes(source);
            return true;
        }

        bool WriteFileBytes(const std::wstring& path, const std::vector<std::uint8_t>& bytes)
        {
            std::ofstream stream(path, std::ios::out | std::ios::binary | std::ios::trunc);
            if (!stream) return false;
            if (!bytes.empty())
            {
                stream.write(
                    reinterpret_cast<const char*>(bytes.data()),
                    static_cast<std::streamsize>(bytes.size()));
            }
            stream.flush();
            return stream.good();
        }

        void TestSaveFileEnvelope(TestRunner& runner)
        {
            TemporarySaveFile file{};
            const SaveData expected{
                7,
                static_cast<std::int32_t>(SceneID::DemoStage2),
                { 0x00, 0x01, 0x7F, 0x80, 0xFF }
            };
            SaveData loaded{};
            const bool round_trip = file.IsReady() &&
                SaveSystem::Save(file.Path(), expected) &&
                SaveSystem::Load(file.Path(), loaded) &&
                EqualSaveData(expected, loaded);
            runner.Check(round_trip, L"round-trip outer save-file envelope");
            if (!round_trip) return;

            std::vector<std::uint8_t> canonical{};
            if (!ReadFileBytes(file.Path(), canonical))
            {
                runner.Check(false, L"read canonical save-file bytes");
                return;
            }

            const auto rejects_without_mutation = [&file](
                const std::vector<std::uint8_t>& candidate)
            {
                SaveData destination{ 99, 88, { 0x11, 0x22, 0x33 } };
                const SaveData before = destination;
                return WriteFileBytes(file.Path(), candidate) &&
                    !SaveSystem::Load(file.Path(), destination) &&
                    EqualSaveData(before, destination);
            };

            auto bad_magic = canonical;
            bad_magic.front() ^= 0xFF;
            runner.Check(
                rejects_without_mutation(bad_magic),
                L"reject bad save-file magic without mutating destination");

            auto bad_version = canonical;
            constexpr std::size_t kVersionOffset = 8;
            bad_version[kVersionOffset] ^= 0x01;
            runner.Check(
                rejects_without_mutation(bad_version),
                L"reject unsupported save-file version without mutating destination");

            auto oversized = canonical;
            constexpr std::size_t kPayloadSizeOffset = 20;
            for (std::size_t index = 0; index < sizeof(std::uint32_t); ++index)
            {
                oversized[kPayloadSizeOffset + index] = 0xFF;
            }
            runner.Check(
                rejects_without_mutation(oversized),
                L"reject oversized save-file payload without allocation");

            auto trailing = canonical;
            trailing.push_back(0xA5);
            runner.Check(
                rejects_without_mutation(trailing),
                L"reject trailing save-file bytes without mutating destination");

            bool rejected_all_truncations = true;
            for (std::size_t size = 0; size < canonical.size(); ++size)
            {
                auto truncated = canonical;
                truncated.resize(size);
                if (!rejects_without_mutation(truncated))
                {
                    rejected_all_truncations = false;
                    break;
                }
            }
            runner.Check(
                rejected_all_truncations,
                L"reject every truncated save-file envelope");
        }

        bool SerializePlayerState(const PlayerEntityState& state, std::vector<std::uint8_t>& bytes)
        {
            std::ostringstream stream(std::ios::out | std::ios::binary);
            StateWriter writer(stream);
            if (!state.Save(writer) || !stream.good())
            {
                return false;
            }
            bytes = ToBytes(stream.str());
            return true;
        }

        bool DeserializePlayerState(const std::vector<std::uint8_t>& bytes, PlayerEntityState& state)
        {
            std::istringstream stream(ToString(bytes), std::ios::in | std::ios::binary);
            StateReader reader(stream);
            return state.Load(reader) && stream.peek() == std::char_traits<char>::eof();
        }

        bool MakeEntityState(EntityManagerState& state)
        {
            EntityManager manager{};
            manager.Spawn<PlayerEntity>(1, 4, 5, 2, 3);
            return manager.CaptureState(state);
        }

        bool EncodeDemoStage2State(const EntityManagerState& entities, std::vector<std::uint8_t>& bytes)
        {
            apps::scenes::phases::AbstractActionPhaseState phase{};
            phase.ready_ui = { 3.0, 0.0, false };
            BgmTransportState bgm{};
            bgm.track_name = L"demo_stage_2_bgm";
            bgm.playback_status = BgmPlaybackStatus::Paused;
            bgm.voices = {
                { ApuVoice::Pulse1, 12'345, 255 },
                { ApuVoice::Noise, 12'360, 192 }
            };
            bgm.master_volume = 255;
            bgm.loop_start_seconds = 11.77;
            bgm.loop_end_seconds = 58.49;
            bgm.fade_target = 255;
            SeTransportState se{};
            se.master_volume = 255;
            se.continuous_instances = {
                {
                    L"rock_buster_charge",
                    SePlaybackStatus::Paused,
                    { { ApuVoice::Pulse2, 3'900, 255 } }
                }
            };

            std::ostringstream stream(std::ios::out | std::ios::binary);
            StateWriter writer(stream);
            if (!writer.WriteU32(kDemoStage2StateVersion) ||
                !writer.WriteU8(kAbstractActionPhaseType) ||
                !writer.WriteU32(phase.scroll.page_index) ||
                !writer.WriteU32(1234) ||
                !phase.Save(writer) || !entities.Save(writer) ||
                !bgm.Save(writer) || !se.Save(writer) || !stream.good())
            {
                return false;
            }
            bytes = ToBytes(stream.str());
            return true;
        }

        bool IsValidDemoStage2Payload(const std::vector<std::uint8_t>& payload)
        {
            SaveData data{};
            data.sceneID = static_cast<std::int32_t>(SceneID::DemoStage2);
            data.scenePayload = payload;
            return apps::scenes::SceneManager::ValidateState(data);
        }

        std::uint64_t Checksum(const std::vector<std::uint8_t>& bytes) noexcept
        {
            constexpr std::uint64_t kOffsetBasis = 14695981039346656037ULL;
            constexpr std::uint64_t kPrime = 1099511628211ULL;
            std::uint64_t checksum = kOffsetBasis;
            for (const std::uint8_t byte : bytes)
            {
                checksum ^= byte;
                checksum *= kPrime;
            }
            return checksum;
        }

        void InjectPlayerServices(
            PlayerEntity& player,
            EmptyInput& input,
            EmptyTerrain& terrain,
            EmptyLadder& ladder,
            EmptyScrollRules& scroll_rules)
        {
            player.SetInput(&input);
            player.SetTerrainProbe(&terrain);
            player.SetLadderService(&ladder);
            player.SetScrollContext(&scroll_rules, 0);
            player.SetViewBounds({ -10'000.0, 10'000.0, -10'000.0, 10'000.0 });
        }

        bool IsVoiceOwner(
            const ApuVoiceArbiter& arbiter,
            ApuVoice voice,
            std::wstring_view expected_owner)
        {
            const auto* owner = arbiter.GetOwner(voice);
            return owner != nullptr && owner->name == expected_owner;
        }

        void TestApuVoiceArbitration(TestRunner& runner)
        {
            ApuVoiceArbiter arbiter{};
            const auto splash = arbiter.Acquire(L"splash", {
                ApuVoiceClaim{ ApuVoice::Pulse2, SePriority::Normal },
                ApuVoiceClaim{ ApuVoice::Noise, SePriority::Normal }
                });
            runner.Check(
                splash.accepted &&
                IsVoiceOwner(arbiter, ApuVoice::Pulse2, L"splash") &&
                IsVoiceOwner(arbiter, ApuVoice::Noise, L"splash"),
                L"acquire all voices for a multi-stem SE");

            const auto rejected = arbiter.Acquire(L"low_priority", {
                ApuVoiceClaim{ ApuVoice::Pulse1, SePriority::High },
                ApuVoiceClaim{ ApuVoice::Noise, SePriority::Low }
                });
            runner.Check(
                !rejected.accepted &&
                !arbiter.IsOwned(ApuVoice::Pulse1) &&
                IsVoiceOwner(arbiter, ApuVoice::Pulse2, L"splash") &&
                IsVoiceOwner(arbiter, ApuVoice::Noise, L"splash"),
                L"reject multi-stem acquisition atomically");

            const auto replacement = arbiter.Acquire(L"buster", {
                ApuVoiceClaim{ ApuVoice::Pulse2, SePriority::Normal }
                });
            runner.Check(
                replacement.accepted && replacement.displacedOwners.size() == 1 &&
                replacement.displacedOwners.front() == L"splash" &&
                IsVoiceOwner(arbiter, ApuVoice::Pulse2, L"buster") &&
                !arbiter.IsOwned(ApuVoice::Noise),
                L"equal priority replaces the prior complete SE");

            const auto independent = arbiter.Acquire(L"ladder", {
                ApuVoiceClaim{ ApuVoice::Triangle, SePriority::Low }
                });
            runner.Check(
                independent.accepted && independent.displacedOwners.empty() &&
                IsVoiceOwner(arbiter, ApuVoice::Pulse2, L"buster") &&
                IsVoiceOwner(arbiter, ApuVoice::Triangle, L"ladder"),
                L"independent APU voices coexist");

            const auto higher_priority = arbiter.Acquire(L"high_priority", {
                ApuVoiceClaim{ ApuVoice::Triangle, SePriority::High }
                });
            runner.Check(
                higher_priority.accepted && higher_priority.displacedOwners.size() == 1 &&
                higher_priority.displacedOwners.front() == L"ladder" &&
                IsVoiceOwner(arbiter, ApuVoice::Triangle, L"high_priority"),
                L"higher priority replaces the prior voice owner");

            const auto duplicate = arbiter.Acquire(L"invalid", {
                ApuVoiceClaim{ ApuVoice::Dpcm, SePriority::High },
                ApuVoiceClaim{ ApuVoice::Dpcm, SePriority::High }
                });
            runner.Check(
                !duplicate.accepted && !arbiter.IsOwned(ApuVoice::Dpcm),
                L"reject duplicate voice claims");
        }

        void TestAudioConfiguration(TestRunner& runner)
        {
            AudioConfigLoader loader{};
            constexpr std::string_view valid = R"json(
                {
                    "bgm": {
                        "track": {
                            "channels": [
                                { "file": "noise.wav", "volume": 128, "voice": "noise" },
                                { "file": "pulse.wav", "voice": "pulse1" }
                            ],
                            "loop_start": 1.5,
                            "loop_end": 2.5
                        }
                    },
                    "se": {
                        "effect": {
                            "channels": [
                                { "file": "pulse.wav", "voice": "pulse2", "priority": 0 },
                                { "file": "dpcm.wav", "voice": "dpcm", "priority": 2 }
                            ]
                        },
                        "continuous": {
                            "file": "loop.wav",
                            "voice": "triangle",
                            "loop_start": 1.0,
                            "loop_end": 2.0,
                            "restore_policy": "continuous"
                        }
                    }
                }
                )json";

            const bool loaded = loader.LoadFromJson(valid);
            const auto bgm = loader.GetBgmConfigs().find(L"track");
            const auto se = loader.GetSeConfigs().find(L"effect");
            runner.Check(
                loaded && bgm != loader.GetBgmConfigs().end() &&
                bgm->second.channels.size() == 2 &&
                bgm->second.channels[0].voice == ApuVoice::Noise &&
                bgm->second.channels[0].volume == 128 &&
                se != loader.GetSeConfigs().end() && se->second.channels.size() == 2 &&
                se->second.channels[0].priority == SePriority::Low &&
                se->second.channels[1].priority == SePriority::High &&
                se->second.restorePolicy == SeRestorePolicy::Transient &&
                loader.GetSeConfigs().at(L"continuous").restorePolicy ==
                    SeRestorePolicy::Continuous,
                L"parse valid explicit APU voice configuration");

            const auto preserved = [&loader]()
            {
                return loader.GetBgmConfigs().size() == 1 &&
                    loader.GetBgmConfigs().contains(L"track") &&
                    loader.GetSeConfigs().size() == 2 &&
                    loader.GetSeConfigs().contains(L"effect") &&
                    loader.GetSeConfigs().contains(L"continuous");
            };
            const auto rejects_without_mutation = [&loader, &preserved](
                std::string_view source)
            {
                return !loader.LoadFromJson(source) && preserved();
            };

            runner.Check(
                rejects_without_mutation(
                    R"json({"se":{"bad":{"channels":[{"file":"x.wav","voice":"saw"}]}}})json"),
                L"reject unknown APU voice without mutating configuration");
            runner.Check(
                rejects_without_mutation(
                    R"json({"se":{"bad":{"channels":[{"file":"a.wav","voice":"noise"},{"file":"b.wav","voice":"noise"}]}}})json"),
                L"reject duplicate APU voices without mutating configuration");
            runner.Check(
                rejects_without_mutation(
                    R"json({"se":{"bad":{"channels":[{"file":"x.wav","voice":"pulse1","priority":3}]}}})json"),
                L"reject out-of-range SE priority without mutating configuration");
            runner.Check(
                rejects_without_mutation(
                    R"json({"bgm":{"bad":{"channels":[{"file":"x.wav","voice":"pulse1","volume":256}]}}})json"),
                L"reject out-of-range volume without mutating configuration");
            runner.Check(
                rejects_without_mutation(
                    R"json({"se":{"bad":{"channels":[{"file":"x.wav"}]}}})json"),
                L"reject missing APU voice without mutating configuration");
            runner.Check(
                rejects_without_mutation(R"json({"se":)json"),
                L"reject malformed audio JSON without mutating configuration");
            runner.Check(
                rejects_without_mutation(
                    R"json({"se":{"bad":{"file":"x.wav","voice":"pulse1","restore_policy":"forever"}}})json"),
                L"reject unknown SE restore policy without mutating configuration");
            runner.Check(
                rejects_without_mutation(
                    R"json({"se":{"bad":{"file":"x.wav","voice":"pulse1","restore_policy":"continuous"}}})json"),
                L"reject continuous SE without a loop range");
        }

        void AddFakeApuChannels(ChannelManager& channels)
        {
            for (std::size_t index = 0; index < apps::systems::audio::kApuVoiceCount; ++index)
            {
                channels.AddChannel(std::make_unique<FakeSoundChannel>());
            }
        }

        bool EqualBgmTransport(
            const BgmTransportState& left,
            const BgmTransportState& right)
        {
            if (left.track_name != right.track_name ||
                left.playback_status != right.playback_status ||
                left.master_volume != right.master_volume ||
                left.loop_start_seconds != right.loop_start_seconds ||
                left.loop_end_seconds != right.loop_end_seconds ||
                left.is_fading != right.is_fading ||
                left.fade_target != right.fade_target ||
                left.fade_step != right.fade_step ||
                left.fade_frames_remaining != right.fade_frames_remaining ||
                left.voices.size() != right.voices.size())
            {
                return false;
            }
            for (std::size_t index = 0; index < left.voices.size(); ++index)
            {
                const auto& left_voice = left.voices[index];
                const auto& right_voice = right.voices[index];
                if (left_voice.voice != right_voice.voice ||
                    left_voice.position_milliseconds != right_voice.position_milliseconds ||
                    left_voice.logical_volume != right_voice.logical_volume)
                {
                    return false;
                }
            }
            return true;
        }

        void TestBgmTransportState(TestRunner& runner)
        {
            const std::vector<std::wstring> files{ L"noise.wav", L"pulse.wav" };
            const std::vector<int> volumes{ 128, 255 };
            const std::vector<ApuVoice> voices{ ApuVoice::Noise, ApuVoice::Pulse1 };

            ChannelManager source_channels{ 0 };
            AddFakeApuChannels(source_channels);
            BgmManager source{ source_channels };
            const bool source_ready =
                source.RegisterBgm(L"track", files, volumes, voices, 1.5, 2.5);
            source.SetMasterVolume(200);
            const bool source_played = source.Play(L"track");
            source_channels.SetPositionMilliseconds(
                static_cast<int>(apps::systems::audio::ToIndex(ApuVoice::Noise)), 1234);
            source_channels.SetPositionMilliseconds(
                static_cast<int>(apps::systems::audio::ToIndex(ApuVoice::Pulse1)), 1250);
            source.Pause();
            source.FadeOut(10);

            BgmTransportState snapshot{};
            const bool captured = source.CaptureState(snapshot);
            runner.Check(
                source_ready && source_played && captured && snapshot.IsValid() &&
                snapshot.playback_status == BgmPlaybackStatus::Paused &&
                snapshot.voices.size() == 2 &&
                snapshot.voices[0].voice == ApuVoice::Noise &&
                snapshot.voices[0].position_milliseconds == 1234 &&
                snapshot.voices[1].voice == ApuVoice::Pulse1 &&
                snapshot.voices[1].position_milliseconds == 1250 &&
                snapshot.is_fading && snapshot.fade_frames_remaining == 10,
                L"capture paused multi-stem BGM transport");

            std::ostringstream encoded_stream(std::ios::out | std::ios::binary);
            StateWriter encoded_writer(encoded_stream);
            const bool encoded = snapshot.Save(encoded_writer);
            std::istringstream decoded_stream(
                encoded_stream.str(), std::ios::in | std::ios::binary);
            StateReader decoded_reader(decoded_stream);
            BgmTransportState decoded{};
            runner.Check(
                encoded && decoded.Load(decoded_reader) &&
                decoded_stream.peek() == std::char_traits<char>::eof() &&
                EqualBgmTransport(snapshot, decoded),
                L"round-trip versioned BGM transport codec");

            const BgmTransportState before_decode_failure = decoded;
            std::string bad_version = encoded_stream.str();
            bad_version.front() ^= 0x01;
            std::istringstream invalid_stream(
                bad_version, std::ios::in | std::ios::binary);
            StateReader invalid_reader(invalid_stream);
            runner.Check(
                !decoded.Load(invalid_reader) &&
                EqualBgmTransport(before_decode_failure, decoded),
                L"invalid BGM codec input preserves destination state");

            ChannelManager restored_channels{ 0 };
            AddFakeApuChannels(restored_channels);
            BgmManager restored{ restored_channels };
            const bool target_ready =
                restored.RegisterBgm(L"track", files, volumes, voices, 1.5, 2.5);
            const bool validated = restored.ValidateState(snapshot);
            const bool restored_ok = restored.RestoreState(snapshot);
            BgmTransportState round_trip{};
            const bool recaptured = restored.CaptureState(round_trip);
            runner.Check(
                target_ready && validated && restored_ok && recaptured &&
                EqualBgmTransport(snapshot, round_trip),
                L"restore logical BGM transport by stable voice IDs");

            const BgmTransportState before_rejection = round_trip;
            auto invalid = snapshot;
            invalid.voices.push_back(invalid.voices.front());
            const bool rejected = !restored.ValidateState(invalid) &&
                !restored.RestoreState(invalid);
            BgmTransportState after_rejection{};
            runner.Check(
                rejected && restored.CaptureState(after_rejection) &&
                EqualBgmTransport(before_rejection, after_rejection),
                L"invalid BGM transport does not mutate live state");

            auto unknown_track = snapshot;
            unknown_track.track_name = L"missing";
            runner.Check(
                !restored.ValidateState(unknown_track),
                L"public BGM state validation rejects unknown resources");

            auto oversized_position = snapshot;
            oversized_position.voices.front().position_milliseconds =
                apps::systems::audio::BgmVoiceTransportState::kMaxPositionMilliseconds + 1;
            runner.Check(
                !restored.ValidateState(oversized_position),
                L"BGM state validation rejects oversized transport positions");
        }

        bool EqualSeTransport(const SeTransportState& left, const SeTransportState& right)
        {
            if (left.master_volume != right.master_volume ||
                left.continuous_instances.size() != right.continuous_instances.size())
            {
                return false;
            }
            for (std::size_t instance_index = 0;
                instance_index < left.continuous_instances.size(); ++instance_index)
            {
                const ContinuousSeTransportState& left_instance =
                    left.continuous_instances[instance_index];
                const ContinuousSeTransportState& right_instance =
                    right.continuous_instances[instance_index];
                if (left_instance.name != right_instance.name ||
                    left_instance.playback_status != right_instance.playback_status ||
                    left_instance.voices.size() != right_instance.voices.size())
                {
                    return false;
                }
                for (std::size_t voice_index = 0;
                    voice_index < left_instance.voices.size(); ++voice_index)
                {
                    const auto& left_voice = left_instance.voices[voice_index];
                    const auto& right_voice = right_instance.voices[voice_index];
                    if (left_voice.voice != right_voice.voice ||
                        left_voice.position_milliseconds != right_voice.position_milliseconds ||
                        left_voice.logical_volume != right_voice.logical_volume)
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        void TestSeTransportState(TestRunner& runner)
        {
            const auto channel_factory = []()
            {
                return std::make_unique<FakeSoundChannel>();
            };
            const std::vector<std::wstring> continuous_files{ L"charge.wav" };
            const std::vector<int> continuous_volumes{ 180 };
            const std::vector<ApuVoice> continuous_voices{ ApuVoice::Pulse2 };
            const std::vector<SePriority> continuous_priorities{ SePriority::High };
            const std::vector<std::wstring> transient_files{ L"splash.wav" };
            const std::vector<int> transient_volumes{ 255 };
            const std::vector<ApuVoice> transient_voices{ ApuVoice::Noise };

            SeManager source{ channel_factory };
            const bool source_ready = source.LoadSe(
                L"charge",
                continuous_files,
                continuous_volumes,
                continuous_voices,
                continuous_priorities,
                1.0,
                2.0,
                SeRestorePolicy::Continuous) &&
                source.LoadSe(
                    L"splash",
                    transient_files,
                    transient_volumes,
                    transient_voices);
            source.SetMasterVolume(200);
            source.PlaySe(L"charge");
            source.PlaySe(L"splash");
            source.Pause();
            source.Update();

            SeTransportState snapshot{};
            const bool captured = source.CaptureState(snapshot);
            runner.Check(
                source_ready && captured && snapshot.IsValid() &&
                snapshot.master_volume == 200 &&
                snapshot.continuous_instances.size() == 1 &&
                snapshot.continuous_instances.front().name == L"charge" &&
                snapshot.continuous_instances.front().playback_status ==
                    SePlaybackStatus::Paused &&
                snapshot.continuous_instances.front().voices.front().logical_volume == 180,
                L"capture only configured continuous SE");

            snapshot.continuous_instances.front().voices.front().position_milliseconds = 1234;
            std::ostringstream encoded_stream(std::ios::out | std::ios::binary);
            StateWriter encoded_writer(encoded_stream);
            const bool encoded = snapshot.Save(encoded_writer);
            std::istringstream decoded_stream(
                encoded_stream.str(), std::ios::in | std::ios::binary);
            StateReader decoded_reader(decoded_stream);
            SeTransportState decoded{};
            runner.Check(
                encoded && decoded.Load(decoded_reader) &&
                decoded_stream.peek() == std::char_traits<char>::eof() &&
                EqualSeTransport(snapshot, decoded),
                L"round-trip versioned continuous SE transport codec");

            const SeTransportState before_decode_failure = decoded;
            std::string bad_version = encoded_stream.str();
            bad_version.front() ^= 0x01;
            std::istringstream invalid_stream(
                bad_version, std::ios::in | std::ios::binary);
            StateReader invalid_reader(invalid_stream);
            runner.Check(
                !decoded.Load(invalid_reader) &&
                EqualSeTransport(before_decode_failure, decoded),
                L"invalid continuous SE codec input preserves destination state");

            SeManager restored{ channel_factory };
            const bool target_ready = restored.LoadSe(
                L"charge",
                continuous_files,
                continuous_volumes,
                continuous_voices,
                continuous_priorities,
                1.0,
                2.0,
                SeRestorePolicy::Continuous) &&
                restored.LoadSe(
                    L"splash",
                    transient_files,
                    transient_volumes,
                    transient_voices);
            restored.PlaySe(L"splash");
            const bool restored_ok = restored.RestoreState(snapshot);
            SeTransportState round_trip{};
            const bool recaptured = restored.CaptureState(round_trip);
            runner.Check(
                target_ready && restored_ok && recaptured &&
                EqualSeTransport(snapshot, round_trip) &&
                restored.IsVoiceOwnedBySe(ApuVoice::Pulse2) &&
                !restored.IsVoiceOwnedBySe(ApuVoice::Noise),
                L"restore continuous SE and discard transient ownership");

            const SeTransportState before_rejection = round_trip;
            auto invalid = snapshot;
            invalid.continuous_instances.front().name = L"missing";
            const bool rejected = !restored.ValidateState(invalid) &&
                !restored.RestoreState(invalid);
            SeTransportState after_rejection{};
            runner.Check(
                rejected && restored.CaptureState(after_rejection) &&
                EqualSeTransport(before_rejection, after_rejection),
                L"invalid continuous SE state does not mutate live audio");
        }

        void TestCorruptionValidation(TestRunner& runner)
        {
            EntityManagerState entities{};
            std::vector<std::uint8_t> valid{};
            const bool built = MakeEntityState(entities) && EncodeDemoStage2State(entities, valid);
            runner.Check(built, L"build canonical DemoStage2 payload");
            if (!built)
            {
                return;
            }

            runner.Check(IsValidDemoStage2Payload(valid), L"accept canonical DemoStage2 payload");

            bool rejected_all_truncations = true;
            for (std::size_t size = 0; size < valid.size(); ++size)
            {
                auto truncated = valid;
                truncated.resize(size);
                if (IsValidDemoStage2Payload(truncated))
                {
                    rejected_all_truncations = false;
                    break;
                }
            }
            runner.Check(rejected_all_truncations, L"reject every truncated payload");

            auto trailing = valid;
            trailing.push_back(0xA5);
            runner.Check(!IsValidDemoStage2Payload(trailing), L"reject trailing payload bytes");

            auto bad_version = valid;
            bad_version[0] ^= 0x01;
            runner.Check(!IsValidDemoStage2Payload(bad_version), L"reject unknown scene version");

            auto obsolete_version = valid;
            obsolete_version[0] = 1;
            obsolete_version[1] = 0;
            obsolete_version[2] = 0;
            obsolete_version[3] = 0;
            runner.Check(
                !IsValidDemoStage2Payload(obsolete_version),
                L"reject obsolete DemoStage2 version without BGM state");

            auto bgm_only_version = valid;
            bgm_only_version[0] = 2;
            runner.Check(
                !IsValidDemoStage2Payload(bgm_only_version),
                L"reject obsolete DemoStage2 version without SE state");

            auto bad_phase = valid;
            bad_phase[4] = 0xFF;
            runner.Check(!IsValidDemoStage2Payload(bad_phase), L"reject unknown phase type");

            auto bad_page = valid;
            bad_page[5] = 0x01;
            runner.Check(!IsValidDemoStage2Payload(bad_page), L"reject inconsistent page identity");

            auto bad_component = entities;
            bad_component.records.front().component_version = 0xFFFF;
            std::vector<std::uint8_t> bad_component_payload{};
            runner.Check(
                EncodeDemoStage2State(bad_component, bad_component_payload) &&
                !IsValidDemoStage2Payload(bad_component_payload),
                L"reject unknown entity component version");

            auto duplicate_player = entities;
            auto duplicate_record = duplicate_player.records.front();
            duplicate_record.instance_id = duplicate_player.next_instance_id;
            ++duplicate_player.next_instance_id;
            duplicate_player.records.emplace_back(std::move(duplicate_record));
            std::vector<std::uint8_t> duplicate_payload{};
            runner.Check(
                EncodeDemoStage2State(duplicate_player, duplicate_payload) &&
                !IsValidDemoStage2Payload(duplicate_payload),
                L"reject duplicate Player records");
        }

        void TestInvalidRestoreIsNonDestructive(TestRunner& runner)
        {
            EntityManager manager{};
            manager.Spawn<PlayerEntity>(1, 4, 5, 2, 3);

            EntityManagerState before{};
            std::vector<std::uint8_t> before_bytes{};
            const bool prepared = manager.CaptureState(before) &&
                EncodeDemoStage2State(before, before_bytes);
            runner.Check(prepared, L"capture entity state before rejected restore");
            if (!prepared)
            {
                return;
            }

            auto invalid = before;
            invalid.records.front().component_version = 0xFFFF;
            const TestAssets assets{};
            const EntityStateFactory factory(assets);
            runner.Check(!manager.RestoreState(invalid, factory), L"reject invalid entity restore");

            EntityManagerState after{};
            std::vector<std::uint8_t> after_bytes{};
            runner.Check(
                manager.CaptureState(after) &&
                EncodeDemoStage2State(after, after_bytes) &&
                before_bytes == after_bytes,
                L"rejected entity restore preserves current state");
        }

        void TestDeterministicContinuation(TestRunner& runner)
        {
            PlayerEntity original(1, 4, 5, 2, 3);
            original.pos = { 128.0, 64.0 };
            original.vel = { 0.25, 0.0 };

            EmptyInput input{};
            EmptyTerrain original_terrain{};
            EmptyTerrain restored_terrain{};
            EmptyLadder original_ladder{};
            EmptyLadder restored_ladder{};
            EmptyScrollRules scroll_rules{};
            InjectPlayerServices(original, input, original_terrain, original_ladder, scroll_rules);

            std::vector<std::uint8_t> snapshot{};
            const bool captured = SerializePlayerState(original.CaptureState(), snapshot);
            runner.Check(captured, L"serialize deterministic continuation origin");
            if (!captured)
            {
                return;
            }

            PlayerEntityState restored_state{};
            PlayerEntity restored(1, 4, 5, 2, 3);
            const bool restored_ok = DeserializePlayerState(snapshot, restored_state) &&
                restored.RestoreState(restored_state);
            runner.Check(restored_ok, L"restore deterministic continuation origin");
            if (!restored_ok)
            {
                return;
            }
            InjectPlayerServices(restored, input, restored_terrain, restored_ladder, scroll_rules);

            bool checksums_match = true;
            constexpr std::uint64_t kFramesToVerify = 240;
            for (std::uint64_t frame = 1; frame <= kFramesToVerify; ++frame)
            {
                input.BeginTick(frame);
                original.Update(nullptr, 1.0 / 60.0);
                restored.Update(nullptr, 1.0 / 60.0);
                (void)original.TakeFrameOutput();
                (void)restored.TakeFrameOutput();
                (void)original.ConsumeScrollRequest();
                (void)restored.ConsumeScrollRequest();

                std::vector<std::uint8_t> original_bytes{};
                std::vector<std::uint8_t> restored_bytes{};
                const auto original_state = original.CaptureState();
                const auto restored_frame_state = restored.CaptureState();
                if (!SerializePlayerState(original_state, original_bytes) ||
                    !SerializePlayerState(restored_frame_state, restored_bytes) ||
                    original_bytes != restored_bytes ||
                    Checksum(original_bytes) != Checksum(restored_bytes))
                {
                    const std::size_t shared_size = (std::min)(original_bytes.size(), restored_bytes.size());
                    std::size_t first_difference = shared_size;
                    for (std::size_t index = 0; index < shared_size; ++index)
                    {
                        if (original_bytes[index] != restored_bytes[index])
                        {
                            first_difference = index;
                            break;
                        }
                    }
                    std::fwprintf(stderr,
                        L"Continuation mismatch at frame %llu, byte %zu, checksums %llu/%llu.\n",
                        static_cast<unsigned long long>(frame), first_difference,
                        static_cast<unsigned long long>(Checksum(original_bytes)),
                        static_cast<unsigned long long>(Checksum(restored_bytes)));
                    std::fwprintf(stderr,
                        L"State valid %d/%d, pos %.3f/%.3f, vel %.3f/%.3f, "
                        L"status %d/%d, animation %d:%d:%d.\n",
                        original_state.IsValid(), restored_frame_state.IsValid(),
                        original_state.kinematic.position.x, original_state.kinematic.position.y,
                        original_state.kinematic.velocity.x, original_state.kinematic.velocity.y,
                        static_cast<int>(original_state.locomotion.status),
                        static_cast<int>(original_state.locomotion.next_status),
                        original_state.animation.tick, original_state.animation.frame,
                        original_state.animation.loops);
                    std::fwprintf(stderr,
                        L"Nested valid kinematic=%d locomotion=%d attack=%d environment=%d "
                        L"animation=%d; texture=%d/%d rock=%d charge=%u:%u:%u attack-charge=%u.\n",
                        original_state.kinematic.IsValid(), original_state.locomotion.IsValid(),
                        original_state.attack.IsValid(), original_state.environment.IsValid(),
                        original_state.animation.IsValid(), original_state.base_texture,
                        original_state.attack_texture, original_state.rock_buster.armTexture,
                        static_cast<unsigned>(original_state.charge.phase),
                        original_state.charge.frames, original_state.charge.phaseFrames,
                        original_state.attack.charge_frames);
                    std::fwprintf(stderr,
                        L"Facing=%d pose=%.3f charging=%d rock-offset=%.3f/%.3f; "
                        L"intro=%d:%d:%.3f:%.3f; bounds=%.3f/%.3f/%.3f/%.3f; "
                        L"origin=%.3f/%.3f page=%u pending=%d.\n",
                        static_cast<int>(original_state.facing),
                        original_state.attack.pose_time_seconds,
                        original_state.attack.charging,
                        original_state.rock_buster.offset.x,
                        original_state.rock_buster.offset.y,
                        original_state.intro.active,
                        static_cast<int>(original_state.intro.phase),
                        original_state.intro.timer,
                        original_state.intro.dropDuration,
                        original_state.view_bounds.leftX,
                        original_state.view_bounds.rightX,
                        original_state.view_bounds.topY,
                        original_state.view_bounds.bottomY,
                        original_state.page_origin.x,
                        original_state.page_origin.y,
                        original_state.scroll_page_index,
                        original_state.pending_scroll.has_value());
                    checksums_match = false;
                    break;
                }
                input.EndTick();
            }
            runner.Check(checksums_match, L"240 post-load simulation checksums match");
        }
    }

    int RunSaveStateTests() noexcept
    {
        const bool attached_to_parent_console =
            ::AttachConsole(ATTACH_PARENT_PROCESS) != FALSE;
        FILE* console_error{};
        (void)freopen_s(&console_error, "CONOUT$", "w", stderr);

        TestRunner runner{};
        try
        {
            TestApuVoiceArbitration(runner);
            TestAudioConfiguration(runner);
            TestBgmTransportState(runner);
            TestSeTransportState(runner);
            TestSaveFileEnvelope(runner);
            TestCorruptionValidation(runner);
            TestInvalidRestoreIsNonDestructive(runner);
            TestDeterministicContinuation(runner);
        }
        catch (const std::exception& exception)
        {
            (void)exception;
            runner.Check(false, L"unexpected standard exception");
        }
        catch (...)
        {
            runner.Check(false, L"unexpected non-standard exception");
        }
        const int result = runner.Result();
        std::fflush(stderr);
        if (attached_to_parent_console)
        {
            ::FreeConsole();
        }
        return result;
    }
}
