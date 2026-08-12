// Copyright 2026 Code Philosophy

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

namespace ZLua.Utils
{
    public sealed class LuaVersionInfo
    {
        public string Id { get; set; }
        public string Series { get; set; }
        public string ApiFamilyDefine { get; set; }
        public string DllLogicalName { get; set; }
        public bool IsLuaJit { get; set; }
        public string SourceDir { get; set; }
    }

    public static class LuaVersionUtil
    {
        private static readonly Regex s_pucRio = new Regex(
            @"^lua-(\d+)\.(\d+)\.(\d+)$",
            RegexOptions.CultureInvariant | RegexOptions.Compiled);

        private static readonly Regex s_luaJit = new Regex(
            @"^luajit-(\d+)\.(\d+)(?:\.(.+))?$",
            RegexOptions.CultureInvariant | RegexOptions.Compiled);

        public static IReadOnlyList<string> ListCachedVersionIds()
        {
            string root = CommonDirs.LuaSrcCacheDir;
            if (!Directory.Exists(root))
            {
                return Array.Empty<string>();
            }

            return Directory.GetDirectories(root)
                .Select(Path.GetFileName)
                .Where(name => !string.Equals(name, "downloads", StringComparison.OrdinalIgnoreCase))
                .Where(name => LuaSourceCache.IsSourceReady(Path.Combine(root, name)))
                .Select(name => TryParse(name, out LuaVersionInfo parsed) ? parsed.Id : name)
                .Distinct(StringComparer.OrdinalIgnoreCase)
                .OrderBy(name => name, StringComparer.OrdinalIgnoreCase)
                .ToList();
        }

        /// <summary>
        /// Parses version id and computes cache path. Does not require sources to exist yet.
        /// </summary>
        public static bool TryParse(string versionId, out LuaVersionInfo info)
        {
            info = null;
            if (string.IsNullOrWhiteSpace(versionId))
            {
                return false;
            }

            string id = versionId.Trim();

            Match jit = s_luaJit.Match(id);
            if (jit.Success)
            {
                if (!LuaSourceCache.TryGetCacheFolderName(id, out string jitFolder))
                {
                    return false;
                }

                string major = jit.Groups[1].Value;
                string minor = jit.Groups[2].Value;
                string jitId = $"luajit-{major}.{minor}";

                info = new LuaVersionInfo
                {
                    Id = jitId,
                    Series = jitId,
                    ApiFamilyDefine = "ZLUA_USE_LUAJIT",
                    DllLogicalName = $"luajit{major}{minor}",
                    IsLuaJit = true,
                    SourceDir = Path.Combine(CommonDirs.LuaSrcCacheDir, jitFolder),
                };
                return true;
            }

            Match m = s_pucRio.Match(id);
            if (m.Success)
            {

                int major = int.Parse(m.Groups[1].Value);
                int minor = int.Parse(m.Groups[2].Value);
                // Editor plugin: lua5{minor}.dll (e.g. lua53 / lua54) — major is implied for 5.x.
                string dll = $"lua{major}{minor}";
                info = new LuaVersionInfo
                {
                    Id = id,
                    Series = $"lua-{major}.{minor}",
                    ApiFamilyDefine = $"ZLUA_LUA_{major}_{minor}",
                    DllLogicalName = dll,
                    IsLuaJit = false,
                    SourceDir = LuaSourceCache.GetCacheDirForVersionId(id),
                };
                return true;
            }
            return false;
        }

        public static string ResolveConfiguredOrDefault(string configuredId, out bool wroteDefault)
        {
            wroteDefault = false;
            if (!string.IsNullOrWhiteSpace(configuredId))
            {
                string trimmed = configuredId.Trim();
                if (TryParse(trimmed, out _))
                {
                    return trimmed;
                }

                throw new InvalidOperationException(
                    $"[ZLua] Invalid Settings.luaVersionId '{configuredId}'. "
                    + "Use lua-X.Y.Z (e.g. lua-5.4.8) or luajit-M.N (e.g. luajit-2.1).");
            }

            wroteDefault = true;
            return LuaSourceCache.DefaultPucRioVersionId;
        }

        /// <summary>
        /// PUC-Rio FastMT / VM patches: all official 5.x series (5.1+).
        /// LuaJIT: Install copies public headers only (no patch); see LocalInstaller.
        /// </summary>
        public static bool UsesLuaVmPatches(LuaVersionInfo info)
        {
            if (info == null || info.IsLuaJit)
            {
                return false;
            }

            Match m = s_pucRio.Match(info.Id ?? string.Empty);
            if (!m.Success)
            {
                return false;
            }

            int major = int.Parse(m.Groups[1].Value);
            int minor = int.Parse(m.Groups[2].Value);
            if (major != 5)
            {
                return major > 5;
            }

            // 5.1/5.2: gettable/settable FastMT. 5.3.0/5.3.1: same. ≥5.3.2: finishget/finishset.
            return minor >= 1;
        }

        /// <summary>
        /// FastMT is supported for all PUC-Rio 5.x micros with a series VM patch
        /// (5.1/5.2/5.3.0/5.3.1: gettable/settable; ≥5.3.2: finishget/finishset).
        /// LuaJIT must keep <c>ZLUA_FAST_METATABLE 0</c>.
        /// </summary>
        public static bool SupportsFastMetatable(LuaVersionInfo info)
        {
            if (info == null || info.IsLuaJit)
            {
                return false;
            }

            Match m = s_pucRio.Match(info.Id ?? string.Empty);
            if (!m.Success)
            {
                return false;
            }

            int major = int.Parse(m.Groups[1].Value);
            if (major != 5)
            {
                return major > 5;
            }

            // All PUC-Rio 5.x micros that Install patches (5.1+) enable FastMT.
            return true;
        }

        /// <summary>
        /// Series dir <c>patches/lua/lua-5.4/</c>: pick <c>{X.Y.Z}.patch</c> with the greatest
        /// version that is still <c>&lt;=</c> the requested micro-version (floor). Exact match is
        /// the fast path of the same rule. Shared ranges keep only the minimum version file.
        /// </summary>
        public static bool TryResolveLuaPatchFile(string versionId, string series, out string patchFile, out string patchKey)
        {
            patchFile = null;
            patchKey = null;
            if (!TryParse(versionId, out LuaVersionInfo parsed) || !UsesLuaVmPatches(parsed))
            {
                return false;
            }

            string seriesDir = Path.Combine(CommonDirs.LuaPatchesPathInPackage, series);
            if (!Directory.Exists(seriesDir))
            {
                return false;
            }

            Match req = s_pucRio.Match(versionId ?? string.Empty);
            if (!req.Success)
            {
                return false;
            }

            int reqMajor = int.Parse(req.Groups[1].Value);
            int reqMinor = int.Parse(req.Groups[2].Value);
            int reqPatch = int.Parse(req.Groups[3].Value);

            // Fast path: exact file name.
            string exactName = $"{reqMajor}.{reqMinor}.{reqPatch}.patch";
            string exactPath = Path.Combine(seriesDir, exactName);
            if (File.Exists(exactPath))
            {
                patchFile = exactPath;
                patchKey = exactName;
                return true;
            }

            string bestPath = null;
            string bestKey = null;
            int bestMajor = -1, bestMinor = -1, bestPatch = -1;

            foreach (string path in Directory.GetFiles(seriesDir, "*.patch", SearchOption.TopDirectoryOnly))
            {
                string name = Path.GetFileName(path);
                Match pm = Regex.Match(name, @"^(\d+)\.(\d+)\.(\d+)\.patch$", RegexOptions.CultureInvariant);
                if (!pm.Success)
                {
                    continue;
                }

                int pMajor = int.Parse(pm.Groups[1].Value);
                int pMinor = int.Parse(pm.Groups[2].Value);
                int pPatch = int.Parse(pm.Groups[3].Value);
                if (CompareSemVer(pMajor, pMinor, pPatch, reqMajor, reqMinor, reqPatch) > 0)
                {
                    continue;
                }

                if (bestPath == null
                    || CompareSemVer(pMajor, pMinor, pPatch, bestMajor, bestMinor, bestPatch) > 0)
                {
                    bestPath = path;
                    bestKey = name;
                    bestMajor = pMajor;
                    bestMinor = pMinor;
                    bestPatch = pPatch;
                }
            }

            if (bestPath == null)
            {
                return false;
            }

            patchFile = bestPath;
            patchKey = bestKey;
            return true;
        }

        /// <returns>Negative if a&lt;b, 0 if equal, positive if a&gt;b.</returns>
        private static int CompareSemVer(int aMaj, int aMin, int aPat, int bMaj, int bMin, int bPat)
        {
            if (aMaj != bMaj)
            {
                return aMaj.CompareTo(bMaj);
            }

            if (aMin != bMin)
            {
                return aMin.CompareTo(bMin);
            }

            return aPat.CompareTo(bPat);
        }
    }
}
