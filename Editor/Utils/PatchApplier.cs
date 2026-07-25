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

            string patchExe = ResolvePatchExecutable();
            string args = $"-p{stripComponents} --forward --batch -i \"{patchFile}\"";
            RunPatch(patchExe, args, workingDirectory, dryRun: true);
            RunPatch(patchExe, args, workingDirectory, dryRun: false);
            Debug.Log($"[ZLua] Applied patch: {patchFile}");
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

        private static string ResolvePatchExecutable()
        {
            string[] candidates =
            {
                @"C:\Program Files\Git\usr\bin\patch.exe",
                @"C:\Program Files (x86)\Git\usr\bin\patch.exe",
                "/usr/bin/patch",
                "/opt/homebrew/bin/gpatch",
                "patch",
            };

            foreach (string c in candidates)
            {
                if (c == "patch")
                {
                    return c;
                }

                if (File.Exists(c))
                {
                    return c;
                }
            }

            throw new InvalidOperationException(
                "[ZLua] GNU patch not found. Install Git for Windows (includes usr\\bin\\patch.exe) or a patch utility.");
        }
    }
}
