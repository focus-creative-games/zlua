// Copyright 2026 Code Philosophy

using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;

namespace ZLua.BuildProcessors
{
    internal sealed class LuaInvokeParamInfo
    {
        public string TypeName;
        public string FullTypeName;
        public string Name;
        public LuaMarshalType MarshalType;
        public Type ClrType;
        public bool IsEnum;
        public bool IsStruct;
    }

    internal sealed class LuaInvokeBindingInfo
    {
        public string ClassName;
        public string NamespaceName;
        public string MethodName;
        public string ModuleName;
        public string LuaMethodName;
        public string AssemblyName;
        public string ReturnType;
        public string ReturnFullTypeName;
        public Type ReturnClrType;
        public bool ReturnIsEnum;
        public LuaMarshalType ReturnMarshal;
        public List<LuaInvokeParamInfo> Parameters = new List<LuaInvokeParamInfo>();
        public bool UseFullBinding;

        public string SiteId => $"kSite_{ClassName}_{MethodName}";
        public string IcFunctionName => $"IC_{ClassName}_{MethodName}";
        public string InternalCallName => string.IsNullOrEmpty(NamespaceName)
            ? $"{ClassName}::{MethodName}"
            : $"{NamespaceName}.{ClassName}::{MethodName}";

        public void CollectStructLayouts(HashSet<string> structLayouts)
        {
            if (!UseFullBinding)
            {
                return;
            }

            foreach (LuaInvokeParamInfo param in Parameters)
            {
                if (param.MarshalType != LuaMarshalType.OpaqueLightUserData)
                {
                    continue;
                }

                string layout = LuaInvokeStructLayoutResolver.TryGetLayout(param.ClrType);
                if (!string.IsNullOrEmpty(layout))
                {
                    structLayouts.Add(layout);
                }
            }
        }

        public string GenerateInternalCallFunction()
        {
            return UseFullBinding ? GenerateFullBinding() : GenerateThinBinding();
        }

        private string GenerateThinBinding()
        {
            var paramNames = Parameters.Select(p => p.Name).ToList();
            string argList = string.Join(", ", paramNames);
            bool isVoid = string.Equals(ReturnType, "void", StringComparison.Ordinal);

            if (isVoid)
            {
                if (paramNames.Count == 0)
                {
                    return $"    static void {IcFunctionName}()\n    {{\n        LuaInvokeRuntime::CallVoid({SiteId});\n    }}";
                }

                return $"    static void {IcFunctionName}({FormatNativeParams(LuaMarshalType.Default)})\n    {{\n        LuaInvokeRuntime::CallVoid({SiteId}, {argList});\n    }}";
            }

            string ret = MapReturnType(ReturnType, ReturnIsEnum, LuaMarshalType.Default);
            if (paramNames.Count == 0)
            {
                return $"    static {ret} {IcFunctionName}()\n    {{\n        return LuaInvokeRuntime::CallRet<{ret}>({SiteId});\n    }}";
            }

            return $"    static {ret} {IcFunctionName}({FormatNativeParams(LuaMarshalType.Default)})\n    {{\n        return LuaInvokeRuntime::CallRet<{ret}>({SiteId}, {argList});\n    }}";
        }

        private string GenerateFullBinding()
        {
            bool isVoid = string.Equals(ReturnType, "void", StringComparison.Ordinal);
            var sb = new StringBuilder();
            sb.AppendLine($"    static {MapReturnType(ReturnType, ReturnIsEnum, ReturnMarshal)} {IcFunctionName}({FormatNativeParams(null)})");
            sb.AppendLine("    {");
            sb.AppendLine("        lua_State* L = LuaEnv::GetState();");
            sb.AppendLine("        const int top = lua_gettop(L);");
            sb.AppendLine("        const int errfunc = LuaEnv::PushErrorHandler(L);");
            sb.AppendLine("        LuaInvokeMarshaling::EnterCall();");
            sb.AppendLine($"        lua_rawgeti(L, LUA_REGISTRYINDEX, {SiteId}.funcRef);");
            foreach (LuaInvokeParamInfo param in Parameters)
            {
                sb.AppendLine($"        {GeneratePushStatement(param)}");
            }

            sb.AppendLine($"        Marshaling::LuaPCall(L, {Parameters.Count}, {(isVoid ? 0 : 1)}, errfunc);");
            if (!isVoid)
            {
                sb.AppendLine($"        {MapReturnType(ReturnType, ReturnIsEnum, ReturnMarshal)} ret = {GeneratePopExpression(ReturnType, ReturnFullTypeName, ReturnIsEnum, ReturnMarshal, -1)};");
                sb.AppendLine("        lua_settop(L, top);");
                sb.AppendLine("        return ret;");
            }
            else
            {
                sb.AppendLine("        lua_settop(L, top);");
            }

            sb.AppendLine("    }");
            return sb.ToString().TrimEnd();
        }

        private string GeneratePushStatement(LuaInvokeParamInfo param)
        {
            switch (param.MarshalType)
            {
                case LuaMarshalType.UserData when string.Equals(param.TypeName, "int", StringComparison.Ordinal):
                    return $"LuaInvokeMarshaling::PushInt32UserData(L, {param.Name});";
                case LuaMarshalType.UserData when string.Equals(param.TypeName, "string", StringComparison.Ordinal):
                    return $"LuaInvokeMarshaling::PushStringUserData(L, {param.Name});";
                case LuaMarshalType.UserData when param.IsEnum:
                    return $"LuaInvokeMarshaling::PushEnumUserData(L, LuaInvokeMarshaling::ResolveClass(\"{EscapeCpp(AssemblyName)}\", \"{EscapeCpp(param.FullTypeName)}\"), static_cast<int32_t>({param.Name}));";
                case LuaMarshalType.Bytes when IsByteArray(param.TypeName):
                    return $"LuaInvokeMarshaling::PushByteArrayBytes(L, {param.Name});";
                case LuaMarshalType.OpaqueLightUserData when param.IsStruct:
                    return $"LuaInvokeMarshaling::PushStructOpaque(L, LuaInvokeMarshaling::ResolveClass(\"{EscapeCpp(AssemblyName)}\", \"{EscapeCpp(param.FullTypeName)}\"), &{param.Name});";
                default:
                    return GenerateDefaultPush(param.TypeName, param.Name);
            }
        }

        private static string GenerateDefaultPush(string typeName, string paramName)
        {
            switch (typeName.Trim())
            {
                case "bool": return $"LuaInvokeMarshaling::PushDefaultBool(L, {paramName});";
                case "char":
                case "byte":
                case "sbyte":
                case "short":
                case "ushort":
                case "int": return $"LuaInvokeMarshaling::PushDefaultInt32(L, {paramName});";
                case "uint": return $"LuaInvokeMarshaling::PushDefaultUInt32(L, {paramName});";
                case "long": return $"LuaInvokeMarshaling::PushDefaultInt64(L, {paramName});";
                case "ulong": return $"LuaInvokeMarshaling::PushDefaultUInt64(L, {paramName});";
                case "float": return $"LuaInvokeMarshaling::PushDefaultFloat(L, {paramName});";
                case "double": return $"LuaInvokeMarshaling::PushDefaultDouble(L, {paramName});";
                case "string": return $"LuaInvokeMarshaling::PushDefaultString(L, {paramName});";
                default: return $"LuaInvokeMarshaling::PushDefaultInt32(L, {paramName});";
            }
        }

        private string GeneratePopExpression(
            string typeName,
            string fullTypeName,
            bool isEnum,
            LuaMarshalType marshalType,
            int index)
        {
            switch (marshalType)
            {
                case LuaMarshalType.UserData when string.Equals(typeName, "int", StringComparison.Ordinal):
                    return $"LuaInvokeMarshaling::PopInt32UserData(L, {index})";
                case LuaMarshalType.UserData when string.Equals(typeName, "string", StringComparison.Ordinal):
                    return $"LuaInvokeMarshaling::PopStringUserData(L, {index})";
                case LuaMarshalType.UserData when isEnum:
                    return $"static_cast<{MapReturnType(typeName, true, marshalType)}>(LuaInvokeMarshaling::PopEnumUserData(L, {index}, LuaInvokeMarshaling::ResolveClass(\"{EscapeCpp(AssemblyName)}\", \"{EscapeCpp(fullTypeName)}\")))";
                case LuaMarshalType.Bytes when IsByteArray(typeName):
                    return $"LuaInvokeMarshaling::PopByteArrayBytes(L, {index})";
                default:
                    return GenerateDefaultPop(typeName, index);
            }
        }

        private static string GenerateDefaultPop(string typeName, int index)
        {
            switch (typeName.Trim())
            {
                case "bool": return $"LuaInvokeMarshaling::PopDefaultBool(L, {index})";
                case "char":
                case "byte":
                case "sbyte":
                case "short":
                case "ushort":
                case "int": return $"LuaInvokeMarshaling::PopDefaultInt32(L, {index})";
                case "uint": return $"LuaInvokeMarshaling::PopDefaultUInt32(L, {index})";
                case "long": return $"LuaInvokeMarshaling::PopDefaultInt64(L, {index})";
                case "ulong": return $"LuaInvokeMarshaling::PopDefaultUInt64(L, {index})";
                case "float": return $"LuaInvokeMarshaling::PopDefaultFloat(L, {index})";
                case "double": return $"LuaInvokeMarshaling::PopDefaultDouble(L, {index})";
                case "string": return $"LuaInvokeMarshaling::PopDefaultString(L, {index})";
                default: return $"LuaInvokeMarshaling::PopDefaultInt32(L, {index})";
            }
        }

        private string FormatNativeParams(LuaMarshalType? defaultMarshal)
        {
            if (Parameters.Count == 0)
            {
                return string.Empty;
            }

            var parts = new List<string>();
            foreach (LuaInvokeParamInfo param in Parameters)
            {
                LuaMarshalType marshal = defaultMarshal ?? param.MarshalType;
                parts.Add($"{MapParamType(param, marshal)} {param.Name}");
            }

            return string.Join(", ", parts);
        }

        private static bool IsByteArray(string typeName)
        {
            return string.Equals(typeName.Trim(), "byte[]", StringComparison.Ordinal);
        }

        private static string MapReturnType(string csharpType, bool isEnum, LuaMarshalType marshalType)
        {
            return MapParamType(csharpType, isEnum, false, marshalType, null, null);
        }

        private static string MapParamType(LuaInvokeParamInfo param, LuaMarshalType marshalType)
        {
            return MapParamType(
                param.TypeName,
                param.IsEnum,
                param.IsStruct,
                marshalType,
                param.ClrType?.Namespace ?? GetNamespaceFromFullName(param.FullTypeName),
                param.ClrType?.Name ?? GetShortName(param.FullTypeName));
        }

        private static string MapParamType(
            string csharpType,
            bool isEnum,
            bool isStruct,
            LuaMarshalType marshalType,
            string namespaceName,
            string shortTypeName)
        {
            if (marshalType == LuaMarshalType.OpaqueLightUserData && isStruct)
            {
                return LuaInvokeStructLayoutResolver.GetNativeStructName(namespaceName, shortTypeName);
            }

            if (isEnum)
            {
                return "int32_t";
            }

            switch (csharpType.Trim())
            {
                case "void": return "void";
                case "bool": return "bool";
                case "char":
                case "byte":
                case "sbyte":
                case "short":
                case "ushort":
                case "int": return "int32_t";
                case "uint": return "uint32_t";
                case "long": return "int64_t";
                case "ulong": return "uint64_t";
                case "float": return "float";
                case "double": return "double";
                case "IntPtr": return "intptr_t";
                case "UIntPtr": return "uintptr_t";
                case "string": return "Il2CppString*";
                case "byte[]": return "Il2CppArray*";
                default: return "int32_t";
            }
        }

        private static string GetNamespaceFromFullName(string fullTypeName)
        {
            if (string.IsNullOrEmpty(fullTypeName))
            {
                return string.Empty;
            }

            int lastDot = fullTypeName.LastIndexOf('.');
            return lastDot < 0 ? string.Empty : fullTypeName.Substring(0, lastDot);
        }

        private static string GetShortName(string fullTypeName)
        {
            if (string.IsNullOrEmpty(fullTypeName))
            {
                return string.Empty;
            }

            int lastDot = fullTypeName.LastIndexOf('.');
            return lastDot < 0 ? fullTypeName : fullTypeName.Substring(lastDot + 1);
        }

        private static string EscapeCpp(string value)
        {
            return value.Replace("\\", "\\\\").Replace("\"", "\\\"");
        }
    }

    internal static class LuaInvokeStructLayoutResolver
    {
        private static readonly Dictionary<string, string> LayoutCache = new Dictionary<string, string>(StringComparer.Ordinal);

        internal static string GetNativeStructName(string namespaceName, string typeName)
        {
            return $"__zlua_ic_{Sanitize(namespaceName)}_{typeName}";
        }

        internal static string TryGetLayout(Type clrType)
        {
            if (clrType == null || !clrType.IsValueType || clrType.IsEnum)
            {
                return null;
            }

            string key = clrType.FullName ?? clrType.Name;
            if (LayoutCache.TryGetValue(key, out string cached))
            {
                return cached;
            }

            var sb = new StringBuilder();
            sb.Append("struct ").Append(GetNativeStructName(clrType.Namespace ?? string.Empty, clrType.Name)).AppendLine();
            sb.AppendLine("{");
            foreach (FieldInfo field in clrType.GetFields(BindingFlags.Public | BindingFlags.Instance))
            {
                sb.Append("    ").Append(MapFieldType(field.FieldType))
                    .Append(' ').Append(field.Name).AppendLine(";");
            }

            sb.Append("};");
            string layout = sb.ToString();
            LayoutCache[key] = layout;
            return layout;
        }

        private static string MapFieldType(Type fieldType)
        {
            switch (LuaInvokeReflectionTypeUtil.GetTypeName(fieldType))
            {
                case "int": return "int32_t";
                case "uint": return "uint32_t";
                case "long": return "int64_t";
                case "ulong": return "uint64_t";
                case "float": return "float";
                case "double": return "double";
                case "bool": return "bool";
                default: return "int32_t";
            }
        }

        private static string Sanitize(string value)
        {
            return string.IsNullOrEmpty(value) ? string.Empty : value.Replace('.', '_');
        }
    }

    internal static class LuaInvokeSiteFileGenerator
    {
        internal static (string header, string source) GenerateFiles(IReadOnlyList<LuaInvokeBindingInfo> bindings)
        {
            var structLayouts = new HashSet<string>(StringComparer.Ordinal);
            foreach (LuaInvokeBindingInfo binding in bindings)
            {
                binding.CollectStructLayouts(structLayouts);
            }

            var header = new StringBuilder();
            header.AppendLine("#pragma once");
            header.AppendLine();
            header.AppendLine("#include \"zlua/methodbridge/LuaInvokeRuntime.h\"");
            header.AppendLine();
            header.AppendLine("namespace zlua");
            header.AppendLine("{");
            foreach (LuaInvokeBindingInfo binding in bindings)
            {
                header.AppendLine($"    extern LuaInvokeSite {binding.SiteId};");
            }

            header.AppendLine();
            header.AppendLine("    void InitLuaInvokeSites();");
            header.AppendLine("    void RegisterGeneratedInternalCalls();");
            header.AppendLine("}");

            var source = new StringBuilder();
            source.AppendLine("#include \"LuaInvokeSites.h\"");
            source.AppendLine();
            source.AppendLine("#include \"zlua/LuaEnv.h\"");
            source.AppendLine("#include \"zlua/StructOpaqueScope.h\"");
            source.AppendLine("#include \"zlua/methodbridge/LuaInvokeMarshaling.h\"");
            source.AppendLine("#include \"zlua/methodbridge/LuaInvokeRuntime.h\"");
            source.AppendLine("#include \"zlua/marshal/Marshaling.h\"");
            source.AppendLine();
            source.AppendLine("#include \"vm/InternalCalls.h\"");
            source.AppendLine();
            foreach (string layout in structLayouts.OrderBy(x => x, StringComparer.Ordinal))
            {
                source.AppendLine(layout);
                source.AppendLine();
            }

            source.AppendLine("namespace zlua");
            source.AppendLine("{");
            foreach (LuaInvokeBindingInfo binding in bindings)
            {
                source.AppendLine($"    LuaInvokeSite {binding.SiteId} = {{ LUA_NOREF, LUA_NOREF }};");
            }

            source.AppendLine();
            source.AppendLine("    void InitLuaInvokeSites()");
            source.AppendLine("    {");
            foreach (LuaInvokeBindingInfo binding in bindings)
            {
                source.AppendLine($"        {binding.SiteId}.moduleRef = LuaEnv::GetModuleRef(\"{EscapeCpp(binding.ModuleName)}\");");
                source.AppendLine($"        {binding.SiteId}.funcRef = LuaEnv::GetOrCreateModuleFunctionRef(\"{EscapeCpp(binding.ModuleName)}\", \"{EscapeCpp(binding.LuaMethodName)}\");");
            }

            source.AppendLine("    }");
            source.AppendLine();

            foreach (LuaInvokeBindingInfo binding in bindings)
            {
                source.AppendLine(binding.GenerateInternalCallFunction());
                source.AppendLine();
            }

            source.AppendLine("    void RegisterGeneratedInternalCalls()");
            source.AppendLine("    {");
            foreach (LuaInvokeBindingInfo binding in bindings)
            {
                source.AppendLine($"        il2cpp::vm::InternalCalls::Add(\"{binding.InternalCallName}\", (Il2CppMethodPointer){binding.IcFunctionName});");
            }

            source.AppendLine("    }");
            source.AppendLine("}");

            return (header.ToString(), source.ToString());
        }

        private static string EscapeCpp(string value)
        {
            return value.Replace("\\", "\\\\").Replace("\"", "\\\"");
        }
    }
}
