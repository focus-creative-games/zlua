# ZLua Mono 热点优化报告（对照 xLua 源码核实版）

依据 `d:\workspace\zlua\3rd\xLua` 源码与 ZLua 当前实现（P1–P7 + Phase 1 元表直连）逐项核对。

**结论：** ZLua fast path 应对标 xLua **Editor CodeEmit 档（档 B）**；全量预生成 Wrap（档 A）为长期目标。P0 已完成（§5）；P1 · 4.5–4.7 已完成（§6）。

---

## 0. 核实结论：常见误解修正

| 先前说法 | 对照 xLua 源码后 |
|----------|------------------|
| xLua 一律「预生成 Wrap + 直接调用、无反射」 | **不准确**。xLua 有 **三档路径**（§1） |
| xLua 重载分派远优于 ZLua | **部分准确**。生成/CodeEmit 与 ZLua 类似；ReflectionWrap 也是 Check 链 + Invoke |
| ZLua 方法调用明显弱于 xLua | **需分场景**。Editor lazy 路径下 ZLua ≈ xLua **CodeEmit 档** |
| xLua 字段访问总是 offset 直读 | **不准确**。生成 Wrap 直读；ReflectionWrap 用 `field.GetValue` |
| xLua Delegate 全无 object[] | **不准确**。`MethodWrapsCache` 仍用 object[] + Invoke；仅 DelegateBridge 生成为 typed push |

---

## 1. xLua 三档调用路径

### 档 A：预生成 Wrap（`[LuaCallCSharp]` + Generator）

- 模板 `LuaClassWrap.tpl.txt`：直接 `gen_to_be_invoked.Method(...)` / typed push/pop
- `__index` 由 libxlua `gen_obj_indexer`（C）组合 method 表 + getter 表

### 档 B：Editor 运行时 CodeEmit（`TryDelayWrapLoader` → `EmitTypeWrap`）

- IL `Call` / `Callvirt` / `Newobj`，无 `MethodInfo.Invoke`
- 与 ZLua `Expression.Compile` **架构同级**

### 档 C：ReflectionWrap / MethodWrapsCache

- `OverloadMethodWrap`：**object[] + Invoke + PushAny**
- 单 overload 无默认参数时可跳过 Check 链

**对比含义：** ZLua fast path 对标档 B；档 C 下 ZLua 通常 **≥ xLua**。

---

## 2. 核心热点核实摘要

| 热点 | vs xLua 档 B | 主要差距 |
|------|--------------|----------|
| Lua→C# 单方法 | 85%–95% | EnsureConvertible、返回值 boxing、scope 开销 |
| Lua→C# 重载 | 85%–92% | 同 argc+lua_type 重载仍走 CanConvert 链 |
| 字段/属性读 | 70%–85% | P0 已挂 metatable getter；**仍经 `__index` C# 回调 + `tostring` key**（GC 热点，见 §3 · 4.8） |
| 构造函数 | 40%–55% | 仍 Invoke stub |
| C#→Lua Invoke | 92%–100% | error handler 已缓存 registry ref |
| Delegate→Lua | 75%–90% | P1 已 typed push/pop，无 object[] |

---

## 3. 优化方案与优先级

### P0（已完成）

| ID | 内容 | 参考 xLua |
|----|------|-----------|
| 4.1 | 构造函数 fast path 直连 | CodeEmit `Newobj` |
| 4.2 | 字段/属性 getter 挂 instance metatable | `obj_getter` + `obj_indexer` |
| 4.3 | InstanceIndex 去双次 GCHandle | getter 内单次 PopTarget |
| 4.4 | 编译桥 typed pop/push（校验与直读分离） | `fixPush` / `GetPushStatement` |

### P1

- 4.5 Delegate 桥去 object[] ✅
- 4.6 重载 `(argc, typeMask)` 快速表 ✅
- 4.7 C#→Lua 缓存 error handler ref ✅
- **4.8 obj_indexer 统一模型 + 成员访问零 C# string key** — 规格见 **`META_TABLE_SPEC.md`**（Mono）；VM 远期见 **`ZLUA_VM_INDEX_SPEC.md`**

#### 4.8 obj_indexer 统一模型 + 零 C# string key

**问题：** P0 虽将 getter/setter 挂到 instance metatable，但 userdata 成员读写仍走 C# `__index` / `__newindex`。每次访问在 `LuaManagerObject.InstanceIndex` 等路径上执行 `LuaDllExtension.tostring(L, 2)`（UTF-8 → 托管 string，GC）+ `Dictionary<FieldKey/PropertyKey>` 等 C# string 查表。高频 `obj.field` 场景下开销显著。

**原则：** hot path **禁止** `lua_tostring` → C# string → 字典查找；成员分派应尽量在 Lua/C 层完成，对标 xLua `gen_obj_indexer`（档 A）。

**涉及文件：** `LuaManagerObject.cs`（`InstanceIndex` / `InstanceNewIndex` / `StaticTypeIndex` / `StaticTypeNewIndex`）、`TypeFieldRegistration.cs`、`TypePropertyRegistration.cs`、注册期 metatable 构建逻辑；阶段 C 可选 native `zlua_obj_indexer`。

| 阶段 | 内容 | 预期收益 |
|------|------|----------|
| **A** | **Lua 栈 key 直查** | 去掉 hot path 每次 GC string + 字典查表 |
| **B** | **注册期 Lua string ref 缓存** | 动态/反射 key 首次 miss 后不再重复 alloc |
| **C（P1）** | **`__index` 模型对齐 xLua** | 已注册成员可能完全不经 C# 回调 |

**阶段 A — Lua 栈 key 直查（必做）**

- `InstanceIndex` / `InstanceNewIndex` / `StaticTypeIndex` / `StaticTypeNewIndex`：去掉首行 `tostring`；`lua_getmetatable` 后 `lua_pushvalue(L, keyIdx)` + `lua_rawget(mt)` 查成员。
- 新增 `RawGetFieldByStackKey(luaState, mt, keyStackIndex)`，替代 `RawGetField(luaState, mt, cSharpString)`（后者会把 C# string 再 push 进 Lua）。
- 命中 metatable 上的 compiled bridge 后直接 `lua_call`，不再 `IsInstanceFieldGetter(typeId, string)`。
- **miss 路径**（反射、`GetField(key)` 等）才允许 `tostring` 或 `lua_tolstring` + 按需 intern。

**阶段 B — 注册期 Lua string ref 缓存**

- 注册成员时在 Lua 侧 `lua_pushliteral` + `luaL_ref` 存 string ref；metatable key 用 ref push（`lua_rawgeti` / registry ref）。
- 维护 `(typeId, lua_string_ptr/hash) → memberId` 弱缓存；动态 key 首次 miss 后复用，避免重复 marshal。

**阶段 C — `__index` / `__newindex` 模型对齐 xLua（P1）** ✅

- 注册期 `TypeMemberLuaIndexer` 为每个 type 生成 Lua `__index`/`__newindex`；getters/setters 作 closure upvalue。
- 已注册成员查找在 **Lua VM 内 O(1) rawget + call** 完成；C# 回调仅 unknown key fallback。
- 可选后续：native `zlua_obj_indexer`（Mono/Il2Cpp 一致，进一步减少 closure 开销）。**完整规格见 `ZLUA_VM_INDEX_SPEC.md`。**

**验收标准**

- 已注册成员 hot path 无 `LuaDllExtension.tostring` / 无 C# string 字典查表（Profiler 可证）。
- 现有 marshal / 成员访问 / 只读属性 / 重载测试全绿；Mono 与 Il2Cpp 行为一致。
- 阶段 C 完成后，字段/属性读热点对标 xLua 档 A/B 的 indexer 模型。

**与 P0 · 4.2 关系：** 4.2 完成「成员桥挂 metatable」；4.8 完成「访问路径不再 marshal key 到 C#」及 indexer 模型统一。

### P2

- 4.9 Editor 预生成 Wrap
- 4.10 ObjectPool 替代 GCHandle
- 4.11 Reflection.Emit 可选
- 4.12 Benchmark 基线

---

## 4. 实施路线

1. **Sprint 1（P0）：** 4.3 → 4.4 → 4.2 → 4.1 ✅
2. **Sprint 2（P1）：** 4.5–4.8 ✅
3. **Sprint 3：** 预生成 + Benchmark

---

## 5. P0 实施记录

| 项 | 状态 | 说明 |
|----|------|------|
| 4.3 InstanceIndex 顺序 | ✅ 完成 | fast path 先 TryGet/TryInvoke，fallback 才 TryGetUserDataTarget |
| 4.4 typed pop/push | ✅ 完成 | **校验与 typed pop 分离**：`EnsureArgumentType`（`CanConvertArgumentValue`）保留类型校验；校验通过后 `Pop*` 直读栈，避免 `ReadValue` boxing；返回值 typed push |
| 4.2 metatable getter 绑定 | ✅ 完成 | `BindInstanceAccessorsToMetatable` + InstanceIndex 直连 invoke |
| 4.1 构造函数 fast path | ✅ 完成 | `LuaToCSharpConstructorBridgeFactory` + 重载 dispatch + 元表直连 |

### P0 实施过程中的 Bugfix

| 问题 | 根因 | 修复 |
|------|------|------|
| `AmbiguousMatchException`（`ValidateExactArgCount`） | `GetMethod` 未指定 `types` 参数 | `LuaToCSharpMethodBridgeFactory.cs`、`LuaToCSharpConstructorBridgeFactory.cs` 补全 `types` |
| `instance metatable missing`（2 个 marshal 测试） | `BindInstanceAccessorsToMetatable` 中 `RawGetFieldPublic` 未 `lua_pop`，栈泄漏导致 `__instance_mt` 被写成 nil | `TypeFieldRegistration.cs`、`TypePropertyRegistration.cs` 每次 `RawGetField` 后 `pop` |
| 全量测试 crash（只读属性赋值） | `LuaCallbackBoundary.Throw` + `catch` + `lua_error` 混用 longjmp | `TrySetInstanceProperty/Field` 等改为直接 `LuaDllExtension.error`；新增 `AssignInstanceProperty` 等 |
| 重载 dispatch crash | `ThrowNoOverload` 返回 void，`invoke` 返回 int，分支类型不一致 | `LuaToCSharpOverloadDispatchFactory.cs`、`LuaToCSharpConstructorOverloadDispatchFactory.cs` 中 `ThrowNoOverload` 改为 `return 0` |
| 类型校验回归（4 个 `expect_error`） | P0 初版去掉 `Pop*` 校验后 `lua_toboolean`/`tostring` 静默接受错误类型 | `LuaToCSharpBridgeMarshaling.cs` 新增 `EnsureArgumentType`，所有 `Pop*` 先校验再直读栈 |

### 新增/修改文件

- `Docs/OPTIMIZATION.md` — 本报告
- `Bridges/LuaToCSharpBridgeExpressionBuilder.cs` — 共享 typed pop/push Expression
- `Bridges/LuaToCSharpConstructorBridgeFactory.cs` — ctor 编译桥（argStart 1/2）
- `Bridges/LuaToCSharpConstructorOverloadDispatchFactory.cs` — ctor 重载分派
- `TypeMethodRegistration.cs` — ctor fast invoker、PushCompiledBridge
- `TypeFieldRegistration.cs` / `TypePropertyRegistration.cs` — metatable 绑定
- `LuaManagerObject.cs` — InstanceIndex/InstanceNewIndex 优化
- `LuaToCSharpBridgeMarshaling.cs` — `EnsureArgumentType` + fast path typed pop

---

## 6. P1 实施记录（4.5–4.7）

| 项 | 状态 | 说明 |
|----|------|------|
| 4.5 Delegate 桥 typed push/pop | ✅ 完成 | `DynamicBridgeFactory` 编译期生成 typed push/pcall/pop，不再 `object[]` + `InvokeWithArgs` |
| 4.6 重载 fast table | ✅ 完成 | **静态**重载 `(argc, LuaDataType)` O(1) 查表；实例重载走 CanConvert 链（含 `this` 校验） |

### P1 Bugfix

| 问题 | 根因 | 修复 |
|------|------|------|
| fast table 崩溃 | UserData 粗粒度 key 误分派 + `Throw` 与 `TryCatch`/`lua_error` 混用 | fast path 增加 `TryCanConvertArguments`；`EnsureArgumentType`/`ValidateExactArgCount` 改 `LuaDllExtension.error` |
| `Calculator.Run` 类型校验失败 | 实例重载 fast path 跳过 `this` 校验且可能误分派 | 实例重载禁用 fast table；`TryCanConvertArguments` 校验 index 1 `this` |
| delegate 测试全失败 | `innerDelegate` 误含 `targetParam` 参数 | `Expression.Lambda(delegateType, body, argExprs)` |
| delegate compile 失败 | `TryFinally` 的 `finally` 引用了仅在 `try` Block 内声明的 `L` | 变量提升到外层 `Expression.Block` 包裹整个 `TryFinally` |
| 4.7 error handler ref | ✅ 完成 | `LuaEnv.EnsureErrorHandlerRef` + `PushErrorHandler`；`LuaCallInvoker` / `LuaInvokeBridge` / `RunLuaFunc` 改用 `lua_rawgeti` |

### 新增/修改文件

- `Bridges/CSharpToLuaBridgeExpressionBuilder.cs` — C#→Lua typed push/pop Expression
- `Bridges/OverloadTypeMask.cs` — `(argc, typeMask)` 快速分派表
- `DynamicBridgeFactory.cs` — 重写为 typed 编译桥
- `LuaEnv.cs` — 缓存 `__zluaErrorHandler` registry ref
- `LuaMethod.cs` — `PushErrorHandlerToStack`
- `LuaCallInvoker.cs` / `Bridges/LuaInvokeBridge.cs` — 使用缓存 ref
- `Bridges/LuaToCSharpOverloadDispatchFactory.cs` / `LuaToCSharpConstructorOverloadDispatchFactory.cs` — 集成 fast table

---

## 7. P1 实施记录（4.8）

| 项 | 状态 | 说明 |
|----|------|------|
| 4.8 阶段 A — Lua 栈 key 直查 | ✅ 完成 | fallback 路径 `trytolstring` + `TypeMemberNameCache`，避免 hot path 首行 `tostring` |
| 4.8 阶段 B — 成员名缓存 | ✅ 完成 | 注册期预填 + miss 路径 UTF-8 hash 缓存 |
| 4.8 阶段 C — Lua VM indexer | ✅ 完成 | 注册期 `TypeMemberLuaIndexer` 生成 Lua `__index`/`__newindex`；getters/setters 作 closure upvalue |

### 架构要点（对齐 xLua 档 A）

- **已注册成员 hot path 在 Lua VM 内完成**，不经 C# `__index` 回调：`rawget(getters,key)` → `getter(obj)`；miss → `rawget(mt,key)` 取 method/enum 常量；再 miss 才调 C# fallback closure。
- **注册期绑定**：`bind_instance_index(getters, setters, fallback_index, fallback_newindex)` / `bind_static_index(mt, getters, setters, ...)` 于 `TypeMemberLuaIndexer.EnsureLoaded` 加载；每个 type 的 getters/setters 表作为 upvalue，运行时无 `"__zlua_getters"` 字符串查找。
- **C# fallback 仅 miss**：`InstanceIndex` / `InstanceNewIndex` / `StaticTypeIndex` / `StaticTypeNewIndex` 保留反射与未注册成员逻辑；不再承担 hot path。
- **静态/实例对称：** `BindStaticMetatable` / `BindInstanceMetatable` 在 class/struct/enum 注册流程统一调用。

### 新增/修改文件

- `TypeMemberLuaIndexer.cs` — Lua 侧 indexer 工厂与 metatable 绑定
- `LuaStackTableAccess.cs` — 子表 ensure、栈 key 工具（注册期）
- `TypeMemberTableNames.cs` — 子表字段名常量
- `TypeMemberNameCache.cs` — fallback 成员名 intern
- `LuaDllExtension.cs` — `trytolstring`（无 alloc 探针）
- `TypeFieldRegistration.cs` / `TypePropertyRegistration.cs` — getter/setter 子表绑定
- `LuaManagerObject.cs` — fallback indexer + 注册期 `Bind*Metatable`

**后续：** Mono 落地 `META_TABLE_SPEC.md`；PUC-Rio 5.4 VM patch 见 `ZLUA_VM_INDEX_SPEC.md`。
