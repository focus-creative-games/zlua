namespace ZLua
{
    /// <summary>
    /// One method alias rule from external XML (spec 04-METHOD-OVERLOAD §5.4).
    /// </summary>
    public sealed class LuaAliasXmlRule
    {
        public string SourcePath { get; set; }

        public string AssemblyName { get; set; }

        public string TypeFullName { get; set; }

        public string MethodName { get; set; }

        public string Signature { get; set; }

        public string Alias { get; set; }

        public string DuplicateKey =>
            AssemblyName + "|" + TypeFullName + "|" + MethodName + "|" + Signature;
    }
}
