using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using UnityEditor;
using UnityEditor.Build;
using UnityEditor.Callbacks;
using UnityEngine;
using ZLua.Utils;

#if UNITY_2022 && !UNITY_2023_1_OR_NEWER

namespace ZLua.BuildProcessors
{
    public static class AddLil2cppSourceCodeToXcodeproj2022OrNewer
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

            string pbxprojFile = GetXcodeProjectFile(pathToBuiltProject);
            RemoveExternalLibil2cppOption(pbxprojFile);
            CopyLibil2cppToXcodeProj(pathToBuiltProject);
        }

        private static string GetXcodeProjectFile(string pathToBuiltProject)
        {
            return Path.Combine(pathToBuiltProject, "Unity-iPhone.xcodeproj", "project.pbxproj");
        }

        private static string TryRemoveDuplicateShellScriptSegment(string pbxprojFile, string pbxprojContent)
        {
            // Unity may append a duplicated Shell Script segment when appending to an existing Xcode project.
            string copyFileComment = @"/\* CopyFiles \*/,\s+([A-Z0-9]{24}) /\* ShellScript \*/,\s+([A-Z0-9]{24}) /\* ShellScript \*/,";
            var m = Regex.Match(pbxprojContent, copyFileComment, RegexOptions.Multiline);
            if (!m.Success)
            {
                return pbxprojContent;
            }

            if (m.Groups[1].Value != m.Groups[2].Value)
            {
                throw new BuildFailedException("find invalid /* ShellScript */ segment");
            }

            int startIndexOfDupShellScript = m.Groups[2].Index;
            int endIndexOfDupShellScript = pbxprojContent.IndexOf(",", startIndexOfDupShellScript);

            pbxprojContent = pbxprojContent.Remove(startIndexOfDupShellScript, endIndexOfDupShellScript + 1 - startIndexOfDupShellScript);
            Debug.LogWarning($"[AddLil2cppSourceCodeToXcodeproj] remove duplicated '/* ShellScript */' from file '{pbxprojFile}'");
            return pbxprojContent;
        }

        private static void RemoveExternalLibil2cppOption(string pbxprojFile)
        {
            string pbxprojContent = File.ReadAllText(pbxprojFile, Encoding.UTF8);
            string removeBuildOption = @"--external-lib-il2-cpp=\""$PROJECT_DIR/Libraries/libil2cpp.a\""";
            if (pbxprojContent.Contains(removeBuildOption))
            {
                pbxprojContent = pbxprojContent.Replace(removeBuildOption, "");
                Debug.Log($"[AddLil2cppSourceCodeToXcodeproj] remove il2cpp build option '{removeBuildOption}' from file '{pbxprojFile}'");
            }
            else
            {
                Debug.LogWarning(
                    $"[AddLil2cppSourceCodeToXcodeproj] project.pbxproj remove building option:'{removeBuildOption}' fail. " +
                    "This may occur when 'Append' to existing xcode project in building");
            }

            pbxprojContent = TryRemoveDuplicateShellScriptSegment(pbxprojFile, pbxprojContent);
            File.WriteAllText(pbxprojFile, pbxprojContent, Encoding.UTF8);
        }

        private static void CopyLibil2cppToXcodeProj(string pathToBuiltProject)
        {
            string srcLibil2cppDir = CommonDirs.LocalLibil2cppPath;
            string destLibil2cppDir = Path.Combine(pathToBuiltProject, "Il2CppOutputProject", "IL2CPP", "libil2cpp");
            DirectoryUtil.RemoveDir(destLibil2cppDir);
            DirectoryUtil.CopyDir(srcLibil2cppDir, destLibil2cppDir, true);
            Debug.Log($"[AddLil2cppSourceCodeToXcodeproj] copied libil2cpp -> {destLibil2cppDir}");
        }
    }
}
#endif
