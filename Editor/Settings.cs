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

        [Tooltip("PUC-Rio: lua-X.Y.Z (downloaded from lua.org into Library/ZLua/LuaSrcCache). LuaJIT: luajit-M.N (clone into LuaSrcCache/luajit-M.N). Empty = lua-5.5.0. See spec 11-MULTI-VERSION.")]
        public string luaVersionId = "lua-5.5.0";

        [Tooltip("MarshalAs XML files or directories (relative to project root or absolute). See spec marshal/02-MARSHAL-AS §9.")]
        public string[] marshalAsXmlPaths;

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
            return Path.GetFullPath($"{CommonDirs.ZLuaDataPathInPackage}/link.xml");
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
