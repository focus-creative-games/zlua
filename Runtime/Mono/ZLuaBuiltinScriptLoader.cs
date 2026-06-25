using System;
using System.IO;
using System.Text;
using UnityEngine;

namespace ZLua
{
    internal static class ZLuaBuiltinScriptLoader
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
