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
using System.Text;
using UnityEngine;

namespace ZLua.Utils
{
    /// <summary>
    /// Keeps a pristine snapshot of HybridCLR's <c>libil2cpp</c> so ZLua's overlay
    /// install is idempotent.
    /// <para>
    /// ZLua patches HybridCLR's tree in place (it is the only tree — see
    /// <see cref="HybridCLRIntegration"/>). Without a baseline, a second Install
    /// would try to patch an already-patched tree: GNU patch would skip the hunks
    /// as reversed and the Lua sources would be re-copied over themselves. So the
    /// first Install snapshots the clean tree, and every later Install restores
    /// from it to get a known-clean starting point.
    /// </para>
    /// <para>
    /// The snapshot is keyed on HybridCLR's installed package version. Re-running
    /// <c>HybridCLR/Installer</c> rewrites libil2cpp from scratch (dropping ZLua's
    /// overlay); when that changes the version, the stale baseline is discarded and
    /// re-taken.
    /// </para>
    /// </summary>
    public static class HybridCLRBaseline
    {
        /// <summary>Marker proving a tree carries ZLua's overlay.</summary>
        private const string ZLuaMarker = "LuaAppDomain::Initialize";

        private static string RuntimeCppPath(string libil2cppDir) =>
            Path.Combine(libil2cppDir, "vm", "Runtime.cpp");

        /// <summary>True when this libil2cpp tree already has the ZLua overlay applied.</summary>
        public static bool LooksOverlaid(string libil2cppDir)
        {
            string runtimeCpp = RuntimeCppPath(libil2cppDir);
            if (!File.Exists(runtimeCpp))
            {
                return false;
            }

            return File.ReadAllText(runtimeCpp, Encoding.UTF8).Contains(ZLuaMarker);
        }

        private static string ReadBaselineStamp()
        {
            string path = CommonDirs.HybridCLRBaselineStampPath;
            if (!File.Exists(path))
            {
                return null;
            }

            try
            {
                return File.ReadAllText(path, Encoding.UTF8).Trim();
            }
            catch (Exception)
            {
                return null;
            }
        }

        private static bool BaselineExists()
        {
            string baseline = CommonDirs.HybridCLRBaselineLibil2cppDir;
            return Directory.Exists(baseline)
                   && File.Exists(RuntimeCppPath(baseline))
                   && Directory.Exists(Path.Combine(baseline, "hybridclr"));
        }

        /// <summary>
        /// Leaves HybridCLR's <c>libil2cpp</c> in a pristine, un-overlaid state and
        /// returns its path. Takes the snapshot on first use, refreshes it when
        /// HybridCLR's version changed, and otherwise restores from it.
        /// </summary>
        public static string EnsurePristineTree()
        {
            string libil2cpp = HybridCLRIntegration.LocalLibil2cppDir;
            if (string.IsNullOrEmpty(libil2cpp) || !Directory.Exists(libil2cpp))
            {
                throw new InvalidOperationException(
                    $"[ZLua] HybridCLR libil2cpp not found at '{libil2cpp}'. "
                    + "Run 'HybridCLR/Installer' before 'ZLua/Install...'.");
            }

            string currentVersion = HybridCLRIntegration.ReadInstalledVersion() ?? "unknown";
            string baselineVersion = ReadBaselineStamp();
            bool haveBaseline = BaselineExists();
            bool versionMatches = haveBaseline
                                  && string.Equals(baselineVersion, currentVersion, StringComparison.Ordinal);

            if (haveBaseline && !versionMatches)
            {
                Debug.Log(
                    $"[ZLua] HybridCLR baseline is stale (baseline='{baselineVersion}', "
                    + $"installed='{currentVersion}'); discarding and re-snapshotting.");
                DirectoryUtil.RemoveDir(CommonDirs.HybridCLRBaselineDir, true);
                haveBaseline = false;
            }

            if (haveBaseline)
            {
                // The live tree may or may not be overlaid; restoring is correct either
                // way and is the only way to drop a previous overlay.
                Debug.Log($"[ZLua] Restoring HybridCLR libil2cpp from baseline ({currentVersion}).");
                DirectoryUtil.CopyDir(CommonDirs.HybridCLRBaselineLibil2cppDir, libil2cpp, true);
                return libil2cpp;
            }

            // No usable baseline: the live tree must be clean, or we would snapshot an
            // overlaid tree and permanently lose the pristine state.
            if (LooksOverlaid(libil2cpp))
            {
                throw new InvalidOperationException(
                    "[ZLua] HybridCLR's libil2cpp already contains a ZLua overlay, but ZLua has no "
                    + "pristine baseline to restore from (it was deleted, or the overlay came from "
                    + "another checkout).\n"
                    + "Fix: re-run 'HybridCLR/Installer' to restore a clean libil2cpp, then re-run "
                    + "'ZLua/Install...'.");
            }

            Debug.Log(
                $"[ZLua] Snapshotting pristine HybridCLR libil2cpp ({currentVersion}) to "
                + CommonDirs.HybridCLRBaselineDir);
            Directory.CreateDirectory(CommonDirs.HybridCLRBaselineDir);
            DirectoryUtil.CopyDir(libil2cpp, CommonDirs.HybridCLRBaselineLibil2cppDir, true);
            File.WriteAllText(
                CommonDirs.HybridCLRBaselineStampPath,
                currentVersion,
                new UTF8Encoding(encoderShouldEmitUTF8Identifier: false));

            return libil2cpp;
        }

        /// <summary>
        /// Drops the snapshot. Used when leaving HybridCLR mode so a later re-enable
        /// snapshots afresh rather than restoring a tree from an older HybridCLR.
        /// </summary>
        public static void Clear()
        {
            if (Directory.Exists(CommonDirs.HybridCLRBaselineDir))
            {
                DirectoryUtil.RemoveDir(CommonDirs.HybridCLRBaselineDir, true);
                Debug.Log("[ZLua] Cleared HybridCLR baseline snapshot.");
            }
        }
    }
}
