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
