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
using System.Collections;
using System.Collections.Generic;
using System.Reflection;
using UnityEngine;
using UnityEditor;
using System.Runtime.CompilerServices;
using MonoHook;
using System.IO;

namespace ZLua.MonoHook
{
#if UNITY_2021_1_OR_NEWER && !UNITY_2023_1_OR_NEWER
    [InitializeOnLoad]
    public class CopyStrippedAOTAssembliesHook
    {
        private static MethodHook _hook;

        static CopyStrippedAOTAssembliesHook()
        {
            // HybridCLR hooks this same native method. MonoHook patches machine code
            // keyed by raw function pointer with no cross-package registry, so two
            // installs stack and only the later override body runs. Stand down here;
            // unlike GetIl2CppFolderHook the two bodies copy to different directories,
            // so ZLua recovers its copy by mirroring HybridCLR's output — see
            // StrippedAotAssemblyDir.EnsurePopulated.
            if (ZLua.Utils.HybridCLRIntegration.HasEquivalentHook(nameof(CopyStrippedAOTAssembliesHook)))
            {
                Debug.Log(
                    "[ZLua] CopyStrippedAOTAssembliesHook: HybridCLR provides an equivalent hook; "
                    + "skipping ZLua's to avoid double-patching AssemblyStripper.StripAssembliesTo. "
                    + "Stripped AOT dlls will be mirrored from HybridCLR's output instead.");
                return;
            }

            if (_hook == null)
            {
                Type type = typeof(UnityEditor.EditorApplication).Assembly.GetType("UnityEditorInternal.AssemblyStripper");
                MethodInfo miTarget = type.GetMethod("StripAssembliesTo", BindingFlags.Static | BindingFlags.NonPublic | BindingFlags.Public);

                MethodInfo miReplacement = new StripAssembliesDel(OverrideStripAssembliesTo).Method;
                MethodInfo miProxy = new StripAssembliesDel(StripAssembliesToProxy).Method;

                _hook = new MethodHook(miTarget, miReplacement, miProxy);
                _hook.Install();
            }
        }

        private delegate bool StripAssembliesDel(string outputFolder, out string output, out string error, IEnumerable<string> linkXmlFiles, object runInformation);

        private static bool OverrideStripAssembliesTo(string outputFolder, out string output, out string error, IEnumerable<string> linkXmlFiles, object runInformation)
        {
            bool result = StripAssembliesToProxy(outputFolder, out output, out error, linkXmlFiles, runInformation);
            if (!result)
            {
                return false;
            }
            UnityEngine.Debug.Log($"== StripAssembliesTo outputDir:{outputFolder}");
            string outputStrippedDir = CommonDirs.GetManagedStrippedDuplicatePath(EditorUserBuildSettings.activeBuildTarget);
            Directory.CreateDirectory(outputStrippedDir);
            foreach (var aotDll in Directory.GetFiles(outputFolder, "*.dll"))
            {
                string dstFile = $"{outputStrippedDir}/{Path.GetFileName(aotDll)}";
                Debug.Log($"[CopyStrippedAOTAssembliesHook] copy aot dll {aotDll} -> {dstFile}");
                File.Copy(aotDll, dstFile, true);
            }
            return result;
        }

        [MethodImpl(MethodImplOptions.NoOptimization)]
        private static bool StripAssembliesToProxy(string outputFolder, out string output, out string error, IEnumerable<string> linkXmlFiles, object runInformation)
        {
            Debug.LogError("== StripAssembliesToProxy ==");
            output = "";
            error = "";
            return true;
        }
    }
#endif
}
