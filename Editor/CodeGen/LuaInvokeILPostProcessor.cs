using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using dnlib.DotNet;
using dnlib.DotNet.Writer;
using Unity.CompilationPipeline.Common.Diagnostics;
using Unity.CompilationPipeline.Common.ILPostProcessing;

namespace ZLua
{
    internal sealed class LuaInvokeILPostProcessor : ILPostProcessor
    {
        public override ILPostProcessor GetInstance() => this;

        public override bool WillProcess(ICompiledAssembly compiledAssembly)
        {
            if (ShouldSkipAssemblyName(compiledAssembly.Name))
            {
                return false;
            }

            return compiledAssembly.References.Any(reference =>
            {
                string fileName = Path.GetFileNameWithoutExtension(reference);
                return string.Equals(fileName, LuaInvokeMonoReferenceImporter.MonoAssemblyName, StringComparison.Ordinal)
                    || string.Equals(fileName, LuaInvokeMonoReferenceImporter.CommonAssemblyName, StringComparison.Ordinal)
                    || string.Equals(fileName, "ZLua.Il2Cpp", StringComparison.Ordinal);
            });
        }

        public override ILPostProcessResult Process(ICompiledAssembly compiledAssembly)
        {
            var diagnostics = new List<DiagnosticMessage>();

            try
            {
                InMemoryAssembly inputAssembly = compiledAssembly.InMemoryAssembly;
                byte[] peData = inputAssembly.PeData.ToArray();
                byte[] pdbData = inputAssembly.PdbData?.ToArray();

                var moduleContext = ModuleDef.CreateModuleContext();
                var assemblyResolver = new LuaInvokeILPostProcessorAssemblyResolver(compiledAssembly, moduleContext);
                moduleContext.AssemblyResolver = assemblyResolver;
                moduleContext.Resolver = new Resolver(assemblyResolver);

                using ModuleDefMD module = ModuleDefMD.Load(peData, new ModuleCreationOptions(moduleContext));
                module.Context = moduleContext;
                assemblyResolver.RegisterInMemoryAssembly(module);

                if (pdbData != null && pdbData.Length > 0)
                {
                    module.LoadPdb(pdbData);
                }

                bool isEditorDevelopment = compiledAssembly.Defines != null
                    && compiledAssembly.Defines.Contains("UNITY_EDITOR");

                bool changed = LuaInvokeAssemblyRewriter.Process(
                    module,
                    isEditorDevelopment,
                    assemblyResolver,
                    out string errorMessage);

                if (errorMessage != null)
                {
                    diagnostics.Add(new DiagnosticMessage
                    {
                        DiagnosticType = DiagnosticType.Error,
                        MessageData = errorMessage,
                    });
                    return new ILPostProcessResult(inputAssembly, diagnostics);
                }

                if (!changed)
                {
                    return new ILPostProcessResult(inputAssembly, diagnostics);
                }

                using var peStream = new MemoryStream();
                MemoryStream pdbStream = pdbData != null && pdbData.Length > 0 ? new MemoryStream() : null;
                var writerOptions = new ModuleWriterOptions(module);
                if (pdbStream != null)
                {
                    writerOptions.WritePdb = true;
                    writerOptions.PdbOptions = new PdbWriterOptions();
                    writerOptions.PdbStream = pdbStream;
                }

                module.Write(peStream, writerOptions);
                byte[] outputPdb = pdbStream?.ToArray() ?? Array.Empty<byte>();
                return new ILPostProcessResult(new InMemoryAssembly(peStream.ToArray(), outputPdb), diagnostics);
            }
            catch (Exception ex)
            {
                diagnostics.Add(new DiagnosticMessage
                {
                    DiagnosticType = DiagnosticType.Error,
                    MessageData = "[ZLua] LuaInvoke IL post-processing failed: " + ex,
                });
                return new ILPostProcessResult(compiledAssembly.InMemoryAssembly, diagnostics);
            }
        }

        private static bool ShouldSkipAssemblyName(string assemblyName)
        {
            if (string.IsNullOrEmpty(assemblyName))
            {
                return true;
            }

            if (string.Equals(assemblyName, "mscorlib", StringComparison.Ordinal)
                || string.Equals(assemblyName, "netstandard", StringComparison.Ordinal)
                || string.Equals(assemblyName, "System", StringComparison.Ordinal)
                || assemblyName.StartsWith("System.", StringComparison.Ordinal))
            {
                return true;
            }

            if (string.Equals(assemblyName, "UnityEngine", StringComparison.Ordinal)
                || string.Equals(assemblyName, "UnityEditor", StringComparison.Ordinal)
                || assemblyName.StartsWith("UnityEngine.", StringComparison.Ordinal)
                || assemblyName.StartsWith("UnityEditor.", StringComparison.Ordinal)
                || assemblyName.StartsWith("Unity.", StringComparison.Ordinal))
            {
                return true;
            }

            return false;
        }
    }
}
