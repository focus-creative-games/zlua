#pragma once

#include <cstring>
#include <functional>
#include <string_view>

namespace zlua
{

struct CsStringHash
{
    size_t operator()(const char* key) const
    {
        return std::hash<std::string_view>()(std::string_view(key != nullptr ? key : ""));
    }
};

struct CsStringEqual
{
    bool operator()(const char* lhs, const char* rhs) const
    {
        if (lhs == rhs)
            return true;
        if (lhs == nullptr || rhs == nullptr)
            return false;
        return std::strcmp(lhs, rhs) == 0;
    }
};

} // namespace zlua
