using dnlib.DotNet;
using ZLua.Meta;
using System;
using System.Collections.Generic;
using System.IO;

namespace ZLua.CppCodeGen
{
    public class CodeGenerator
    {
        public class Options
        {
            public AssemblyCache AssemblyCache { get; set; }

            public List<string> AssemblyNames { get; set; }

            public string OutputDir { get; set; }

            public bool Development { get; set; }
        }


        private readonly List<GenericMethod> _genericMethods;

        private readonly AssemblyCache _assemblyCache;

        private readonly List<string> _assemblyNames;

        private readonly string _outputDir;

        private readonly bool _development;

        public CodeGenerator(Options options)
        {
            //List<(GenericMethod, string)> genericMethodInfo = options.GenericMethods.Select(m => (m, m.ToString())).ToList();
            //genericMethodInfo.Sort((a, b) => string.CompareOrdinal(a.Item2, b.Item2));
            //_genericMethods = genericMethodInfo.Select(m => m.Item1).ToList();

            _assemblyCache = options.AssemblyCache;
            _assemblyNames = options.AssemblyNames;
            _outputDir = options.OutputDir;
            _development = options.Development;
        }



        private void GenerateCode()
        {
            //List<string> lines = new List<string>(20_0000)
            //{
            //    "\n",
            //    $"// DEVELOPMENT={(_development ? 1 : 0)}",
            //    "\n"
            //};

            //var classInfos = new List<ClassInfo>();
            //var classTypeSet = new Dictionary<TypeInfo, ClassInfo>();
            //foreach (var type in structTypes)
            //{
            //    GenerateClassInfo(type, classTypeSet, classInfos);
            //}

            //GenerateStructDefines(classInfos, lines);

            //// use structTypes0 to generate signature
            //GenerateStructureSignatureStub(_structTypes0, lines);

            //foreach (var method in _managed2NativeMethodList)
            //{
            //    GenerateManaged2NativeMethod(method, lines);
            //}

            //GenerateManaged2NativeStub(_managed2NativeMethodList, lines);

            //foreach (var method in _native2ManagedMethodList)
            //{
            //    GenerateNative2ManagedMethod(method, lines);
            //}

            //GenerateNative2ManagedStub(_native2ManagedMethodList, lines);

            //Directory.CreateDirectory(Path.GetDirectoryName(_outputFile));

            //File.WriteAllLines(_outputFile, lines, new UTF8Encoding(false));
        }

        public void Generate()
        {
            Directory.CreateDirectory(_outputDir);
            //PrepareMethodBridges();
            //CollectTypesAndMethods();
            //OptimizationTypesAndMethods();
            //GenerateCode();
            var builtinScriptsCodegen = new BuiltinScriptsCodegen(_outputDir);
            builtinScriptsCodegen.Generate();
            var propertyCodegen = new PropertyBridgdeCodegen(_assemblyCache, _assemblyNames, _outputDir);
            propertyCodegen.Generate();
            var methodCodegen = new MethodBridgeCodegen(_assemblyCache, _assemblyNames, _outputDir);
            methodCodegen.Generate();
            var delegateCodegen = new DelegateBridgeCodgen(_assemblyCache, _assemblyNames, _outputDir);
            delegateCodegen.Generate();
            var marshalAsCodegen = new MarshalAsCodegen(_outputDir);
            marshalAsCodegen.Generate();
            var compositeSpecializedCodegen = new CompositeSpecializedCodegen(_assemblyCache, _assemblyNames, _outputDir);
            compositeSpecializedCodegen.Generate();
            var aliasCodegen = new AliasCodegen(_outputDir);
            aliasCodegen.Generate();
        }

    }
}
