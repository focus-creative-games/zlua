# ZLua

ZLua是一个针对Il2Cpp极致优化的现代、简洁、易用的Unity Lua脚本方案。

- 文档：[doc.zlua.cn](https://doc.zlua.cn)
- Demo：[zlua-demo](https://github.com/focus-creative-games/zlua-demo)
- 选型长文：[为什么选择 ZLua](https://doc.zlua.cn/docs/concepts/why-zlua)

---

## 为什么选择 ZLua

相对 xLua / toLua / SLua，ZLua 的核心主张是：

| | |
|--|--|
| **更易用** | 设计更现代；极度简单；**零配置**（无需 per-type C# Wrap 白名单） |
| **更完备** | 标准和完备的 C#↔Lua 交互，几乎覆盖全部常用 C# 特性 |
| **更高效** | Il2Cpp 实测：约 **98%** 对齐用例快于 xLua；Lua→C# 平均约 **2.62×**；常见字段 / 属性 / 调用约 **4×** |
| **更少 GC** | 引用类型与 struct（含含引用字段的 struct）默认 **0 GC**；另有 OpaqueValue 等策略 |
| **极小桥接** | 同签名合并的高效 **C++** stub；体积可小一个数量级；支持 **0 桥接函数** |
| **版本更广** | Lua **5.1–5.5**、**LuaJIT**；Unity **2021+**、**团结引擎** |
| **维护更积极** | 全职专业团队；Bug 响应与特性迭代更快 |

性能细节见 [性能对比](https://doc.zlua.cn/docs/compare/PERFORMANCE)。

---

## 特性

### 互操作模型

- **C# → Lua**：`[LuaInvoke("module", "fn")]` + `static extern`，类比 P/Invoke
- **编组覆盖**：`[LuaMarshalAs]`
- **Lua → C#**：`CSharp` 根表懒加载类型；语法贴近 C#（`Type.Static()` / `obj:Method()` / `obj.field`）
- **双后端**：Editor **Mono** 与 Player **Il2Cpp** Lua 可见语义一致

### 完备的 C# 能力

包括但不限于：class / struct / interface / enum / nullable、静态与实例成员、泛型类与泛型方法、delegate、数组、方法重载、`ref` / `out` / `in`、Event（`add_` / `remove_`）等。

### Il2Cpp 性能与体积

- 内嵌 Lua，C++ 直桥：`methodPointer` / 字段 offset，**无**海量 C# Wrap
- 托管对象 userdata 直含对象指针；GCRoots 在 native 侧维护
- **同签名桥接函数合并**；字段 / 无参属性可不生成独立桥接
- 发版前执行 **`ZLua/Generate/All`**（生成 **C++ stub**，不是 xLua 式 C# Wrap）

### 平台与版本

- Lua 5.1 / 5.3 / 5.4 / 5.5、LuaJIT（主验证线：Lua **5.4**）
- Unity **2021+** LTS、团结引擎；Mono + Il2Cpp
- Il2Cpp 支持的平台（含 WebGL、微信小游戏、鸿蒙 / 车机等）

---

## 快速示例

完整可运行工程见 [zlua-demo](https://github.com/focus-creative-games/zlua-demo)。下面三步体现 ZLua 的优点：**零 Wrap 配置**、**声明式调 Lua**、**Lua 侧像写 C#**。

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

### 2. C# 调用 Lua（`[LuaInvoke]`）

```csharp
public class GameEntry : MonoBehaviour
{
    [LuaInvoke("app", "add")]
    static extern int AppAdd(int a, int b);

    void Start()
    {
        Debug.Log(AppAdd(10, 20)); // 30
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

**Player 发布：** 执行菜单 **`ZLua/Generate/All`**，并将 Lua 同步到 `StreamingAssets`（Demo 工程已含同步脚本）。

---

## 当前状态

| 运行时 | 状态 |
|--------|------|
| **Il2Cpp（Player）** | 已完成 |
| **Mono（Editor）** | 已完成（与 Il2Cpp 语义一致） |

主验证环境：Unity **2022.3.62f3** + Lua **5.4**。功能与语义以 [规范文档](https://doc.zlua.cn/docs/spec/00-OVERVIEW) 为准。

---

## 许可证

MIT。欢迎自由使用、修改和分发。

## 联系我们

- GitHub Issue：[focus-creative-games/zlua](https://github.com/focus-creative-games/zlua)
- 邮件：`zlua@code-philosophy.com`
- QQ 群 **ZLua 交流群**：824793773
- Discord：https://discord.gg/htmr44jW6A
