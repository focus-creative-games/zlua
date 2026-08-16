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

using System;
using System.Reflection;

namespace ZLua.Utils
{
    /// <summary>
    /// Reflective access to <c>ZLua.Settings</c> (Editor assembly) from Runtime Mono
    /// without an Editor asmref cycle.
    /// </summary>
    internal static class EditorSettingsAccess
    {
        private const BindingFlags StaticPublic = BindingFlags.Public | BindingFlags.Static;

        internal static bool TryGetInstance(out object settings, out Type settingsType)
        {
            settings = null;
            settingsType = Type.GetType("ZLua.Settings, ZLua.Editor");
            if (settingsType == null)
            {
                return false;
            }

            PropertyInfo instanceProp = settingsType.GetProperty("Instance", StaticPublic);
            settings = instanceProp?.GetValue(null);
            return settings != null;
        }

        internal static bool TryGetField<T>(object settings, Type settingsType, string fieldName, out T value)
        {
            value = default;
            if (settings == null || settingsType == null)
            {
                return false;
            }

            FieldInfo field = settingsType.GetField(fieldName);
            if (field == null)
            {
                return false;
            }

            object raw = field.GetValue(settings);
            if (raw is T typed)
            {
                value = typed;
                return true;
            }

            return false;
        }
    }
}
