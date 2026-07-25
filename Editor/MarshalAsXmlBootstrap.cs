// Copyright 2026 Code Philosophy

using System;
using System.IO;
using UnityEditor;
using UnityEngine;

namespace ZLua
{
    /// <summary>
    /// Loads Settings marshalAsXmlPaths into <see cref="LuaMarshalAsXmlRegistry"/> for Editor Mono.
    /// </summary>
    [InitializeOnLoad]
    internal static class MarshalAsXmlBootstrap
    {
        static MarshalAsXmlBootstrap()
        {
            TryLoad(logSuccess: false);
        }

        [MenuItem("ZLua/Reload MarshalAs XML", priority = 520)]
        private static void ReloadMenu()
        {
            string projectRoot = Path.GetDirectoryName(Application.dataPath);
            LuaMarshalAsXmlRegistry.Load(Settings.Instance.marshalAsXmlPaths, projectRoot);
            Debug.Log("[ZLua] MarshalAs XML loaded: " + LuaMarshalAsXmlRegistry.Rules.Count + " rule(s).");
        }

        internal static void TryLoad(bool logSuccess)
        {
            try
            {
                string projectRoot = Path.GetDirectoryName(Application.dataPath);
                LuaMarshalAsXmlRegistry.Load(Settings.Instance.marshalAsXmlPaths, projectRoot);
                if (logSuccess)
                {
                    Debug.Log("[ZLua] MarshalAs XML loaded: " + LuaMarshalAsXmlRegistry.Rules.Count + " rule(s).");
                }
            }
            catch (Exception ex)
            {
                // Do not rethrow from InitializeOnLoad — keep Editor usable; Generate/menu still fail hard.
                Debug.LogError("[ZLua] MarshalAs XML load failed:\n" + ex.Message);
            }
        }
    }
}
