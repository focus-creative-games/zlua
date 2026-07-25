// Copyright 2026 Code Philosophy

using System;
using System.IO;
using System.Text;
using UnityEngine;

namespace ZLua.Utils
{
    [Serializable]
    public sealed class InstallFingerprintData
    {
        public string unityVersion;
        public string luaVersionId;
        public string luaSeries;
        public string libil2cppPatchKey;
        public string luaPatchKey;
        public string packageContentStamp;
        public string defines;
    }

    public static class InstallFingerprint
    {
        public static string Path =>
            System.IO.Path.Combine(CommonDirs.InstallRootDir, "install_fingerprint.json");

        public static void Write(InstallFingerprintData data)
        {
            Directory.CreateDirectory(CommonDirs.InstallRootDir);
            string json = JsonUtility.ToJson(data, true);
            File.WriteAllText(Path, json, Encoding.UTF8);
        }

        public static bool TryRead(out InstallFingerprintData data)
        {
            data = null;
            if (!File.Exists(Path))
            {
                return false;
            }

            try
            {
                data = JsonUtility.FromJson<InstallFingerprintData>(File.ReadAllText(Path, Encoding.UTF8));
                return data != null;
            }
            catch
            {
                return false;
            }
        }
    }
}
