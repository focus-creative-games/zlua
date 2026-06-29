using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using dnlib.DotNet;
using Unity.CompilationPipeline.Common.ILPostProcessing;

namespace ZLua
{
    internal sealed class LuaInvokeILPostProcessorAssemblyResolver : IAssemblyResolver
    {
        private readonly ICompiledAssembly _compiledAssembly;
        private readonly ModuleContext _moduleContext;
        private readonly Dictionary<string, AssemblyDef> _cache = new(StringComparer.OrdinalIgnoreCase);

        internal LuaInvokeILPostProcessorAssemblyResolver(ICompiledAssembly compiledAssembly, ModuleContext moduleContext)
        {
            _compiledAssembly = compiledAssembly;
            _moduleContext = moduleContext;
        }

        internal void RegisterInMemoryAssembly(ModuleDefMD module)
        {
            if (module?.Assembly == null)
            {
                return;
            }

            _cache[module.Assembly.Name.String] = module.Assembly;
        }

        internal AssemblyDef ResolveAssembly(string assemblyName)
        {
            return Resolve(new AssemblyNameInfo(assemblyName), null);
        }

        public AssemblyDef Resolve(IAssembly assemblyRef, ModuleDef sourceModule)
        {
            if (assemblyRef == null)
            {
                return null;
            }

            string assemblyName = assemblyRef.Name;
            if (_cache.TryGetValue(assemblyName, out AssemblyDef cached))
            {
                return cached;
            }

            string referencePath = FindReferencePath(assemblyName);
            if (string.IsNullOrEmpty(referencePath) || !File.Exists(referencePath))
            {
                return null;
            }

            byte[] peData = File.ReadAllBytes(referencePath);
            ModuleDefMD loadedModule = ModuleDefMD.Load(peData, new ModuleCreationOptions(_moduleContext));
            loadedModule.Context = _moduleContext;
            AssemblyDef assembly = loadedModule.Assembly;
            if (assembly == null)
            {
                assembly = new AssemblyDefUser(new AssemblyNameInfo(assemblyName));
                assembly.Modules.Add(loadedModule);
            }

            _cache[assemblyName] = assembly;
            return assembly;
        }

        private string FindReferencePath(string assemblyName)
        {
            foreach (string reference in _compiledAssembly.References)
            {
                if (string.Equals(Path.GetFileNameWithoutExtension(reference), assemblyName, StringComparison.OrdinalIgnoreCase))
                {
                    return reference;
                }
            }

            foreach (string reference in _compiledAssembly.References)
            {
                string directory = Path.GetDirectoryName(reference);
                if (string.IsNullOrEmpty(directory))
                {
                    continue;
                }

                string siblingPath = Path.Combine(directory, assemblyName + ".dll");
                if (File.Exists(siblingPath))
                {
                    return siblingPath;
                }
            }

            return null;
        }

        public void AddToCache(AssemblyDef asm)
        {
            if (asm != null)
            {
                _cache[asm.Name.String] = asm;
            }
        }

        public bool AddToCache_NoThrow(AssemblyDef asm)
        {
            try
            {
                AddToCache(asm);
                return true;
            }
            catch
            {
                return false;
            }
        }

        public void Dispose()
        {
            _cache.Clear();
        }
    }
}
