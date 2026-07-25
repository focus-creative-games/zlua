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

#if UNITY_6000_3_OR_NEWER && UNITY_EDITOR_OSX
#define NEW_IL2CPP_PATH
#endif

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using UnityEditor;
using UnityEditor.Build;
using UnityEditor.PackageManager;
using UnityEngine;
using ZLua.Utils;
using Debug = UnityEngine.Debug;
using PackageInfo = UnityEditor.PackageManager.PackageInfo;

namespace ZLua
{
    public class LocalInstaller
    {
        private static readonly string[] s_zluaDefinePrefixes =
        {
            "ZLUA_LUA_",
            "ZLUA_USE_LUAJIT",
        };

        private readonly UnityVersion _curVersion;

        public int MajorVersion => _curVersion.major;

        public UnityVersion CurVersion => _curVersion;

        /// <summary>True when Install changed API-family defines or series DLL expectation.</summary>
        public bool RequiresEditorRestart { get; private set; }

        public LocalInstaller()
        {
            _curVersion = new UnityVersion(Application.unityVersion);
        }

        public string ApplicationIl2cppPath
        {
            get
            {
#if NEW_IL2CPP_PATH
#if UNITY_IOS
                string platformDirName = "iOSSupport";
#elif UNITY_TVOS
                string platformDirName = "AppleTVSupport";
#elif UNITY_VISIONOS
                string platformDirName = "VisionOSPlayer";
#else
                string platformDirName = "iOSSupport";
#endif
                return $"{EditorApplication.applicationContentsPath}/../../PlaybackEngines/{platformDirName}/il2cpp";
#else
                return $"{EditorApplication.applicationContentsPath}/il2cpp";
#endif
            }
        }

        public void InstallLocal()
        {
            RequiresEditorRestart = false;
            try
            {
                RunInitLocalIl2CppData(ApplicationIl2cppPath);
            }
            catch (Exception ex)
            {
                Debug.LogError($"[ZLua] Install failed:\n{ex}");
                throw;
            }
        }

        public bool HasInstalledToLocal()
        {
            return Directory.Exists(CommonDirs.LocalLibil2cppPath)
                   && Directory.Exists(CommonDirs.LocalZluaPath)
                   && Directory.Exists(CommonDirs.LocalLuaSrcPath);
        }

        public bool NeedReinstallAfterUpdatePackage()
        {
            if (!HasInstalledToLocal())
            {
                return false;
            }

            if (!InstallFingerprint.TryRead(out InstallFingerprintData saved))
            {
                return true;
            }

            string luaId = LuaVersionUtil.ResolveConfiguredOrDefault(Settings.Instance.luaVersionId, out _);
            if (!string.Equals(saved.luaVersionId, luaId, StringComparison.Ordinal)
                || !string.Equals(saved.unityVersion, Application.unityVersion, StringComparison.Ordinal)
                || !string.Equals(saved.packageContentStamp, ComputePackageContentStamp(), StringComparison.Ordinal))
            {
                return true;
            }

            return false;
        }

        public static string PlatformDirName
        {
            get
            {
#if UNITY_EDITOR_WIN
                string platformSuffix = "win";
#elif UNITY_EDITOR_OSX
                string platformSuffix = "osx";
#elif UNITY_EDITOR_LINUX
                string platformSuffix = "linux";
#else
                string platformSuffix = "unknown";
#endif
                string archSuffix;
                if (RuntimeInformation.ProcessArchitecture == Architecture.Arm)
                {
                    archSuffix = "arm";
                }
                else if (RuntimeInformation.ProcessArchitecture == Architecture.Arm64)
                {
                    archSuffix = "arm64";
                }
                else if (RuntimeInformation.ProcessArchitecture == Architecture.X64)
                {
                    archSuffix = "x64";
                }
                else if (RuntimeInformation.ProcessArchitecture == Architecture.X86)
                {
                    archSuffix = "x86";
                }
                else
                {
                    archSuffix = "unknownarch";
                }

                return $"{platformSuffix}-{archSuffix}";
            }
        }

        private void RunInitLocalIl2CppData(string editorIl2cppPath)
        {
            if (!Directory.Exists(editorIl2cppPath))
            {
                throw new InvalidOperationException($"Editor il2cpp path not found: {editorIl2cppPath}");
            }

            if (!Directory.Exists(CommonDirs.ZluaRuntimePathInPackage))
            {
                throw new InvalidOperationException(
                    $"zlua-runtime missing: {CommonDirs.ZluaRuntimePathInPackage}");
            }

            string luaVersionId = LuaVersionUtil.ResolveConfiguredOrDefault(
                Settings.Instance.luaVersionId, out bool wroteDefault);
            if (wroteDefault)
            {
                Settings.Instance.luaVersionId = luaVersionId;
                Settings.Save();
                Debug.Log($"[ZLua] Settings.luaVersionId defaulted to {luaVersionId}");
            }

            if (!LuaVersionUtil.TryParse(luaVersionId, out LuaVersionInfo luaInfo))
            {
                throw new InvalidOperationException($"Invalid luaVersionId: {luaVersionId}");
            }

            LuaSourceCache.EnsureAvailable(luaInfo);
            WarnIfEditorPluginMissing(luaInfo);

            Directory.CreateDirectory(CommonDirs.InstallRootDir);
            string localIl2CppDataDir = CommonDirs.LocalIl2CppDataPath;
            DirectoryUtil.RecreateDir(localIl2CppDataDir);

#if !NEW_IL2CPP_PATH
            DirectoryUtil.CopyDir(
                $"{Directory.GetParent(editorIl2cppPath)}/MonoBleedingEdge",
                $"{localIl2CppDataDir}/MonoBleedingEdge",
                true);
#endif
            DirectoryUtil.CopyDir(editorIl2cppPath, CommonDirs.LocalIl2CppPath, true);
#if NEW_IL2CPP_PATH
            string buildDir = $"{CommonDirs.LocalIl2CppPath}/build";
            if (RuntimeInformation.ProcessArchitecture == Architecture.Arm
                || RuntimeInformation.ProcessArchitecture == Architecture.Arm64)
            {
                DirectoryUtil.CopyDir($"{buildDir}/deploy_arm64", $"{buildDir}/deploy", false);
            }
            else
            {
                DirectoryUtil.CopyDir($"{buildDir}/deploy_x86_64", $"{buildDir}/deploy", false);
            }
#endif

            if (!UnityIl2CppPatchUtil.TryResolvePatchFile(
                    Application.unityVersion, out string il2cppPatchFile, out string il2cppPatchKey))
            {
                string series = UnityIl2CppPatchUtil.GetSeriesKey(Application.unityVersion);
                throw new InvalidOperationException(
                    $"[ZLua] No libil2cpp patch for Unity {Application.unityVersion}. "
                    + $"Expected a floor patch X.Y.Z.patch (greatest version <= Editor) under "
                    + Path.Combine(CommonDirs.Libil2cppPatchesPathInPackage, series ?? "?"));
            }

            Debug.Log($"[ZLua] Applying libil2cpp patch {il2cppPatchKey}: {il2cppPatchFile}");
            // Patch paths are libil2cpp/... — apply with cwd = LocalIl2CppPath.
            PatchApplier.Apply(il2cppPatchFile, CommonDirs.LocalIl2CppPath, stripComponents: 1);

            DirectoryUtil.CopyDir(CommonDirs.ZluaRuntimePathInPackage, CommonDirs.LocalZluaPath, true);

            string luaPatchKey = InstallLuaSources(luaInfo);

            string defines = ApplyScriptingDefines(luaInfo);
            // Spec §12: authoritative ZLuaConf.inc after zlua-runtime overlay + Lua install.
            ZLuaConfWriter.WriteLocal(luaInfo, Application.unityVersion);
            ValidateLocalTree();

            InstallFingerprint.Write(new InstallFingerprintData
            {
                unityVersion = Application.unityVersion,
                luaVersionId = luaInfo.Id,
                luaSeries = luaInfo.Series,
                libil2cppPatchKey = il2cppPatchKey,
                luaPatchKey = luaPatchKey,
                packageContentStamp = ComputePackageContentStamp(),
                defines = defines,
            });

            // Legacy stamp for older checks.
            WriteLegacyPackageMtimeStamp();

            DirectoryUtil.RemoveDir("Library/Il2cppBuildCache", true);
            DirectoryUtil.RemoveDir("Library/Bee", true);

            if (!HasInstalledToLocal())
            {
                throw new InvalidOperationException("[ZLua] Installation failed: local tree incomplete.");
            }

            RequiresEditorRestart = true;
            Debug.Log(
                $"[ZLua] Install succeeded. unity={Application.unityVersion} lua={luaInfo.Id} "
                + $"libil2cppPatch={il2cppPatchKey} luaPatch={luaPatchKey} defines={defines}. "
                + "Restart the Unity Editor if the Lua series/DLL or scripting defines changed.");
        }

        private static string InstallLuaSources(LuaVersionInfo luaInfo)
        {
            string stagingRoot = Path.Combine(CommonDirs.InstallRootDir, "tmp-lua-stage");
            DirectoryUtil.RecreateDir(stagingRoot);
            string stagedVersion = Path.Combine(stagingRoot, luaInfo.Id);
            DirectoryUtil.CopyDir(luaInfo.SourceDir, stagedVersion, true);

            if (!LuaVersionUtil.TryResolveLuaPatchFile(
                    luaInfo.Id, luaInfo.Series, out string luaPatchFile, out string luaPatchKey))
            {
                throw new InvalidOperationException(
                    $"[ZLua] No Lua patch for {luaInfo.Id} (series {luaInfo.Series}). "
                    + "Expected a floor patch X.Y.Z.patch (greatest version <= requested) under "
                    + Path.Combine(CommonDirs.LuaPatchesPathInPackage, luaInfo.Series));
            }

            Debug.Log($"[ZLua] Applying Lua patch {luaPatchKey}: {luaPatchFile}");
            // Patch paths are src/... — apply with cwd = staged version root (contains src/).
            if (!Directory.Exists(Path.Combine(stagedVersion, "src")))
            {
                throw new InvalidOperationException(
                    $"[ZLua] Lua version '{luaInfo.Id}' must use upstream layout with a src/ directory.");
            }

            try
            {
                PatchApplier.Apply(luaPatchFile, stagedVersion, stripComponents: 1);
            }
            catch (Exception ex)
            {
                throw new InvalidOperationException(
                    $"[ZLua] Failed to apply Lua patch for {luaInfo.Id}: {luaPatchFile}. "
                    + "Regenerate against that exact source tree, or add a version-specific patch "
                    + $"(e.g. patches/lua/{luaInfo.Series}/5.4.8.patch).\n"
                    + ex.Message,
                    ex);
            }

            string stagedSrc = Path.Combine(stagedVersion, "src");
            DirectoryUtil.CopyDir(stagedSrc, CommonDirs.LocalLuaSrcPath, true);

            string luacPath = Path.Combine(CommonDirs.LocalLuaSrcPath, "luac.c");
            if (File.Exists(luacPath))
            {
                File.Delete(luacPath);
            }

            DirectoryUtil.RemoveDir(stagingRoot, true);
            return luaPatchKey;
        }

        private static void WarnIfEditorPluginMissing(LuaVersionInfo luaInfo)
        {
#if UNITY_EDITOR_WIN
            string fileName = luaInfo.DllLogicalName + ".dll";
#elif UNITY_EDITOR_OSX
            string fileName = luaInfo.DllLogicalName + ".dylib";
#elif UNITY_EDITOR_LINUX
            string fileName = "lib" + luaInfo.DllLogicalName + ".so";
#else
            string fileName = luaInfo.DllLogicalName;
#endif
            string pluginsRoot = CommonDirs.PackagePluginsRoot;
            bool found = Directory.Exists(pluginsRoot)
                         && Directory.EnumerateFiles(pluginsRoot, fileName, SearchOption.AllDirectories).Any();
            if (!found)
            {
                Debug.LogWarning(
                    $"[ZLua] Editor plugin '{fileName}' not found under {pluginsRoot}. "
                    + "Replace/add the matching series binary yourself (e.g. lua53.dll / lua54.dll). "
                    + "Il2Cpp Player uses downloaded sources; Editor Mono needs the plugin DLL.");
            }
        }

        private string ApplyScriptingDefines(LuaVersionInfo luaInfo)
        {
            string wanted = luaInfo.IsLuaJit ? "ZLUA_USE_LUAJIT" : luaInfo.ApiFamilyDefine;
            var targets = new HashSet<NamedBuildTarget>
            {
                NamedBuildTarget.FromBuildTargetGroup(EditorUserBuildSettings.selectedBuildTargetGroup),
                NamedBuildTarget.Standalone,
                NamedBuildTarget.Android,
                NamedBuildTarget.iOS,
                NamedBuildTarget.Server,
            };

            foreach (NamedBuildTarget target in targets)
            {
                try
                {
                    string current = PlayerSettings.GetScriptingDefineSymbols(target);
                    var list = current.Split(new[] { ';' }, StringSplitOptions.RemoveEmptyEntries)
                        .Where(d => !IsZluaLuaDefine(d))
                        .ToList();
                    list.Add(wanted);
                    string next = string.Join(";", list);
                    if (!string.Equals(current, next, StringComparison.Ordinal))
                    {
                        PlayerSettings.SetScriptingDefineSymbols(target, next);
                        RequiresEditorRestart = true;
                    }
                }
                catch (ArgumentException)
                {
                    // NamedBuildTarget not valid on this Unity / platform set.
                }
            }

            return wanted;
        }

        private static bool IsZluaLuaDefine(string define)
        {
            foreach (string prefix in s_zluaDefinePrefixes)
            {
                if (define.StartsWith(prefix, StringComparison.Ordinal)
                    || string.Equals(define, prefix, StringComparison.Ordinal))
                {
                    return true;
                }
            }

            return false;
        }

        private static void ValidateLocalTree()
        {
            if (!File.Exists(Path.Combine(CommonDirs.LocalLibil2cppPath, "vm", "Runtime.cpp")))
            {
                throw new InvalidOperationException("[ZLua] Local libil2cpp incomplete after install.");
            }

            if (!Directory.Exists(CommonDirs.LocalZluaPath))
            {
                throw new InvalidOperationException("[ZLua] Local zlua missing after install.");
            }

            if (!File.Exists(Path.Combine(CommonDirs.LocalLuaSrcPath, "lua.h")))
            {
                throw new InvalidOperationException("[ZLua] Local lua sources missing after install.");
            }

            if (!File.Exists(ZLuaConfWriter.LocalConfPath))
            {
                throw new InvalidOperationException(
                    "[ZLua] generated/ZLuaConf.inc missing after install.");
            }

            // Minimal post-patch anchors.
            string runtimeCpp = File.ReadAllText(
                Path.Combine(CommonDirs.LocalLibil2cppPath, "vm", "Runtime.cpp"), Encoding.UTF8);
            if (!runtimeCpp.Contains("zlua::LuaAppDomain::Initialize")
                && !runtimeCpp.Contains("LuaAppDomain::Initialize"))
            {
                throw new InvalidOperationException(
                    "[ZLua] libil2cpp patch validation failed: LuaAppDomain::Initialize not found in Runtime.cpp.");
            }

            if (!File.Exists(Path.Combine(CommonDirs.LocalLuaSrcPath, "zlua_fastmt.c"))
                && !File.Exists(Path.Combine(CommonDirs.LocalLuaSrcPath, "zlua_fastmt.h")))
            {
                Debug.LogWarning(
                    "[ZLua] zlua_fastmt.* not found under local lua; ensure the Lua patch applied if fast-mt is required.");
            }
        }

        private static string GetZLuaPackageRootPath()
        {
            var packageInfo = PackageInfo.FindForAssembly(typeof(LocalInstaller).Assembly);
            if (!string.IsNullOrEmpty(packageInfo?.resolvedPath))
            {
                return packageInfo.resolvedPath;
            }

            return Path.GetFullPath(Path.Combine("Packages", CommonDirs.PackageName));
        }

        private static string ComputePackageContentStamp()
        {
            return ComputePackageLatestFileWriteTimeUtcTicks().ToString();
        }

        private static long ComputePackageLatestFileWriteTimeUtcTicks()
        {
            string root = GetZLuaPackageRootPath();
            if (!Directory.Exists(root))
            {
                return 0L;
            }

            return Directory.EnumerateFiles(root, "*", SearchOption.AllDirectories)
                .Where(f => !f.EndsWith(".meta", StringComparison.OrdinalIgnoreCase))
                .Select(f => File.GetLastWriteTimeUtc(f).Ticks)
                .DefaultIfEmpty(0L)
                .Max();
        }

        private static void WriteLegacyPackageMtimeStamp()
        {
            long ticks = ComputePackageLatestFileWriteTimeUtcTicks();
            if (ticks == 0L)
            {
                return;
            }

            string path = Path.Combine(CommonDirs.InstallRootDir, "installed_package_max_mtime_utc_ticks.txt");
            Directory.CreateDirectory(Path.GetDirectoryName(path));
            File.WriteAllText(path, ticks.ToString(), Encoding.UTF8);
        }
    }
}
