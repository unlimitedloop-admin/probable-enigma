//==============================================================================
// 
//  Project: mm2hack
//  Parameters.h
// 
//  Parameter transport storage for template expressions.
// 
//==============================================================================
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace mm2hack::apps::parameters
{
    // Parameters class to hold key-value pairs of various types
    class Parameters
    {
    public:
        Parameters() = default;

        template<typename T>
        Parameters& With(const std::wstring& key, const T& value)
        {
            _data[key] = std::make_shared<Holder<T>>(value);
            return *this;
        }

        template<typename T>
        std::optional<T> Get(const std::wstring& key) const
        {
            auto it = _data.find(key);
            if (it != _data.end())
            {
                if (auto holder = std::dynamic_pointer_cast<Holder<T>>(it->second))
                {
                    return holder->value;
                }
            }
            return std::nullopt;
        }

    private:
        struct IHolder
        {
            virtual ~IHolder() = default;
        };

        template<typename T>
        struct Holder : IHolder
        {
            Holder(const T& v) : value(v) {}
            T value;
        };

        std::unordered_map<std::wstring, std::shared_ptr<IHolder>> _data;
    };
}