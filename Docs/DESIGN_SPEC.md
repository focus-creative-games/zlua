# design spec v1.0

## 核心设计目标

### 使用方式

- 类比于 P/Invoke、 MonoPInvokeCallback, MarshalAs， zlua 有同样对应的概念 L/Invoke、MonoLuaCallback, LuaMarshalAs
- c# 与 lua 之间交互是高度统一的：
  - c#可以调用 标记为 `[LuaInvoke]` 的static c#函数，调用lua函数
  - 所有c#类都通过lazy register的方式，使用时自动注册到lua环境。类型访问、元表与成员规则见 **`TYPE_SYSTEM_SPEC.md`**（含命名空间类型须 `CSharp.{asm}['Ns.Type']` 括号访问）。通过类型表访问静态成员、通过 `obj:Method()` 调用实例成员，语义与 C# 一致。
  - c# 与 lua之间的 交互代码，都是自动生成的，对开发者完全感。 在editor下生成c#代码，发布到il2cpp时，生成 c++ 代码。
- 深度集成，启动时就有初始化好的全局CLR和luaState

### 发布时优化

- 交互函数全是c++代码
- 复用相同签名的交互函数，不像{abc}lua那样，每个字段或者函数都生成一个单独的函数。 就如hybridclr那样，直接生成所有必要的桥接文件，总大小仍然可控。
- 调用 lua接口并不需要通过 LuaDll 里的 extern 调用，而是c++层面直接调用lua api
- 访问类成员变量也是直接c++ 代码中 obj + 偏移 直接访问，不需要调用c#函数。访问类静态成员变量也类似。
- 访问类成员（静态和非静态）函数，也是通过il2cpp的 MethodInfo中的 methodPointer直接调用，不需要经过 c#包装类
- 为托管对象生成lua userData时，在userData中直接记录了object指针。同时在native代码维护了一个 object列表，这个列表被注册到 gc roots。等lua释放userDatas时，再将这个对象从列表中清除。

## 实现

### zlua 库

见[LIB_SPEC](./LIB_SPEC.md)。

### LuaInvokeAttribute

标记了 `[LuaInvoke]`的函数应该是一个 特殊的c#函数，它在editor下实现为 调用 LuaAppDomain.RunLuaFunc(moduleName, methodName); 其中 methodName和moduleName都是通过反射从 LuaPInvoke获得的。 如果有返回值 ，还需要处理返回值。 在非editor环境下。它会在发布过程中被IL修改为一个 extern 函数， 它的entryPointer指向一个生成的 extern "C" 函数。也就是在发布到il2cpp时，它直接调用了一个 c++ 函数。整个过程中透明的。

在editor模式下，开发者定义标记`[LuaInvoke]`函数时，必须为`static extern`函数。 插件会在编译dll后自动修改注入，实现真正的代码。
当unity 的dll 编译后， 使用dnlib 修改 dll，对于包含 [LuaInvoke]的函数：

- 检查它必须是static extern 的，不能是泛型类的成员函数，也不能自身是泛型函数，否则抛出异常
- 从 [LuaInvoke] 中获取 moduleName和methodName
- 如果在Editor下
  - 调用 LuaMonoAppDomain类中的 RunLuaFunc 或 `RunLuaFunc<T>`函数
- 如果不在Editor下
  - 移除[LuaInvoke]
  - 添加 [MethodImpl(MethodImplOptions.InternalCall)]

### MonoLuaCallbackAttribute

类似于 MonoPInvokeCallback，意味着会从lua调用这个函数。 一般来说，只有获得一个c#函数的指针，并且传递给lua调用时才会需要。然而lua仅支持调用 `int (lua_State* L)`
这种签名的函数，这意味着 这个作用不大。

### lua调用c#函数

c#函数，都是在lua 第一次调用时自动注册的。无需`[MonoLuaCallback]`。 每一种签名的函数都会生成一个唯一的 桥接函数,这个跟 MonoLuaCallbackAttribute无关。

#### 处理函数重载

c#类中可能存在同名函数，如 `void Run(int x)` 和 `void Run(string x)`。完整规范见 **`METHOD_OVERLOAD_SPEC.md`**。概要：

1. **默认 dispatch**：`obj:Run(x)` 运行时分派（多重重载时）。
2. **`[LuaAlias]` / XML**：类内唯一别名。
3. **运行时签名**：`get_method` 查找后缓存或 `register_method` 注册。

```lua
local sig_i32 = zlua.signature(zlua.types.int32)
local run_i32 = zlua.get_method(obj, "Run", sig_i32, false)
run_i32(obj, 10)
zlua.register_method("run_i32", run_i32)
obj:run_i32(10)
```

**不推荐** `obj[sig](obj, ...)` 按签名字符串键查找。

别名细节（`[LuaAlias]`、XML 格式）见 `METHOD_OVERLOAD_SPEC.md` §5。

## Mono 和 Il2Cpp 实现

为了最大化正式发布时的运行效率，实现了两套代码，分别为 ZLua.Mono和ZLua.Il2Cpp。Editor下使用ZLua.Mono中的实现，正式发布时使用ZLua.Il2Cpp中实现。

LuaInvokeAttriubte、MonoLuaCallbackAttribute、LuaMarshalAsAttribute这些类在ZLua.Common模块中定义。

ZLua.Common中定义了 LuaAppDomain 门面类，它会将真正实现转发到 ZLua.Mono中的LuaMonoAppDomain或 ZLua.Il2Cpp中的LuaIl2CppAppDomain。

```csharp
namespace ZLua
{
    public class LuaAppDomain
    {
        public static void Initialize(Func<string, string> moduleLoader)
        {
            string assemblyName = Application.isEditor ? "ZLua.Mono" : "ZLua.Il2Cpp";
            Assembly assembly = AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault(a => a.GetName().Name == assemblyName);
            string typeName = Application.isEditor ? "ZLua.LuaMonoAppDomain" : "ZLua.LuaIl2CppAppDomain";
            assembly.GetType(typeName).GetMethod("Initialize", BindingFlags.Public | BindingFlags.Static)
                .Invoke(null, new object[] { moduleLoader });
        }
    }
}
```

### ZLua.Il2Cpp 的实现

在C#层面，Il2Cpp版本实现在 ZLua.Il2Cpp 程序集内，仅有一个非常薄的实现：

```csharp
    public static class LuaIl2CppAppDomain
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void InitializeInternal(Func<string, string> moduleLoader);

        public static void Initialize(Func<string, string> moduleLoader)
        {
            InitializeInternal(moduleLoader);
        }
    }
```

与ZLua.Mono对应的代码， 全部在c++层实现。

我们需要修改Unity的原始libil2cpp代码：

- `libil2cpp/lua` 为 lua源码（当前使用lua 5.4）。这个我们已经手动添加了。
- `libil2cpp/zlua`目录为 zlua的源码目录

以上代码unity在构建时会自动将它加入编译，最终将lua、zlua及libil2cpp及il2cpp生成的c++代码静态编译到同一个二进制模块（.a或.dll或.so）。

