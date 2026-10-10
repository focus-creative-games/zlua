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
using System.Reflection;
using UnityEngine;

namespace ZLua.Utils
{
    /// <summary>
    /// Detects the HybridCLR package and reads its install layout.
    /// <para>
    /// Everything here goes through reflection on purpose: ZLua must compile and
    /// behave identically whether or not <c>com.code-philosophy.hybridclr</c> is in
    /// the project, so <c>ZLua.Editor.asmdef</c> must not reference it. Paths are
    /// read from HybridCLR's own <c>SettingsUtil</c> rather than reconstructed here,
    /// so a layout change on their side does not silently desync.
    /// </para>
    /// <para>
    /// When enabled, HybridCLR's local il2cpp tree is the single tree: ZLua overlays
    /// itself onto it instead of building its own. HybridCLR hardcodes its own
    /// directory for codegen output, the iOS source copy and the MethodBridge.inc
    /// check, so with its package left unmodified there is no other option.
    /// </para>
    /// </summary>
    public static class HybridCLRIntegration
    {
        public const string PackageName = "com.code-philosophy.hybridclr";

        /// <summary>Unity LTS series with both a HybridCLR release and a ZLua patch.</summary>
        private static readonly string[] s_supportedSeries = { "2022.3", "6000.0", "6000.3" };

        private const string SettingsUtilTypeName = "HybridCLR.SettingsUtil";
        private const string HybridCLRSettingsTypeName = "HybridCLR.Settings.HybridCLRSettings";

        private static Type s_settingsUtilType;
        private static bool s_settingsUtilSearched;

        /// <summary>
        /// <c>HybridCLR.SettingsUtil</c>, or null when the package is absent.
        /// Resolved once per domain; a package add/remove triggers a domain reload.
        /// </summary>
        private static Type SettingsUtilType
        {
            get
            {
                if (s_settingsUtilSearched)
                {
                    return s_settingsUtilType;
                }

                s_settingsUtilSearched = true;
                s_settingsUtilType = FindType(SettingsUtilTypeName);
                return s_settingsUtilType;
            }
        }

        private static Type FindType(string fullName)
        {
            foreach (Assembly asm in AppDomain.CurrentDomain.GetAssemblies())
            {
                Type type;
                try
                {
                    type = asm.GetType(fullName, throwOnError: false);
                }
                catch (Exception)
                {
                    // Dynamic / unloadable assembly; keep scanning.
                    continue;
                }

                if (type != null)
                {
                    return type;
                }
            }

            return null;
        }

        /// <summary>True when the HybridCLR Editor assembly is loaded in this project.</summary>
        public static bool IsPackagePresent => SettingsUtilType != null;

        /// <summary>
        /// True when the user opted in <em>and</em> the package is actually present.
        /// This is the single switch every ZLua code path should branch on; it never
        /// turns itself on, per the opt-in-only design.
        /// </summary>
        public static bool Enabled => IsPackagePresent;

        /// <summary><c>Library/HybridCLR/LocalIl2CppData-{platform}/il2cpp</c>.</summary>
        public static string LocalIl2CppDir => GetStringProperty("LocalIl2CppDir");

        /// <summary><c>{LocalIl2CppDir}/libil2cpp</c>.</summary>
        public static string LocalLibil2cppDir =>
            CombineUnix(LocalIl2CppDir, "libil2cpp");

        /// <summary>HybridCLR's own generated cpp dir, inside its libil2cpp tree.</summary>
        public static string GeneratedCppDir => GetStringProperty("GeneratedCppDir");

        /// <summary>Stripped AOT dlls HybridCLR collects during a build.</summary>
        public static string GetAssembliesPostIl2CppStripDir(UnityEditor.BuildTarget target)
        {
            Type type = SettingsUtilType;
            MethodInfo method = type?.GetMethod(
                "GetAssembliesPostIl2CppStripDir",
                BindingFlags.Public | BindingFlags.Static);
            if (method == null)
            {
                return null;
            }

            try
            {
                return method.Invoke(null, new object[] { target }) as string;
            }
            catch (Exception ex)
            {
                Debug.LogWarning($"[ZLua] HybridCLR GetAssembliesPostIl2CppStripDir failed: {ex.Message}");
                return null;
            }
        }

        private static string GetStringProperty(string name)
        {
            Type type = SettingsUtilType;
            if (type == null)
            {
                return null;
            }

            PropertyInfo prop = type.GetProperty(name, BindingFlags.Public | BindingFlags.Static);
            if (prop == null)
            {
                Debug.LogWarning(
                    $"[ZLua] HybridCLR.SettingsUtil.{name} not found; HybridCLR package layout changed?");
                return null;
            }

            try
            {
                return prop.GetValue(null) as string;
            }
            catch (Exception ex)
            {
                Debug.LogWarning($"[ZLua] Reading HybridCLR.SettingsUtil.{name} failed: {ex.Message}");
                return null;
            }
        }

        /// <summary>
        /// HybridCLR's own enable flag. ZLua's overlay is pointless when HybridCLR is
        /// installed but disabled: its CheckSettings clears UNITY_IL2CPP_PATH and the
        /// build would not use the shared tree at all.
        /// </summary>
        public static bool IsHybridCLREnabledInSettings()
        {
            Type settingsType = FindType(HybridCLRSettingsTypeName);
            PropertyInfo instanceProp = settingsType?.GetProperty(
                "instance", BindingFlags.Public | BindingFlags.Static | BindingFlags.FlattenHierarchy);
            if (instanceProp == null)
            {
                return false;
            }

            try
            {
                object instance = instanceProp.GetValue(null);
                FieldInfo enableField = instance?.GetType().GetField(
                    "enable", BindingFlags.Public | BindingFlags.Instance);
                return enableField != null && enableField.GetValue(instance) is bool b && b;
            }
            catch (Exception ex)
            {
                Debug.LogWarning($"[ZLua] Reading HybridCLRSettings.enable failed: {ex.Message}");
                return false;
            }
        }

        /// <summary>True when <c>HybridCLR/Installer</c> has produced a local tree.</summary>
        public static bool HasInstalled()
        {
            string libil2cpp = LocalLibil2cppDir;
            return !string.IsNullOrEmpty(libil2cpp)
                   && Directory.Exists(CombineUnix(libil2cpp, "hybridclr"));
        }

        /// <summary>
        /// HybridCLR package version recorded in its installed tree. ZLua snapshots a
        /// pristine baseline keyed on this, so a HybridCLR reinstall or upgrade
        /// invalidates the baseline instead of stacking a second overlay.
        /// </summary>
        public static string ReadInstalledVersion()
        {
            string libil2cpp = LocalLibil2cppDir;
            if (string.IsNullOrEmpty(libil2cpp))
            {
                return null;
            }

            string file = CombineUnix(libil2cpp, "hybridclr/generated/libil2cpp-version.txt");
            if (!File.Exists(file))
            {
                return null;
            }

            try
            {
                return File.ReadAllText(file, System.Text.Encoding.UTF8).Trim();
            }
            catch (Exception)
            {
                return null;
            }
        }

        /// <summary>Series key for the running Editor, or null when unparseable.</summary>
        public static string GetSeriesKey(string unityVersion)
        {
            if (!UnityIl2CppPatchUtil.TryParseUnityTriplet(unityVersion, out int major, out int minor, out _))
            {
                return null;
            }

            return $"{major}.{minor}";
        }

        public static bool IsSupportedSeries(string unityVersion)
        {
            string series = GetSeriesKey(unityVersion);
            return series != null && s_supportedSeries.Contains(series, StringComparer.Ordinal);
        }

        public static string SupportedSeriesDescription =>
            "2022.3.x (including Tuanjie), 6000.0.x, 6000.3.x";

        /// <summary>
        /// Validates every precondition for the overlay install and throws with
        /// actionable text when one fails. Called at the top of Install and from the
        /// build-time check so the failure is loud and early.
        /// </summary>
        public static void AssertReadyForInstall()
        {
            if (!IsPackagePresent)
            {
                throw new InvalidOperationException(
                    "[ZLua] 'Enable HybridCLR Support' is on but the HybridCLR package was not found. "
                    + $"Install {PackageName}, or turn the option off in ZLua/Settings.");
            }

            if (!IsSupportedSeries(Application.unityVersion))
            {
                throw new InvalidOperationException(
                    $"[ZLua] HybridCLR support is not available for Unity {Application.unityVersion}. "
                    + $"Supported: {SupportedSeriesDescription}. "
                    + "Turn off 'Enable HybridCLR Support' to use ZLua standalone on this Editor.");
            }

            if (!IsHybridCLREnabledInSettings())
            {
                throw new InvalidOperationException(
                    "[ZLua] HybridCLR is installed but disabled in its own settings. "
                    + "Enable it under HybridCLR/Settings (its build hook clears UNITY_IL2CPP_PATH "
                    + "while disabled, so the shared il2cpp tree would not be used).");
            }

            if (string.IsNullOrEmpty(LocalIl2CppDir))
            {
                throw new InvalidOperationException(
                    "[ZLua] Could not read HybridCLR.SettingsUtil.LocalIl2CppDir. "
                    + "The HybridCLR package layout may have changed; this integration expects "
                    + "a HybridCLR version exposing SettingsUtil.LocalIl2CppDir.");
            }

            if (!HasInstalled())
            {
                throw new InvalidOperationException(
                    "[ZLua] HybridCLR is not installed locally yet. Run 'HybridCLR/Installer' first, "
                    + "then 'ZLua/Install...'. ZLua overlays itself onto HybridCLR's il2cpp tree, "
                    + "so HybridCLR must be installed before ZLua.");
            }
        }

        /// <summary>
        /// True when HybridCLR ships an active MonoHook for the same Unity internal
        /// that ZLua hooks.
        /// <para>
        /// Both packages vendor the same MonoHook library and patch the target's native
        /// machine code, keyed by raw function pointer with no cross-package registry.
        /// Installing both means the second write lands on top of the first, so ZLua
        /// stands down whenever HybridCLR's equivalent is present. HybridCLR's hook
        /// classes only exist when their <c>#if</c> guards match — which are identical
        /// to ZLua's — so type presence is a reliable signal. It is also independent of
        /// <c>[InitializeOnLoad]</c> ordering: the type exists whether or not its static
        /// constructor has run yet.
        /// </para>
        /// <para>
        /// Deliberately keyed on package presence, not on <see cref="Enabled"/>: two
        /// hooks on one native method is never safe, regardless of ZLua's toggle.
        /// </para>
        /// </summary>
        public static bool HasEquivalentHook(string hookTypeShortName)
        {
            return FindType($"HybridCLR.BuildProcessors.Hooks.{hookTypeShortName}") != null;
        }

        /// <summary>Forward slashes, matching the style of HybridCLR's own paths.</summary>
        private static string CombineUnix(string left, string right)
        {
            if (string.IsNullOrEmpty(left))
            {
                return null;
            }

            return $"{left.TrimEnd('/', '\\')}/{right}";
        }
    }
}
