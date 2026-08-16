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
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEditor;
using UnityEditor.Build;
using ZLua.CppCodeGen;
using ZLua.Meta;

namespace ZLua.Commands
{
    public static class PrebuildCommand
    {
        public static void GenerateAll()
        {
            var installer = new LocalInstaller();
            if (!installer.HasInstalledToLocal())
            {
                throw new BuildFailedException($"You have not initialized ZLua, please install it via menu 'ZLua/Installer'");
            }

            // Spec §12: refresh conf from Settings / Editor version; never keep a stale file.
            ZLua.Utils.ZLuaConfWriter.WriteOrRefreshLocalFromSettings();

            BuildTarget target = EditorUserBuildSettings.activeBuildTarget;

            StrippedAotAssemblyGenrator.Generate(target);

            string strippedAotAssemblyDir = CommonDirs.GetManagedStrippedDuplicatePath(target);
            var assemblyNames = Directory.GetFiles(strippedAotAssemblyDir, "*.dll", SearchOption.TopDirectoryOnly)
                .Select(Path.GetFileNameWithoutExtension)
                .ToList();
            // sort assembly names to ensure deterministic output
            assemblyNames.Sort((a, b) => a.CompareTo(b));

            var optons = new CodeGenerator.Options()
            {
                AssemblyCache = new AssemblyCache(new PathAssemblyResolver(strippedAotAssemblyDir)),
                AssemblyNames = assemblyNames,
                Development = EditorUserBuildSettings.development,
                OutputDir = CommonDirs.GeneratedZLuaPath,
            };

            var generator = new CodeGenerator(optons);
            generator.Generate();
            UnityEngine.Debug.Log("[PrebuildCommand] GenerateAll finished");
        }
    }
}
