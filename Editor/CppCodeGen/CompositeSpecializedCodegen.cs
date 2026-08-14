// Copyright 2026 Code Philosophy

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using dnlib.DotNet;
using UnityEngine;
using ZLua;
using ZLua.Meta;
using ZLua.Utils;

namespace ZLua.CppCodeGen
{
    internal sealed class CompositeSpecializedCodegen
    {
        private sealed class MemberSpec
        {
            public string ClrName;
            public bool Optional;
            public FieldDef Field;
            public TypeSig FieldType;
            public string CppFieldName;
            public string CppTypeName;
        }

        private sealed class TypeSpec
        {
            public string AssemblyName;
            public string TypeFullName;
            public TypeDef TypeDef;
            public TypeSig TypeSig;
            public LuaMarshalType MarshalType;
            public List<MemberSpec> Members;
            public string CppTypeName;
            public string FuncPrefix;
        }

        private readonly AssemblyCache _assemblyCache;
        private readonly List<string> _assemblyNames;
        private readonly string _outputDir;

        public CompositeSpecializedCodegen(AssemblyCache assemblyCache, List<string> assemblyNames, string outputDir)
        {
            _assemblyCache = assemblyCache;
            _assemblyNames = assemblyNames;
            _outputDir = outputDir;
        }

        public void Generate()
        {
            string projectRoot = Path.GetDirectoryName(Application.dataPath);
            try
            {
                LuaMarshalAsXmlRegistry.Load(Settings.Instance.marshalAsXmlPaths, projectRoot);
            }
            catch (Exception ex)
            {
                throw new Exception("[ZLua] CompositeSpecializedCodegen: MarshalAs XML load failed: " + ex.Message, ex);
            }

            List<TypeSpec> specs = CollectSpecs();
            WriteHeader();
            WriteSource(specs);
            Debug.Log("[ZLua] CompositeSpecializedCodegen: emitted " + specs.Count + " specialized type(s).");
        }

        private List<TypeSpec> CollectSpecs()
        {
            var byKey = new Dictionary<string, TypeSpec>(StringComparer.Ordinal);

            foreach (string assName in _assemblyNames)
            {
                ModuleDef mod = _assemblyCache.LoadModule(assName);
                string assemblyName = mod.Assembly?.Name;
                if (string.IsNullOrEmpty(assemblyName))
                {
                    assemblyName = assName;
                }

                foreach (TypeDef typeDef in mod.GetTypes())
                {
                    if (typeDef.HasGenericParameters || !typeDef.IsValueType || typeDef.IsEnum)
                    {
                        continue;
                    }

                    LuaMarshalAsInfo attrInfo = MarshalMetaUtil.GetLuaMarshalAsInfoForType(typeDef);
                    if (attrInfo == null)
                    {
                        continue;
                    }

                    TryAddSpec(byKey, assemblyName, typeDef, attrInfo);
                }
            }

            IReadOnlyList<LuaMarshalAsXmlRule> rules = LuaMarshalAsXmlRegistry.Rules;
            for (int i = 0; i < rules.Count; i++)
            {
                LuaMarshalAsXmlRule rule = rules[i];
                if (rule.Kind != LuaMarshalAsXmlTargetKind.Type)
                {
                    continue;
                }

                if (rule.MarshalType != LuaMarshalType.Table && rule.MarshalType != LuaMarshalType.UnpackedValues)
                {
                    continue;
                }

                ModuleDef mod = null;
                try
                {
                    mod = _assemblyCache.LoadModule(rule.AssemblyName);
                }
                catch
                {
                    continue;
                }

                TypeDef typeDef = mod.Find(rule.TypeFullName, isReflectionName: false)
                    ?? mod.GetTypes().FirstOrDefault(t => t.FullName == rule.TypeFullName);
                if (typeDef == null || typeDef.HasGenericParameters || !typeDef.IsValueType || typeDef.IsEnum)
                {
                    continue;
                }

                var info = new LuaMarshalAsInfo
                {
                    marshalType = rule.MarshalType,
                    members = rule.Members != null ? new List<string>(rule.Members) : null,
                };
                TryAddSpec(byKey, rule.AssemblyName, typeDef, info);
            }

            return byKey.Values.OrderBy(s => s.AssemblyName + "|" + s.TypeFullName, StringComparer.Ordinal).ToList();
        }

        private static void TryAddSpec(
            Dictionary<string, TypeSpec> byKey,
            string assemblyName,
            TypeDef typeDef,
            LuaMarshalAsInfo info)
        {
            if (info.marshalType != LuaMarshalType.Table && info.marshalType != LuaMarshalType.UnpackedValues)
            {
                return;
            }

            if (info.members == null || info.members.Count == 0)
            {
                return;
            }

            if (!TryBuildMembers(typeDef, info, out List<MemberSpec> members))
            {
                return;
            }

            string key = assemblyName + "|" + typeDef.FullName + "|" + info.marshalType;
            if (byKey.ContainsKey(key))
            {
                return;
            }

            TypeSig typeSig = typeDef.ToTypeSig();
            byKey[key] = new TypeSpec
            {
                AssemblyName = assemblyName,
                TypeFullName = typeDef.FullName,
                TypeDef = typeDef,
                TypeSig = typeSig,
                MarshalType = info.marshalType,
                Members = members,
                CppTypeName = NameUtil.GetTypeName(typeSig),
                FuncPrefix = "Spec_" + SanitizeFuncName(typeDef.FullName) + "_" + info.marshalType,
            };
        }

        private static bool TryBuildMembers(TypeDef typeDef, LuaMarshalAsInfo info, out List<MemberSpec> members)
        {
            members = new List<MemberSpec>();
            for (int i = 0; i < info.members.Count; i++)
            {
                string raw = info.members[i];
                if (string.IsNullOrEmpty(raw))
                {
                    return false;
                }

                bool optional = raw.EndsWith("?", StringComparison.Ordinal);
                if (optional && info.marshalType != LuaMarshalType.Table)
                {
                    return false;
                }

                string clrName = optional ? raw.Substring(0, raw.Length - 1) : raw;
                if (string.IsNullOrEmpty(clrName))
                {
                    return false;
                }

                FieldDef field = typeDef.Fields.FirstOrDefault(f => !f.IsStatic && f.Name == clrName);
                if (field == null)
                {
                    // Property → no specialize
                    return false;
                }

                TypeSig fieldType = field.FieldType.RemovePinnedAndModifiers();
                if (!IsDirectWritable(fieldType))
                {
                    return false;
                }

                members.Add(new MemberSpec
                {
                    ClrName = clrName,
                    Optional = optional,
                    Field = field,
                    FieldType = fieldType,
                    CppTypeName = NameUtil.GetTypeName(fieldType),
                });
            }

            return members.Count > 0;
        }

        private static bool IsDirectWritable(TypeSig typeSig)
        {
            TypeSig t = typeSig.RemovePinnedAndModifiers();
            switch (t.ElementType)
            {
                case ElementType.Boolean:
                case ElementType.I1:
                case ElementType.U1:
                case ElementType.I2:
                case ElementType.U2:
                case ElementType.I4:
                case ElementType.U4:
                case ElementType.I8:
                case ElementType.U8:
                case ElementType.R4:
                case ElementType.R8:
                case ElementType.I:
                case ElementType.U:
                case ElementType.Char:
                case ElementType.String:
                    return true;
                case ElementType.ValueType:
                {
                    TypeDef td = t.ToTypeDefOrRef().ResolveTypeDef();
                    return td != null && td.IsEnum;
                }
                default:
                    return false;
            }
        }

        private void WriteHeader()
        {
            var writer = new CodeWriter();
            writer.WriteLine("// Generated by CompositeSpecializedCodegen. Do not edit.");
            writer.WriteLine("#pragma once");
            writer.WriteLine();
            writer.WriteLine("#define ZLUA_HAS_COMPOSITE_SPECIALIZED_BINDINGS 1");
            writer.WriteLine();
            writer.WriteLine("namespace zlua");
            writer.WriteLine("{");
            writer.WriteLine("    void RegisterCompositeSpecializedWriters();");
            writer.WriteLine("}");
            writer.Save(Path.Combine(_outputDir, "CompositeSpecializedBindings.h"));
        }

        private void WriteSource(List<TypeSpec> specs)
        {
            var forward = new ForwardDeclareStructsCodeGen();
            foreach (TypeSpec spec in specs)
            {
                forward.CollectStructType(spec.TypeSig);
                for (int i = 0; i < spec.Members.Count; i++)
                {
                    MemberSpec member = spec.Members[i];
                    if (!forward.TryGetCppFieldName(spec.TypeSig, member.ClrName, out string cppFieldName))
                    {
                        throw new Exception(
                            "[ZLua] CompositeSpecializedCodegen: field '" + member.ClrName
                            + "' not found in forward layout for " + spec.TypeFullName);
                    }

                    member.CppFieldName = cppFieldName;
                }
            }

            var writer = new CodeWriter();
            writer.WriteLine("// Generated by CompositeSpecializedCodegen. Do not edit.");
            writer.WriteLine("#include \"CompositeSpecializedBindings.h\"");
            writer.WriteLine("#include \"../marshal/CompositeSpecializedTable.h\"");
            writer.WriteLine("#include \"../marshal/TypedMarshal.h\"");
            writer.WriteLine("#include \"../marshal/StringMarshal.h\"");
            writer.WriteLine("#include \"../utils/LuaException.h\"");
            writer.WriteLine("#include <cstring>");
            writer.WriteLine();
            writer.WriteLine("namespace zlua");
            writer.WriteLine("{");
            writer.WriteLine("namespace composite_specialized_bindings");
            writer.WriteLine("{");

            forward.WriteForwardDeclareStructs(writer);
            writer.WriteLine();

            foreach (TypeSpec spec in specs)
            {
                WriteLua2Cs(writer, spec);
                writer.WriteLine();
                WriteCs2Lua(writer, spec);
                writer.WriteLine();
            }

            if (specs.Count > 0)
            {
                writer.WriteLine("static const CompositeSpecializedEntry kEntries[] = {");
                writer.IncreaseIndent();
                foreach (TypeSpec spec in specs)
                {
                    int stackSlots = spec.MarshalType == LuaMarshalType.UnpackedValues ? spec.Members.Count : 1;
                    writer.WriteLine("{");
                    writer.IncreaseIndent();
                    writer.WriteLine(ToCStringLiteral(spec.AssemblyName) + ",");
                    writer.WriteLine(ToCStringLiteral(spec.TypeFullName) + ",");
                    writer.WriteLine("LuaMarshalType::" + spec.MarshalType + ",");
                    writer.WriteLine(spec.FuncPrefix + "_Lua2Cs,");
                    writer.WriteLine(spec.FuncPrefix + "_Cs2Lua,");
                    writer.WriteLine(stackSlots + ",");
                    writer.DecreaseIndent();
                    writer.WriteLine("},");
                }

                writer.DecreaseIndent();
                writer.WriteLine("};");
            }

            writer.WriteLine("} // namespace composite_specialized_bindings");
            writer.WriteLine();
            writer.WriteLine("void RegisterCompositeSpecializedWriters()");
            writer.WriteLine("{");
            writer.IncreaseIndent();
            if (specs.Count == 0)
            {
                writer.WriteLine("CompositeSpecializedTable::Clear();");
            }
            else
            {
                writer.WriteLine(
                    "CompositeSpecializedTable::RegisterEntries(composite_specialized_bindings::kEntries, sizeof(composite_specialized_bindings::kEntries) / sizeof(composite_specialized_bindings::kEntries[0]));");
            }

            writer.DecreaseIndent();
            writer.WriteLine("}");
            writer.WriteLine("} // namespace zlua");
            writer.Save(Path.Combine(_outputDir, "CompositeSpecializedBindings.cpp"));
        }

        private static void WriteLua2Cs(CodeWriter writer, TypeSpec spec)
        {
            writer.WriteLine(
                "static void " + spec.FuncPrefix + "_Lua2Cs(lua_State* L, int valueIdx, void* address, const MarshalMetaInfo* meta)");
            writer.WriteLine("{");
            writer.IncreaseIndent();
            writer.WriteLine("(void)meta;");
            writer.WriteLine("auto* p = reinterpret_cast<" + spec.CppTypeName + "*>(address);");
            writer.WriteLine("std::memset(p, 0, sizeof(*p));");

            if (spec.MarshalType == LuaMarshalType.Table)
            {
                writer.WriteLine("if (!lua_istable(L, valueIdx))");
                writer.WriteLine("{");
                writer.IncreaseIndent();
                writer.WriteLine(
                    "LuaException::ThrowFormat(\"zlua: expected table for LuaMarshalType.Table ("
                    + EscapeForFormat(spec.TypeFullName) + "), got %s\", lua_typename(L, lua_type(L, valueIdx)));");
                writer.DecreaseIndent();
                writer.WriteLine("}");

                foreach (MemberSpec member in spec.Members)
                {
                    writer.WriteLine("{");
                    writer.IncreaseIndent();
                    writer.WriteLine("const int ft = lua_getfield(L, valueIdx, " + ToCStringLiteral(member.ClrName) + ");");
                    writer.WriteLine("if (ft == LUA_TNIL)");
                    writer.WriteLine("{");
                    writer.IncreaseIndent();
                    writer.WriteLine("lua_pop(L, 1);");
                    if (member.Optional)
                    {
                        writer.WriteLine("// optional missing key");
                    }
                    else
                    {
                        writer.WriteLine(
                            "LuaException::ThrowFormat(\"zlua: missing table key '%s' for "
                            + EscapeForFormat(spec.TypeFullName) + "\", " + ToCStringLiteral(member.ClrName) + ");");
                    }

                    writer.DecreaseIndent();
                    writer.WriteLine("}");
                    if (member.Optional)
                    {
                        writer.WriteLine("else");
                        writer.WriteLine("{");
                        writer.IncreaseIndent();
                        writer.WriteLine(GenerateAssignFromStack(member, "-1"));
                        writer.WriteLine("lua_pop(L, 1);");
                        writer.DecreaseIndent();
                        writer.WriteLine("}");
                    }
                    else
                    {
                        writer.WriteLine("else");
                        writer.WriteLine("{");
                        writer.IncreaseIndent();
                        writer.WriteLine(GenerateAssignFromStack(member, "-1"));
                        writer.WriteLine("lua_pop(L, 1);");
                        writer.DecreaseIndent();
                        writer.WriteLine("}");
                    }

                    writer.DecreaseIndent();
                    writer.WriteLine("}");
                }
            }
            else
            {
                for (int i = 0; i < spec.Members.Count; i++)
                {
                    writer.WriteLine(GenerateAssignFromStack(spec.Members[i], "valueIdx + " + i));
                }
            }

            writer.DecreaseIndent();
            writer.WriteLine("}");
        }

        private static void WriteCs2Lua(CodeWriter writer, TypeSpec spec)
        {
            writer.WriteLine(
                "static void " + spec.FuncPrefix + "_Cs2Lua(lua_State* L, void* address, const MarshalMetaInfo* meta)");
            writer.WriteLine("{");
            writer.IncreaseIndent();
            writer.WriteLine("(void)meta;");
            writer.WriteLine("auto* p = reinterpret_cast<" + spec.CppTypeName + "*>(address);");

            if (spec.MarshalType == LuaMarshalType.Table)
            {
                writer.WriteLine("lua_createtable(L, 0, " + spec.Members.Count + ");");
                writer.WriteLine("const int tableIdx = lua_gettop(L);");
                foreach (MemberSpec member in spec.Members)
                {
                    writer.WriteLine(GeneratePushField(member));
                    writer.WriteLine("lua_setfield(L, tableIdx, " + ToCStringLiteral(member.ClrName) + ");");
                }
            }
            else
            {
                foreach (MemberSpec member in spec.Members)
                {
                    writer.WriteLine(GeneratePushField(member));
                }
            }

            writer.DecreaseIndent();
            writer.WriteLine("}");
        }

        private static string GenerateAssignFromStack(MemberSpec member, string idxExpr)
        {
            if (member.FieldType.ElementType == ElementType.String)
            {
                return "p->" + member.CppFieldName + " = StringMarshal::Pop(L, " + idxExpr + ");";
            }

            string primitivePop = CodegenCommon.TryGeneratePrimitivePop(member.CppTypeName, "p->" + member.CppFieldName, idxExpr);
            if (primitivePop != null)
            {
                return primitivePop;
            }

            return "p->" + member.CppFieldName + " = DefaultTypedMarshal<" + member.CppTypeName + ">::Pop(L, " + idxExpr + ");";
        }

        private static string GeneratePushField(MemberSpec member)
        {
            if (member.FieldType.ElementType == ElementType.String)
            {
                return "StringMarshal::Push(L, p->" + member.CppFieldName + ");";
            }

            string primitivePush = CodegenCommon.TryGeneratePrimitivePush(member.CppTypeName, "p->" + member.CppFieldName);
            if (primitivePush != null)
            {
                return primitivePush;
            }

            return "DefaultTypedMarshal<" + member.CppTypeName + ">::Push(L, p->" + member.CppFieldName + ");";
        }

        private static string SanitizeFuncName(string fullName)
        {
            var sb = new StringBuilder(fullName.Length);
            foreach (char c in fullName)
            {
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
                {
                    sb.Append(c);
                }
                else
                {
                    sb.Append('_');
                }
            }

            return sb.ToString();
        }

        private static string EscapeForFormat(string value)
        {
            return value.Replace("\\", "\\\\").Replace("%", "%%").Replace("\"", "\\\"");
        }

        private static string ToCStringLiteral(string value)
        {
            if (value == null)
            {
                return "\"\"";
            }

            var sb = new StringBuilder(value.Length + 2);
            sb.Append('"');
            foreach (char c in value)
            {
                switch (c)
                {
                    case '\\':
                        sb.Append("\\\\");
                        break;
                    case '"':
                        sb.Append("\\\"");
                        break;
                    case '\n':
                        sb.Append("\\n");
                        break;
                    case '\r':
                        sb.Append("\\r");
                        break;
                    case '\t':
                        sb.Append("\\t");
                        break;
                    default:
                        if (c < 32 || c > 126)
                        {
                            sb.Append("\\x");
                            sb.Append(((int)c).ToString("x2"));
                        }
                        else
                        {
                            sb.Append(c);
                        }

                        break;
                }
            }

            sb.Append('"');
            return sb.ToString();
        }
    }
}
