# ZLua

ZLua是一个针对Unity Il2Cpp 极致优化的现代原生lua方案。

**完整文档：** [https://doc.zlua.cn](https://doc.zlua.cn)

## 为什么选择ZLua

- 极致易用简洁优雅，统一了c#与lua双向调用。把Lua当作另一种`Native`，开创性清晰提出 `[LuaInvoke]`、`[LuaCallback]`、 `[LuaMarshalAs]`的概念。对开发者完全屏蔽了底层复杂易错的Lua的操作。
- 极致高效。在il2cpp中内嵌lua，绕开lua与c#之间低效的C#交互接口，在c++层面让il2cpp和lua虚拟机直接相互操作，数倍甚至十部以上优化了c#与lua之间的调用开销。
- 彻底解决传统lua方案wrapper极其庞大的问题。所有字段和Property访问直接按偏移访问内存，不需要生成wrapper函数，所有相同签名的函数调用共享同一个桥接函数。不再需要单独优化wrapper函数的问题。
- 专职维护。及时跟进Unity版本变化，支持lua 5.1 - 5.5、luajit等所有lua版本。快速响应解决bug。

→ [为什么选择 ZLua](https://doc.zlua.cn/docs/concepts/why-zlua) · [与 xLua 对比](https://doc.zlua.cn/docs/concepts/comparison-with-xlua)

## 特性

- 使用清晰的规则统一了c#与lua之间的双向调用，并且**几乎**支持lua调用c#的所有功能：包括class对象访问、struct对象访问、静态成员访问、成员函数和静态函数调用、泛型类、泛型函数调用，数组，函数重载访问，函数回调，协程、以及支持ref\out\in之类的函数参数等。
- 极致易用优雅。 只需用`[LuaInvoke]` 标记c#函数，自动实现调用代码。lua中访问c#类时自动注册meta table信息。无论在Editor下还是发布到il2cpp后，所有操作对开发者都是无感的。
- 极致高效。数倍甚至十倍以上优化了c#与lua之间的调用开销。在il2cpp中内嵌lua，绕开lua与c#之间低效的C#交互接口，在c++层面让il2cpp和lua虚拟机直接相互操作。
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

## 文档

官方文档站：[https://doc.zlua.cn](https://doc.zlua.cn)

| 分类 | 链接 |
|------|------|
| 介绍 | [docs/intro](https://doc.zlua.cn/docs/intro) |
| 入门 | [快速开始](https://doc.zlua.cn/docs/getting-started/quick-start) · [安装与集成](https://doc.zlua.cn/docs/getting-started/installation) · [项目状态](https://doc.zlua.cn/docs/getting-started/project-status) · [兼容性](https://doc.zlua.cn/docs/getting-started/compatibility) |
| 使用指南 | [C# 调用 Lua](https://doc.zlua.cn/docs/guides/csharp-to-lua) · [Lua 访问 C#](https://doc.zlua.cn/docs/guides/lua-to-csharp-basics) · [全部指南](https://doc.zlua.cn/docs/category/guides) |
| 核心概念 | [为什么选择 ZLua](https://doc.zlua.cn/docs/concepts/why-zlua) · [与 xLua 对比](https://doc.zlua.cn/docs/concepts/comparison-with-xlua) · [全部概念](https://doc.zlua.cn/docs/category/concepts) |
| API 参考 | [概览](https://doc.zlua.cn/docs/reference/overview) · [LuaInvoke](https://doc.zlua.cn/docs/reference/csharp/lua-invoke) · [编组速查](https://doc.zlua.cn/docs/reference/marshal-cheatsheet) · [全部参考](https://doc.zlua.cn/docs/category/reference) |
| 架构与性能 | [调用路径概览](https://doc.zlua.cn/docs/architecture/call-path-overview) · [Il2Cpp 架构](https://doc.zlua.cn/docs/architecture/il2cpp-architecture) · [全部架构](https://doc.zlua.cn/docs/category/architecture) |
| 规范文档 | [设计规范](https://doc.zlua.cn/docs/spec/design-spec) · [全部规范](https://doc.zlua.cn/docs/category/spec) |
| 社区 | [FAQ](https://doc.zlua.cn/docs/community/faq) · [路线图](https://doc.zlua.cn/docs/community/roadmap) · [贡献指南](https://doc.zlua.cn/docs/community/contributing) · [联系与支持](https://doc.zlua.cn/docs/community/contact) |

## 支持的版本和平台

- 支持Lua 5.1、Lua 5.3、Lua 5.4、Lua 5.5及LuaJIT
- 支持[Luau](https://luau.org/), Luau is a small, fast, and embeddable programming language based on Lua with a gradual type system.
- 支持 unity 2021+ LTS版本及团结引擎 LTS版本
- 支持 mono、il2cpp backend。
- 支持il2cpp支持的所有平台（含webgl、微信小游戏及团结引擎支持的鸿蒙和车机平台）。

→ [兼容性说明](https://doc.zlua.cn/docs/getting-started/compatibility)

## 示例

示例项目 [ZLua-Demo](https://github.com/focus-creative-games/zlua-demo)。

→ [快速开始](https://doc.zlua.cn/docs/getting-started/quick-start) · [安装与集成](https://doc.zlua.cn/docs/getting-started/installation)

下表汇总 ZLua 常见用法（**不需要任何 wrap 配置**，初始化时设置 `LoadLuaModule` 即可）。完整可运行工程见 [zlua-demo](https://github.com/focus-creative-games/zlua-demo)。

| 方向 | 用法 | 示例代码 | 文档 |
|------|------|----------|------|
| 初始化 | 启动时注册 Lua 加载器 | C#: `[RuntimeInitializeOnLoadMethod] static void Init() { LuaAppDomain.Initialize(LoadLuaModule); }` | [快速开始](https://doc.zlua.cn/docs/getting-started/quick-start) |
| C# → Lua | 调用无参 Lua 函数 | C#: `[LuaInvoke("app", "main")] static extern void AppMain();` → `AppMain()` | [C# 调用 Lua](https://doc.zlua.cn/docs/guides/csharp-to-lua) |
| C# → Lua | 带参 / 返回值 | C#: `[LuaInvoke("app", "add")] static extern int AppAdd(int a, int b);` → `AppAdd(10, 20)`<br>Lua: `local function add(a, b) return a + b end` | [C# 调用 Lua](https://doc.zlua.cn/docs/guides/csharp-to-lua) |
| Lua 模块 | 导出函数表 | Lua: `return { main = main, add = add }` | [模块加载](https://doc.zlua.cn/docs/guides/lua-module-loading) |
| Lua → C# | 程序集短别名 | Lua: `CSharp['AC'] = CSharp['Assembly-CSharp']` | [Lua 访问 C#](https://doc.zlua.cn/docs/guides/lua-to-csharp-basics) |
| Lua → C# | 访问类型 | Lua: `CSharp.AC.Demo` · `CSharp.AC['MyGame.UI.Panel']` | [Lua 访问 C#](https://doc.zlua.cn/docs/guides/lua-to-csharp-basics) |
| Lua → C# | 构造实例 | Lua: `local demo = CSharp.AC.Demo()` | [Lua 访问 C#](https://doc.zlua.cn/docs/guides/lua-to-csharp-basics) |
| Lua → C# | 静态方法 | Lua: `CSharp.AC.Demo.Add(3, 5)` | [Lua 访问 C#](https://doc.zlua.cn/docs/guides/lua-to-csharp-basics) |
| Lua → C# | 静态字段 / Property | Lua: `CSharp.AC.Demo.s_x = 10` · `CSharp.AC.Demo.GetSX()` | [字段与 Property](https://doc.zlua.cn/docs/guides/fields-and-properties) |
| Lua → C# | 实例方法 | Lua: `demo:SetX(10)` · `demo:Run(10)` | [Lua 访问 C#](https://doc.zlua.cn/docs/guides/lua-to-csharp-basics) |
| Lua → C# | 实例字段 / Property | Lua: `demo.x = 20` · `print(demo.x)` | [字段与 Property](https://doc.zlua.cn/docs/guides/fields-and-properties) |
| Lua → C# | 方法重载 | Lua: `local m = zlua.get_method(demo, "Run", zlua.signature(zlua.types.int32), false)`<br>`m(demo, 10)` | [方法重载](https://doc.zlua.cn/docs/guides/methods-and-overloads) |
| C# 侧 | 重载别名 | C#: `[LuaAlias("RunI32")] public void Run(int v) { ... }`<br>Lua: `demo:RunI32(10)` | [方法重载](https://doc.zlua.cn/docs/guides/methods-and-overloads) |
| Lua → C# | Lua 作 delegate | Lua: `host:RegisterCallback(function(v) print(v) end)` | [回调与 Delegate](https://doc.zlua.cn/docs/guides/callbacks-and-delegates) |
| Lua → C# | ref / out / in | Lua: `local n = zlua.new_ref(zlua.types.int32, 5)`<br>`Counter.Increment(n)` | [ref/out/in](https://doc.zlua.cn/docs/guides/marshal-ref-out-in) |
| Lua → C# | 泛型 List | Lua: ``local ListInt = zlua.make_generic_type(CSharp.mscorlib['System.Collections.Generic.List`1'], zlua.types.int32); local list = ListInt(); list:Add(10)`` | [泛型与数组](https://doc.zlua.cn/docs/guides/generics-and-arrays) |
| Lua → C# | 数组 | Lua: `local arr = zlua.new_szarray_by_element_type(zlua.types.int32, 4)`<br>`arr[0] = 10` | [泛型与数组](https://doc.zlua.cn/docs/guides/generics-and-arrays) |
| Lua → C# | 枚举 | Lua: `local Color = CSharp.AC['MyGame.Color']`<br>`host:SetColor(Color.Red)` | [枚举与 struct](https://doc.zlua.cn/docs/guides/enums-and-structs) |
| Lua → C# | struct | Lua: `local Point2D = CSharp.AC['MyGame.Point2D']`<br>`local p = Point2D(1, 2)` | [枚举与 struct](https://doc.zlua.cn/docs/guides/enums-and-structs) |
| Lua → C# | Event 订阅 | Lua: `local h = function(v) print(v) end`<br>`EventPublisher.OnGlobalTick.get(h)` | [Event](https://doc.zlua.cn/docs/guides/events) |
| 编组 | 自定义 Marshal | C#: `void Send([LuaMarshalAs(LuaMarshalType.Bytes)] byte[] data)` | [LuaMarshalAs](https://doc.zlua.cn/docs/reference/csharp/lua-marshal-as) |

## 当前状态

目前还处于早期阶段，仅在Unity 2022.3.62f3版本、lua 5.4版本上测试通过。目前仅支持基础的交互，功能远未完善。
预计在2026年8月发布正式版本。

→ [项目状态](https://doc.zlua.cn/docs/getting-started/project-status) · [路线图](https://doc.zlua.cn/docs/community/roadmap)

## 许可证

ZLua 采用 MIT 许可证发布，欢迎自由使用、修改和分发。

## 联系我们

如有问题、建议或错误报告，请在用以下方式联系我们：

- [GitHub Issues](https://github.com/focus-creative-games/zlua/issues)
- 邮件联系维护者：`zlua#code-philosophy.com`
- QQ群 **ZLua交流群**： 824793773
- [Discord](https://discord.gg/htmr44jW6A)
- [联系与 FAQ](https://doc.zlua.cn/docs/community/contact)
