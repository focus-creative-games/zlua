// Copyright 2026 Code Philosophy

using System;
using UnityEditor;
using UnityEngine;
using ZLua.Utils;

namespace ZLua
{
    /// <summary>
    /// Batchmode / -executeMethod entry points (no dialogs).
    /// Optional env <c>ZLUA_LUA_VERSION_ID</c> overrides Settings before Install.
    /// </summary>
    public static class BatchCommands
    {
        public static void Install()
        {
            try
            {
                ApplyLuaVersionFromEnvironment();
                var installer = new LocalInstaller();
                installer.InstallLocal();
                Debug.Log("[ZLua] BatchCommands.Install OK");
                EditorApplication.Exit(0);
            }
            catch (Exception ex)
            {
                Debug.LogError($"[ZLua] BatchCommands.Install FAILED:\n{ex}");
                EditorApplication.Exit(1);
            }
        }

        /// <summary>Apply env override and save Settings; does not Install.</summary>
        public static void ApplyLuaVersionFromEnvironment()
        {
            string envId = Environment.GetEnvironmentVariable("ZLUA_LUA_VERSION_ID");
            if (string.IsNullOrWhiteSpace(envId))
            {
                return;
            }

            envId = envId.Trim();
            if (!LuaVersionUtil.TryParse(envId, out _))
            {
                throw new InvalidOperationException($"Invalid ZLUA_LUA_VERSION_ID: {envId}");
            }

            Settings.Instance.luaVersionId = envId;
            Settings.Save();
            Debug.Log($"[ZLua] Settings.luaVersionId set from env: {envId}");
        }
    }
}
