using System;
using System.IO;
using System.Text;
using UnityEngine;

namespace NovaLua
{
    internal static class NovaLuaBuiltinScriptLoader
    {
        public const string PackageName = "com.code-philosophy.novalua";

        public static string LuaLibDirectoryInPackage =>
            Path.GetFullPath(Path.Combine(Application.dataPath, "..", "Packages", PackageName, "NovaLua~", "lualib"));

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
                    $"NovaLua built-in script '{fileName}' was not found at: {path}",
                    path);
            }

            return File.ReadAllText(path, Encoding.UTF8);
        }

        private static string ResolveScriptPath(string fileName)
        {
#if UNITY_EDITOR
            return Path.Combine(LuaLibDirectoryInPackage, fileName);
#else
            return Path.Combine(Application.streamingAssetsPath, "NovaLua", "lualib", fileName);
#endif
        }
    }
}
