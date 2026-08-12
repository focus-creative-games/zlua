// Copyright 2026 Code Philosophy

using System;
using System.IO;
using UnityEditor;
using UnityEngine;

namespace ZLua
{
    /// <summary>
    /// Loads Settings luaExtensionXmlPaths into <see cref="LuaExtensionXmlRegistry"/> for Editor Mono.
    /// </summary>
    [InitializeOnLoad]
    internal static class LuaExtensionXmlBootstrap
    {
        static LuaExtensionXmlBootstrap()
        {
            TryLoad(logSuccess: false);
        }

        [MenuItem("ZLua/Reload LuaExtension XML", priority = 522)]
        private static void ReloadMenu()
        {
            string projectRoot = Path.GetDirectoryName(Application.dataPath);
            LuaExtensionXmlRegistry.Load(Settings.Instance.luaExtensionXmlPaths, projectRoot);
            Debug.Log("[ZLua] LuaExtension XML loaded: " + LuaExtensionXmlRegistry.Rules.Count + " rule(s).");
        }

        internal static void TryLoad(bool logSuccess)
        {
            try
            {
                string projectRoot = Path.GetDirectoryName(Application.dataPath);
                LuaExtensionXmlRegistry.Load(Settings.Instance.luaExtensionXmlPaths, projectRoot);
                if (logSuccess)
                {
                    Debug.Log("[ZLua] LuaExtension XML loaded: " + LuaExtensionXmlRegistry.Rules.Count + " rule(s).");
                }
            }
            catch (Exception ex)
            {
                Debug.LogError("[ZLua] LuaExtension XML load failed:\n" + ex.Message);
            }
        }
    }
}
