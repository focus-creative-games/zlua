// Copyright 2026 Code Philosophy

using System.IO;
using System.Text;

namespace ZLua.CppCodeGen
{
    public class BuiltinScriptsCodegen
    {
        private readonly string _outputDir;

        public BuiltinScriptsCodegen(string outputDir)
        {
            _outputDir = outputDir;
        }

        public void Generate()
        {

            string globalsPath = CommonDirs.GetLuaLibScriptPath("globals.lua");
            string libPath = CommonDirs.GetLuaLibScriptPath("zlualib.lua");

            var writer = new CodeWriter();
            writer.WriteLine(EmbedLua("kZLuaGlobalsLua", File.Exists(globalsPath) ? File.ReadAllText(globalsPath, Encoding.UTF8) : string.Empty));
            writer.WriteLine();
            writer.WriteLine(EmbedLua("kZLuaLibLua", File.Exists(libPath) ? File.ReadAllText(libPath, Encoding.UTF8) : string.Empty));
            writer.Save(Path.Combine(_outputDir, "BuiltinScripts.inc"));
        }

        private static string EmbedLua( string name, string content)
        {
            var sb = new StringBuilder();
            sb.Append("static const char ").Append(name).Append("[] = R\"zlua(").Append(content).Append(")zlua\";");
            return sb.ToString();
        }
    }
}
