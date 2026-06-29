using UnityEditor;
using UnityEditor.Compilation;
using UnityEngine;

namespace ZLua
{
    internal static class LuaInvokeWeaver
    {
        internal static void ForceRecompileAllScripts()
        {
            CompilationPipeline.RequestScriptCompilation(RequestScriptCompilationOptions.CleanBuildCache);
            Debug.Log("[ZLua] Requested full script recompilation (CleanBuildCache). LuaInvoke IL post-processor runs during assembly build.");
        }
    }
}
