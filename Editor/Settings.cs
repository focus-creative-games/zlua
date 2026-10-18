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

using System;
using System.IO;
using UnityEditor;
using UnityEditorInternal;
using UnityEngine;

namespace ZLua
{

    [FilePath("ProjectSettings/ZLua.asset", FilePathAttribute.Location.ProjectFolder)]
    public class Settings : ScriptableSingleton<Settings>
    {
        [Tooltip("Enable ZLua")]
        public bool enable = true;

        [Tooltip("PUC-Rio: lua-X.Y.Z. LuaJIT: luajit-M.N. Luau: luau-M.N (e.g. luau-0.743). Empty = lua-5.5.0.")]
        public string luaVersionId = "lua-5.5.0";

        [Tooltip("MarshalAs XML files or directories (relative to project root or absolute). See spec marshal/02-MARSHAL-AS §9.")]
        public string[] marshalAsXmlPaths;

        /// <summary>User <see cref="marshalAsXmlPaths"/> plus built-in Unity optimization MarshalAs XML when that switch is on.</summary>
        public string[] MarshalAsXmlPaths =>
            MergeUnityOptimizationPath(marshalAsXmlPaths, CommonPaths.UnityMarshalAsXmlPathInPackage);

        [Tooltip("LuaAlias XML files or directories (relative to project root or absolute). Root element ZLuaAlias. See spec 04-METHOD-OVERLOAD §5.4.")]
        public string[] luaAliasXmlPaths;

        /// <summary>User <see cref="luaAliasXmlPaths"/> plus built-in Unity optimization alias XML when that switch is on.</summary>
        public string[] LuaAliasXmlPaths =>
            MergeUnityOptimizationPath(luaAliasXmlPaths, CommonPaths.UnityAliasXmlPathInPackage);

        [Tooltip("LuaExtension XML files or directories (relative to project root or absolute). Root element ZLuaExtensions. See spec 13-EXTENSION-METHODS §2.2.")]
        public string[] luaExtensionXmlPaths;

        [Tooltip("Unity-specific binding adjustments shipped with ZLua. Currently: GameObject/Component GetComponent aliases and runtime-type returns. On by default. See spec 15-UNITY-OPTIMIZATIONS.")]
        public bool enableUnityOptimizations = true;

        [Tooltip("Editor Mono only: inject EmmyLua emmy_core after Initialize. Windows ships lua51–lua55 and luajit under Plugins/emmylua/; other OS currently lua55 only (self-build otherwise). Spec build/04-EMMYLUA-DEBUGGER.")]
        public bool enableDebugger = false;

        [Tooltip("EmmyLua tcpListen port (IDE Attach). Default 9966.")]
        public int debuggerPort = 9966;

        [Tooltip("If true, dbg.waitIDE() blocks the Unity main thread until the IDE attaches. Leave false unless you need to break before first business Lua.")]
        public bool debuggerWaitIDE = false;

        public static Settings Instance => instance;

        public static bool EnableForCurrentBuildTarget => Instance.enable;

        public static string GetPreservedLinkXmlPath()
        {
            return Path.GetFullPath($"{CommonPaths.ZLuaDataPathInPackage}/link.xml");
        }

        private string[] MergeUnityOptimizationPath(string[] userPaths, string builtinRelativePath)
        {
            if (!enableUnityOptimizations)
            {
                return userPaths;
            }

            if (userPaths == null || userPaths.Length == 0)
            {
                return new[] { builtinRelativePath };
            }

            var merged = new string[userPaths.Length + 1];
            merged[0] = builtinRelativePath;
            Array.Copy(userPaths, 0, merged, 1, userPaths.Length);
            return merged;
        }




        public static void Save()
        {
            if (!instance)
            {
                return;
            }

            instance.Save(true);

        }
    }
}
