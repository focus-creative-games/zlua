// Copyright 2026 Code Philosophy

using System;
using System.Diagnostics;
using System.IO;
using System.Text;
using Debug = UnityEngine.Debug;

namespace ZLua.Utils
{
    /// <summary>
    /// Applies unified / git-format patches via GNU <c>patch</c> (bundled with Git for Windows when available).
    /// </summary>
    public static class PatchApplier
    {
        public static void ApplyAllInDirectory(string patchDir, string workingDirectory, int stripComponents = 1)
        {
            if (!Directory.Exists(patchDir))
            {
                throw new InvalidOperationException($"Patch directory not found: {patchDir}");
            }

            string[] patches = Directory.GetFiles(patchDir, "*.patch", SearchOption.TopDirectoryOnly);
            Array.Sort(patches, StringComparer.OrdinalIgnoreCase);
            if (patches.Length == 0)
            {
                throw new InvalidOperationException($"No .patch files in: {patchDir}");
            }

            foreach (string patch in patches)
            {
                Apply(patch, workingDirectory, stripComponents);
            }
        }

        public static void Apply(string patchFile, string workingDirectory, int stripComponents = 1)
        {
            if (!File.Exists(patchFile))
            {
                throw new InvalidOperationException($"Patch file not found: {patchFile}");
            }

            if (!Directory.Exists(workingDirectory))
            {
                throw new InvalidOperationException($"Patch working directory not found: {workingDirectory}");
            }

            // macOS /usr/bin/patch rejects CRLF unified diffs against LF sources; normalize to LF.
            string patchInput = EnsureLfPatchFile(patchFile);
            try
            {
                string patchExe = ResolvePatchExecutable();
                string args = $"-p{stripComponents} --forward --batch -i \"{patchInput}\"";
                RunPatch(patchExe, args, workingDirectory, dryRun: true);
                RunPatch(patchExe, args, workingDirectory, dryRun: false);
                Debug.Log($"[ZLua] Applied patch: {patchFile}");
            }
            finally
            {
                if (!string.Equals(patchInput, patchFile, StringComparison.Ordinal)
                    && File.Exists(patchInput))
                {
                    try { File.Delete(patchInput); } catch { /* best-effort */ }
                }
            }
        }

        /// <summary>Returns <paramref name="patchFile"/> if already LF; otherwise a temp LF copy.</summary>
        private static string EnsureLfPatchFile(string patchFile)
        {
            byte[] bytes = File.ReadAllBytes(patchFile);
            bool hasCr = false;
            for (int i = 0; i < bytes.Length; i++)
            {
                if (bytes[i] == (byte)'\r')
                {
                    hasCr = true;
                    break;
                }
            }

            if (!hasCr)
            {
                return patchFile;
            }

            string text = Encoding.UTF8.GetString(bytes).Replace("\r\n", "\n").Replace("\r", "\n");
            string temp = Path.Combine(Path.GetTempPath(), "zlua-" + Path.GetFileName(patchFile) + ".lf.patch");
            File.WriteAllText(temp, text, new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));
            return temp;
        }

        private static void RunPatch(string patchExe, string args, string workingDirectory, bool dryRun)
        {
            string fullArgs = dryRun ? $"{args} --dry-run" : args;
            var psi = new ProcessStartInfo
            {
                FileName = patchExe,
                Arguments = fullArgs,
                WorkingDirectory = workingDirectory,
                UseShellExecute = false,
                RedirectStandardInput = true,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                CreateNoWindow = true,
            };

            using (var proc = Process.Start(psi))
            {
                proc.StandardInput.Close();
                string stdout = proc.StandardOutput.ReadToEnd();
                string stderr = proc.StandardError.ReadToEnd();
                proc.WaitForExit();
                if (proc.ExitCode != 0)
                {
                    var sb = new StringBuilder();
                    sb.AppendLine(dryRun
                        ? $"[ZLua] Patch dry-run failed (exit {proc.ExitCode})."
                        : $"[ZLua] Patch apply failed (exit {proc.ExitCode}).");
                    sb.AppendLine($"  exe: {patchExe}");
                    sb.AppendLine($"  cwd: {workingDirectory}");
                    sb.AppendLine($"  args: {fullArgs}");
                    if (!string.IsNullOrWhiteSpace(stdout))
                    {
                        sb.AppendLine(stdout.TrimEnd());
                    }

                    if (!string.IsNullOrWhiteSpace(stderr))
                    {
                        sb.AppendLine(stderr.TrimEnd());
                    }

                    throw new InvalidOperationException(sb.ToString());
                }
            }
        }

        private const string PatchExecutableEnvVar = "ZLUA_PATCH_PATH";

        private static string ResolvePatchExecutable()
        {
            string configured = Environment.GetEnvironmentVariable(PatchExecutableEnvVar);
            if (!string.IsNullOrWhiteSpace(configured))
            {
                configured = configured.Trim().Trim('"');
                if (!File.Exists(configured))
                {
                    throw new InvalidOperationException(
                        $"[ZLua] {PatchExecutableEnvVar} is set but patch executable was not found: {configured}");
                }

                return configured;
            }

            // GNU patch first. /usr/bin/patch on macOS is BSD patch and does not accept the flags we pass.
            string[] candidates =
            {
                @"C:\Program Files\Git\usr\bin\patch.exe",
                @"C:\Program Files (x86)\Git\usr\bin\patch.exe",
                "/opt/homebrew/bin/gpatch",
                "/usr/local/bin/gpatch",
                "/opt/local/bin/gpatch",
                "/usr/bin/gpatch",
            };

            foreach (string c in candidates)
            {
                if (File.Exists(c))
                {
                    return c;
                }
            }

            string[] pathNames = IsWindows()
                ? new[] { "patch.exe", "patch" }
                : new[] { "gpatch", "patch" };
            string fromPath = FindOnPath(pathNames);
            if (fromPath != null)
            {
                return fromPath;
            }

            throw new InvalidOperationException(
                "[ZLua] GNU patch not found. Set ZLUA_PATCH_PATH to the patch executable, install Git for Windows (includes usr\\bin\\patch.exe), or put patch/gpatch on PATH.");
        }

        private static bool IsWindows()
        {
            PlatformID platform = Environment.OSVersion.Platform;
            return platform == PlatformID.Win32NT
                || platform == PlatformID.Win32Windows
                || platform == PlatformID.Win32S;
        }

        /// <summary>
        /// Editor <c>Process.Start</c> with <c>UseShellExecute = false</c> does not search PATH on Windows.
        /// Resolve to an absolute path here so a non-default install still works.
        /// </summary>
        private static string FindOnPath(string[] fileNames)
        {
            string pathEnv = Environment.GetEnvironmentVariable("PATH");
            if (string.IsNullOrEmpty(pathEnv))
            {
                return null;
            }

            string[] dirs = pathEnv.Split(Path.PathSeparator);
            for (int i = 0; i < dirs.Length; i++)
            {
                string dir = dirs[i].Trim().Trim('"');
                if (dir.Length == 0)
                {
                    continue;
                }

                for (int n = 0; n < fileNames.Length; n++)
                {
                    string full = Path.Combine(dir, fileNames[n]);
                    if (File.Exists(full))
                    {
                        return full;
                    }
                }
            }

            return null;
        }
    }
}
