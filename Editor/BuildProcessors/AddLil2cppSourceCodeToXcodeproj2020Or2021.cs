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
using UnityEditor;
using UnityEditor.Callbacks;
using UnityEngine;
using ZLua.Utils;

#if (UNITY_2020 || UNITY_2021) && (UNITY_IOS || UNITY_TVOS)
using UnityEditor.iOS.Xcode;

namespace ZLua.BuildProcessors
{
    /// <summary>
    /// Unity 2020/2021 iOS ships a prebuilt <c>libil2cpp.a</c>. Replace it with Local Il2Cpp
    /// sources (lumps) so ZLua C++ and icalls are linked. Do not gate this file on
    /// <c>UNITY_IOS</c>: that define is often absent when the Editor assembly last compiled
    /// for a non-iOS active target, which would skip PostProcessBuild registration.
    /// </summary>
    public static class AddLil2cppSourceCodeToXcodeproj2020Or2021
    {
        [PostProcessBuild(50)]
        public static void OnPostProcessBuild(BuildTarget target, string pathToBuiltProject)
        {
            if (!Settings.EnableForCurrentBuildTarget)
            {
                return;
            }

            if (target != BuildTarget.iOS && target != BuildTarget.tvOS)
            {
                return;
            }

            /*
             *  1. 生成 lump，并且添加到工程
             *  2. 将 libil2cpp 目录复制到 Libraries/
             *  3. 将 external 复制到 Libraries/external；OSX baselib → IOS
             *  4. 移除 libil2cpp.a，改为编译 lump / extra .c
             */

            string pbxprojFile = GetXcodeProjectFile(pathToBuiltProject);
            string srcLibil2cppDir = CommonDirs.LocalLibil2cppPath;
            string dstLibil2cppDir = Path.Combine(pathToBuiltProject, "Libraries", "libil2cpp");
            string lumpDir = Path.Combine(pathToBuiltProject, "Libraries", "lumps");
            string srcExternalDir = Path.Combine(CommonDirs.LocalIl2CppPath, "external");
            string dstExternalDir = Path.Combine(pathToBuiltProject, "Libraries", "external");

            if (!Directory.Exists(srcLibil2cppDir))
            {
                throw new InvalidOperationException(
                    $"[ZLua] Local libil2cpp missing at '{srcLibil2cppDir}'. Run ZLua Install first.");
            }

            CopyLibil2cppToXcodeProj(srcLibil2cppDir, dstLibil2cppDir);
            CopyExternalToXcodeProj(srcExternalDir, dstExternalDir);
            var lumpFiles = CreateLumps(dstLibil2cppDir, lumpDir);
            var extraSources = GetExtraSourceFiles(dstExternalDir, dstLibil2cppDir);
            var cflags = new List<string>
            {
                "-DIL2CPP_MONO_DEBUGGER_DISABLED",
            };
            ModifyPBXProject(pathToBuiltProject, pbxprojFile, lumpFiles, extraSources, cflags);
            Debug.Log($"[AddLil2cppSourceCodeToXcodeproj] 2020/2021 iOS: lumps={lumpFiles.Count} extras={extraSources.Count}");
        }

        private static string GetXcodeProjectFile(string pathToBuiltProject)
        {
            return Path.Combine(pathToBuiltProject, "Unity-iPhone.xcodeproj", "project.pbxproj");
        }

        private static string GetRelativePathFromProj(string path)
        {
            return path.Substring(path.IndexOf("Libraries", StringComparison.Ordinal)).Replace('\\', '/');
        }

        private static void ModifyPBXProject(string pathToBuiltProject, string pbxprojFile, List<LumpFile> lumpFiles, List<string> extraFiles, List<string> cflags)
        {
            var proj = new PBXProject();
            proj.ReadFromFile(pbxprojFile);
            string targetGUID = proj.GetUnityFrameworkTargetGuid();
            var libil2cppGUID = proj.FindFileGuidByProjectPath("Libraries/libil2cpp.a");
            if (!string.IsNullOrEmpty(libil2cppGUID))
            {
                proj.RemoveFileFromBuild(targetGUID, libil2cppGUID);
                proj.RemoveFile(libil2cppGUID);
                File.Delete(Path.Combine(pathToBuiltProject, "Libraries", "libil2cpp.a"));
            }

            foreach (var lumpFile in lumpFiles)
            {
                string lumpFileName = Path.GetFileName(lumpFile.lumpFile);
                string projPathOfFile = $"Classes/Lumps/{lumpFileName}";
                string relativePathOfFile = GetRelativePathFromProj(lumpFile.lumpFile);
                string lumpGuid = proj.FindFileGuidByProjectPath(projPathOfFile);
                if (!string.IsNullOrEmpty(lumpGuid))
                {
                    proj.RemoveFileFromBuild(targetGUID, lumpGuid);
                    proj.RemoveFile(lumpGuid);
                }

                lumpGuid = proj.AddFile(relativePathOfFile, projPathOfFile, PBXSourceTree.Source);
                proj.AddFileToBuild(targetGUID, lumpGuid);
            }

            foreach (var extraFile in extraFiles)
            {
                string projPathOfFile = $"Classes/Extrals/{Path.GetFileName(extraFile)}";
                string extraFileGuid = proj.FindFileGuidByProjectPath(projPathOfFile);
                if (!string.IsNullOrEmpty(extraFileGuid))
                {
                    proj.RemoveFileFromBuild(targetGUID, extraFileGuid);
                    proj.RemoveFile(extraFileGuid);
                }

                var lumpGuid = proj.AddFile(GetRelativePathFromProj(extraFile), projPathOfFile, PBXSourceTree.Source);
                proj.AddFileToBuild(targetGUID, lumpGuid);
            }

            foreach (var configName in proj.BuildConfigNames())
            {
                string configGuid = proj.BuildConfigByName(targetGUID, configName);
                string headerSearchPaths = "HEADER_SEARCH_PATHS";
                string hspProp = proj.GetBuildPropertyForConfig(configGuid, headerSearchPaths) ?? string.Empty;
                string newPro = hspProp.Replace("libil2cpp/include", "libil2cpp")
                    .Replace("Libraries/bdwgc", "Libraries/external/bdwgc");

                if (!newPro.Contains("Libraries/libil2cpp/os/ClassLibraryPAL/brotli/include"))
                {
                    newPro += " $(SRCROOT)/Libraries/libil2cpp/os/ClassLibraryPAL/brotli/include";
                }

                if (!newPro.Contains("Libraries/external/xxHash"))
                {
                    newPro += " $(SRCROOT)/Libraries/external/xxHash";
                }

                newPro += " $(SRCROOT)/Libraries/external/mono";
                proj.SetBuildPropertyForConfig(configGuid, headerSearchPaths, newPro);

                string cflagKey = "OTHER_CFLAGS";
                string cfProp = proj.GetBuildPropertyForConfig(configGuid, cflagKey) ?? string.Empty;
                foreach (var flag in cflags)
                {
                    if (!cfProp.Contains(flag))
                    {
                        cfProp += " " + flag;
                    }
                }

                if (configName.Contains("Debug") && !cfProp.Contains("-DIL2CPP_DEBUG="))
                {
                    cfProp += " -DIL2CPP_DEBUG=1 -DDEBUG=1";
                }

                proj.SetBuildPropertyForConfig(configGuid, cflagKey, cfProp);
            }

            proj.WriteToFile(pbxprojFile);
        }

        private static void CopyLibil2cppToXcodeProj(string srcLibil2cppDir, string dstLibil2cppDir)
        {
            DirectoryUtil.RemoveDir(dstLibil2cppDir);
            DirectoryUtil.CopyDir(srcLibil2cppDir, dstLibil2cppDir, true);
        }

        private static void CopyExternalToXcodeProj(string srcExternalDir, string dstExternalDir)
        {
            if (!Directory.Exists(srcExternalDir))
            {
                throw new InvalidOperationException(
                    $"[ZLua] Local il2cpp external missing at '{srcExternalDir}'. Run ZLua Install first.");
            }

            DirectoryUtil.RemoveDir(dstExternalDir);
            DirectoryUtil.CopyDir(srcExternalDir, dstExternalDir, true);

            string baselibPlatfromsDir = Path.Combine(dstExternalDir, "baselib", "Platforms");
            string osx = Path.Combine(baselibPlatfromsDir, "OSX");
            string ios = Path.Combine(baselibPlatfromsDir, "IOS");
            if (Directory.Exists(osx))
            {
                DirectoryUtil.RemoveDir(ios);
                DirectoryUtil.CopyDir(osx, ios, true);
            }
        }

        class LumpFile
        {
            public List<string> cppFiles = new List<string>();

            public readonly string lumpFile;

            public readonly string il2cppConfigFile;

            public LumpFile(string lumpFile, string il2cppConfigFile)
            {
                this.lumpFile = lumpFile;
                this.il2cppConfigFile = il2cppConfigFile;
                this.cppFiles.Add(il2cppConfigFile);
            }

            public void SaveFile()
            {
                var lumpFileContent = new List<string>();
                foreach (var file in cppFiles)
                {
                    lumpFileContent.Add($"#include \"{GetRelativePathFromProj(file)}\"");
                }

                File.WriteAllLines(lumpFile, lumpFileContent, Encoding.UTF8);
            }
        }

        private static List<LumpFile> CreateLumps(string libil2cppDir, string outputDir)
        {
            DirectoryUtil.RecreateDir(outputDir);

            string il2cppConfigFile = Path.Combine(libil2cppDir, "il2cpp-config.h");
            var lumpFiles = new List<LumpFile>();
            int lumpFileIndex = 0;
            foreach (var cppDir in Directory.GetDirectories(libil2cppDir, "*", SearchOption.AllDirectories).Concat(new string[] { libil2cppDir }))
            {
                var lumpFile = new LumpFile(
                    Path.Combine(outputDir, $"lump_{Path.GetFileName(cppDir)}_{lumpFileIndex}.cpp"),
                    il2cppConfigFile);
                foreach (var file in Directory.GetFiles(cppDir, "*.cpp", SearchOption.TopDirectoryOnly))
                {
                    lumpFile.cppFiles.Add(file);
                }

                // Skip empty lumps (only the config include).
                if (lumpFile.cppFiles.Count <= 1)
                {
                    continue;
                }

                lumpFile.SaveFile();
                lumpFiles.Add(lumpFile);
                ++lumpFileIndex;
            }

            var mmFiles = Directory.GetFiles(libil2cppDir, "*.mm", SearchOption.AllDirectories);
            if (mmFiles.Length > 0)
            {
                var lumpFile = new LumpFile(Path.Combine(outputDir, "lump_mm.mm"), il2cppConfigFile);
                foreach (var file in mmFiles)
                {
                    lumpFile.cppFiles.Add(file);
                }

                lumpFile.SaveFile();
                lumpFiles.Add(lumpFile);
            }

            return lumpFiles;
        }

        private static List<string> GetExtraSourceFiles(string externalDir, string libil2cppDir)
        {
            var files = new List<string>();
            foreach (string extraDir in new string[]
            {
                Path.Combine(externalDir, "zlib"),
                Path.Combine(externalDir, "xxHash"),
                Path.Combine(libil2cppDir, "os", "ClassLibraryPAL", "brotli"),
            })
            {
                if (!Directory.Exists(extraDir))
                {
                    continue;
                }

                files.AddRange(Directory.GetFiles(extraDir, "*.c", SearchOption.AllDirectories));
            }

            return files;
        }
    }
}
#endif
