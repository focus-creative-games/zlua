#pragma once

namespace zlua
{
struct UnityVector2
{
    float x;
    float y;
};

struct UnityVector3
{
    float x;
    float y;
    float z;
};

struct UnityVector4
{
    float x;
    float y;
    float z;
    float w;
};

enum class IntrinsicTypeKind : int
{
    None = 0,
    Vector2,
    Vector3,
    Vector4,
};

IntrinsicTypeKind GetIntrinsicType(const char* namespaze, const char* name);
} // namespace zlua
