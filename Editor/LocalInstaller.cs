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
using System.Text.RegularExpressions;
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
            "ZLUA_LUAJIT_",
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

        /// <summary>
        /// Dev helper: refresh install fingerprint stamps to match the current package mtime
        /// without re-running Install. Use in the ZLua development project when editing package
        /// sources frequently so Generate/Build stale-install checks stay quiet.
        /// </summary>
        public void MarkAsInstalled()
        {
            if (!HasInstalledToLocal())
            {
                throw new InvalidOperationException(
                    "[ZLua] Local install not found. Run menu 'ZLua/Install...' first.");
            }

            string luaId = LuaVersionUtil.ResolveConfiguredOrDefault(Settings.Instance.luaVersionId, out _);
            if (!LuaVersionUtil.TryParse(luaId, out LuaVersionInfo luaInfo))
            {
                throw new InvalidOperationException($"[ZLua] Invalid luaVersionId: {luaId}");
            }

            InstallFingerprint.TryRead(out InstallFingerprintData saved);
            InstallFingerprint.Write(new InstallFingerprintData
            {
                unityVersion = Application.unityVersion,
                luaVersionId = luaInfo.Id,
                luaSeries = saved != null && !string.IsNullOrEmpty(saved.luaSeries)
                    ? saved.luaSeries
                    : luaInfo.Series,
                libil2cppPatchKey = saved?.libil2cppPatchKey ?? string.Empty,
                luaPatchKey = saved?.luaPatchKey ?? string.Empty,
                packageContentStamp = ComputePackageContentStamp(),
                defines = saved?.defines ?? string.Empty,
            });
            WriteLegacyPackageMtimeStamp();
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
            ValidateLocalTree(luaInfo);

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

            // Patch paths are src/... — apply with cwd = staged version root (contains src/).
            if (!Directory.Exists(Path.Combine(stagedVersion, "src")))
            {
                throw new InvalidOperationException(
                    $"[ZLua] Lua version '{luaInfo.Id}' must use upstream layout with a src/ directory.");
            }

            string luaPatchKey;
            if (!LuaVersionUtil.UsesLuaVmPatches(luaInfo))
            {
                // Lua 5.1 / 5.2: no FastMT VM patch; install clean upstream sources.
                // LuaJIT: headers-only into libil2cpp/lua (see InstallLuaJitHeadersOnly).
                Debug.Log(
                    $"[ZLua] Skipping Lua VM patch for {luaInfo.Id} "
                    + "(FastMT / series patches apply only to PUC-Rio 5.3+).");
                luaPatchKey = "none";
            }
            else
            {
                if (!LuaVersionUtil.TryResolveLuaPatchFile(
                        luaInfo.Id, luaInfo.Series, out string luaPatchFile, out luaPatchKey))
                {
                    throw new InvalidOperationException(
                        $"[ZLua] No Lua patch for {luaInfo.Id} (series {luaInfo.Series}). "
                        + "Expected a floor patch X.Y.Z.patch (greatest version <= requested) under "
                        + Path.Combine(CommonDirs.LuaPatchesPathInPackage, luaInfo.Series));
                }

                Debug.Log($"[ZLua] Applying Lua patch {luaPatchKey}: {luaPatchFile}");
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
            }

            string stagedSrc = Path.Combine(stagedVersion, "src");
            DirectoryUtil.RecreateDir(CommonDirs.LocalLuaSrcPath);

            if (luaInfo.IsLuaJit)
            {
                // Spec build/02-LUAJIT: Il2Cpp gets public headers only; .a from Plugins.
                InstallLuaJitHeadersOnly(stagedSrc, CommonDirs.LocalLuaSrcPath);
            }
            else
            {
                DirectoryUtil.CopyDir(stagedSrc, CommonDirs.LocalLuaSrcPath, true);

                // Standalone interpreter / compiler must not enter Il2Cpp (duplicate main, unused tools).
                RemoveLuaStandaloneSources(CommonDirs.LocalLuaSrcPath);

                // iOS/tvOS/watchOS: system(3) is unavailable; enable Lua's IOS profile (+ fallback).
                EnsureAppleMobileLuaOsProfile(CommonDirs.LocalLuaSrcPath);

                // Lua 5.1/5.2: luai_num* / number tricks / luai_hashnum gated on LUA_CORE / ltable_c;
                // Il2Cpp lumps many .c into one TU so the first header include wins (include guard).
                // Lua 5.1 only: lua_tmpnam gated on loslib_c (5.2+ defines it inside loslib.c).
                int family = EngineVersionUtil.EncodeLuaApiFamily(luaInfo);
                if (family < 503)
                {
                    EnsureLuaiNumMacrosForIl2CppLump(CommonDirs.LocalLuaSrcPath);
                    EnsureLuaiHashnumMacrosForIl2CppLump(CommonDirs.LocalLuaSrcPath);
                }

                if (family < 502)
                {
                    EnsureLuaTmpnamMacrosForIl2CppLump(CommonDirs.LocalLuaSrcPath);
                    EnsureLoadlibAnsiApisForWin32(CommonDirs.LocalLuaSrcPath);
                }
            }

            if (!LuaVersionUtil.SupportsFastMetatable(luaInfo))
            {
                EnsureFastMetatableDisabled(CommonDirs.LocalLuaSrcPath);
            }

            DirectoryUtil.RemoveDir(stagingRoot, true);
            return luaPatchKey;
        }

        /// <summary>
        /// LuaJIT Il2Cpp: copy public API headers only (no <c>lj_*.c</c> / <c>host/</c>).
        /// Player links developer-supplied static libs via Plugins (spec build/02-LUAJIT).
        /// </summary>
        private static void InstallLuaJitHeadersOnly(string stagedSrc, string destLuaDir)
        {
            string[] requiredHeaders =
            {
                "lua.h",
                "lauxlib.h",
                "lualib.h",
                "luaconf.h",
                "luajit.h",
            };

            foreach (string name in requiredHeaders)
            {
                string src = Path.Combine(stagedSrc, name);
                if (!File.Exists(src))
                {
                    throw new InvalidOperationException(
                        $"[ZLua] LuaJIT header missing in upstream src/: {name} (under {stagedSrc}).");
                }

                File.Copy(src, Path.Combine(destLuaDir, name), overwrite: true);
            }

            // Optional C++ wrapper if present in the tree.
            string luaHpp = Path.Combine(stagedSrc, "lua.hpp");
            if (File.Exists(luaHpp))
            {
                File.Copy(luaHpp, Path.Combine(destLuaDir, "lua.hpp"), overwrite: true);
            }

            Debug.Log(
                $"[ZLua] LuaJIT: installed public headers only under {destLuaDir} "
                + "(provide libluajit.a in Plugins for Android/iOS Il2Cpp; see spec build/02-LUAJIT).");
        }

        /// <summary>
        /// Drop PUC-Rio CLI sources from <c>libil2cpp/lua</c> so Player builds only link the library.
        /// </summary>
        private static void RemoveLuaStandaloneSources(string luaSrcDir)
        {
            // lua.c: interpreter main. luac.c: compiler main. print.c: luac helper (Lua 5.1 only).
            string[] standaloneFiles = { "lua.c", "luac.c", "print.c" };
            foreach (string name in standaloneFiles)
            {
                string path = Path.Combine(luaSrcDir, name);
                if (!File.Exists(path))
                {
                    continue;
                }

                File.Delete(path);
                Debug.Log($"[ZLua] Removed Lua standalone source: {path}");
            }
        }

        /// <summary>
        /// Lua 5.1/5.2 gate <c>luai_num*</c> on per-TU macros (<c>LUA_CORE</c>,
        /// <c>lobject_c</c>/<c>lvm_c</c>). Il2Cpp may lump many <c>.c</c> into one TU; the first
        /// include of <c>luaconf.h</c> without those macros locks the include guard, so later
        /// core files see no macros and link as extern <c>luai_numadd</c> / <c>luai_nummod</c> etc.
        /// Force those macros on (do not define the TU tags from ZLua headers).
        /// </summary>
        private static void EnsureLuaiNumMacrosForIl2CppLump(string luaSrcDir)
        {
            string luaconf = Path.Combine(luaSrcDir, "luaconf.h");
            if (!File.Exists(luaconf))
            {
                throw new InvalidOperationException(
                    $"[ZLua] luaconf.h not found under {luaSrcDir}; cannot fix luai_num* for Il2Cpp lump.");
            }

            string text = File.ReadAllText(luaconf, Encoding.UTF8);
            string next = text;
            bool changed = false;

            // 5.1: #if LUA_CORE / #include math.h / #define luai_numadd(a,b)
            // 5.2: #if LUA_CORE / #define luai_numadd(L,a,b)
            {
                const string pattern =
                    @"#if\s+defined\s*\(\s*LUA_CORE\s*\)(\r?\n(?:[ \t]*#include[^\r\n]*\r?\n)?)([ \t]*#define\s+luai_numadd\b)";
                const string replacement =
                    "#if 1 /* ZLua: Il2Cpp lump-safe — always define luai_num* (do not rely on LUA_CORE) */$1$2";
                string replaced = Regex.Replace(next, pattern, replacement, RegexOptions.CultureInvariant);
                if (!string.Equals(replaced, next, StringComparison.Ordinal))
                {
                    next = replaced;
                    changed = true;
                }
                else if (next.Contains("luai_numadd")
                         && !next.Contains("ZLua: Il2Cpp lump-safe — always define luai_num*")
                         && Regex.IsMatch(next, @"#if\s+defined\s*\(\s*LUA_CORE\s*\)"))
                {
                    throw new InvalidOperationException(
                        $"[ZLua] Failed to ungate luai_numadd in {luaconf}. "
                        + "Expected '#if defined(LUA_CORE)' immediately before '#define luai_numadd'.");
                }
            }

            // 5.2: #if lobject_c || lvm_c / #include math.h / #define luai_nummod / luai_numpow
            {
                const string pattern =
                    @"#if\s+defined\s*\(\s*lobject_c\s*\)\s*\|\|\s*defined\s*\(\s*lvm_c\s*\)";
                const string replacement =
                    "#if 1 /* ZLua: Il2Cpp lump-safe — always define luai_nummod/pow (do not rely on lobject_c/lvm_c) */";
                if (next.Contains("luai_nummod") && next.Contains("lobject_c"))
                {
                    string replaced = Regex.Replace(next, pattern, replacement, RegexOptions.CultureInvariant);
                    if (!string.Equals(replaced, next, StringComparison.Ordinal))
                    {
                        next = replaced;
                        changed = true;
                    }
                    else if (!next.Contains("ZLua: Il2Cpp lump-safe — always define luai_nummod/pow"))
                    {
                        throw new InvalidOperationException(
                            $"[ZLua] Failed to ungate luai_nummod/pow in {luaconf}. "
                            + "Expected '#if defined(lobject_c) || defined(lvm_c)'.");
                    }
                }
            }

            if (!changed)
            {
                return;
            }

            File.WriteAllText(luaconf, next, new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));
            Debug.Log($"[ZLua] Ungated luai_num* macros for Il2Cpp lump in {luaconf}");
        }

        /// <summary>
        /// Lua 5.2 defines <c>luai_hashnum</c> either via <c>LUA_IEEE754TRICK</c> (set only under
        /// <c>LUA_CORE</c> in <c>luaconf.h</c>) or a frexp fallback gated on <c>ltable_c</c> in
        /// <c>llimits.h</c>. Il2Cpp lump + include guards mean those per-TU macros may be unset when
        /// headers are first parsed, so <c>ltable.c</c> later sees an undefined <c>luai_hashnum</c>
        /// (MSVC C4013 / C4700). Always enable the number→int trick block and the frexp fallback.
        /// </summary>
        private static void EnsureLuaiHashnumMacrosForIl2CppLump(string luaSrcDir)
        {
            string luaconf = Path.Combine(luaSrcDir, "luaconf.h");
            string llimits = Path.Combine(luaSrcDir, "llimits.h");
            if (!File.Exists(luaconf) || !File.Exists(llimits))
            {
                throw new InvalidOperationException(
                    $"[ZLua] luaconf.h/llimits.h not found under {luaSrcDir}; cannot fix luai_hashnum for Il2Cpp lump.");
            }

            // luaconf.h: ungate LUA_CORE around MS_ASMTRICK / LUA_IEEE754TRICK (feeds llimits.h).
            {
                string text = File.ReadAllText(luaconf, Encoding.UTF8);
                if (!text.Contains("ZLua: Il2Cpp lump-safe — number→int tricks"))
                {
                    const string pattern =
                        @"#if\s+defined\s*\(\s*LUA_CORE\s*\)([^\r\n]*\r?\n\r?\n#if\s+defined\s*\(\s*LUA_NUMBER_DOUBLE\s*\))";
                    const string replacement =
                        "#if 1 /* ZLua: Il2Cpp lump-safe — number→int tricks (do not rely on LUA_CORE) */$1";
                    string next = Regex.Replace(text, pattern, replacement, RegexOptions.CultureInvariant);
                    if (string.Equals(next, text, StringComparison.Ordinal))
                    {
                        if (text.Contains("LUA_IEEE754TRICK") || text.Contains("MS_ASMTRICK"))
                        {
                            throw new InvalidOperationException(
                                $"[ZLua] Failed to ungate LUA_IEEE754TRICK/MS_ASMTRICK in {luaconf}. "
                                + "Expected '#if defined(LUA_CORE)' before '#if defined(LUA_NUMBER_DOUBLE)'.");
                        }
                    }
                    else
                    {
                        File.WriteAllText(luaconf, next, new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));
                        Debug.Log($"[ZLua] Ungated number→int tricks for Il2Cpp lump in {luaconf}");
                    }
                }
            }

            // llimits.h: frexp fallback must not require ltable_c (MS_ASMTRICK path has no luai_hashnum).
            {
                string text = File.ReadAllText(llimits, Encoding.UTF8);
                if (text.Contains("ZLua: Il2Cpp lump-safe — always define luai_hashnum"))
                {
                    return;
                }

                const string pattern =
                    @"#if\s+defined\s*\(\s*ltable_c\s*\)\s*&&\s*!defined\s*\(\s*luai_hashnum\s*\)";
                const string replacement =
                    "#if !defined(luai_hashnum) /* ZLua: Il2Cpp lump-safe — always define luai_hashnum (do not rely on ltable_c) */";
                string next = Regex.Replace(text, pattern, replacement, RegexOptions.CultureInvariant);
                if (string.Equals(next, text, StringComparison.Ordinal))
                {
                    if (text.Contains("luai_hashnum") && text.Contains("ltable_c"))
                    {
                        throw new InvalidOperationException(
                            $"[ZLua] Failed to ungate luai_hashnum in {llimits}. "
                            + "Expected '#if defined(ltable_c) && !defined(luai_hashnum)'.");
                    }

                    return;
                }

                File.WriteAllText(llimits, next, new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));
                Debug.Log($"[ZLua] Ungated luai_hashnum frexp fallback for Il2Cpp lump in {llimits}");
            }
        }

        /// <summary>
        /// Lua 5.1 gates <c>lua_tmpnam</c> / <c>LUA_TMPNAMBUFSIZE</c> on <c>loslib_c</c> in
        /// <c>luaconf.h</c>. Same Il2Cpp lump/include-guard issue as <c>LUA_CORE</c>/<c>luai_num*</c>.
        /// Always define those macros (5.2+ already keeps them in <c>loslib.c</c>).
        /// Do not <c>#define loslib_c</c> from ZLua headers.
        /// </summary>
        private static void EnsureLuaTmpnamMacrosForIl2CppLump(string luaSrcDir)
        {
            string luaconf = Path.Combine(luaSrcDir, "luaconf.h");
            if (!File.Exists(luaconf))
            {
                throw new InvalidOperationException(
                    $"[ZLua] luaconf.h not found under {luaSrcDir}; cannot fix lua_tmpnam for Il2Cpp lump.");
            }

            string text = File.ReadAllText(luaconf, Encoding.UTF8);
            if (text.Contains("ZLua: Il2Cpp lump-safe — always define lua_tmpnam"))
            {
                return;
            }

            const string pattern =
                @"#if\s+defined\s*\(\s*loslib_c\s*\)\s*\|\|\s*defined\s*\(\s*luaall_c\s*\)";
            const string replacement =
                "#if 1 /* ZLua: Il2Cpp lump-safe — always define lua_tmpnam (do not rely on loslib_c) */";

            string next = Regex.Replace(text, pattern, replacement, RegexOptions.CultureInvariant);
            if (string.Equals(next, text, StringComparison.Ordinal))
            {
                if (text.Contains("LUA_TMPNAMBUFSIZE") && text.Contains("loslib_c"))
                {
                    throw new InvalidOperationException(
                        $"[ZLua] Failed to ungate lua_tmpnam in {luaconf}. "
                        + "Expected '#if defined(loslib_c) || defined(luaall_c)'.");
                }

                return;
            }

            File.WriteAllText(luaconf, next, new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));
            Debug.Log($"[ZLua] Ungated lua_tmpnam macros for Il2Cpp lump in {luaconf}");
        }

        /// <summary>
        /// Lua 5.1 <c>loadlib.c</c> calls bare <c>GetModuleFileName</c> / <c>FormatMessage</c> /
        /// <c>LoadLibrary</c> with <c>char*</c> buffers. Unity/Il2Cpp defines <c>UNICODE</c>, so those
        /// macros resolve to the <c>*W</c> APIs and write UTF-16 into ANSI buffers — <c>strrchr</c>
        /// then hits the first code-unit's trailing <c>NUL</c>. Lua 5.2+ already uses <c>*A</c>.
        /// </summary>
        private static void EnsureLoadlibAnsiApisForWin32(string luaSrcDir)
        {
            string loadlib = Path.Combine(luaSrcDir, "loadlib.c");
            if (!File.Exists(loadlib))
            {
                throw new InvalidOperationException(
                    $"[ZLua] loadlib.c not found under {luaSrcDir}; cannot fix Win32 ANSI APIs.");
            }

            string text = File.ReadAllText(loadlib, Encoding.UTF8);
            if (text.Contains("ZLua: Il2Cpp/Unity defines UNICODE"))
            {
                return;
            }

            if (!text.Contains("GetModuleFileName(NULL"))
            {
                // Already GetModuleFileNameA, or no DLL backend in this tree.
                return;
            }

            string next = text;
            next = next.Replace(
                "DWORD n = GetModuleFileName(NULL, buff, nsize);",
                "/* ZLua: Il2Cpp/Unity defines UNICODE; bare GetModuleFileName → W and writes\n"
                + "     UTF-16 into char buff so strrchr stops at the first ASCII code unit's NUL. */\n"
                + "  DWORD n = GetModuleFileNameA(NULL, buff, nsize);");
            next = next.Replace(
                "if (FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,",
                "if (FormatMessageA(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,");
            next = next.Replace(
                "HINSTANCE lib = LoadLibrary(path);",
                "HINSTANCE lib = LoadLibraryA(path);");

            if (string.Equals(next, text, StringComparison.Ordinal)
                || !next.Contains("GetModuleFileNameA(NULL"))
            {
                throw new InvalidOperationException(
                    $"[ZLua] Failed to rewrite Win32 ANSI APIs in {loadlib}.");
            }

            File.WriteAllText(loadlib, next, new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));
            Debug.Log($"[ZLua] Forced loadlib Win32 *A APIs for UNICODE builds in {loadlib}");
        }

        /// <summary>
        /// Apple mobile SDKs mark <c>system(3)</c> unavailable. PUC-Rio 5.4+ gates this behind
        /// <c>LUA_USE_IOS</c>; older series call <c>system</c> directly. Inject auto-detect so
        /// Il2Cpp iOS/simulator builds do not need per-target compiler flags.
        /// </summary>
        private static void EnsureAppleMobileLuaOsProfile(string luaSrcDir)
        {
            string luaconf = Path.Combine(luaSrcDir, "luaconf.h");
            if (!File.Exists(luaconf))
            {
                throw new InvalidOperationException(
                    $"[ZLua] luaconf.h not found under {luaSrcDir}; cannot apply Apple mobile OS profile.");
            }

            string text = File.ReadAllText(luaconf, Encoding.UTF8);
            const string marker = "ZLua: Apple mobile";
            if (text.Contains(marker))
            {
                return;
            }

            const string block =
                "\n"
                + "/* ZLua: Apple mobile — system(3) is unavailable on iOS/tvOS/watchOS. */\n"
                + "#if defined(__APPLE__)\n"
                + "#include <TargetConditionals.h>\n"
                + "#if TARGET_OS_IPHONE\n"
                + "#if !defined(LUA_USE_IOS)\n"
                + "#define LUA_USE_IOS\n"
                + "#endif\n"
                + "/* Fallback for older Lua that call system() directly (pre-LUA_USE_IOS). */\n"
                + "#include <stdlib.h>\n"
                + "#undef system\n"
                + "#define system(s) ((s) == NULL ? 1 : -1)\n"
                + "#endif\n"
                + "#endif\n";

            // Prefer inserting before the final include-guard #endif.
            int lastEndif = text.LastIndexOf("#endif", StringComparison.Ordinal);
            if (lastEndif >= 0)
            {
                text = text.Insert(lastEndif, block + "\n");
            }
            else
            {
                text = text.TrimEnd() + "\n" + block;
            }

            File.WriteAllText(luaconf, text, new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));
            Debug.Log($"[ZLua] Ensured Apple mobile Lua OS profile in {luaconf}");
        }

        /// <summary>
        /// Spec §5.4 / §12.5: FastMT unsupported combinations must keep
        /// <c>ZLUA_FAST_METATABLE 0</c> in upstream <c>luaconf.h</c>.
        /// </summary>
        private static void EnsureFastMetatableDisabled(string luaSrcDir)
        {
            string luaconf = Path.Combine(luaSrcDir, "luaconf.h");
            if (!File.Exists(luaconf))
            {
                throw new InvalidOperationException(
                    $"[ZLua] luaconf.h not found under {luaSrcDir}; cannot set ZLUA_FAST_METATABLE=0.");
            }

            string text = File.ReadAllText(luaconf, Encoding.UTF8);
            const string marker = "ZLUA_FAST_METATABLE";
            if (Regex.IsMatch(text, @"#\s*define\s+ZLUA_FAST_METATABLE\s+\d+"))
            {
                text = Regex.Replace(
                    text,
                    @"#\s*define\s+ZLUA_FAST_METATABLE\s+\d+",
                    "#define ZLUA_FAST_METATABLE 0");
            }
            else if (!text.Contains(marker))
            {
                text = text.TrimEnd()
                       + "\n\n"
                       + "/* ZLua: FastMT unsupported on this Lua series/micro — keep off (spec §5.4). */\n"
                       + "#if !defined(ZLUA_FAST_METATABLE)\n"
                       + "#define ZLUA_FAST_METATABLE 0\n"
                       + "#endif\n";
            }

            File.WriteAllText(luaconf, text, new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));
            Debug.Log($"[ZLua] Ensured ZLUA_FAST_METATABLE=0 in {luaconf}");
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
                    $"[ZLua] Editor plugin '{fileName}' not found under {pluginsRoot} (expected Plugins/lua/<series>/). "
                    + "Replace/add the matching series binary yourself (e.g. lua/lua53/lua53.dll or lua/luajit21/luajit21.dylib). "
                    + "Il2Cpp Player uses downloaded sources; Editor Mono needs the plugin DLL.");
            }
        }

        private string ApplyScriptingDefines(LuaVersionInfo luaInfo)
        {
            string[] wantedDefines = luaInfo.IsLuaJit
                ? new[] { "ZLUA_USE_LUAJIT", GetLuaJitVersionDefine(luaInfo) }
                : new[] { luaInfo.ApiFamilyDefine };
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
                    foreach (string wanted in wantedDefines)
                    {
                        if (!list.Contains(wanted))
                        {
                            list.Add(wanted);
                        }
                    }

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

            return string.Join(";", wantedDefines);
        }

        private static string GetLuaJitVersionDefine(LuaVersionInfo luaInfo)
        {
            Match match = Regex.Match(luaInfo.Id ?? string.Empty, @"^luajit-(\d+)\.(\d+)$", RegexOptions.CultureInvariant);
            if (!match.Success)
            {
                throw new InvalidOperationException($"[ZLua] Invalid LuaJIT version id: {luaInfo.Id}");
            }

            return $"ZLUA_LUAJIT_{match.Groups[1].Value}_{match.Groups[2].Value}";
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

        private static void ValidateLocalTree(LuaVersionInfo luaInfo)
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

            if (luaInfo.IsLuaJit
                && !File.Exists(Path.Combine(CommonDirs.LocalLuaSrcPath, "luajit.h")))
            {
                throw new InvalidOperationException(
                    "[ZLua] Local LuaJIT headers incomplete after install (luajit.h missing).");
            }

            if (luaInfo.IsLuaJit
                && Directory.EnumerateFiles(CommonDirs.LocalLuaSrcPath, "*.c", SearchOption.TopDirectoryOnly).Any())
            {
                throw new InvalidOperationException(
                    "[ZLua] LuaJIT install must not place .c sources under libil2cpp/lua "
                    + "(headers only; see spec build/02-LUAJIT).");
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

            if (LuaVersionUtil.UsesLuaVmPatches(luaInfo)
                && !File.Exists(Path.Combine(CommonDirs.LocalLuaSrcPath, "zlua_fastmt.c"))
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
