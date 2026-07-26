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
using UnityEngine;

namespace ZLua
{
    public static class CommonDirs
    {
        public static string PackageName => "com.code-philosophy.zlua";

        public static string InstallRootDir => Path.GetFullPath($"Library/ZLua");

        public static string ZLuaDataPathInPackage => $"Packages/{PackageName}/ZLua~";

        public static string LuaLibPathInPackage => $"{ZLuaDataPathInPackage}/lualib";

        public static string GetLuaLibScriptPath(string fileName) =>
            Path.GetFullPath(Path.Combine(LuaLibPathInPackage, fileName));

        public static string ZluaRuntimePathInPackage =>
            Path.GetFullPath(Path.Combine(ZLuaDataPathInPackage, "zlua-runtime"));

        /// <summary>
        /// Cached upstream Lua / LuaJIT trees (not shipped in the UPM package).
        /// PUC-Rio tarballs are downloaded here; LuaJIT is cloned into a staging dir and moved in.
        /// </summary>
        public static string LuaSrcCacheDir => Path.GetFullPath(Path.Combine(InstallRootDir, "LuaSrcCache"));

        public static string Libil2cppPatchesPathInPackage =>
            Path.GetFullPath(Path.Combine(ZLuaDataPathInPackage, "patches", "libil2cpp"));

        public static string LuaPatchesPathInPackage =>
            Path.GetFullPath(Path.Combine(ZLuaDataPathInPackage, "patches", "lua"));

        public static string LocalIl2CppDataPath => $"{InstallRootDir}/LocalIl2CppData-{Application.platform}";

        public static string LocalIl2CppPath => $"{LocalIl2CppDataPath}/il2cpp";

        public static string LocalLibil2cppPath => $"{LocalIl2CppPath}/libil2cpp";

        public static string LocalLuaSrcPath => $"{LocalLibil2cppPath}/lua";

        public static string LocalZluaPath => Path.Combine(LocalLibil2cppPath, "zlua");

        public static string GeneratedZLuaPath => Path.GetFullPath(Path.Combine(LocalZluaPath, "generated"));

        public static string PackageGeneratedZLuaPath =>
            Path.GetFullPath(Path.Combine(ZluaRuntimePathInPackage, "generated"));

        public static string BuildWin64GeneratedZLuaPath =>
            Path.GetFullPath(Path.Combine(Directory.GetCurrentDirectory(), "build-win64", "Il2CppOutputProject", "IL2CPP", "libil2cpp", "zlua", "generated"));

        public static string GetManagedStrippedDuplicatePath(BuildTarget buildTarget) =>
            $"{InstallRootDir}/ManagedStripped/{buildTarget}";

        public static string Il2CppBuildCacheDir { get; } = "Library/Bee/artifacts";

        public static string GetTempAotProjectOutputDir(BuildTarget target)
        {
            return $"Temp/TempAotProject/{target}";
        }

        public static string PackagePluginsRoot =>
            Path.GetFullPath(Path.Combine("Packages", PackageName, "Plugins"));
    }
}
