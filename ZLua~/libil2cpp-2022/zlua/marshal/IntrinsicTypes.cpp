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
