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
