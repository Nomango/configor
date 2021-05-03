// Copyright (c) 2018-2020 jsonxx - Nomango
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once
#include <memory>  // std::unique_ptr, std::shared_ptr, std::make_unique, std::make_shared
#include "json_exception.hpp"
#include "json_value.hpp"
#include "json_iterator.hpp"
#include "json_parser.hpp"
#include "json_serializer.hpp"
#include "json_utils.hpp"
#include "json_basic.hpp"

namespace jsonxx
{
    using json = basic_json<>;
    using jsonw = basic_json<std::map, std::vector, std::wstring>;

    //
    // Implements
    //

#if defined(JSONXX_ENABLE_RAW_POINTER)
    template <typename _Ty>
    struct json_bind<_Ty*>
    {
        void to_json(json& j, _Ty* const& v)
        {
            if (v != nullptr)
            {
                ::jsonxx::to_json(j, *v);
            }
            else
            {
                j = nullptr;
            }
        }

        void from_json(const json& j, _Ty*& v)
        {
            if (j.is_null())
            {
                v = nullptr;
            }
            else
            {
                if (v == nullptr)
                {
                    v = new _Ty;
                }
                ::jsonxx::from_json(j, *v);
            }
        }
    };
#endif

    template <typename _Ty>
    struct json_bind<std::unique_ptr<_Ty>>
    {
        void to_json(json& j, std::unique_ptr<_Ty> const& v)
        {
            if (v != nullptr)
            {
                ::jsonxx::to_json(j, *v);
            }
            else
            {
                j = nullptr;
            }
        }

        void from_json(const json& j, std::unique_ptr<_Ty>& v)
        {
            if (j.is_null())
            {
                v = nullptr;
            }
            else
            {
                if (v == nullptr)
                {
                    v.reset(new _Ty);
                }
                ::jsonxx::from_json(j, *v);
            }
        }
    };

    template <typename _Ty>
    struct json_bind<std::shared_ptr<_Ty>>
    {
        void to_json(json& j, std::shared_ptr<_Ty> const& v)
        {
            if (v != nullptr)
            {
                ::jsonxx::to_json(j, *v);
            }
            else
            {
                j = nullptr;
            }
        }

        void from_json(const json& j, std::shared_ptr<_Ty>& v)
        {
            if (j.is_null())
            {
                v = nullptr;
            }
            else
            {
                if (v == nullptr)
                {
                    v = std::make_shared<_Ty>();
                }
                ::jsonxx::from_json(j, *v);
            }
        }
    };

    template <typename _Ty>
    struct json_bind<std::vector<_Ty>>
    {
        void to_json(json& j, const std::vector<_Ty>& v)
        {
            j = json_type::array;
            for (size_t i = 0; i < v.size(); i++)
            {
                ::jsonxx::to_json(j[i], v[i]);
            }
        }

        void from_json(const json& j, std::vector<_Ty>& v)
        {
            v.resize(j.size());
            for (size_t i = 0; i < j.size(); i++)
            {
                ::jsonxx::from_json(j[i], v[i]);
            }
        }
    };

    template <typename _Ty>
    struct json_bind<std::map<std::string, _Ty>>
    {
        void to_json(json& j, const std::map<std::string, _Ty>& v)
        {
            j = json_type::object;
            for (const auto& p : v)
            {
                ::jsonxx::to_json(j[p.first], p.second);
            }
        }

        void from_json(const json& j, std::map<std::string, _Ty>& v)
        {
            for (auto iter = j.cbegin(); iter != j.cend(); iter++)
            {
                _Ty item = _Ty();
                ::jsonxx::from_json(iter.value(), item);
                v.insert(std::make_pair(iter.key(), item));
            }
        }
    };

    template <>
    struct json_bind<std::string>
    {
        using value_type = std::string;

        void to_json(json& j, const value_type& v) { j = v; }
        void from_json(const json& j, value_type& v) { v = static_cast<value_type>(j); }
    };

    template <>
    struct json_bind<int>
    {
        using value_type = int;

        void to_json(json& j, const value_type& v) { j = v; }
        void from_json(const json& j, value_type& v) { v = static_cast<value_type>(j); }
    };

    template <>
    struct json_bind<unsigned int>
    {
        using value_type = unsigned int;

        void to_json(json& j, const value_type& v) { j = v; }
        void from_json(const json& j, value_type& v) { v = static_cast<value_type>(j); }
    };

    template <>
    struct json_bind<short>
    {
        using value_type = short;

        void to_json(json& j, const value_type& v) { j = v; }
        void from_json(const json& j, value_type& v) { v = static_cast<value_type>(j); }
    };

    template <>
    struct json_bind<unsigned short>
    {
        using value_type = unsigned short;

        void to_json(json& j, const value_type& v) { j = v; }
        void from_json(const json& j, value_type& v) { v = static_cast<value_type>(j); }
    };

    template <>
    struct json_bind<long>
    {
        using value_type = long;

        void to_json(json& j, const value_type& v) { j = v; }
        void from_json(const json& j, value_type& v) { v = static_cast<value_type>(j); }
    };

    template <>
    struct json_bind<unsigned long>
    {
        using value_type = unsigned long;

        void to_json(json& j, const value_type& v) { j = v; }
        void from_json(const json& j, value_type& v) { v = static_cast<value_type>(j); }
    };

    template <>
    struct json_bind<float>
    {
        using value_type = float;

        void to_json(json& j, const value_type& v) { j = v; }
        void from_json(const json& j, value_type& v) { v = static_cast<value_type>(j); }
    };

    template <>
    struct json_bind<double>
    {
        using value_type = double;

        void to_json(json& j, const value_type& v) { j = v; }
        void from_json(const json& j, value_type& v) { v = static_cast<value_type>(j); }
    };

    template <>
    struct json_bind<bool>
    {
        using value_type = bool;

        void to_json(json& j, const value_type& v) { j = v; }
        void from_json(const json& j, value_type& v) { v = static_cast<value_type>(j); }
    };

    template <>
    struct json_bind<json>
    {
        using value_type = json;

        void to_json(json& j, const value_type& v) { j = v; }
        void from_json(const json& j, value_type& v) { v = j; }
    };
}

namespace std
{
    template <>
    struct hash<::jsonxx::json>
    {
        std::size_t operator()(const ::jsonxx::json &json) const
        {
            return hash<::jsonxx::json::string_type>{}(json.dump());
        }
    };

    template <>
    inline void swap<::jsonxx::json>(::jsonxx::json &lhs, ::jsonxx::json &rhs)
    {
        lhs.swap(rhs);
    }
} // namespace std
