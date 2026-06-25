// Copyright 2026 Code Philosophy

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using UnityEditor;
using UnityEditor.Build;
using UnityEditor.Build.Reporting;
using UnityEngine;

namespace ZLua.BuildProcessors
{
    internal sealed class LuaInvokeCodegen : IPreprocessBuildWithReport
    {
        private const string LuaInvokePattern =
            @"\[LuaInvoke\s*\(\s*""([^""]+)""\s*,\s*""([^""]+)""\s*\)\]";

        public int callbackOrder => 20;

        public void OnPreprocessBuild(BuildReport report)
        {
            if (!Settings.EnableForCurrentBuildTarget)
            {
                return;
            }

            if (!new LocalInstaller().HasInstalledToLocal())
            {
                Debug.LogWarning("[LuaInvokeCodegen] ZLua local il2cpp is not installed; skip codegen.");
                return;
            }

            var bindings = CollectBindings();
            string generatedDir = Settings.GeneratedZLuaPath;
            Directory.CreateDirectory(generatedDir);
            WriteLuaInvokeSites(generatedDir, bindings);
            WriteBuiltinScriptsInc(generatedDir);
            TryMirrorGeneratedToBuildOutput(generatedDir);
            Debug.Log($"[LuaInvokeCodegen] Generated {bindings.Count} LuaInvoke binding(s) to: {generatedDir}");
        }

        private static List<LuaInvokeBindingInfo> CollectBindings()
        {
            var results = new List<LuaInvokeBindingInfo>();
            var seen = new HashSet<string>(StringComparer.Ordinal);

            foreach (string csPath in EnumerateCsFiles())
            {
                string text = File.ReadAllText(csPath, Encoding.UTF8);
                foreach (Match match in Regex.Matches(text, LuaInvokePattern))
                {
                    if (!TryParseBinding(text, match, out LuaInvokeBindingInfo binding))
                    {
                        continue;
                    }

                    string key = binding.InternalCallName;
                    if (!seen.Add(key))
                    {
                        continue;
                    }

                    results.Add(binding);
                }
            }

            results.Sort((a, b) => string.CompareOrdinal(a.SiteId, b.SiteId));
            return results;
        }

        private static IEnumerable<string> EnumerateCsFiles()
        {
            string assetsDir = Path.Combine(Directory.GetCurrentDirectory(), "Assets");
            if (Directory.Exists(assetsDir))
            {
                foreach (string file in Directory.GetFiles(assetsDir, "*.cs", SearchOption.AllDirectories))
                {
                    yield return file;
                }
            }

            string packagesDir = Path.Combine(Directory.GetCurrentDirectory(), "Packages");
            if (!Directory.Exists(packagesDir))
            {
                yield break;
            }

            foreach (string file in Directory.GetFiles(packagesDir, "*.cs", SearchOption.AllDirectories))
            {
                if (file.Contains($"{Path.DirectorySeparatorChar}Editor{Path.DirectorySeparatorChar}"))
                {
                    continue;
                }
                yield return file;
            }
        }

        private static bool TryParseBinding(string text, Match match, out LuaInvokeBindingInfo binding)
        {
            binding = null;
            int scanEnd = Math.Min(text.Length, match.Index + 512);
            string chunk = text.Substring(match.Index, scanEnd - match.Index);
            var methodMatch = Regex.Match(chunk,
                @"static\s+extern\s+([\w<>\[\],\s]+)\s+(\w+)\s*\(([^\)]*)\)\s*;");
            if (!methodMatch.Success)
            {
                return false;
            }

            string returnType = methodMatch.Groups[1].Value.Trim();
            string methodName = methodMatch.Groups[2].Value.Trim();
            string parameters = methodMatch.Groups[3].Value.Trim();
            string className = FindEnclosingTypeName(text, match.Index);
            if (string.IsNullOrEmpty(className))
            {
                className = "UnknownType";
            }

            bool useFullBinding = HasNonDefaultMarshal(text, match.Index, chunk);
            binding = new LuaInvokeBindingInfo
            {
                ClassName = className,
                MethodName = methodName,
                ModuleName = match.Groups[1].Value,
                LuaMethodName = match.Groups[2].Value,
                ReturnType = returnType,
                Parameters = parameters,
                UseFullBinding = useFullBinding,
            };
            return true;
        }

        private static string FindEnclosingTypeName(string text, int index)
        {
            string head = text.Substring(0, index);
            var matches = Regex.Matches(head, @"(?:class|struct)\s+(\w+)");
            if (matches.Count == 0)
            {
                return null;
            }
            return matches[matches.Count - 1].Groups[1].Value;
        }

        private static bool HasNonDefaultMarshal(string text, int index, string methodLine)
        {
            if (Regex.IsMatch(methodLine, @"\[.*?LuaMarshalAs\s*\(\s*LuaMarshalType\.(?!Default\b)\w+"))
            {
                return true;
            }

            int searchStart = Math.Max(0, index - 512);
            string window = text.Substring(searchStart, index - searchStart);
            return Regex.IsMatch(window, @"\[return:\s*LuaMarshalAs\s*\(\s*LuaMarshalType\.(?!Default\b)\w+");
        }

        private static void WriteLuaInvokeSites(string generatedDir, List<LuaInvokeBindingInfo> bindings)
        {
            var header = new StringBuilder();
            header.AppendLine("#pragma once");
            header.AppendLine();
            header.AppendLine("#include \"zlua/LuaInvokeRuntime.h\"");
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
            source.AppendLine("#include \"zlua/LuaInvokeRuntime.h\"");
            source.AppendLine("#include \"zlua/Marshaling.h\"");
            source.AppendLine();
            source.AppendLine("#include \"vm/InternalCalls.h\"");
            source.AppendLine();
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
            }

            source.AppendLine("    void RegisterGeneratedInternalCalls()");
            source.AppendLine("    {");
            foreach (LuaInvokeBindingInfo binding in bindings)
            {
                source.AppendLine($"        il2cpp::vm::InternalCalls::Add(\"{binding.InternalCallName}\", (Il2CppMethodPointer){binding.IcFunctionName});");
            }
            source.AppendLine("    }");
            source.AppendLine("}");

            File.WriteAllText(Path.Combine(generatedDir, "LuaInvokeSites.h"), header.ToString(), Encoding.UTF8);
            File.WriteAllText(Path.Combine(generatedDir, "LuaInvokeSites.cpp"), source.ToString(), Encoding.UTF8);
        }

        private static void WriteBuiltinScriptsInc(string generatedDir)
        {
            string globalsPath = Settings.GetLuaLibScriptPath("globals.lua");
            string libPath = Settings.GetLuaLibScriptPath("zlualib.lua");

            var sb = new StringBuilder();
            sb.AppendLine(EmbedLua("kZLuaGlobalsLua", File.Exists(globalsPath) ? File.ReadAllText(globalsPath, Encoding.UTF8) : ""));
            sb.AppendLine();
            sb.AppendLine(EmbedLua("kZLuaLibLua", File.Exists(libPath) ? File.ReadAllText(libPath, Encoding.UTF8) : ""));

            File.WriteAllText(Path.Combine(generatedDir, "BuiltinScripts.inc"), sb.ToString(), Encoding.UTF8);
        }

        private static string EmbedLua(string name, string content)
        {
            var sb = new StringBuilder();
            sb.Append("static const char ").Append(name).Append("[] = R\"zlua(").Append(content).Append(")zlua\";");
            return sb.ToString();
        }

        private static void TryMirrorGeneratedToBuildOutput(string generatedDir)
        {
            string buildDir = Settings.BuildWin64GeneratedZLuaPath;
            if (!Directory.Exists(Path.GetDirectoryName(buildDir)))
            {
                return;
            }

            Directory.CreateDirectory(buildDir);
            foreach (string file in Directory.GetFiles(generatedDir))
            {
                File.Copy(file, Path.Combine(buildDir, Path.GetFileName(file)), overwrite: true);
            }
        }

        private static string EscapeCpp(string value)
        {
            return value.Replace("\\", "\\\\").Replace("\"", "\\\"");
        }

        private sealed class LuaInvokeBindingInfo
        {
            public string ClassName;
            public string MethodName;
            public string ModuleName;
            public string LuaMethodName;
            public string ReturnType;
            public string Parameters;
            public bool UseFullBinding;

            public string SiteId => $"kSite_{ClassName}_{MethodName}";
            public string IcFunctionName => $"IC_{ClassName}_{MethodName}";
            public string InternalCallName => $"{ClassName}::{MethodName}";

            public string GenerateInternalCallFunction()
            {
                if (UseFullBinding)
                {
                    return GenerateFullBinding();
                }

                return GenerateThinBinding();
            }

            private string GenerateThinBinding()
            {
                var paramNames = ParseParameterNames();
                string argList = string.Join(", ", paramNames);
                bool isVoid = string.Equals(ReturnType, "void", StringComparison.Ordinal);

                if (isVoid)
                {
                    if (paramNames.Count == 0)
                    {
                        return $"    static void {IcFunctionName}()\n    {{\n        LuaInvokeRuntime::CallVoid({SiteId});\n    }}";
                    }
                    return $"    static void {IcFunctionName}({FormatNativeParams()})\n    {{\n        LuaInvokeRuntime::CallVoid({SiteId}, {argList});\n    }}";
                }

                string ret = MapReturnType(ReturnType);
                if (paramNames.Count == 0)
                {
                    return $"    static {ret} {IcFunctionName}()\n    {{\n        return LuaInvokeRuntime::CallRet<{ret}>({SiteId});\n    }}";
                }
                return $"    static {ret} {IcFunctionName}({FormatNativeParams()})\n    {{\n        return LuaInvokeRuntime::CallRet<{ret}>({SiteId}, {argList});\n    }}";
            }

            private string GenerateFullBinding()
            {
                var sb = new StringBuilder();
                sb.AppendLine($"    static {MapReturnType(ReturnType)} {IcFunctionName}({FormatNativeParams()})");
                sb.AppendLine("    {");
                sb.AppendLine("        lua_State* L = LuaEnv::GetState();");
                sb.AppendLine("        const int top = lua_gettop(L);");
                sb.AppendLine($"        lua_rawgeti(L, LUA_REGISTRYINDEX, {SiteId}.funcRef);");
                sb.AppendLine("        // TODO: non-default marshal FullBinding expansion");
                sb.AppendLine("        Marshaling::LuaPCall(L, 0, 0);");
                sb.AppendLine("        lua_settop(L, top);");
                if (!string.Equals(ReturnType, "void", StringComparison.Ordinal))
                {
                    sb.AppendLine($"        return ({MapReturnType(ReturnType)})0;");
                }
                sb.AppendLine("    }");
                return sb.ToString().TrimEnd();
            }

            private List<string> ParseParameterNames()
            {
                if (string.IsNullOrWhiteSpace(Parameters))
                {
                    return new List<string>();
                }

                return Parameters.Split(',')
                    .Select(p => Regex.Replace(p.Trim(), @"\[.*?\]", string.Empty).Trim())
                    .Select(p => p.Split(' ').Last().Trim())
                    .Where(p => !string.IsNullOrEmpty(p))
                    .ToList();
            }

            private string FormatNativeParams()
            {
                if (string.IsNullOrWhiteSpace(Parameters))
                {
                    return string.Empty;
                }

                var parts = new List<string>();
                foreach (string raw in Parameters.Split(','))
                {
                    string cleaned = Regex.Replace(raw.Trim(), @"\[.*?\]", string.Empty).Trim();
                    if (string.IsNullOrEmpty(cleaned))
                    {
                        continue;
                    }

                    string[] tokens = cleaned.Split(new[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
                    if (tokens.Length < 2)
                    {
                        continue;
                    }

                    string typeName = string.Join(" ", tokens, 0, tokens.Length - 1);
                    string paramName = tokens[tokens.Length - 1];
                    parts.Add($"{MapParamType(typeName)} {paramName}");
                }

                return string.Join(", ", parts);
            }

            private static string MapReturnType(string csharpType)
            {
                switch (csharpType.Trim())
                {
                    case "void": return "void";
                    case "int": return "int32_t";
                    case "bool": return "bool";
                    case "float": return "float";
                    case "double": return "double";
                    case "long": return "int64_t";
                    case "string": return "Il2CppString*";
                    default: return "int32_t";
                }
            }

            private static string MapParamType(string csharpType)
            {
                return MapReturnType(csharpType);
            }
        }
    }
}
