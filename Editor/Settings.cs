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

using System.IO;
using UnityEditorInternal;
using UnityEngine;

namespace ZLua
{

    public class Settings : ScriptableObject
    {
        [Tooltip("Enable ZLua")]
        public bool enable = true;

        [Tooltip("PUC-Rio: lua-X.Y.Z (downloaded from lua.org into Library/ZLua/LuaSrcCache). LuaJIT: luajit-M.N (clone into LuaSrcCache/luajit-M-N). Empty = lua-5.3.6. See spec 11-MULTI-VERSION.")]
        public string luaVersionId = "lua-5.3.6";

        [Tooltip("MarshalAs XML files or directories (relative to project root or absolute). See spec marshal/02-MARSHAL-AS §9.")]
        public string[] marshalAsXmlPaths;

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

        public static bool EnableForCurrentBuildTarget => Instance.enable;

        public static string GetPreservedLinkXmlPath()
        {
            return Path.GetFullPath($"{CommonDirs.ZLuaDataPathInPackage}/link.xml");
        }

        private static string SettingsPath => "ProjectSettings/ZLua.asset";


        private static Settings LoadOrCreate()
        {
            string filePath = SettingsPath;
            Object[] arr = InternalEditorUtility.LoadSerializedFileAndForget(filePath);

            // After package upgrade, ZLua.asset may deserialize as Missing Script / Unity "fake null".
            // C# `??` does not treat Unity fake-null as null — must use Unity's overloaded ==.
            Settings loaded = null;
            if (arr != null && arr.Length > 0 && arr[0] is Settings settings && settings)
            {
                loaded = settings;
            }

            if (!loaded)
            {
                // Drop any stale fake-null singleton before creating a new asset.
                s_Instance = CreateInstance<Settings>();
                Save();
                return s_Instance;
            }

            s_Instance = loaded;
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
