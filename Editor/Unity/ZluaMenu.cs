using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

namespace NextLua.Unity
{

    public static class NextLuaMenu
    {
        [MenuItem("NextLua/Settings...", priority = 1)]
        public static void OpenSettings() => SettingsService.OpenProjectSettings("Project/NextLua");

        [MenuItem("NextLua/Documents/About")]
        public static void OpenAbout() => Application.OpenURL("https://www.nextlua.cc/docs/intro");

        [MenuItem("NextLua/Documents/GitHub")]
        public static void OpenGitHub() => Application.OpenURL("https://github.com/focus-creative-games/nextlua");
    }
}

