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
            if (jit.Success || Regex.IsMatch(id, @"^luajit-\d+-\d+$", RegexOptions.CultureInvariant))
            {
                if (!LuaSourceCache.TryGetCacheFolderName(id, out string jitFolder))
                {
                    return false;
                }

                string jitId = id.StartsWith("luajit-", StringComparison.Ordinal) && id.Contains(".")
                    ? id
                    : $"luajit-{jitFolder.Substring("luajit-".Length).Replace('-', '.')}";
                // Normalize display id: luajit-2.1 from folder luajit-2-1
                if (Regex.IsMatch(id, @"^luajit-\d+-\d+$", RegexOptions.CultureInvariant))
                {
                    string[] parts = id.Substring("luajit-".Length).Split('-');
                    jitId = $"luajit-{parts[0]}.{parts[1]}";
                }

                info = new LuaVersionInfo
                {
                    Id = jitId,
                    Series = "luajit",
                    ApiFamilyDefine = "ZLUA_USE_LUAJIT",
                    DllLogicalName = "luajit",
                    IsLuaJit = true,
                    SourceDir = Path.Combine(CommonDirs.LuaSrcCacheDir, jitFolder),
                };
                return true;
            }

            Match m = s_pucRio.Match(id);
            if (!m.Success)
            {
                return false;
            }

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
        /// PUC-Rio FastMT / VM patches exist only for Lua 5.3+.
        /// Lua 5.1, 5.2, and LuaJIT: Install copies clean upstream sources (no patch);
        /// <c>ZLUA_FAST_METATABLE</c> stays 0.
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

            return minor >= 3;
        }

        /// <summary>
        /// FastMT (finishget path) is supported only for PUC-Rio <c>≥ 5.3.2</c> (and 5.4+ / 5.5+).
        /// <c>5.3.0</c> / <c>5.3.1</c>, 5.1, 5.2, and LuaJIT must keep <c>ZLUA_FAST_METATABLE 0</c>
        /// (spec 11-MULTI-VERSION §5.4).
        /// </summary>
        public static bool SupportsFastMetatable(LuaVersionInfo info)
        {
            if (!UsesLuaVmPatches(info))
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
            int patch = int.Parse(m.Groups[3].Value);
            if (major != 5)
            {
                return major > 5;
            }

            if (minor > 3)
            {
                return true;
            }

            if (minor < 3)
            {
                return false;
            }

            // 5.3.x: finishget exists from 5.3.2.
            return patch >= 2;
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
