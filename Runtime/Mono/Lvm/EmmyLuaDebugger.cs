using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using UnityEngine;

namespace ZLua.Lvm
{
    /// <summary>
    /// Editor EmmyLua inject (spec build/04-EMMYLUA-DEBUGGER). Layout:
    /// Plugins/emmylua/{series}/win32-x64|darwin-arm64|darwin-x64|linux-x64/emmy_core.*
    /// Series folder: lua55 / … / luajit (JIT does not split 2.0 vs 2.1 for emmy_core).
    /// </summary>
    internal static class EmmyLuaDebugger
    {
        private const string PackageName = BuiltinScripts.PackageName;

        /// <summary>Compiled Lua series folder under Plugins/emmylua/.</summary>
        public static string SeriesFolderName
        {
            get
            {
#if ZLUA_LUAJIT_2_1 || ZLUA_LUAJIT_2_0 || ZLUA_USE_LUAJIT
                return "luajit";
#elif ZLUA_LUA_5_5
                return "lua55";
#elif ZLUA_LUA_5_4
                return "lua54";
#elif ZLUA_LUA_5_3
                return "lua53";
#elif ZLUA_LUA_5_2
                return "lua52";
#elif ZLUA_LUA_5_1
                return "lua51";
#else
                return "lua53";
#endif
            }
        }

        public static string ResolveSeriesRoot()
        {
            return Path.GetFullPath(Path.Combine(
                Application.dataPath, "..", "Packages", PackageName, "Plugins", "emmylua", SeriesFolderName));
        }

        public static bool TryResolveNativeDir(out string nativeDir, out string cpathExt, out string missingPath)
        {
            nativeDir = null;
            cpathExt = null;
            string seriesRoot = ResolveSeriesRoot();
            if (!Directory.Exists(seriesRoot))
            {
                missingPath = seriesRoot;
                return false;
            }

            if (!TryGetPlatformDirName(out string platformDir, out cpathExt))
            {
                missingPath = seriesRoot + " (unsupported Editor OS/arch for EmmyLua)";
                return false;
            }

            nativeDir = Path.Combine(seriesRoot, platformDir);
            if (!Directory.Exists(nativeDir))
            {
                missingPath = nativeDir;
                return false;
            }

            missingPath = null;
            return true;
        }

        private static bool TryGetPlatformDirName(out string platformDir, out string cpathExt)
        {
            platformDir = null;
            cpathExt = null;
#if UNITY_EDITOR_WIN
            platformDir = "win32-x64";
            cpathExt = "dll";
            return true;
#elif UNITY_EDITOR_OSX
            cpathExt = "dylib";
            platformDir = RuntimeInformation.ProcessArchitecture == Architecture.Arm64
                ? "darwin-arm64"
                : "darwin-x64";
            return true;
#elif UNITY_EDITOR_LINUX
            platformDir = "linux-x64";
            cpathExt = "so";
            return true;
#else
            return false;
#endif
        }

        public static string BuildInitChunk(string nativeDir, string cpathExt, int port, bool waitIde)
        {
            // Lua prefers forward slashes in package.cpath on all platforms.
            string dirForLua = nativeDir.Replace('\\', '/');
            if (!dirForLua.EndsWith("/", StringComparison.Ordinal))
            {
                dirForLua += "/";
            }

            var sb = new StringBuilder(512);
            sb.Append("do\n");
            sb.Append("  local dir = [[");
            sb.Append(dirForLua);
            sb.Append("]]\n");
            sb.Append("  local pattern = dir .. '?.' .. '");
            sb.Append(cpathExt);
            sb.Append("'\n");
            sb.Append("  if not package.cpath:find(pattern, 1, true) then\n");
            sb.Append("    package.cpath = package.cpath .. ';' .. pattern\n");
            sb.Append("  end\n");
            sb.Append("  local dbg = require('emmy_core')\n");
            sb.Append("  dbg.tcpListen('127.0.0.1', ");
            sb.Append(port);
            sb.Append(")\n");
            if (waitIde)
            {
                sb.Append("  dbg.waitIDE()\n");
            }

            sb.Append("end\n");
            return sb.ToString();
        }
    }
}
