// Copyright 2026 Code Philosophy

using System;
using System.IO;
using System.Text.RegularExpressions;

namespace ZLua.Utils
{
    public static class UnityIl2CppPatchUtil
    {
        private static readonly Regex s_unityVer = new Regex(
            @"^(\d+)\.(\d+)\.(\d+)([a-zA-Z].*)?$",
            RegexOptions.CultureInvariant | RegexOptions.Compiled);

        private static readonly Regex s_patchFile = new Regex(
            @"^(\d+)\.(\d+)\.(\d+)\.patch$",
            RegexOptions.CultureInvariant | RegexOptions.Compiled);

        /// <summary>
        /// Resolve patch under <c>patches/libil2cpp/{series}/</c> with the same floor rule as Lua:
        /// greatest <c>{major}.{minor}.{patch}.patch</c> with version &lt;= Editor, no <c>default.patch</c>.
        /// Series dirs tried: <c>major.minor</c> then <c>major</c> when <c>major &gt;= 6000</c>.
        /// </summary>
        public static bool TryResolvePatchFile(string unityVersion, out string patchFile, out string patchKey)
        {
            patchFile = null;
            patchKey = null;
            string root = CommonDirs.Libil2cppPatchesPathInPackage;
            if (!Directory.Exists(root))
            {
                return false;
            }

            if (!TryParseUnityTriplet(unityVersion, out int reqMajor, out int reqMinor, out int reqPatch))
            {
                return false;
            }

            foreach (string series in EnumerateSeriesDirs(unityVersion))
            {
                string seriesDir = Path.Combine(root, series);
                if (!Directory.Exists(seriesDir))
                {
                    continue;
                }

                // Fast path: exact / stripped keys (same as previous exact candidates).
                foreach (string key in EnumerateExactFileKeys(unityVersion))
                {
                    string candidate = Path.Combine(seriesDir, key + ".patch");
                    if (File.Exists(candidate))
                    {
                        patchFile = candidate;
                        patchKey = $"{series}/{key}.patch";
                        return true;
                    }
                }

                string bestPath = null;
                string bestName = null;
                int bestMajor = -1, bestMinor = -1, bestPatch = -1;

                foreach (string path in Directory.GetFiles(seriesDir, "*.patch", SearchOption.TopDirectoryOnly))
                {
                    string name = Path.GetFileName(path);
                    Match pm = s_patchFile.Match(name);
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
                        bestName = name;
                        bestMajor = pMajor;
                        bestMinor = pMinor;
                        bestPatch = pPatch;
                    }
                }

                if (bestPath != null)
                {
                    patchFile = bestPath;
                    patchKey = $"{series}/{bestName}";
                    return true;
                }
            }

            return false;
        }

        /// <summary>Primary series key used in error messages (major.minor).</summary>
        public static string GetSeriesKey(string unityVersion)
        {
            string[] dirs = EnumerateSeriesDirs(unityVersion);
            return dirs.Length > 0 ? dirs[0] : null;
        }

        public static string[] EnumerateSeriesDirs(string unityVersion)
        {
            if (!TryParseUnityTriplet(unityVersion, out int major, out int minor, out _))
            {
                return Array.Empty<string>();
            }

            // 6000.x.y: try major.minor then major (shared 6000/ tree).
            if (major >= 6000)
            {
                return new[] { $"{major}.{minor}", major.ToString() };
            }

            return new[] { $"{major}.{minor}" };
        }

        public static string[] EnumerateExactFileKeys(string unityVersion)
        {
            if (string.IsNullOrWhiteSpace(unityVersion))
            {
                return Array.Empty<string>();
            }

            string v = unityVersion.Trim();
            Match m = s_unityVer.Match(v);
            if (!m.Success)
            {
                if (!TryParseUnityTriplet(v, out int maj, out int min, out int pat))
                {
                    return Array.Empty<string>();
                }

                return new[] { $"{maj}.{min}.{pat}", v };
            }

            string major = m.Groups[1].Value;
            string minor = m.Groups[2].Value;
            string patch = m.Groups[3].Value;
            string suffix = m.Groups[4].Success ? m.Groups[4].Value : string.Empty;

            return new[]
            {
                v,
                $"{major}.{minor}.{patch}{suffix}",
                $"{major}.{minor}.{patch}",
            };
        }

        public static bool TryParseUnityTriplet(string unityVersion, out int major, out int minor, out int patch)
        {
            major = minor = patch = 0;
            if (string.IsNullOrWhiteSpace(unityVersion))
            {
                return false;
            }

            Match m = s_unityVer.Match(unityVersion.Trim());
            if (m.Success)
            {
                major = int.Parse(m.Groups[1].Value);
                minor = int.Parse(m.Groups[2].Value);
                patch = int.Parse(m.Groups[3].Value);
                return true;
            }

            try
            {
                var uv = new UnityVersion(unityVersion);
                major = uv.major;
                minor = uv.minor1;
                patch = uv.minor2;
                return true;
            }
            catch
            {
                return false;
            }
        }

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
