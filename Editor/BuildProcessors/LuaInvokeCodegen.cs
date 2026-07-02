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
    /// Generates Il2Cpp LuaInvoke sites by reflecting [LuaInvoke] methods from user assemblies in AppDomain.
    /// </summary>
    internal sealed class LuaInvokeCodegen : IPreprocessBuildWithReport
    {
        public int callbackOrder => 20;

        public void OnPreprocessBuild(BuildReport report)
        {
            try
            {
                GenerateLuaInvokeSites(showDialog: false, failBuildOnError: true);
            }
            catch (Exception ex) when (ex is not BuildFailedException)
            {
                throw new BuildFailedException("[LuaInvokeCodegen] " + ex);
            }
        }

        internal static void GenerateLuaInvokeSites(bool showDialog)
        {
            GenerateLuaInvokeSites(showDialog, failBuildOnError: false);
        }

        private static void GenerateLuaInvokeSites(bool showDialog, bool failBuildOnError)
        {
            if (!Settings.EnableForCurrentBuildTarget)
            {
                if (showDialog)
                {
                    EditorUtility.DisplayDialog("ZLua", "LuaInvoke codegen is disabled for the current build target.", "OK");
                }

                return;
            }

            if (!new LocalInstaller().HasInstalledToLocal())
            {
                const string message = "[LuaInvokeCodegen] ZLua local il2cpp is not installed; skip codegen.";
                Debug.LogWarning(message);
                if (showDialog)
                {
                    EditorUtility.DisplayDialog("ZLua", message, "OK");
                }

                return;
            }

            IReadOnlyList<Assembly> assemblies = LuaInvokeUserAssemblyResolver.ResolvePlayerAssembliesFromAppDomain();
            if (assemblies.Count == 0)
            {
                const string message =
                    "[LuaInvokeCodegen] No user Player assemblies found in AppDomain. " +
                    "Ensure scripts are compiled before build.";
                if (failBuildOnError)
                {
                    throw new BuildFailedException(message);
                }

                Debug.LogWarning(message);
                if (showDialog)
                {
                    EditorUtility.DisplayDialog("ZLua", message, "OK");
                }

                return;
            }

            List<LuaInvokeBindingInfo> bindings = LuaInvokeBindingReflectionCollector.Collect(assemblies);
            if (bindings.Count == 0)
            {
                const string message =
                    "[LuaInvokeCodegen] No [LuaInvoke] methods found in user assemblies. " +
                    "Ensure IL weaving keeps LuaInvokeAttribute metadata.";
                if (failBuildOnError)
                {
                    throw new BuildFailedException(message);
                }

                Debug.LogWarning(message);
                if (showDialog)
                {
                    EditorUtility.DisplayDialog("ZLua", message, "OK");
                }

                return;
            }

            string generatedDir = Settings.GeneratedZLuaPath;
            DirectoryUtil.RecreateDir(generatedDir);
            WriteLuaInvokeSites(generatedDir, bindings);
            WriteBuiltinScriptsInc(generatedDir);
            MarshalAsCodegen.WriteGeneratedMetadataPublic(generatedDir);
            MirrorGeneratedDirectory(generatedDir);

            string summary =
                $"[LuaInvokeCodegen] Generated {bindings.Count} LuaInvoke binding(s) from {assemblies.Count} user assembly(ies) to: {generatedDir}";
            Debug.Log(summary);
            if (showDialog)
            {
                EditorUtility.DisplayDialog("ZLua", summary, "OK");
            }
        }

        private static void WriteLuaInvokeSites(string generatedDir, List<LuaInvokeBindingInfo> bindings)
        {
            (string header, string source) = LuaInvokeSiteFileGenerator.GenerateFiles(bindings);
            File.WriteAllText(Path.Combine(generatedDir, "LuaInvokeSites.h"), header, Encoding.UTF8);
            File.WriteAllText(Path.Combine(generatedDir, "LuaInvokeSites.cpp"), source, Encoding.UTF8);
        }

        private static void WriteBuiltinScriptsInc(string generatedDir)
        {
            string globalsPath = Settings.GetLuaLibScriptPath("globals.lua");
            string libPath = Settings.GetLuaLibScriptPath("zlualib.lua");

            var sb = new StringBuilder();
            sb.AppendLine(EmbedLua("kZLuaGlobalsLua", File.Exists(globalsPath) ? File.ReadAllText(globalsPath, Encoding.UTF8) : string.Empty));
            sb.AppendLine();
            sb.AppendLine(EmbedLua("kZLuaLibLua", File.Exists(libPath) ? File.ReadAllText(libPath, Encoding.UTF8) : string.Empty));

            File.WriteAllText(Path.Combine(generatedDir, "BuiltinScripts.inc"), sb.ToString(), Encoding.UTF8);
        }

        private static string EmbedLua(string name, string content)
        {
            var sb = new StringBuilder();
            sb.Append("static const char ").Append(name).Append("[] = R\"zlua(").Append(content).Append(")zlua\";");
            return sb.ToString();
        }

        private static void MirrorGeneratedDirectory(string generatedDir)
        {
            MirrorGeneratedToDirectory(generatedDir, Settings.PackageGeneratedZLuaPath);
            MirrorGeneratedToDirectory(generatedDir, Settings.BuildWin64GeneratedZLuaPath);
        }

        private static void MirrorGeneratedToDirectory(string generatedDir, string targetDir)
        {
            if (string.IsNullOrEmpty(targetDir))
            {
                return;
            }

            string parentDir = Path.GetDirectoryName(targetDir);
            if (string.IsNullOrEmpty(parentDir) || !Directory.Exists(parentDir))
            {
                return;
            }

            DirectoryUtil.RecreateDir(targetDir);
            foreach (string file in Directory.GetFiles(generatedDir))
            {
                File.Copy(file, Path.Combine(targetDir, Path.GetFileName(file)), overwrite: true);
            }
        }
    }
}
