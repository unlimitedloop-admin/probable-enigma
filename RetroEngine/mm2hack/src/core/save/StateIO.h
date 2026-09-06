//==============================================================================
//
//  Project: mm2hack
//  StateIO.h
//
//  Fixed-width, little-endian binary I/O for save-state payloads.
//
//==============================================================================
#pragma once

#include <bit>
#include <cstdint>
#include <istream>
#include <limits>
#include <ostream>
#include <span>

namespace mm2hack::core::save
{
    class StateWriter final
    {
    public:
        explicit StateWriter(std::ostream& out) noexcept : _out(out) {}

        bool WriteU8(std::uint8_t value)
        {
            return WriteBytes(std::span<const std::uint8_t>(&value, 1));
        }

        bool WriteU16(std::uint16_t value)
        {
            const std::uint8_t bytes[] = {
                static_cast<std::uint8_t>(value),
                static_cast<std::uint8_t>(value >> 8)
            };
            return WriteBytes(bytes);
        }

        bool WriteU32(std::uint32_t value)
        {
            const std::uint8_t bytes[] = {
                static_cast<std::uint8_t>(value),
                static_cast<std::uint8_t>(value >> 8),
                static_cast<std::uint8_t>(value >> 16),
                static_cast<std::uint8_t>(value >> 24)
            };
            return WriteBytes(bytes);
        }

        bool WriteU64(std::uint64_t value)
        {
            const std::uint8_t bytes[] = {
                static_cast<std::uint8_t>(value),
                static_cast<std::uint8_t>(value >> 8),
                static_cast<std::uint8_t>(value >> 16),
                static_cast<std::uint8_t>(value >> 24),
                static_cast<std::uint8_t>(value >> 32),
                static_cast<std::uint8_t>(value >> 40),
                static_cast<std::uint8_t>(value >> 48),
                static_cast<std::uint8_t>(value >> 56)
            };
            return WriteBytes(bytes);
        }

        bool WriteI32(std::int32_t value)
        {
            return WriteU32(std::bit_cast<std::uint32_t>(value));
        }

        bool WriteBool(bool value)
        {
            return WriteU8(value ? 1U : 0U);
        }

        bool WriteF32(float value)
        {
            static_assert(sizeof(float) == sizeof(std::uint32_t));
            static_assert(std::numeric_limits<float>::is_iec559);
            return WriteU32(std::bit_cast<std::uint32_t>(value));
        }

        bool WriteF64(double value)
        {
            static_assert(sizeof(double) == sizeof(std::uint64_t));
            static_assert(std::numeric_limits<double>::is_iec559);
            return WriteU64(std::bit_cast<std::uint64_t>(value));
        }

        bool WriteBytes(std::span<const std::uint8_t> bytes)
        {
            if (!bytes.empty())
            {
                _out.write(
                    reinterpret_cast<const char*>(bytes.data()),
                    static_cast<std::streamsize>(bytes.size()));
            }
            return _out.good();
        }

        [[nodiscard]] bool Good() const noexcept { return _out.good(); }

    private:
        std::ostream& _out;
    };

    class StateReader final
    {
    public:
        explicit StateReader(std::istream& in) noexcept : _in(in) {}

        bool ReadU8(std::uint8_t& value)
        {
            return ReadBytes(std::span<std::uint8_t>(&value, 1));
        }

        bool ReadU16(std::uint16_t& value)
        {
            std::uint8_t bytes[2]{};
            if (!ReadBytes(bytes))
            {
                return false;
            }

            value =
                static_cast<std::uint16_t>(bytes[0]) |
                static_cast<std::uint16_t>(
                    static_cast<std::uint16_t>(bytes[1]) << 8);
            return true;
        }

        bool ReadU32(std::uint32_t& value)
        {
            std::uint8_t bytes[4]{};
            if (!ReadBytes(bytes))
            {
                return false;
            }

            value =
                static_cast<std::uint32_t>(bytes[0]) |
                (static_cast<std::uint32_t>(bytes[1]) << 8) |
                (static_cast<std::uint32_t>(bytes[2]) << 16) |
                (static_cast<std::uint32_t>(bytes[3]) << 24);
            return true;
        }

        bool ReadU64(std::uint64_t& value)
        {
            std::uint8_t bytes[8]{};
            if (!ReadBytes(bytes))
            {
                return false;
            }

            value =
                static_cast<std::uint64_t>(bytes[0]) |
                (static_cast<std::uint64_t>(bytes[1]) << 8) |
                (static_cast<std::uint64_t>(bytes[2]) << 16) |
                (static_cast<std::uint64_t>(bytes[3]) << 24) |
                (static_cast<std::uint64_t>(bytes[4]) << 32) |
                (static_cast<std::uint64_t>(bytes[5]) << 40) |
                (static_cast<std::uint64_t>(bytes[6]) << 48) |
                (static_cast<std::uint64_t>(bytes[7]) << 56);
            return true;
        }

        bool ReadI32(std::int32_t& value)
        {
            std::uint32_t encoded{};
            if (!ReadU32(encoded))
            {
                return false;
            }
            value = std::bit_cast<std::int32_t>(encoded);
            return true;
        }

        bool ReadBool(bool& value)
        {
            std::uint8_t encoded{};
            if (!ReadU8(encoded) || encoded > 1U)
            {
                return false;
            }
            value = encoded != 0;
            return true;
        }

        bool ReadF32(float& value)
        {
            static_assert(sizeof(float) == sizeof(std::uint32_t));
            static_assert(std::numeric_limits<float>::is_iec559);

            std::uint32_t encoded{};
            if (!ReadU32(encoded))
            {
                return false;
            }
            value = std::bit_cast<float>(encoded);
            return true;
        }

        bool ReadF64(double& value)
        {
            static_assert(sizeof(double) == sizeof(std::uint64_t));
            static_assert(std::numeric_limits<double>::is_iec559);

            std::uint64_t encoded{};
            if (!ReadU64(encoded))
            {
                return false;
            }
            value = std::bit_cast<double>(encoded);
            return true;
        }

        bool ReadBytes(std::span<std::uint8_t> bytes)
        {
            if (!bytes.empty())
            {
                _in.read(
                    reinterpret_cast<char*>(bytes.data()),
                    static_cast<std::streamsize>(bytes.size()));
            }
            return _in.good();
        }

        [[nodiscard]] bool Good() const noexcept { return _in.good(); }

    private:
        std::istream& _in;
    };
}
