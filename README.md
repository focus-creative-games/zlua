# ZLua

ZLua是一个针对Il2Cpp极致优化的现代、简洁、易用的Unity Lua脚本方案。

- 文档：[doc.zlua.cn](https://doc.zlua.cn)
- Demo：[zlua-demo](https://github.com/focus-creative-games/zlua-demo)
- 性能基准：[zlua-benchmark](https://github.com/focus-creative-games/zlua-benchmark)
- 选型长文：[为什么选择 ZLua](https://doc.zlua.cn/docs/concepts/why-zlua)

---

## 为什么选择 ZLua

相对 xLua / toLua / SLua，ZLua 的核心优点是：

| | |
|--|--|
| **更易用** | 设计更现代；极度简单；**零配置**（无需 per-type C# Wrap 白名单） |
| **更完备** | 标准和完备的 C#↔Lua 交互，几乎覆盖全部常用 C# 特性 |
| **更高效** | Il2Cpp 实测（相对 zlua 的平均 ratio，>1 更慢）：Lua→C# 上 xLua ≈ **2.57×**、toLua ≈ **3.52×**、SLua ≈ **7.68×**；C#→Lua 上 xLua ≈ **1.59×**、toLua ≈ **3.27×**、SLua ≈ **14.9×** |
| **更少 GC** | 引用类型与 struct（含含引用字段的 struct）默认 **0 GC**；另有 OpaqueValue 等策略 |
| **极小桥接** | 合并相同签名的桥接函数，并且直接生成c++代码；生成的桥接函数大小相比xlua的wrapper文件总大小小一个数量级；支持 **0 桥接函数**，即使不生成任何桥接函数在大多数情况下性能仍然高于其他方案生成wrapper时的性能 |
| **版本更广** | Lua **5.1–5.5** 与 **LuaJIT 2.x**；Unity **2021.3 / 2022.3 / 6000.0 / 6000.3 / 6000.5**；**团结引擎**全系列 |
| **维护更积极** | 全职专业团队；Bug 响应与特性迭代更快 |

### 性能摘要（2026-07-28）

条件：Unity 2022.3.62f3，Il2Cpp Win64 Release；主指标 `mid_ns_per_op`；ratio = 其他框架 / zlua（>1 更慢）。公开仓库：[zlua-benchmark](https://github.com/focus-creative-games/zlua-benchmark)。

| direction | cases | xlua | tolua | slua |
|-----------|------:|-----:|------:|-----:|
| cs2lua（C#→Lua） | 54 | 1.59× | 3.27× | 14.9× |
| lua2cs（Lua→C#） | 231 | 2.57× | 3.52× | 7.68× |

相对 xLua：约 **98.6%** 用例领先（281/285）。完整报告：[comparison_20260728_121554.md](https://github.com/focus-creative-games/zlua-benchmark/blob/main/reports/comparison_20260728_121554.md)。文档侧说明见 [性能对比](https://doc.zlua.cn/docs/compare/PERFORMANCE)。

---

## 特性

- 极度易用，**零配置**，开箱即用。默认C#与lua之间既可任意相互调用，没有烦人的wrapper配置及各种复杂的配置
- 统一和完备的C#与lua交互，lua中能像C#那样原生访问几乎所有C#特性
- 显著高效：四方 Il2Cpp 基准见上表与 [zlua-benchmark](https://github.com/focus-creative-games/zlua-benchmark)
- 更少的GC，更统一和强大的参数传递策略，支持0GC传递任意对象，包括其他lua方案中不支持的含引用字段的struct类型。
- 极小的桥接函数，通过合并相同签名的桥接函数，并且直接生成c++代码，即使全量生成的桥接函数，总大小相比于其他方案wrapper文件翻译的c++代码至少小了一个数量级。而且支持**0**桥接函数，即使不生成任何桥接函数，交互效率在大多数情况下也高于其他方案生成wrapper的情形
- 内置支持 emmylua 调试
- 开发期Mono + 发布时Il2Cpp双runtime，良好平衡开发效率和运行效率

### 平台与版本

| 类别 | 已支持 |
|------|--------|
| **Lua（PUC-Rio）** | **5.1.x**、**5.2.x**、**5.3.x**、**5.4.x**、**5.5.x** |
| **LuaJIT** | **2.x**（Editor Mono；Il2Cpp **仅 iOS / Android** 发布） |
| **Unity** | **2021.3.x**、**2022.3.x**、**6000.0.x**、**6000.3.x**、**6000.5.x** |
| **团结引擎** | 全部 **x.y.z** |
| **运行时** | Editor **Mono** + Player **Il2Cpp** |
| **平台（PUC-Rio）** | Il2Cpp 支持的平台（含 WebGL、微信小游戏、鸿蒙 / 车机等） |
| **平台（LuaJIT）** | Editor；Player 仅 **iOS / Android**（需自备静态库，详见文档） |

---

## 快速示例

完整可运行工程见 [zlua-demo](https://github.com/focus-creative-games/zlua-demo)。

### 1. 初始化（只需 Loader）

```csharp
using System.IO;
using System.Text;
using UnityEngine;
using ZLua;

public static class ZLuaBootstrap
{
    static string LoadLuaModule(string module)
    {
#if UNITY_EDITOR
        var path = Path.Combine(Application.dataPath, "..", "LuaScripts", module + ".lua");
#else
        var path = Path.Combine(Application.streamingAssetsPath, "LuaScripts", module + ".lua.txt");
#endif
        return File.Exists(path) ? File.ReadAllText(path, Encoding.UTF8) : null;
    }

    [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.BeforeSceneLoad)]
    static void Init() => LuaAppDomain.Initialize(LoadLuaModule);
}
```

无需 `LuaCallCSharp` 列表，无需为每个类型 Generate C# Wrap。

### 2. C# 调用 Lua（`GetFunction`）

```csharp
public class GameEntry : MonoBehaviour
{
    void Start()
    {
        var add = LuaAppDomain.GetFunction<Func<int, int, int>>("app", "add");
        Debug.Log(add(10, 20)); // 30
    }
}
```

```lua
-- LuaScripts/app.lua
local function add(a, b)
    return a + b
end

return { add = add }
```

### 3. Lua 访问 C#（懒加载，语法贴近 C#）

```csharp
// 任意 public 类型，无需导出配置
public class Demo
{
    public int x;
    public static int Add(int a, int b) => a + b;
    public void SetX(int v) => x = v;
}
```

```lua
CSharp['AC'] = CSharp['Assembly-CSharp']

print(CSharp.AC.Demo.Add(3, 5))   -- 静态方法

local d = CSharp.AC.Demo()        -- 构造
d.x = 10                          -- 字段
d:SetX(20)                        -- 实例方法
print(d.x)
```

---

## 许可证

MIT。欢迎自由使用、修改和分发。

## 联系我们

- GitHub Issue：[focus-creative-games/zlua](https://github.com/focus-creative-games/zlua)
- 邮件：`zlua@code-philosophy.com`
- QQ 群 **ZLua 交流群**：824793773
- Discord：<https://discord.gg/htmr44jW6A>
