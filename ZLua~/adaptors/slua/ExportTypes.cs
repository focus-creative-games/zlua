// ZLua migration helper for SLua projects.
// Copy into your SLua project's Editor folder, then menu ZLua/ExportTypes.
// Spec: docs/spec/12-MIGRATION-ADAPTORS.md

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using SLua;
using UnityEditor;
using UnityEngine;

public static class ZLuaSLuaExportTypes
{
    public const string OutputRelativePath = "ZLua/slua_export_types.lua";

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
        var byAssembly = new SortedDictionary<string, SortedSet<Entry>>(StringComparer.Ordinal);
        int skipped = 0;

        void Add(Type t, string exportNameOverride)
        {
            if (!TryAddType(t, exportNameOverride, byAssembly, out string reason))
            {
                skipped++;
                if (!string.IsNullOrEmpty(reason))
                {
                    Debug.LogWarning("ZLua ExportTypes (SLua): skip " + Describe(t) + " — " + reason);
                }
            }
        }

        CollectCustomTypes(Add);

        string absPath = Path.Combine(Application.dataPath, OutputRelativePath.Replace('/', Path.DirectorySeparatorChar));
        WriteLuaModule(absPath, byAssembly);
        AssetDatabase.Refresh();

        int count = byAssembly.Sum(kv => kv.Value.Count);
        Debug.Log(string.Format(
            "ZLua ExportTypes (SLua): wrote {0} type(s) in {1} assembly(ies), skipped {2}.\n{3}",
            count, byAssembly.Count, skipped, absPath));
        if (!Application.isBatchMode)
        {
            EditorUtility.RevealInFinder(absPath);
        }
    }

    static void CollectCustomTypes(LuaCodeGen.ExportGenericDelegate add)
    {
        HashSet<string> namespaces = CustomExport.OnAddCustomNamespace() ?? new HashSet<string>();

        object[] unused = null;
        List<object> customNsSets = LuaCodeGen.InvokeEditorMethod<ICustomExportPost>("OnAddCustomNamespace", ref unused);
        if (customNsSets != null)
        {
            foreach (object cNsSet in customNsSets)
            {
                var set = cNsSet as HashSet<string>;
                if (set == null)
                {
                    continue;
                }

                foreach (string strNs in set)
                {
                    namespaces.Add(strNs);
                }
            }
        }

        ScanAssembly("Assembly-CSharp-firstpass", namespaces, add);
        ScanAssembly("Assembly-CSharp", namespaces, add);

        CustomExport.OnAddCustomClass(add);

        object[] args = { add };
        LuaCodeGen.InvokeEditorMethod<ICustomExportPost>("OnAddCustomClass", ref args);
    }

    static void ScanAssembly(string assemblyName, HashSet<string> namespaces, LuaCodeGen.ExportGenericDelegate add)
    {
        try
        {
            Assembly assembly = Assembly.Load(assemblyName);
            Type[] types = assembly.GetExportedTypes();
            for (int i = 0; i < types.Length; i++)
            {
                Type t = types[i];
                if (t.IsDefined(typeof(CustomLuaClassAttribute), false)
                    || (t.Namespace != null && namespaces.Contains(t.Namespace)))
                {
                    add(t, null);
                }
            }
        }
        catch (Exception)
        {
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

    static void WriteLuaModule(string absPath, SortedDictionary<string, SortedSet<Entry>> byAssembly)
    {
        string dir = Path.GetDirectoryName(absPath);
        if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
        {
            Directory.CreateDirectory(dir);
        }

        var sb = new StringBuilder(8 * 1024);
        sb.AppendLine("-- Auto-generated by ZLua/ExportTypes (SLua).");
        sb.AppendLine("-- " + DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"));
        sb.AppendLine("-- Do not edit by hand; regenerate after changing SLua custom export config.");
        sb.AppendLine("return {");
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
