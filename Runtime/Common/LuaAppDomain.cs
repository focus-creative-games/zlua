using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;

namespace NextLua
{
    public class LuaAppDomain
    {
        public static void Initialize(Func<string, string> moduleLoader)
        {
            string assemblyName = Application.isEditor ? "NextLua.Mono" : "NextLua.Il2Cpp";
            Assembly assembly = AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault(a => a.GetName().Name == assemblyName);
            string typeName = Application.isEditor ? "NextLua.LuaMonoAppDomain" : "NextLua.LuaIl2CppAppDomain";
            assembly.GetType(typeName).GetMethod("Initialize", BindingFlags.Public | BindingFlags.Static)
                .Invoke(null, new object[] { moduleLoader });
        }
    }
}
