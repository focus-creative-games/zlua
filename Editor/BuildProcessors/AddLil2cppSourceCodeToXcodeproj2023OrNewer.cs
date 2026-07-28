using System.IO;
using UnityEditor;
using UnityEditor.Callbacks;
using UnityEngine;
using ZLua.Utils;

#if UNITY_2023_1_OR_NEWER && (UNITY_IOS || UNITY_TVOS || UNITY_VISIONOS)

namespace ZLua.BuildProcessors
{
    public static class AddLil2cppSourceCodeToXcodeproj2023OrNewer
    {
        [PostProcessBuild]
        public static void OnPostProcessBuild(BuildTarget target, string pathToBuiltProject)
        {
            if (!Settings.EnableForCurrentBuildTarget)
            {
                return;
            }

            CopyLibil2cppToXcodeProj(pathToBuiltProject);
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
