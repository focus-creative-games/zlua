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
using ZLua.Utils;

namespace ZLua
{
    public static class CommonPaths
    {
        public static string PackageName => "com.code-philosophy.zlua";

        public static string InstallRootDir => Path.GetFullPath($"Library/ZLua");

        /// <summary>
        /// Pristine copy of HybridCLR's <c>libil2cpp</c>, taken before ZLua overlays
        /// itself. Install restores from here first so re-running it is idempotent
        /// rather than stacking a second overlay on an already-patched tree.
        /// </summary>
        public static string HybridCLRBaselineDir =>
            Path.Combine(InstallRootDir, "hybridclr-baseline");

        public static string HybridCLRBaselineLibil2cppDir =>
            Path.Combine(HybridCLRBaselineDir, "libil2cpp");

        public static string HybridCLRBaselineStampPath =>
            Path.Combine(HybridCLRBaselineDir, "baseline_version.txt");

        public static string ZLuaDataPathInPackage => $"Packages/{PackageName}/ZLua~";

        public static string UnityOptimizationsDirInPackage => $"{ZLuaDataPathInPackage}/unity";

        public static string UnityAliasXmlPathInPackage =>
            $"{UnityOptimizationsDirInPackage}/UnityOptimizations.alias.xml";

        public static string UnityMarshalAsXmlPathInPackage =>
            $"{UnityOptimizationsDirInPackage}/UnityOptimizations.marshalas.xml";

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

        /// <summary>
        /// Patches re-anchored for HybridCLR's (il2cpp_plus) libil2cpp. One per Unity
        /// LTS series: HybridCLR swaps libil2cpp wholesale per series, so micro
        /// versions share a tree and the floor patch is always <c>X.Y.0.patch</c>.
        /// </summary>
        public static string Libil2cppHybridCLRPatchesPathInPackage =>
            Path.GetFullPath(Path.Combine(ZLuaDataPathInPackage, "patches", "libil2cpp-hybridclr"));

        /// <summary>The patch root matching the active mode.</summary>
        public static string ActiveLibil2cppPatchesPath =>
            HybridCLRIntegration.Enabled
                ? Libil2cppHybridCLRPatchesPathInPackage
                : Libil2cppPatchesPathInPackage;

        public static string LuaPatchesPathInPackage =>
            Path.GetFullPath(Path.Combine(ZLuaDataPathInPackage, "patches", "lua"));

        /// <summary>
        /// ZLua's own il2cpp staging root. Unused in HybridCLR mode, where the
        /// HybridCLR tree is the only tree.
        /// </summary>
        public static string LocalIl2CppDataPath => $"{InstallRootDir}/LocalIl2CppData-{Application.platform}";

        /// <summary>
        /// The il2cpp tree this project builds from. In HybridCLR mode this is
        /// HybridCLR's tree, which ZLua overlays rather than replaces; every derived
        /// path below (<see cref="LocalLibil2cppPath"/>, <see cref="LocalLuaSrcPath"/>,
        /// <see cref="LocalZluaPath"/>, <see cref="GeneratedZLuaPath"/>) follows it.
        /// </summary>
        public static string LocalIl2CppPath
        {
            get
            {
                if (HybridCLRIntegration.Enabled)
                {
                    string hybridclrDir = HybridCLRIntegration.LocalIl2CppDir;
                    if (!string.IsNullOrEmpty(hybridclrDir))
                    {
                        return hybridclrDir;
                    }
                }

                return $"{LocalIl2CppDataPath}/il2cpp";
            }
        }

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
