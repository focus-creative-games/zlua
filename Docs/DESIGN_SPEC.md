# design spec v1.0

## 核心设计目标

### 使用方式

- 类比于 P/Invoke、 MonoPInvokeCallback, MarshalAs， nextlua 有同样对应的概念 L/Invoke、MonoLuaCallback, LuaMarshalAs
- c# 与 lua 之间交互是高度统一的：
  - c#可以调用 标记为 `[LuaInvoke]` 的static c#函数，调用lua函数
  - 所有c#类都通过lazy register的方式，使用时自动注册到lua环境。  lua通过 `CSharp.<Module>.<Type>` 可以访问类， 通过`<Type>.XXX` 访问静态成员和函数，通过 `obj:Fun` 调用成员函数和属性，无论它是struct、class、泛型还是array。语义完全等价到c#中以这种方式调用c#函数。没有特殊概念，完全统一
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

### nextlua 库

`Packages\com.code-philosophy.nextlua\Resources\nextlua\nextlualib.lua` 文件包含了一些全局公共的nextlua 辅助函数。 这个文件在luaState初始化时被读取然后执行`dostring`。

有以下函数：

- typeof 。根据传入的类型，返回对应的System.Type对象。 例如  `local t = nextlua.typeof(CSharp.mscorlib['System.Int32'])` ，t等价于 c#里`typeof(int)`的值
- make_generic_type。 根据传入的泛型基类和泛型实例化参数，返回最终的类型。如 `local t = nextlua.make_generic_type(CSharp.mscorlib['System.Collections.Generic.List'], CSharp.mscorlib['System.Int32'])`
- create_signature。返回一个函数签名。如`local t = nextlua.create_signature('run', CSharp.mscorlib['System.Int32'])`


有以下字段：

- corlibtypes。 包含常见类型的typeof的值。如 ` int32 = nextlua.typeof(CSharp.mscorlib['System.Int32'])`。

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
  - 添加 [DllImport("__Internal", entryPoint="{entryPoint}")] ，其中entryPoint的值为  {assembly_name}_{full_type_name}_{method_name})，assembly full_type method name中的不能出现在c函数名上的字符全部要移除。

### MonoLuaCallbackAttribute

类似于 MonoPInvokeCallback，意味着会从lua调用这个函数。 一般来说，只有获得一个c#函数的指针，并且传递给lua调用时才会需要。然而lua仅支持调用 `int (lua_State* L)`
这种签名的函数，这意味着 这个作用不大。

### lua调用c#函数

c#函数，都是在lua 第一次调用时自动注册的。无需`[MonoLuaCallback]`。 每一种签名的函数都会生成一个唯一的 桥接函数,这个跟 MonoLuaCallbackAttribute无关。

#### 处理函数重载

c#类中可能存在同名函数，如 `void Run(int x)` 和 `void Run(string x)`。而lua无法区分这个。解决办法有几个：

1. 引入 signature 机制。

```lua

local sig_run_int32 = nextlua.signature("Run", nextlua.corlibtypes.int32)

-- 有两种调用方式：
-- 方法1

obj[sig_run_int32](obj)

-- 方法2

local run_i32 = nextlua.get_method(obj, sig_run_int32)
nextlua.register_method(obj, "run_i32", run_i32)

obj:run_i32(10)


```

2. 引入别名

在c#函数中加上`[MonoLuaCallback("Run_i32)]`。

```csharp

[MonoLuaCallback("Run_i32")]
void Run(int x)
{

}

```

在lua中可以直接 `obj:Run_i32(10)` 调用。 缺点是对于预编译好的dll或者第三方代码没法这么做

3. 引入别名配置

```xml

<assembly fullname="Assembly-CSharp">
    <type fullname="Demo">
        <method signature="void Run(System.Int32)" lua_call_name="Run_i32"/>
    </type>
</assembly>

```