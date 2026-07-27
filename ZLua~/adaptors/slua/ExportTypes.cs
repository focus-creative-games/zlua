// ZLua migration helper for SLua projects.
// Copy this file into your SLua project's Editor folder, then use menu ZLua/ExportTypes.
// Spec: docs/spec/12-MIGRATION-ADAPTORS.md
//
// Collects types the same way as SLua menu "SLua/Custom/Make" (CustomLuaClass,
// custom namespaces, OnAddCustomClass / ICustomExportPost).

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
    /// <summary>Change if you want a different require-friendly path under Assets.</summary>
    public const string OutputRelativePath = "ZLua/slua_export_types.lua";

    [MenuItem("ZLua/ExportTypes")]
    public static void ExportTypes()
    {
        List<Type> types = CollectCustomTypes();
        int skipped = 0;
        var byAssembly = new SortedDictionary<string, SortedSet<string>>(StringComparer.Ordinal);
        foreach (Type type in types)
        {
            if (!TryAddType(type, byAssembly, out string reason))
            {
                skipped++;
                if (!string.IsNullOrEmpty(reason))
                {
                    Debug.LogWarning("ZLua ExportTypes (SLua): skip " + Describe(type) + " — " + reason);
                }
            }
        }

        string absPath = Path.Combine(Application.dataPath, OutputRelativePath.Replace('/', Path.DirectorySeparatorChar));
        WriteLuaModule(absPath, "SLua", byAssembly);
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

    static List<Type> CollectCustomTypes()
    {
        var result = new List<Type>();
        var seen = new HashSet<Type>();

        void Add(Type t, string ns)
        {
            if (t != null && seen.Add(t))
            {
                result.Add(t);
            }
        }

        HashSet<string> namespaces = CustomExport.OnAddCustomNamespace() ?? new HashSet<string>();

        // Merge ICustomExportPost.OnAddCustomNamespace results (same as LuaCodeGen.Custom).
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

        ScanAssembly("Assembly-CSharp-firstpass", namespaces, Add);
        ScanAssembly("Assembly-CSharp", namespaces, Add);

        CustomExport.OnAddCustomClass(Add);

        object[] args = { (LuaCodeGen.ExportGenericDelegate)Add };
        LuaCodeGen.InvokeEditorMethod<ICustomExportPost>("OnAddCustomClass", ref args);

        return result;
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
            // firstpass / missing assembly is normal.
        }
    }

    static bool TryAddType(Type type, SortedDictionary<string, SortedSet<string>> byAssembly, out string reason)
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

        SortedSet<string> set;
        if (!byAssembly.TryGetValue(asmName, out set))
        {
            set = new SortedSet<string>(StringComparer.Ordinal);
            byAssembly[asmName] = set;
        }

        set.Add(fullName);
        return true;
    }

    static string Describe(Type type)
    {
        if (type == null)
        {
            return "<null>";
        }

        return type.FullName ?? type.Name;
    }

    static void WriteLuaModule(string absPath, string scheme, SortedDictionary<string, SortedSet<string>> byAssembly)
    {
        string dir = Path.GetDirectoryName(absPath);
        if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
        {
            Directory.CreateDirectory(dir);
        }

        var sb = new StringBuilder(8 * 1024);
        sb.AppendLine("-- Auto-generated by ZLua/ExportTypes (" + scheme + ").");
        sb.AppendLine("-- " + DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"));
        sb.AppendLine("-- Do not edit by hand; regenerate after changing SLua custom export config.");
        sb.AppendLine("return {");

        bool firstAsm = true;
        foreach (KeyValuePair<string, SortedSet<string>> kv in byAssembly)
        {
            if (!firstAsm)
            {
                sb.AppendLine(",");
            }

            firstAsm = false;
            sb.Append("  [");
            sb.Append(LuaQuote(kv.Key));
            sb.AppendLine("] = {");

            bool firstType = true;
            foreach (string fullName in kv.Value)
            {
                if (!firstType)
                {
                    sb.AppendLine(",");
                }

                firstType = false;
                sb.Append("    ");
                sb.Append(LuaQuote(fullName));
            }

            if (kv.Value.Count > 0)
            {
                sb.AppendLine();
            }

            sb.Append("  }");
        }

        if (byAssembly.Count > 0)
        {
            sb.AppendLine();
        }

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
