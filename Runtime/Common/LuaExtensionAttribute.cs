using System;

namespace ZLua
{
    /// <summary>
    /// Marks an extended type with one or more extension classes (spec 13-EXTENSION-METHODS §2.1).
    /// Place on the <em>extended</em> type, not on the extension class.
    /// </summary>
    [AttributeUsage(AttributeTargets.Class | AttributeTargets.Struct | AttributeTargets.Interface,
        AllowMultiple = true, Inherited = false)]
    public sealed class LuaExtensionAttribute : Attribute
    {
        public Type[] ExtensionTypes { get; }

        public LuaExtensionAttribute(params Type[] extensionTypes)
        {
            ExtensionTypes = extensionTypes ?? Array.Empty<Type>();
        }
    }
}
