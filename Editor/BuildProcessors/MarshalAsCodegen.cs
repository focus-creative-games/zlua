// Copyright 2026 Code Philosophy

using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Text;
using UnityEditor;
using UnityEditor.Build;
using UnityEditor.Build.Reporting;
using UnityEngine;

namespace ZLua.BuildProcessors
{
    /// <summary>
    /// Editor bind-time validation and Il2Cpp marshal metadata generation (Phase 8.1).
    /// </summary>
    internal sealed class MarshalAsCodegen : IPreprocessBuildWithReport
    {
        public int callbackOrder => 21;

        public void OnPreprocessBuild(BuildReport report)
        {
            if (!Settings.EnableForCurrentBuildTarget)
            {
                return;
            }

            if (!new LocalInstaller().HasInstalledToLocal())
            {
                return;
            }

            try
            {
                ValidateAllAssemblies();
                WriteGeneratedMetadata(Settings.GeneratedZLuaPath);
            }
            catch (LuaMarshalAsConfigurationException ex)
            {
                throw new BuildFailedException(ex.Message);
            }
        }

        [MenuItem("ZLua/Validate LuaMarshalAs Bindings")]
        internal static void ValidateMenu()
        {
            try
            {
                ValidateAllAssemblies();
                Debug.Log("[ZLua] LuaMarshalAs bind-time validation passed.");
            }
            catch (LuaMarshalAsConfigurationException ex)
            {
                Debug.LogError(ex.Message);
            }
        }

        private static void ValidateAllAssemblies()
        {
            var seenMethods = new HashSet<string>(StringComparer.Ordinal);
            foreach (Assembly assembly in AppDomain.CurrentDomain.GetAssemblies())
            {
                if (assembly.IsDynamic || string.IsNullOrEmpty(assembly.Location))
                {
                    continue;
                }

                string name = assembly.GetName().Name;
                if (name.StartsWith("Unity.", StringComparison.Ordinal)
                    || name.StartsWith("System.", StringComparison.Ordinal)
                    || name.StartsWith("mscorlib", StringComparison.Ordinal)
                    || name.StartsWith("netstandard", StringComparison.Ordinal)
                    || name.Equals("ZLua.Tests", StringComparison.Ordinal))
                {
                    continue;
                }

                ValidateAssembly(assembly, seenMethods);
            }
        }

        private static void ValidateAssembly(Assembly assembly, HashSet<string> seenMethods)
        {
            Type[] types;
            try
            {
                types = assembly.GetTypes();
            }
            catch (ReflectionTypeLoadException ex)
            {
                types = ex.Types;
            }

            if (types == null)
            {
                return;
            }

            for (int i = 0; i < types.Length; i++)
            {
                Type type = types[i];
                if (type == null)
                {
                    continue;
                }

                ValidateTypeConfiguration(type);

                const BindingFlags flags = BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static | BindingFlags.Instance | BindingFlags.DeclaredOnly;
                MethodInfo[] methods = type.GetMethods(flags);
                for (int m = 0; m < methods.Length; m++)
                {
                    MethodInfo method = methods[m];
                    if (method.IsAbstract || method.ContainsGenericParameters)
                    {
                        continue;
                    }

                    string key = method.DeclaringType?.FullName + "." + method.Name + method.MetadataToken;
                    if (!seenMethods.Add(key))
                    {
                        continue;
                    }

                    if (HasLuaMarshalAs(method))
                    {
                        LuaMarshalAsValidation.ValidateMethodConfiguration(method);
                    }
                }

                ConstructorInfo[] ctors = type.GetConstructors(flags);
                for (int c = 0; c < ctors.Length; c++)
                {
                    ConstructorInfo ctor = ctors[c];
                    if (ctor.ContainsGenericParameters)
                    {
                        continue;
                    }

                    if (HasLuaMarshalAs(ctor))
                    {
                        LuaMarshalAsValidation.ValidateMethodConfiguration(ctor);
                    }
                }
            }
        }

        private static void ValidateTypeConfiguration(Type type)
        {
            LuaMarshalAsAttribute attribute = type.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false);
            if (attribute == null || attribute.LuaMarshalType == LuaMarshalType.Default)
            {
                return;
            }

            if (attribute.LuaMarshalType == LuaMarshalType.Table
                || attribute.LuaMarshalType == LuaMarshalType.UnpackedValues)
            {
                if (attribute.FieldOrPropertyNames == null || attribute.FieldOrPropertyNames.Length == 0)
                {
                    throw new LuaMarshalAsConfigurationException(
                        "[ZLua] LuaMarshalAs configuration error: " + type.FullName + "\n  "
                        + "LuaMarshalType." + attribute.LuaMarshalType + " requires non-empty FieldOrPropertyNames.");
                }
            }
        }

        private static bool HasLuaMarshalAs(MethodBase method)
        {
            if (method.GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false) != null)
            {
                return true;
            }

            if (method is MethodInfo methodInfo && methodInfo.ReturnType != typeof(void))
            {
                foreach (LuaMarshalAsAttribute attribute in methodInfo.ReturnParameter.GetCustomAttributes(typeof(LuaMarshalAsAttribute), inherit: false))
                {
                    if (attribute.LuaMarshalType != LuaMarshalType.Default)
                    {
                        return true;
                    }
                }
            }

            ParameterInfo[] parameters = method.GetParameters();
            for (int i = 0; i < parameters.Length; i++)
            {
                if (parameters[i].GetCustomAttribute<LuaMarshalAsAttribute>(inherit: false) != null)
                {
                    return true;
                }
            }

            return false;
        }

        private static void WriteGeneratedMetadata(string generatedDir)
        {
            WriteGeneratedMetadataPublic(generatedDir);
        }

        internal static void WriteGeneratedMetadataPublic(string generatedDir)
        {
            Directory.CreateDirectory(generatedDir);
            var sb = new StringBuilder();
            sb.AppendLine("// Generated by MarshalAsCodegen (Phase 8.1). Do not edit.");
            sb.AppendLine("#pragma once");
            sb.AppendLine();
            sb.AppendLine("#define ZLUA_HAS_MARSHAL_BINDINGS 1");
            sb.AppendLine();
            sb.AppendLine("namespace zlua");
            sb.AppendLine("{");
            sb.AppendLine("    inline void RegisterMarshalBindingTables() { }");
            sb.AppendLine("}");

            File.WriteAllText(Path.Combine(generatedDir, "MarshalBindings.inc"), sb.ToString(), Encoding.UTF8);
        }
    }
}
