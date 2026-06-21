using System;
using System.IO;
using UnityEditor.Build;

namespace NovaLua.BuildProcessors
{

	public static class XCodeUtil
	{

        public static string GetXcodeProjectFile(string pathToBuiltProject)
        {
            foreach (string dir in Directory.GetDirectories(pathToBuiltProject, "*.xcodeproj", SearchOption.TopDirectoryOnly))
            {
                string pbxprojFile = $"{dir}/project.pbxproj";
                if (File.Exists(pbxprojFile))
                {
                    return pbxprojFile;
                }
            }
            throw new BuildFailedException($"can't find xxxx.xcodeproj/project.pbxproj in {pathToBuiltProject}");
        }
    }
}

