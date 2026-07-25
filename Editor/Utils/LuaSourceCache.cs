// Copyright 2026 Code Philosophy

using System;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Text.RegularExpressions;
using Debug = UnityEngine.Debug;

namespace ZLua.Utils
{
    /// <summary>
    /// Resolves PUC-Rio Lua / LuaJIT trees under <see cref="CommonDirs.LuaSrcCacheDir"/>.
    /// PUC-Rio: download https://www.lua.org/ftp/lua-X.Y.Z.tar.gz when missing.
    /// LuaJIT: developer must clone into the cache folder (no auto-download).
    /// </summary>
    public static class LuaSourceCache
    {
        public const string DefaultPucRioVersionId = "lua-5.3.6";

        private static readonly Regex s_pucRio = new Regex(
            @"^lua-(\d+)\.(\d+)\.(\d+)$",
            RegexOptions.CultureInvariant | RegexOptions.Compiled);

        private static readonly Regex s_luaJit = new Regex(
            @"^luajit-(\d+)\.(\d+)(?:\.(.+))?$",
            RegexOptions.CultureInvariant | RegexOptions.Compiled);

        public static string GetCacheDirForVersionId(string versionId)
        {
            if (!TryGetCacheFolderName(versionId, out string folder))
            {
                throw new InvalidOperationException($"Invalid luaVersionId: {versionId}");
            }

            return Path.Combine(CommonDirs.LuaSrcCacheDir, folder);
        }

        /// <summary>
        /// PUC-Rio: <c>lua-5.5.0</c>. LuaJIT: <c>luajit-{major}-{minor}</c> (e.g. <c>luajit-2-1</c>).
        /// </summary>
        public static bool TryGetCacheFolderName(string versionId, out string folderName)
        {
            folderName = null;
            if (string.IsNullOrWhiteSpace(versionId))
            {
                return false;
            }

            string id = versionId.Trim();
            Match puc = s_pucRio.Match(id);
            if (puc.Success)
            {
                folderName = id;
                return true;
            }

            Match jit = s_luaJit.Match(id);
            if (jit.Success)
            {
                folderName = $"luajit-{jit.Groups[1].Value}-{jit.Groups[2].Value}";
                return true;
            }

            // Allow already-canonical jit cache folder as id: luajit-2-1
            if (Regex.IsMatch(id, @"^luajit-\d+-\d+$", RegexOptions.CultureInvariant))
            {
                folderName = id;
                return true;
            }

            return false;
        }

        public static bool IsSourceReady(string sourceRoot)
        {
            return File.Exists(Path.Combine(sourceRoot, "src", "lua.h"))
                   || File.Exists(Path.Combine(sourceRoot, "src", "lua.hpp"))
                   || File.Exists(Path.Combine(sourceRoot, "src", "luajit.h"))
                   || File.Exists(Path.Combine(sourceRoot, "src", "lj_obj.h"));
        }

        /// <summary>
        /// Ensures sources exist in cache; downloads PUC-Rio tarball when needed.
        /// </summary>
        public static string EnsureAvailable(LuaVersionInfo info)
        {
            string cacheDir = info.SourceDir;
            if (IsSourceReady(cacheDir))
            {
                Debug.Log($"[ZLua] Using cached Lua sources: {cacheDir}");
                return cacheDir;
            }

            if (info.IsLuaJit)
            {
                throw new InvalidOperationException(
                    $"[ZLua] LuaJIT sources not found at:\n  {cacheDir}\n"
                    + "Clone LuaJIT into that directory yourself (ZLua does not download LuaJIT). "
                    + $"Expected layout: {cacheDir}/src/...");
            }

            DownloadAndExtractPucRio(info.Id, cacheDir);
            if (!IsSourceReady(cacheDir))
            {
                throw new InvalidOperationException(
                    $"[ZLua] Downloaded Lua sources look incomplete: {cacheDir}");
            }

            return cacheDir;
        }

        private static void DownloadAndExtractPucRio(string versionId, string cacheDir)
        {
            Match m = s_pucRio.Match(versionId);
            if (!m.Success)
            {
                throw new InvalidOperationException($"Not a PUC-Rio version id: {versionId}");
            }

            string ver = $"{m.Groups[1].Value}.{m.Groups[2].Value}.{m.Groups[3].Value}";
            string url = $"https://lua.org/ftp/lua-{ver}.tar.gz";
            string downloads = Path.Combine(CommonDirs.LuaSrcCacheDir, "downloads");
            Directory.CreateDirectory(downloads);
            string tarball = Path.Combine(downloads, $"lua-{ver}.tar.gz");

            if (!File.Exists(tarball) || new FileInfo(tarball).Length < 1024)
            {
                Debug.Log($"[ZLua] Downloading {url} ...");
                try
                {
                    using (var wc = new WebClient())
                    {
                        wc.DownloadFile(url, tarball);
                    }
                }
                catch (Exception ex)
                {
                    if (File.Exists(tarball))
                    {
                        File.Delete(tarball);
                    }

                    throw new InvalidOperationException(
                        $"[ZLua] Failed to download {url}. "
                        + "Check the version id exists on https://lua.org/ftp/ (e.g. lua-5.3.6, lua-5.4.8, lua-5.5.0).\n"
                        + ex.Message,
                        ex);
                }
            }
            else
            {
                Debug.Log($"[ZLua] Reusing downloaded archive: {tarball}");
            }

            string extractParent = CommonDirs.LuaSrcCacheDir;
            Directory.CreateDirectory(extractParent);
            if (Directory.Exists(cacheDir))
            {
                DirectoryUtil.RemoveDir(cacheDir, true);
            }

            // Windows 10+ / macOS / Linux: bsdtar/tar understands .tar.gz
            var psi = new ProcessStartInfo
            {
                FileName = "tar",
                Arguments = $"-xzf \"{tarball}\" -C \"{extractParent}\"",
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                CreateNoWindow = true,
            };
            using (var proc = Process.Start(psi))
            {
                string stderr = proc.StandardError.ReadToEnd();
                proc.WaitForExit();
                if (proc.ExitCode != 0)
                {
                    throw new InvalidOperationException(
                        $"[ZLua] tar extract failed (exit {proc.ExitCode}) for {tarball}.\n{stderr}");
                }
            }

            // Official tarball extracts to lua-X.Y.Z/; rename/move if needed.
            string extracted = Path.Combine(extractParent, $"lua-{ver}");
            if (!string.Equals(Path.GetFullPath(extracted), Path.GetFullPath(cacheDir), StringComparison.OrdinalIgnoreCase))
            {
                if (Directory.Exists(extracted) && !Directory.Exists(cacheDir))
                {
                    Directory.Move(extracted, cacheDir);
                }
            }

            Debug.Log($"[ZLua] Lua sources ready: {cacheDir}");
        }
    }
}
