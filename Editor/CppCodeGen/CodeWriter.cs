using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ZLua.CppCodeGen
{
    public class CodeWriter
    {
        private readonly List<string> _lines = new List<string>();

        private int _indentLevel = 0;

        private string _indent = string.Empty;
        
        public void IncreaseIndent()
        {
            _indentLevel++;
            _indent = new string(' ', _indentLevel * 4);
        }
        
        public void DecreaseIndent()
        {
            _indentLevel--;
            _indent = new string(' ', _indentLevel * 4);
        }

        public void ResetIndent()
        {
            _indentLevel = 0;
            _indent = string.Empty;
        }

        public void WriteLine(string line)
        {
            _lines.Add($"{_indent}{line}");
        }

        public void WriteLine()
        {
            _lines.Add($"{_indent}");
        }

        public void Save(string filePath)
        {
            System.IO.File.WriteAllLines(filePath, _lines, new UTF8Encoding(false));
            UnityEngine.Debug.Log($"write file to {filePath}");
        }
    }
}
