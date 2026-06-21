// Copyright 2026 Code Philosophy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

using NovaLua.BuildProcessors;
using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using UnityEditor;
using UnityEngine;

namespace NovaLua
{
    public static class Menus
    {

        [MenuItem("NovaLua/Settings...")]
        public static void OpenSettings() => SettingsService.OpenProjectSettings("Project/NovaLua");

        [MenuItem("NovaLua/Install...", priority = 100)]
        public static void Install()
        {
            var installer = new LocalInstaller();
            if (installer.HasInstalledToLocal())
            {
                if (EditorUtility.DisplayDialog("NovaLua is already installed", "Do you want to reinstall it?", "Yes", "No"))
                {
                    installer.InstallLocal();
                }
            }
            else
            {
                installer.InstallLocal();
            }
        }

        [MenuItem("NovaLua/Documents/About", priority = 200)]
        public static void OpenAbout() => Application.OpenURL("https://www.novalua.cn/docs/intro");

        [MenuItem("NovaLua/Documents/Home")]
        public static void OpenHomePage() => Application.OpenURL("https://www.novalua.cn");

        [MenuItem("NovaLua/Documents/GitHub")]
        public static void OpenGitHub() => Application.OpenURL("https://github.com/focus-creative-games/novalua");
    }

}