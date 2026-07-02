using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;

namespace ZLua.Meta
{

    public interface IAssemblyResolver
    {
        string ResolveAssembly(string assemblyName, bool throwExIfNotFind);
    }

    public abstract class AssemblyResolverBase : IAssemblyResolver
    {
        public string ResolveAssembly(string assemblyName, bool throwExIfNotFind)
        {
            if (TryResolveAssembly(assemblyName, out string assemblyPath))
            {
                return assemblyPath;
            }
            if (throwExIfNotFind)
            {
                throw new Exception($"resolve AOT dll:{assemblyName} failed! Please make sure that the AOT project has referenced the dll and generated the trimmed AOT dll correctly.");
            }
            return null;
        }

        protected abstract bool TryResolveAssembly(string assemblyName, out string assemblyPath);
    }

    public class PathAssemblyResolver : AssemblyResolverBase
    {
        private readonly string _dir;
        public PathAssemblyResolver(string dir)
        {
            _dir = dir;
        }

        protected override bool TryResolveAssembly(string assemblyName, out string assemblyPath)
        {
            string path = _dir;
            assemblyPath = Path.Combine(path, $"{assemblyName}.dll");
            if (File.Exists(assemblyPath))
            {
                Debug.Log($"resolve {assemblyName} at {assemblyPath}");
                return true;
            }
            assemblyPath = Path.Combine(path, $"{assemblyName}.dll.bytes");
            if (File.Exists(assemblyPath))
            {
                Debug.Log($"resolve {assemblyName} at {assemblyPath}");
                return true;
            }
            return false;
        }
    }
}
