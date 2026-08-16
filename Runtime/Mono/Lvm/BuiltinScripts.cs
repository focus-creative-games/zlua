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
using System.Text;
using UnityEngine;

namespace ZLua.Lvm
{
    internal static class BuiltinScripts
    {
        public const string PackageName = "com.code-philosophy.zlua";

        public static string LuaLibDirectoryInPackage =>
            Path.GetFullPath(Path.Combine(Application.dataPath, "..", "Packages", PackageName, "ZLua~", "lualib"));

        public static string Load(string fileName)
        {
            if (string.IsNullOrWhiteSpace(fileName))
            {
                throw new ArgumentException("Built-in script file name is required.", nameof(fileName));
            }

            string path = ResolveScriptPath(fileName);
            if (!File.Exists(path))
            {
                throw new FileNotFoundException(
                    $"ZLua built-in script '{fileName}' was not found at: {path}",
                    path);
            }

            return File.ReadAllText(path, Encoding.UTF8);
        }

        private static string ResolveScriptPath(string fileName)
        {
#if UNITY_EDITOR
            return Path.Combine(LuaLibDirectoryInPackage, fileName);
#else
            return Path.Combine(Application.streamingAssetsPath, "ZLua", "lualib", fileName);
#endif
        }
    }
}
