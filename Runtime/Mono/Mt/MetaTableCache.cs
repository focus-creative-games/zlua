using System;
using System.Collections.Generic;

namespace ZLua.Mt
{
    /// <summary>
    /// Type table ref cache (aligns with Il2Cpp mt/MetaTableCache).
    /// Interned refs live on <see cref="TypeRegistry"/>.
    /// </summary>
    internal static class MetaTableCache
    {
        internal static void PushInternedTypeTable(IntPtr luaState, Type type)
        {
            TypeRegistry.PushInternedTypeTable(luaState, type);
        }
    }
}
