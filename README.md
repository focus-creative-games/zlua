# novalua

NovaLua是一个针对Unity Il2Cpp 极致优化的现代原生lua方案。

## 为什么选择NovaLua

- 极致易用简洁优雅，统一了c#与lua双向调用。把Lua当作另一种`Native`，开创性清晰提出 `[LuaInvoke]`、`[LuaCallback]`、 `[LuaMarshalAs]`的概念。对开发者完全屏蔽了底层复杂易错的Lua的操作。
- 极致高效。在il2cpp中内嵌lua，抛开lua与c#之间交互绕开低效的C#接口，在c++层面让il2cpp和lua虚拟机直接相互操作，数倍甚至十部以上优化了c#与lua之间的调用开销。
- 彻底解决传统lua方案wrapper极其庞大的问题。所有字段和Property访问直接按偏移访问内存，不需要生成wrapper函数，所有相同签名的函数调用共享同一个桥接函数。不再需要单独优化wrapper函数的问题。
- 专职维护。及时跟进Unity版本变化，支持lua 5.1 - 5.5、luajit等所有lua版本。快速响应解决bug。

## 特性

- 使用清晰的规则统一了c#与lua之间的双向调用，并且**几乎**支持lua调用c#的所有功能：包括class对象访问、struct对象访问、静态成员访问、成员函数和静态函数调用、泛型类、泛型函数调用，数组，函数重载访问，函数回调，协程、以及支持ref\out\in之类的函数参数等。
- 极致易用优雅。 只需用`[LuaInvoke]` 标记c#函数，自动实现调用代码。lua中访问c#类时自动注册meta table信息。无论在Editor下还是发布到il2cpp后，所有操作对开发者都是无感的。
- 极致高效。数倍甚至十倍以上优化了c#与lua之间的调用开销。在il2cpp中内嵌lua，抛开lua与c#之间交互绕开低效的C#接口，在c++层面让il2cpp和lua虚拟机直接相互操作。
  - 托管对象的UserData中直接包含了对象指针，相比于xlua之类传统方案十倍以上优化了this对象查找到开销。
  - 在c++层维护了lua引用的托管对象，并且注册到il2cpp的GCRoots。相比于xlua之类传统方案数倍优化了维护托管对象引用的开销。
  - 在c++层直接内存访问成员和静态成员变量。完全不需要经过低效的c# wrapper函数，十倍以上优化的字段访问的开销。
  - 在c++层直接调用Property函数，完全不需要经过低效的c# wrapper函数，大幅减少了属性的访问开销。
  - 在c++层直接调用c#函数，完全不经过低效的c# wrapper函数，大幅减少了调用开销。
- 将简单的`int X {get; set;}`之类的Property访问重构为字段访问，十倍以上减少了访问开销。
- 彻底解决传统lua方案wrapper极其庞大的问题。
  - 不生成任何c# wrapper函数，仅生成紧凑的c++ 桥接函数。
  - 不需要为field和property生成任何桥接函数
  - 相同签名的函数共享同一个桥接函数，极大减少了桥接函数的大小
- **TODO** 深度统一bdwgc 和 lua的gc系统。彻底解决il2cpp和lua循环引用引发的内存泄露问题及维护引用的高额开销。

## 支持的版本和平台

- 支持luajit(5.1)及lua 5.3+版本
- 支持 unity 2021+版本及团结引擎
- 支持 mono、il2cpp backend。
- 支持il2cpp支持的所有平台（含webgl、微信小游戏及团结引擎支持的鸿蒙和车机平台）。

## 示例

示例项目 [NovaLua-Demo](https://github.com/focus-creative-games/novalua-demo)。

极致简单：

- **不需要任何配置**，仅需要初始化时设置LuaLoader即可。
- 定义`static extern` c#函数用于调用lua函数，添加`[LuaInvoke("app", "main")]`指定要调用的lua函数。
- 通过`CSharp.{assembly name}.{full type name}`访问c#类型，操作跟c#代码中调用c#完全一致。
- 通过`obj.x`访问成员变量和属性，通过`{class}.x`访问静态成员变量和属性
- 通过`obj:foo(..)` 调用成员函数，通过`{class}.foo(...)`调用静态成员函数

```csharp
public class Bootstrap : MonoBehaviour
{
    private static string LoadLuaModule(string module)
    {
        string path = Path.Combine(Application.streamingAssetsPath, "LuaScripts", module + ".lua.txt");
        return File.Exists(path) ? File.ReadAllText(path, Encoding.UTF8) : null;
    }
    
    [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.BeforeSceneLoad)]
    private static void InitNovaLuaOnStartup()
    {
        LuaAppDomain.Initialize(LoadLuaModule);
    }

    [LuaInvoke("app", "main")]
    private static extern void AppMain();

    [LuaInvoke("app", "add")]
    private static extern int AppAdd(int a, int b);

    void Start()
    {
        AppMain();
        int value = AppAdd(10, 20);
        Debug.Log($"AppAdd(10,20)={value}");
    }
}

public class Demo
{
    public static int s_x;

    public static int GetSX()
    {
        return s_x;
    }

    public static void SetSX(int value)
    {
        s_x = value;
    }

    public int x;

    public static int Add(int a, int b)
    {
        return a + b;
    }

    public static int Multi(int a, int b)
    {
        return a * b;
    }

    public int GetX()
    {
        return x;
    }

    public void SetX(int value)
    {
        x = value;
    }

    public void Run(int value)
    {
        x = value;
    }

    public void Run(string value)
    {
        x = value == null ? 0 : value.Length;
    }
}
```

```lua
CSharp['AC'] = CSharp['Assembly-CSharp']


function add(a, b)
    return a + b
end

function concat(a, b)
    return a .. b
end

function print_mt(t)
    for k, v in pairs(t) do
        print("===", k, v)
    end
    print(t.Add(3, 4))
    print(t.Multi(3, 4))
end

local function test_call_static_method()
    print("[test_call_static_method] start")
    print("Demo.Add:", CSharp.AC.Demo.Add(3, 5))
    print("Demo.Multi:", CSharp.AC.Demo.Multi(3, 5))
end

local function test_call_instance_method()
    print("[test_call_instance_method] start")
    local demo = CSharp.AC.Demo()
    print("Demo:GetX():", demo:GetX())
end

local function test_access_instance_field()
    print("[test_access_instance_field] start")
    local demo = CSharp.AC.Demo()
    demo:SetX(10)
    local x = demo.x
    assert(x == 10)
    print("x:", x)

    demo.x = 20
    local new_x = demo:GetX()
    print("After set x:", new_x)
    assert(new_x == 20)
end

local function test_access_static_field()
    print("[test_access_static_field] start")
    CSharp.AC.Demo.s_x = 10
    local x = CSharp.AC.Demo.GetSX()
    assert(x == 10)
    print("x:", x)
end

local function test_overload_signature()
    print("[test_overload_signature] start")
    local demo = CSharp.AC.Demo()
    local sig_run_i32 = novalua.signature("Run", novalua.corlibtypes.int32)
    demo[sig_run_i32](demo, 10)
    print("After Run(int):", demo:GetX())

    local run_i32 = novalua.get_method(demo, sig_run_i32)
    novalua.register_method(demo, "run_i32", run_i32)
    demo:run_i32(20)
    print("After run_i32 alias:", demo:GetX())
end

local function main()
    print("lua main start")
    test_call_static_method()
    test_call_instance_method()
    test_access_instance_field()
    test_access_static_field()
    -- test_overload_signature()
end

local function add(a, b)
    return a + b
end

return {
    main = main,
    add = add,
}
```

## 当前状态

目前还处于早期阶段，仅在Unity 2022.3.62f3版本、lua 5.4版本上测试通过。目前仅支持基础的交互，功能远未完善。
预计在2026年8月发布正式版本。

## 许可证

NovaLua 采用 MIT 许可证发布，欢迎自由使用、修改和分发。

## 联系我们

如有问题、建议或错误报告，请在用以下方式联系我们：

- GitHub 上提交 Issue
- 邮件联系维护者：`novalua#code-philosophy.com`
- QQ群 **NovaLua交流群**： 824793773
- discord频道 `https://discord.gg/htmr44jW6A`
