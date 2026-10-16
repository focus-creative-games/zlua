// Copyright 2026 Code Philosophy

using System;
using System.IO;
using UnityEditor;
using UnityEngine;

namespace ZLua
{
    /// <summary>
    /// Loads Settings luaAliasXmlPaths into <see cref="LuaAliasXmlRegistry"/> for Editor Mono.
    /// </summary>
    [InitializeOnLoad]
    internal static class LuaAliasXmlBootstrap
    {
        static LuaAliasXmlBootstrap()
        {
            TryLoad(logSuccess: false);
        }

        [MenuItem("ZLua/Reload LuaAlias XML", priority = 521)]
        private static void ReloadMenu()
        {
            string projectRoot = Path.GetDirectoryName(Application.dataPath);
            LuaAliasXmlRegistry.Current.Load(Settings.Instance.LuaAliasXmlPaths, projectRoot);
            Debug.Log("[ZLua] LuaAlias XML loaded: " + LuaAliasXmlRegistry.Current.Rules.Count + " rule(s).");
        }

        internal static void TryLoad(bool logSuccess)
        {
            try
            {
                string projectRoot = Path.GetDirectoryName(Application.dataPath);
                LuaAliasXmlRegistry.Current.Load(Settings.Instance.LuaAliasXmlPaths, projectRoot);
                if (logSuccess)
                {
                    Debug.Log("[ZLua] LuaAlias XML loaded: " + LuaAliasXmlRegistry.Current.Rules.Count + " rule(s).");
                }
            }
            catch (Exception ex)
            {
                Debug.LogError("[ZLua] LuaAlias XML load failed:\n" + ex.Message);
            }
        }
    }
}
