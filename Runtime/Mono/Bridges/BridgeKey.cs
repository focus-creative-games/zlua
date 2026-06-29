namespace ZLua
{
    /// <summary>
    /// Identifies a reusable Mono bridge signature (future Lua→C# / delegate phases).
    /// </summary>
    internal readonly struct BridgeKey
    {
        internal BridgeDirection Direction { get; }
        internal string Signature { get; }

        internal BridgeKey(BridgeDirection direction, string signature)
        {
            Direction = direction;
            Signature = signature ?? string.Empty;
        }
    }

    internal enum BridgeDirection
    {
        LuaToCSharp,
        CSharpToLua,
    }
}
