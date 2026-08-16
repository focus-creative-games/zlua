// Copyright 2026 Code Philosophy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "IntrinsicTypes.h"

#include <cstring>
#include <functional>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace zlua
{
using IntrinsicTypeKey = std::pair<const char*, const char*>;

struct IntrinsicTypeKeyHash
{
    size_t operator()(const IntrinsicTypeKey& key) const
    {
        size_t h = std::hash<std::string_view>()(std::string_view(key.first));
        h ^= std::hash<std::string_view>()(std::string_view(key.second)) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

struct IntrinsicTypeKeyEqual
{
    bool operator()(const IntrinsicTypeKey& lhs, const IntrinsicTypeKey& rhs) const
    {
        return std::strcmp(lhs.first, rhs.first) == 0 && std::strcmp(lhs.second, rhs.second) == 0;
    }
};

static const std::unordered_map<IntrinsicTypeKey, IntrinsicTypeKind, IntrinsicTypeKeyHash, IntrinsicTypeKeyEqual>& GetIntrinsicTypeMap()
{
    static const std::unordered_map<IntrinsicTypeKey, IntrinsicTypeKind, IntrinsicTypeKeyHash, IntrinsicTypeKeyEqual> s_map = {
        {{"UnityEngine", "Vector2"}, IntrinsicTypeKind::Vector2},
        {{"UnityEngine", "Vector3"}, IntrinsicTypeKind::Vector3},
        {{"UnityEngine", "Vector4"}, IntrinsicTypeKind::Vector4},
    };
    return s_map;
}

IntrinsicTypeKind GetIntrinsicType(const char* namespaze, const char* name)
{
    const auto& map = GetIntrinsicTypeMap();
    auto it = map.find(IntrinsicTypeKey{namespaze, name});
    return it != map.end() ? it->second : IntrinsicTypeKind::None;
}
} // namespace zlua
