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

using System;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEngine;

namespace ZLua.Utils
{
    /// <summary>
    /// Makes sure ZLua's stripped-AOT dll directory is populated before codegen reads it.
    /// <para>
    /// ZLua normally fills <c>Library/ZLua/ManagedStripped/{target}</c> from two places:
    /// a MonoHook on <c>AssemblyStripper.StripAssembliesTo</c> (Unity 2021.1–2022) and an
    /// <c>IPostprocessBuildWithReport</c> that reads Unity's
    /// <c>Library/Bee/artifacts/.../ManagedStripped</c>. With HybridCLR present the hook
    /// stands down (both packages patch the same native method), so on those Unity
    /// versions the directory can come up empty.
    /// </para>
    /// <para>
    /// HybridCLR collects the same dlls into its own directory, so mirror from there.
    /// This is a copy of the same upstream artifacts, not a second strip pass.
    /// </para>
    /// </summary>
    public static class StrippedAotAssemblyDir
    {
        /// <summary>
        /// Ensures ZLua's stripped-AOT directory for <paramref name="target"/> contains
        /// dlls, mirroring HybridCLR's output when ZLua's own copy is missing.
        /// </summary>
        /// <returns>True when the directory holds at least one dll afterwards.</returns>
        public static bool EnsurePopulated(BuildTarget target)
        {
            string zluaDir = CommonPaths.GetManagedStrippedDuplicatePath(target);
            if (HasDlls(zluaDir))
            {
                return true;
            }

            // Keyed on package presence, not on HybridCLRIntegration.Enabled: the hook
            // stands down whenever HybridCLR ships an equivalent one, which is also
            // keyed on presence. Gating this on the toggle would leave the directory
            // empty when HybridCLR is installed but ZLua's integration is off.
            if (!HybridCLRIntegration.IsPackagePresent)
            {
                return false;
            }

            string hybridclrDir = HybridCLRIntegration.GetAssembliesPostIl2CppStripDir(target);
            if (string.IsNullOrEmpty(hybridclrDir) || !HasDlls(hybridclrDir))
            {
                return false;
            }

            Directory.CreateDirectory(zluaDir);
            int copied = 0;
            foreach (string src in Directory.GetFiles(hybridclrDir, "*.dll", SearchOption.TopDirectoryOnly))
            {
                string dst = Path.Combine(zluaDir, Path.GetFileName(src));
                try
                {
                    File.Copy(src, dst, overwrite: true);
                    copied++;
                }
                catch (Exception ex)
                {
                    Debug.LogWarning($"[ZLua] Could not mirror stripped AOT dll '{src}': {ex.Message}");
                }
            }

            Debug.Log(
                $"[ZLua] Mirrored {copied} stripped AOT dll(s) from HybridCLR "
                + $"('{hybridclrDir}' -> '{zluaDir}').");
            return copied > 0;
        }

        private static bool HasDlls(string dir)
        {
            return !string.IsNullOrEmpty(dir)
                   && Directory.Exists(dir)
                   && Directory.EnumerateFiles(dir, "*.dll", SearchOption.TopDirectoryOnly).Any();
        }
    }
}
