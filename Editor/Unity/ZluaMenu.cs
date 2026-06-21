using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

namespace NovaLua.Unity
{

    public static class NovaLuaMenu
    {
        [MenuItem("NovaLua/Settings...", priority = 1)]
        public static void OpenSettings() => SettingsService.OpenProjectSettings("Project/NovaLua");

        [MenuItem("NovaLua/Documents/About")]
        public static void OpenAbout() => Application.OpenURL("https://www.novalua.cc/docs/intro");

        [MenuItem("NovaLua/Documents/GitHub")]
        public static void OpenGitHub() => Application.OpenURL("https://github.com/focus-creative-games/novalua");
    }
}

