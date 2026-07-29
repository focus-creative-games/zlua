#pragma once

struct MethodInfo;

namespace zlua
{
/// Shared helpers for LuaAlias / LuaMarshalAs XML name→token binding.
namespace XmlBindingUtil
{
/// Map inflated / generic-instance MethodInfo to the definition MethodInfo used for token keys.
const MethodInfo* NormalizeMethodForToken(const MethodInfo* method);
} // namespace XmlBindingUtil
} // namespace zlua
