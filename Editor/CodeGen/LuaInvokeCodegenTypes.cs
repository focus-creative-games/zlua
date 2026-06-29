namespace ZLua
{
    internal enum LuaMarshalDirection
    {
        CSharpToLua,
        LuaToCSharp,
    }

    internal enum LuaMarshalType
    {
        Default,
        UserData,
        Bytes,
        OpaqueLightUserData,
    }
}
