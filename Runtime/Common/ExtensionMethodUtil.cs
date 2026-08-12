using System.Reflection;
using System.Runtime.CompilerServices;

namespace ZLua
{
    /// <summary>
    /// Shared ExtensionAttribute detection for Mono bind / emit (spec 13-EXTENSION-METHODS §3–4).
    /// </summary>
    public static class ExtensionMethodUtil
    {
        public static bool IsExtensionMethod(MethodBase method)
        {
            return method is MethodInfo methodInfo
                   && methodInfo.IsStatic
                   && methodInfo.IsDefined(typeof(ExtensionAttribute), inherit: false);
        }
    }
}
