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
using System.Runtime.InteropServices;
using UnityEditor;
using UnityEditorInternal;
using UnityEngine;

namespace NextLua
{


    public class Settings : ScriptableObject
    {
        [Tooltip("Enable NextLua")]
        public bool enable = true;

        private static Settings s_Instance;

        public static Settings Instance
        {
            get
            {
                if (!s_Instance)
                {
                    LoadOrCreate();
                }
                return s_Instance;
            }
        }

        public static bool EnableForCurrentBuildTarget
        {
            get
            {
                if (!Instance.enable)
                {
                    return false;
                }
                // #if UNITY_WEBGL || UNITY_WEIXINMINIGAME
                return true;
                // #else
                //                 return false;
                // #endif
            }
        }

        public static string InstallRootDir => Path.GetFullPath($"Library/NextLua");

        public static string GetPreservedLinkXmlPath()
        {
            return Path.GetFullPath($"{NextLuaDataPathInPackage}/link.xml");
        }

        public static string SettingsPath => "ProjectSettings/NextLua.asset";

        public static string PackageName => "com.code-philosophy.nextlua";

        public static string NextLuaDataPathInPackage => $"Packages/{PackageName}/NextLua~";

        public static string Libil2cppCppPathInPackage
        {
            get
            {
                var unityVersion = new UnityVersion(Application.unityVersion);
                string branch = unityVersion.isTuanjieEngine ? "tuanjie" : $"{unityVersion.major}";
                return $"{NextLuaDataPathInPackage}/libil2cpp-{branch}";
            }
        }

        public static string LocalIl2CppDataPath => $"{InstallRootDir}/LocalIl2CppData-{Application.platform}";

        public static string LocalIl2CppPath => $"{LocalIl2CppDataPath}/il2cpp";

        public static string LocalLibil2cppPath => $"{LocalIl2CppPath}/libil2cpp";

        private static Settings LoadOrCreate()
        {
            string filePath = SettingsPath;
            var arr = InternalEditorUtility.LoadSerializedFileAndForget(filePath);
            //Debug.Log($"typeof arr:{arr?.GetType()} arr[0]:{(arr != null && arr.Length > 0 ? arr[0].GetType(): null)}");

            if (arr != null && arr.Length > 0 && arr[0] is Settings obfuzSettings)
            {
                s_Instance = obfuzSettings;
            }
            else
            {
                s_Instance = s_Instance ?? CreateInstance<Settings>();
            }
            return s_Instance;
        }

        public static void Save()
        {
            if (!s_Instance)
            {
                return;
            }

            string filePath = SettingsPath;
            string directoryName = Path.GetDirectoryName(filePath);
            Directory.CreateDirectory(directoryName);
            UnityEngine.Object[] obj = new Settings[1] { s_Instance };
            InternalEditorUtility.SaveToSerializedFileAndForget(obj, filePath, true);
        }
    }
}
