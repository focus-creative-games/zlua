using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;

namespace NovaLua
{
    public class LuaAppDomain
    {
        public static void Initialize(Func<string, object> moduleLoader)
        {
            string assemblyName = Application.isEditor ? "NovaLua.Mono" : "NovaLua.Il2Cpp";
            Assembly assembly = AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault(a => a.GetName().Name == assemblyName);
            string typeName = Application.isEditor ? "NovaLua.LuaMonoAppDomain" : "NovaLua.LuaIl2CppAppDomain";
            assembly.GetType(typeName).GetMethod("Initialize", BindingFlags.Public | BindingFlags.Static)
                .Invoke(null, new object[] { moduleLoader });
        }
    }
}
