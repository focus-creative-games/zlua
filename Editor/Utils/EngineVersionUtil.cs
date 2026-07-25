// Copyright 2026 Code Philosophy

using System;
using System.Reflection;
using System.Text.RegularExpressions;
using UnityEngine;

namespace ZLua.Utils
{
    /// <summary>
    /// Unity / Tuanjie version encoding for <c>ZLuaConf.inc</c> (spec 11-MULTI-VERSION §12).
    /// </summary>
    public static class EngineVersionUtil
    {
        private static readonly Regex s_semVer = new Regex(
            @"(\d+)\.(\d+)\.(\d+)",
            RegexOptions.CultureInvariant | RegexOptions.Compiled);

        /// <summary>True when running under Tuanjie Engine (not stock Unity).</summary>
        public static bool IsTuanjieEngine(string unityVersionStr = null)
        {
#if TUANJIE_2022_3_OR_NEWER
            return true;
#else
            if (TryGetTuanjieVersionString(out _))
            {
                return true;
            }

            string v = unityVersionStr ?? Application.unityVersion;
            // e.g. 2022.3.62t11 — avoid matching letter "f" release tags.
            return Regex.IsMatch(v ?? string.Empty, @"\d+t\d", RegexOptions.CultureInvariant);
#endif
        }

        /// <summary>Reads <c>Application.tuanjieVersion</c> when the property exists.</summary>
        public static bool TryGetTuanjieVersionString(out string version)
        {
            version = null;
            PropertyInfo prop = typeof(Application).GetProperty(
                "tuanjieVersion", BindingFlags.Public | BindingFlags.Static);
            if (prop == null || prop.PropertyType != typeof(string))
            {
                return false;
            }

            try
            {
                version = prop.GetValue(null, null) as string;
                return !string.IsNullOrWhiteSpace(version);
            }
            catch
            {
                return false;
            }
        }

        /// <summary>
        /// <c>YYYY * 10000 + minor * 100 + patch</c> — no leading zeros (C octal hazard).
        /// </summary>
        public static int EncodeUnityVersion(UnityVersion uv)
        {
            return uv.major * 10000 + uv.minor1 * 100 + uv.minor2;
        }

        public static int EncodeUnityVersion(string unityVersionStr)
        {
            return EncodeUnityVersion(new UnityVersion(unityVersionStr));
        }

        /// <summary>Same triplet encoding for Tuanjie product versions (e.g. 1.9.3 → 10903).</summary>
        public static int EncodeSemVerTriplet(string versionStr)
        {
            if (string.IsNullOrWhiteSpace(versionStr))
            {
                return 0;
            }

            Match m = s_semVer.Match(versionStr.Trim());
            if (!m.Success)
            {
                return 0;
            }

            int major = int.Parse(m.Groups[1].Value);
            int minor = int.Parse(m.Groups[2].Value);
            int patch = int.Parse(m.Groups[3].Value);
            return major * 10000 + minor * 100 + patch;
        }

        /// <summary>Lua API family code: 5.3 → 503; LuaJIT → 501.</summary>
        public static int EncodeLuaApiFamily(LuaVersionInfo luaInfo)
        {
            if (luaInfo == null)
            {
                throw new ArgumentNullException(nameof(luaInfo));
            }

            if (luaInfo.IsLuaJit)
            {
                return 501;
            }

            Match m = Regex.Match(luaInfo.Id ?? string.Empty, @"^lua-(\d+)\.(\d+)\.", RegexOptions.CultureInvariant);
            if (!m.Success)
            {
                throw new InvalidOperationException($"Cannot encode Lua API family from id '{luaInfo.Id}'.");
            }

            int major = int.Parse(m.Groups[1].Value);
            int minor = int.Parse(m.Groups[2].Value);
            return major * 100 + minor;
        }

        public static string BuildConfId(string luaVersionId, UnityVersion unityLine, string tuanjieVersionLabel)
        {
            string tj = string.IsNullOrEmpty(tuanjieVersionLabel) ? "0" : tuanjieVersionLabel;
            return $"{luaVersionId}|unity-{unityLine}|tuanjie-{tj}";
        }

        /// <summary>
        /// Resolves Tuanjie product version string for conf id / numeric encode.
        /// Unity → label "0" and numeric 0.
        /// </summary>
        public static void ResolveTuanjieFields(
            string unityVersionStr,
            out int tuanjieEngineFlag,
            out int tuanjieVersionCode,
            out string tuanjieLabel)
        {
            if (!IsTuanjieEngine(unityVersionStr))
            {
                tuanjieEngineFlag = 0;
                tuanjieVersionCode = 0;
                tuanjieLabel = "0";
                return;
            }

            tuanjieEngineFlag = 1;
            if (TryGetTuanjieVersionString(out string raw) && !string.IsNullOrWhiteSpace(raw))
            {
                tuanjieLabel = raw.Trim();
                tuanjieVersionCode = EncodeSemVerTriplet(tuanjieLabel);
                return;
            }

            // Fallback: still mark as Tuanjie even if product version property is unavailable.
            tuanjieLabel = "unknown";
            tuanjieVersionCode = 0;
        }
    }
}
