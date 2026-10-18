// Copyright 2026 Code Philosophy

using System;
using System.Diagnostics;
using System.IO;
using System.Text;
using Debug = UnityEngine.Debug;

namespace ZLua.Utils
{
    /// <summary>
    /// Builds Luau VM + compiler as Win64 /MD static libraries (C++17).
    /// Il2Cpp stays on C++11 and only links these libs.
    /// </summary>
    public static class LuauWin64Build
    {
        private static readonly string[] s_libs =
        {
            "Luau.Compiler.lib",
            "Luau.Ast.lib",
            "Luau.Bytecode.lib",
            "Luau.Inliner.lib",
            "Luau.VM.lib",
            "Luau.Common.lib",
        };

        public static string BuildDir(LuaVersionInfo info)
        {
            return Path.Combine(CommonPaths.InstallRootDir, "LuauBuild", info.Id);
        }

        public static string LibDir(LuaVersionInfo info)
        {
            // Win64 Development player links libil2cpp with /MTd. Match that CRT.
            return Path.Combine(BuildDir(info), "Debug");
        }

        public static bool LibsReady(LuaVersionInfo info)
        {
            string dir = LibDir(info);
            foreach (string lib in s_libs)
            {
                if (!File.Exists(Path.Combine(dir, lib)))
                {
                    return false;
                }
            }

            return true;
        }

        /// <summary>Configure and build if the Release libs are missing. Returns the directory that contains the .lib files.</summary>
        public static string Ensure(LuaVersionInfo info)
        {
            if (info == null || !info.IsLuau)
            {
                throw new ArgumentException("[ZLua] LuauWin64Build.Ensure requires a Luau version.");
            }

            if (LibsReady(info))
            {
                Debug.Log($"[ZLua] Using cached Luau Win64 libs: {LibDir(info)}");
                return LibDir(info);
            }

            if (!Directory.Exists(info.SourceDir) || !File.Exists(Path.Combine(info.SourceDir, "CMakeLists.txt")))
            {
                throw new InvalidOperationException(
                    $"[ZLua] Luau sources missing at {info.SourceDir}. Install should download them first.");
            }

            string buildDir = BuildDir(info);
            Directory.CreateDirectory(buildDir);

            RunCmake(
                "cmake",
                "-S \"" + info.SourceDir + "\" -B \"" + buildDir + "\" -G \"Visual Studio 17 2022\" -A x64 "
                + "-DLUAU_BUILD_CLI=OFF -DLUAU_BUILD_TESTS=OFF -DLUAU_BUILD_WEB=OFF -DLUAU_WERROR=OFF -DLUAU_STATIC_CRT=ON "
                + "-DCMAKE_CXX_FLAGS=\"/EHsc /D_ITERATOR_DEBUG_LEVEL=0\"");

            var targets = new StringBuilder();
            foreach (string lib in s_libs)
            {
                targets.Append(' ');
                targets.Append(Path.GetFileNameWithoutExtension(lib));
            }

            RunCmake(
                "cmake",
                "--build \"" + buildDir + "\" --config Debug --target" + targets);

            if (!LibsReady(info))
            {
                throw new InvalidOperationException(
                    "[ZLua] Luau Win64 build finished but static libraries are missing under " + LibDir(info));
            }

            Debug.Log($"[ZLua] Built Luau Win64 libs: {LibDir(info)}");
            return LibDir(info);
        }

        private static void RunCmake(string fileName, string arguments)
        {
            var psi = new ProcessStartInfo
            {
                FileName = fileName,
                Arguments = arguments,
                UseShellExecute = false,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                CreateNoWindow = true,
            };

            using (Process proc = Process.Start(psi))
            {
                string stdout = proc.StandardOutput.ReadToEnd();
                string stderr = proc.StandardError.ReadToEnd();
                proc.WaitForExit();
                if (proc.ExitCode != 0)
                {
                    throw new InvalidOperationException(
                        $"[ZLua] {fileName} failed (exit {proc.ExitCode}).\n{stdout}\n{stderr}");
                }
            }
        }
    }
}
