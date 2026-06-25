# ZLua 与 xLua 对比分析

本文档从 **架构、调用路径、理论开销** 三方面对比 ZLua 与 xLua（Tencent，Unity Il2Cpp 常用方案）。用于技术选型与设计评审。

**说明：**

- 开销数据为 **路径级推演 + 经验量级**，非仓库内 benchmark。
- ZLua 对比对象为其 **Player / Il2Cpp 设计目标**；当前实现仍为 MVP（见 `IL2CPP_DESIGN_SPEC.md`）。
- 性能对比 **仅对 Player Il2Cpp 有意义**；ZLua Editor（Mono 反射）通常 **不比 xLua 快**。

---

## 1. 定位概览

| 维度 | xLua | ZLua |
|------|------|---------|
| 成熟度 | 生产级，生态大 | 早期，规范为主 |
| Lua 引擎 | 独立 **libxlua**（动态库 / P/Invoke） | 链入 **libil2cpp**（与 VM 同二进制） |
| Lua→C# 桥 | 生成 **C# Wrap** + `LuaDLL` | **C++ MethodBridge** + `methodPointer` |
| C#→Lua | C# `LuaEnv` + 多次 `LuaDLL` | `[LuaInvoke]` **InternalCall** + C++ 模板 |
| 字段访问 | 多数经生成 Wrap / 属性 | 规范：`__index` **offset 直读**（Il2Cpp） |
| 维护成本 | 插件升级，少改 libil2cpp | **fork libil2cpp**，Unity 版本需 merge |
| 易用性 | 文档、工具链完善 | 目标「无感」+ 与 C# 语义对齐 |

```text
浅 ←────────────────────────────────────────→ 深（Il2Cpp 侵入）

反射绑定 / 纯 C# 桥
  xLua（libxlua + 生成 Wrap）
    ★ ZLua 目标
      HybridCLR 级 VM 改造
```

ZLua 与 xLua 同属 **高性能原生绑定** 路线，而非 HybridCLR 式执行模型改造。

---

## 2. 调用路径对比

### 2.1 Lua → C#：例 `Demo.Add(3, 5)` / `demo:GetX()`

**xLua**

```text
lua_pcall
  → [MonoPInvokeCallback] 生成的 C# Wrap（Il2Cpp AOT 机器码，入口无托管调度）
      → LuaDLL.lua_tointeger / lua_push*（P/Invoke → libxlua）
      → 回到 Wrap
      → 调用目标 C# 方法（Il2Cpp 生成代码）
      → 再经 LuaDLL 写回栈
  → （实例方法）ObjectTranslator：userdata → pool index → object
```

**ZLua**

```text
lua_pcall
  → C++ MethodBridge（与 lua 同链接域）
      → lua_tointeger / lua_push*（直接 lua API）
      → method->methodPointer(...)（Il2Cpp 生成体同级）
  → ObjectRegistry：userdata → slot（弱表缓存）
```

ZLua MVP 示例（`MethodBridge.cpp`）对 `Add(int,int)` 即为：`lua_tointeger` ×2 → `methodPointer` → `lua_pushinteger`，无 Wrap、无 libxlua 往返。

### 2.2 C# → Lua：例 `[LuaInvoke]` / 脚本回调

**xLua**

```text
C# 业务代码
  → LuaEnv / DelegateBridge（C#）
  → 多次 LuaDLL（getglobal、push、pcall、pop…）
  → libxlua → lua VM
```

单次调用常见 **5–15 次** LuaDLL 跨界。

**ZLua**

```text
C# [LuaInvoke] → InternalCall（一次进 native）
  → LuaInvokeRuntime::Call（C++ 模板，内联）
      → lua_rawgeti(funcRef) + PushDefault* + lua_pcall + Pop
```

构建期解析 `moduleRef` / `funcRef`，运行时无按名 `getglobal`。

### 2.3 Lua function → C# Delegate

| | xLua | ZLua |
|---|------|---------|
| 机制 | C# delegate bridge + translator + LuaDLL | `LuaMethod` + `funcRef` + **closed delegate** + C++ `DelegateBridge` |
| 脚本侧 | 传 function，内部转换 | **隐式 marshal**（方法形参，同其他类型） |
| 详见 | xLua `lua_csfunction` / `DelegateBridge` | `FUNCTION_MARSHAL_SPEC.md` §4.0 |

绑定完成后，两者都要走「C# 调 invoke → 进 Lua `pcall`」；ZLua 少 C# Wrap / LuaDLL 层，但 **string / 重对象** 仍占主导。

---

## 3. 分项开销（理论量级）

**环境假设：** ARM64 / x64、Release、热循环、简单 blittable 签名。单位为 **纳秒级区间**，仅供推演。

| 成本项 | xLua (Il2Cpp) | ZLua (目标) | 备注 |
|--------|---------------|----------------|------|
| Lua 进入 C 回调 | ~5–15 | ~5–15 | 同在 lua VM 内 |
| 栈读写单次 | ~30–150（经 LuaDLL） | ~3–15（内联 lua API） | **主要差距来源** |
| C#↔native 边界 | ~20–80 / 次 | 0 或 IC 1 次 | xLua 栈操作多次跨界 |
| userdata → 对象指针 | ~20–80 | ~10–40 | 均为池 + 缓存思路 |
| 方法分派 | ~0–30（Wrap 常写死） | ~0–50（map / dispatch） | 重载 dispatch 两边都可能贵 |
| 调用 Il2Cpp 方法体 | ~1–5 | ~1–5 | 同为 `methodPointer` |
| 读 `int` 实例字段 | 常经 Wrap，~150–500+ | offset 直读 ~15–50 | **字段差距最大** |
| `string` 往返 | ~200–2000+ | 类似 | UTF 转换 + GC，瓶颈一致 |

---

## 4. 分场景理论估计

定义：

- **总耗时倍数** = T_xLua / T_ZLua（越大 ZLua 越快）
- **互调开销削减** ≈ (T_xLua − T_nova) / (T_xLua − T_函数体)，适用于极短函数体

### 场景 A：热循环 `int Add(int a, int b)`（静态）

| 指标 | 估计 |
|------|------|
| xLua 粗算 | ~150–600 ns/次 |
| ZLua 粗算 | ~30–100 ns/次 |
| 总耗时 | **约 2–5×**（常见 ~3×） |
| 互调开销削减 | **约 70%–90%** |

### 场景 B：实例 `int GetX()`（无参）

| 指标 | 估计 |
|------|------|
| 总耗时 | **约 3–6×** |
| 互调开销削减 | **约 75%–90%** |

### 场景 C：实例字段 `obj.x`（`int`）

| 指标 | 估计 |
|------|------|
| xLua | `__index` → Wrap → getter |
| ZLua | `__index` → C++ offset getter |
| 总耗时 | **约 5–15×** |
| 互调开销削减 | **约 85%–95%** |

### 场景 D：C# → Lua，`void Foo(int)` / `int Bar(int,int)`

| 指标 | 估计 |
|------|------|
| 总耗时 | **约 2–4×** |
| 互调开销削减 | **约 60%–85%** |

参数越多，xLua「每次 push 都跨界」放大越明显。

### 场景 E：Delegate 回调（已绑定后）

| 指标 | 估计 |
|------|------|
| 总耗时 | **约 2–3×** |
| 互调开销削减 | **约 50%–75%** |

首次绑定、泛型约束、签名未 codegen 时两边都贵。

### 场景 F：含 `string`、复杂 object / `List`

| 指标 | 估计 |
|------|------|
| 总耗时 | **约 1.1–1.8×** |
| 互调开销削减 | **约 20%–50%** |

marshal 与分配占主导，固定边界成本占比下降。

### 汇总表（Player / 轻量签名）

| 场景 | 总耗时倍数 (xLua/ZLua) | 互调开销大约减少 |
|------|---------------------------|------------------|
| Lua→C# `int,int→int` | **2–5×** | **70%–90%** |
| Lua→C# 实例 getter | **3–6×** | **75%–90%** |
| Lua 读 `int` 字段 | **5–15×** | **85%–95%** |
| C#→Lua 少参函数 | **2–4×** | **60%–85%** |
| Delegate 回调 | **2–3×** | **50%–75%** |
| 重 string / 对象 API | **1.1–1.8×** | **20%–50%** |

**保守结论（热路径、blittable、高频）：**

> 互调固定开销有机会减少 **约 50%–90%**；端到端常见 **快 2–5 倍**；字段直读等路径可更高。

---

## 5. 为何 ZLua 能更快（根因）

| 根因 | 说明 |
|------|------|
| 消灭 libxlua 往返 | 栈操作不再每次 P/Invoke |
| 消灭生成 C# Wrap 层 | Lua→C# 在 C++ 一次完成 marshal + `methodPointer` |
| C#→Lua 单次 IC | 非 C# 循环调 LuaDLL |
| 字段 / 无参属性快路径 | 与 Il2Cpp 生成 C++ 同级内存访问 |
| 签名复用桥 | 同 HybridCLR 思路，控制代码体积 |

**不是**单一「Il2Cpp 黑科技」值 10 倍，而是 **减少重复的 native↔managed↔native 折返**。

---

## 6. Il2Cpp 侵入性对比

| 层级 | xLua | ZLua |
|------|------|---------|
| 插件形态 | 独立 native + 生成 C# | **嵌入 libil2cpp** + 少量 Runtime/GC 挂钩 |
| 方法调用 | Wrap + LuaDLL | `methodPointer` 直调 |
| Delegate | xLua 自有 bridge | `SetClosedDelegateInvoke` + 生成 bridge |
| GC | 一般不改 Boehm | non-blittable struct 可能 hook `push_other_roots` |
| 升级 Unity | 主要升 xLua 包 | **merge libil2cpp 补丁**（工程债更大） |

ZLua **刻意避免**运行时伪造完整 `MethodInfo`（见 `FUNCTION_MARSHAL_SPEC.md`），在技巧深度上有所克制。

---

## 7. 功能与工程面对比（非性能）

| 维度 | xLua | ZLua |
|------|------|---------|
| 文档 / 社区 | 强 | 建设中 |
| 代码生成 | 成熟 XLua Generate | Codegen + Weaver（`[LuaInvoke]`） |
| 热更配套 | 大量现成实践 | 需自建 |
| 语义模型 | xLua 自有约定 | 目标贴近 C#（`TYPE_SYSTEM_SPEC`） |
| Editor 体验 | 与 Player 较一致 | Mono 反射 / Player 原生 **双轨** |
| 迁移 | — | 见 `DEV.md` 迁移指南（规划） |

---

## 8. 必须打折的因素

### 8.1 ZLua 实现阶段

当前 MVP 仅覆盖少量 `MethodBridge` 签名；以下尚未完全落地：

- 完整 overload dispatch  
- `ReadDelegate` / `DelegateBridges`  
- 字段 `__index` 快路径全覆盖  
- struct handle、GC root  

**上文倍数为规范落地后的 Player 理论值**，非今日 Demo 全状态。

### 8.2 ZLua 固定成本

- 首次 `EnsureBinding`、继承 promotion  
- 运行时重载 dispatch  
- `string`、non-blittable、泛型方法 inflation  

### 8.3 xLua 可优化空间

- 生成 Wrap 已避免反射  
- 可对热点减少 LuaDLL 调用次数  
- 架构上仍难消除 **libxlua 边界**

### 8.4 帧预算视角

互调即使快 **5 倍**，若脚本边界只占帧时间 2%，整帧仅省 **~1.6%**。  
**务必先 profiling** 确认 Lua↔C# 是否为热点（战斗公式、UI 每帧上千次小调用等）。

---

## 9. 理论下限参考

同进程嵌入 Lua 调同签名 native 函数的下限约：

```text
lua 回调 + 2×读栈 + 1×写栈 + methodPointer ≈ 20–60 ns
```

ZLua 热路径 **接近** 该下限；xLua 因多次 LuaDLL，通常 **高一个数量级**。

---

## 10. 建议基准测试（待实测）

在 **相同 Il2Cpp Player** 下对比 xLua 与 ZLua：

```lua
-- 1. Lua → C# 静态
for i = 1, 1000000 do Demo.Add(1, 2) end

-- 2. Lua → C# 实例方法
for i = 1, 1000000 do o:GetX() end

-- 3. Lua 读字段
for i = 1, 1000000 do local _ = o.x end

-- 4. C# 循环 [LuaInvoke] 空调用 / int 返回

-- 5. C# 持 Action，循环回调 Lua function
```

记录：**ns/call**、**GC Alloc/帧**。预期与 §4 趋势一致；实测后在本文件追加「实测数据」一节。

---

## 11. 选型建议（简要）

| 选 xLua | 选 ZLua（目标态） |
|---------|---------------------|
| 现在要上线、要少踩坑 | 性能边界是瓶颈且愿维护 libil2cpp fork |
| 团队已有 xLua 资产 | 希望语义与 C# 统一、极致 Player 性能 |
| 不愿改 Unity 引擎层 | 可投入 Codegen + 双运行时测试矩阵 |

两者 **不是** 简单替换关系；迁移成本见后续迁移文档。

---

## 12. 相关文档

| 文档 | 内容 |
|------|------|
| `DESIGN_SPEC.md` | ZLua 总体目标 |
| `IL2CPP_DESIGN_SPEC.md` | Player 架构、LuaInvoke、MethodBridge |
| `TYPE_SYSTEM_SPEC.md` | 元表、字段快路径 |
| `FUNCTION_MARSHAL_SPEC.md` | Delegate / 函数 marshal |
| `STRUCT_MARSHAL_SPEC.md` | struct 零 GC 路径 |

---

*文档版本：理论分析稿；实测数据待补充。*
