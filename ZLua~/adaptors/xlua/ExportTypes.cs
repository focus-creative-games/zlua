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

// ZLua migration helper for xLua projects.
// Copy into your xLua project's Editor folder, then menu ZLua/ExportTypes.
// Spec: docs/spec/12-MIGRATION-ADAPTORS.md

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using CSObjectWrapEditor;
using UnityEditor;
using UnityEngine;
using XLua;

public static class ZLuaXLuaExportTypes
{
    public const string OutputRelativePath = "ZLua/xlua_export_types.lua";
    public const string TopNamespace = "CS";

    private struct Entry : IComparable<Entry>
    {
        public string FullName;
        public string ExportName;

        public int CompareTo(Entry other)
        {
            int c = string.CompareOrdinal(FullName, other.FullName);
            if (c != 0)
            {
                return c;
            }

            return string.CompareOrdinal(ExportName, other.ExportName);
        }
    }

    [MenuItem("ZLua/ExportTypes")]
    public static void ExportTypes()
    {
        Generator.GetGenConfig(XLua.Utils.GetAllTypes());
        List<Type> types = Generator.LuaCallCSharp;
        if (types == null)
        {
            Debug.LogError("ZLua ExportTypes (xLua): Generator.LuaCallCSharp is null after GetGenConfig.");
            return;
        }

        int skipped = 0;
        var byAssembly = new SortedDictionary<string, SortedSet<Entry>>(StringComparer.Ordinal);
        foreach (Type type in types)
        {
            if (!TryAddType(type, null, byAssembly, out string reason))
            {
                skipped++;
                if (!string.IsNullOrEmpty(reason))
                {
                    Debug.LogWarning("ZLua ExportTypes (xLua): skip " + Describe(type) + " — " + reason);
                }
            }
        }

        string absPath = Path.Combine(Application.dataPath, OutputRelativePath.Replace('/', Path.DirectorySeparatorChar));
        WriteLuaModule(absPath, "xLua", TopNamespace, byAssembly);
        AssetDatabase.Refresh();

        int count = byAssembly.Sum(kv => kv.Value.Count);
        Debug.Log(string.Format(
            "ZLua ExportTypes (xLua): wrote {0} type(s) in {1} assembly(ies), skipped {2}.\n{3}",
            count, byAssembly.Count, skipped, absPath));
        if (!Application.isBatchMode)
        {
            EditorUtility.RevealInFinder(absPath);
        }
    }

    static bool TryAddType(
        Type type,
        string exportNameOverride,
        SortedDictionary<string, SortedSet<Entry>> byAssembly,
        out string reason)
    {
        reason = null;
        if (type == null)
        {
            reason = "null type";
            return false;
        }

        if (type.IsGenericTypeDefinition)
        {
            reason = "open generic (not mounted by adaptor MVP)";
            return false;
        }

        if (type.IsArray)
        {
            reason = "array type";
            return false;
        }

        string fullName = type.FullName;
        if (string.IsNullOrEmpty(fullName))
        {
            reason = "empty FullName";
            return false;
        }

        if (fullName.IndexOf('[') >= 0)
        {
            reason = "FullName contains '[' (closed generic / special name)";
            return false;
        }

        string asmName = type.Assembly.GetName().Name;
        if (string.IsNullOrEmpty(asmName))
        {
            reason = "empty assembly name";
            return false;
        }

        string exportName = exportNameOverride;
        if (string.IsNullOrEmpty(exportName))
        {
            exportName = fullName.Replace('+', '.');
        }

        SortedSet<Entry> set;
        if (!byAssembly.TryGetValue(asmName, out set))
        {
            set = new SortedSet<Entry>();
            byAssembly[asmName] = set;
        }

        set.Add(new Entry { FullName = fullName, ExportName = exportName });
        return true;
    }

    static string Describe(Type type)
    {
        return type == null ? "<null>" : (type.FullName ?? type.Name);
    }

    static void WriteLuaModule(
        string absPath,
        string scheme,
        string topNamespace,
        SortedDictionary<string, SortedSet<Entry>> byAssembly)
    {
        string dir = Path.GetDirectoryName(absPath);
        if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
        {
            Directory.CreateDirectory(dir);
        }

        var sb = new StringBuilder(8 * 1024);
        sb.AppendLine("-- Auto-generated by ZLua/ExportTypes (" + scheme + ").");
        sb.AppendLine("-- " + DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"));
        sb.AppendLine("-- Do not edit by hand; regenerate after changing LuaCallCSharp whitelist.");
        sb.AppendLine("return {");
        if (!string.IsNullOrEmpty(topNamespace))
        {
            sb.Append("  top_namespace = ");
            sb.Append(LuaQuote(topNamespace));
            sb.AppendLine(",");
        }

        sb.AppendLine("  types = {");

        bool firstAsm = true;
        foreach (KeyValuePair<string, SortedSet<Entry>> kv in byAssembly)
        {
            if (!firstAsm)
            {
                sb.AppendLine(",");
            }

            firstAsm = false;
            sb.Append("    [");
            sb.Append(LuaQuote(kv.Key));
            sb.AppendLine("] = {");

            bool firstType = true;
            foreach (Entry e in kv.Value)
            {
                if (!firstType)
                {
                    sb.AppendLine(",");
                }

                firstType = false;
                sb.Append("      { full_name = ");
                sb.Append(LuaQuote(e.FullName));
                if (!string.Equals(e.ExportName, e.FullName, StringComparison.Ordinal))
                {
                    sb.Append(", export_name = ");
                    sb.Append(LuaQuote(e.ExportName));
                }
                sb.Append(" }");
            }

            if (kv.Value.Count > 0)
            {
                sb.AppendLine();
            }

            sb.Append("    }");
        }

        if (byAssembly.Count > 0)
        {
            sb.AppendLine();
        }

        sb.AppendLine("  },");
        sb.AppendLine("}");

        File.WriteAllText(absPath, sb.ToString(), new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));
    }

    static string LuaQuote(string s)
    {
        if (s == null)
        {
            s = "";
        }

        return "\"" + s
            .Replace("\\", "\\\\")
            .Replace("\"", "\\\"")
            .Replace("\r", "\\r")
            .Replace("\n", "\\n")
            + "\"";
    }
}
