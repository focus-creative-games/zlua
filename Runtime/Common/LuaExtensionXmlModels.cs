namespace ZLua
{
    /// <summary>
    /// One extension-class rule from external XML (spec 13-EXTENSION-METHODS §2.2).
    /// </summary>
    public sealed class LuaExtensionXmlRule
    {
        public string SourcePath { get; set; }

        public string TargetAssemblyName { get; set; }

        public string TargetTypeFullName { get; set; }

        public string ExtensionAssemblyName { get; set; }

        public string ExtensionTypeFullName { get; set; }

        public string DuplicateKey =>
            TargetAssemblyName + "|" + TargetTypeFullName + "|"
            + ExtensionAssemblyName + "|" + ExtensionTypeFullName;
    }
}
