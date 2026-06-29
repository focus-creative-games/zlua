# ZLua 元表索引规格（obj_indexer）v1.0

Lua 侧通过 **`__index` / `__newindex`** 访问 C# **静态成员**（类型表 `T` + 静态元表 `SMT`）与 **实例成员**（userdata + 实例元表 `IMT`）的分派规则。

**适用范围：**

| 运行时 | 实现 | 本文档 |
|--------|------|--------|
| **Mono（Editor）** | Lua function `__index` / `__newindex` + 三表 upvalue | **权威规格（§3–§8）** |
| **Il2Cpp（Player）** | C closure `DispatchIndex` / `DispatchNewIndex`（`MetaBinding.cpp`） | 语义对齐 §3–§5；实现见 `IL2CPP_DESIGN_SPEC.md` |
| **PUC-Rio Lua 5.4（远期）** | VM `OP_GETFIELD` 快路径 | 见 `ZLUA_VM_INDEX_SPEC.md`；落地后 Mono 可退役 Lua indexer |
| **LuaJIT** | 同 Mono（Lua indexer）或 Il2Cpp（C indexer） | 不 patch VM |

**关联文档：** `TYPE_SYSTEM_SPEC.md`（类型表结构）、`OPTIMIZATION.md` §4.8、`METHOD_OVERLOAD_SPEC.md`、`ZLUA_VM_INDEX_SPEC.md`

---

## 1. 设计目标

| 目标 | 说明 |
|------|------|
| Hot path 零 C# string key | 已注册成员分派在 **Lua VM 内**完成，不调用 C# `InstanceIndex` / `tostring` |
| 三表分流 | method / field-get / field-set 注册期分离，运行时顺序查表 |
| Strict miss | 未注册成员 **直接 Lua error**，不走 C# 反射 fallback |
| 静实例隔离 | 静态与实例各一套 `methodTable` / `fieldGetterTable` / `fieldSetterTable` |
| 注册期预计算 | 所有 Lua 可见 public 成员在 `EnsureBinding` 时写入三表；继承链 **Bind 期扁平化** |

---

## 2. 背景：为何不用 table 嵌套 `__index`

曾考虑 `mt.__index = methodTable A`，且 `A` 的 metatable 上挂 field 用 `__index` 函数。  
Lua VM 语义下，对 **实例 userdata** 访问 field 时，嵌套 `__index` 函数收到的第一个参数是 **table A**，而非 **userdata obj**，无法读取实例 field。

**结论：** field 与 method 统一由 **接收 `(obj, key)` 的普通 Lua function** 分派（与 xLua `obj_indexer(obj, key)` 同构，实现语言为 Lua 而非 C）。

---

## 3. 三表职责

每个绑定（静态一套、实例一套）在注册期构建三张 **普通 Lua table**（建议 registry ref 或 upvalue 持有，**不**挂到 `IMT`/`SMT` 可见键上）。

### 3.1 `methodTable`

| 成员种类 | 表中值 | `__index` 行为 |
|----------|--------|----------------|
| 实例 / 静态 **方法** | compiled bridge closure | **直接返回**，不 call |
| **索引器 property**（`this[...]`，即带参 property） | 包装 closure / dispatch closure | **直接返回** |
| **event** | `{ get = fn, set = fn, fire = fn? }` 子表 | **直接返回** 子表 |
| 构造函数元数据 | 不在此表（见 `TYPE_SYSTEM_SPEC.md` §4.6；`_ctor` / `__call` 在类型表或 `SMT`） | — |

**不得**将 field getter 或普通 property getter 放入 `methodTable`。

### 3.2 `fieldGetterTable`

| 成员种类 | 表中值 | `__index` 行为 |
|----------|--------|----------------|
| **字段**（instance / static） | getter closure：`function(obj) ... end` | `return getter(obj)` |
| **无参可读 property** | 同上（compiled getter bridge） | `return getter(obj)` |

- **只读 property**：仅出现在 `fieldGetterTable`；`__newindex` 对该键 strict error。
- **enum 静态常量**：底层整型值；可 **直接写入类型表 `E`**（integer），或写入 static `fieldGetterTable` 的常量 getter；**不得**为 userdata。

### 3.3 `fieldSetterTable`

| 成员种类 | 表中值 | `__newindex` 行为 |
|----------|--------|-------------------|
| **可写字段** | setter closure：`function(obj, value) ... end` | `setter(obj, value)` |
| **无参可写 property** | 同上 | `setter(obj, value)` |

- **只写 property**：仅出现在 `fieldSetterTable`；`__index` 对该键 strict error。
- **readonly 字段 / 只读 property**：不在此表；写入时 strict error。

### 3.4 命名冲突

同一绑定内键名唯一。若 method 与 property/field 同名（极少见），**`methodTable` 优先**（`__index` 先查 method 表）。

---

## 4. `__index` / `__newindex` 语义

### 4.1 实例 userdata（`IMT`）

**`__index(obj, key)`**（`obj` 为 full userdata）：

```lua
local rawget = rawget

local function index(obj, key)
  local member = rawget(methodTable, key)
  if member ~= nil then
    return member
  end
  local getter = rawget(fieldGetterTable, key)
  if getter ~= nil then
    return getter(obj)
  end
  error("zlua: instance member not found: " .. tostring(key))
end
```

**`__newindex(obj, key, value)`**：

```lua
local function newindex(obj, key, value)
  local setter = rawget(fieldSetterTable, key)
  if setter ~= nil then
    setter(obj, value)
    return
  end
  error("zlua: instance member not writable: " .. tostring(key))
end
```

说明：

- 使用 **`rawget`**，避免三表被用户篡改 metatable 影响分派。
- getter / setter 为注册期生成的 **C# compiled bridge closure**；hot path **无** `lua_tostring` → C# string 字典。
- **`__newindex` 无返回值**（不 return setter 结果）。
- **Strict miss**：不返回 `nil`，不调用 C# fallback。

### 4.2 静态类型表（`SMT`）

逻辑与 §4.1 **相同**，但：

- `__index` / `__newindex` 的第一个参数 `obj` 为 **类型表 `T`**（静态门面）。
- static getter / setter closure 按静态语义实现（无 instance `GCHandle` pop，静态 field 直接读类型数据）。
- 静态与实例 **各持独立三表**，不可共用 upvalue。

### 4.3 与 `TYPE_SYSTEM_SPEC.md` 的差异（Mono）

| 项 | `TYPE_SYSTEM_SPEC.md`（Il2Cpp 通用） | 本文 Mono obj_indexer |
|----|--------------------------------------|------------------------|
| `__index` miss | 实例可返回 `nil` + 运行时继承提升 | **strict error** |
| 继承 | 实例运行时沿链查找 + promotion | **Bind 期扁平化**到三表（同静态 §5.1） |
| C# 反射 fallback | Il2Cpp 部分场景 | **禁止** |

Il2Cpp 实现可保留 promotion / nil；**Lua 可见错误消息与成员集合**须与 Mono strict 模式一致（同一类型同一键要么可访问，要么同样报错）。

---

## 5. 元表布局

### 5.1 实例元表 `IMT`

注册完成后：

```
IMT
├─ __index      → Lua closure（upvalue: methodTable, fieldGetterTable）
├─ __newindex   → Lua closure（upvalue: fieldSetterTable）
├─ __gc         → ReleaseUserData（C closure）
├─ __type       → 类型表 T
└─ （可选）__tostring / __len（数组等）

不在 IMT 根上重复挂载 method / getter 键（避免双路径）。
```

### 5.2 静态元表 `SMT`

```
SMT
├─ __index      → Lua closure（static 三表 upvalue）
├─ __newindex   → Lua closure（static fieldSetterTable）
├─ __call       → 实例构造函数 dispatch（class/struct/enum，见 TYPE_SYSTEM_SPEC）
└─ __tostring   → 可选

T（类型表）本体
├─ __assembly / __fullname / __name / __typeid / __instance_mt  （元数据）
├─ _ctor        → 构造 closure（若适用）
└─ （enum 常量可直接为 integer 键值）
```

### 5.3 注册顺序（Mono）

与 `TYPE_SYSTEM_SPEC.md` 及历史 bugfix 一致：

1. 构建 `SMT`（含 `__call` 等）。
2. `PushInstanceMetatable` → 构建 `IMT` 与三表 → 绑定 `__index` / `__newindex`。
3. **`lua_setfield(T, "__instance_mt")`**（此时 `T` **尚无** `SMT`）。
4. **`lua_setmetatable(T, SMT)`**（栈顶为 `SMT`）。

避免先 `setmetatable` 再写 `__instance_mt` 触发静态 `__newindex`。

---

## 6. Bootstrap 与绑定 API

启动时 **一次** `dostring` 加载工厂函数（registry 缓存 ref），注册期按类型调用：

```lua
-- 工厂（示意，registry 存 ref）
local function bind_indexer(methodTable, fieldGetterTable, fieldSetterTable)
  local rawget = rawget
  local function index(obj, key) ... end   -- §4.1
  local function newindex(obj, key, value) ... end
  return index, newindex
end
```

C# 侧（示意）：

```csharp
// EnsureLoaded(luaState) — 启动一次
// BindInstanceMetatable(luaState, imtIndex, methodTableRef, getterTableRef, setterTableRef)
// BindStaticMetatable(luaState, smtIndex, ...)
```

每类型 **不生成独立 Lua 源码**；三表为 registry ref 或栈上 table，作为 closure upvalue 传入工厂。

---

## 7. 注册期成员扫描规则

Bind 期扫描 **public** 成员，写入对应表（**仅 Lua 可访问**）：

| C# 成员 | 表 |
|---------|-----|
| Method（含重载 dispatch） | `methodTable` |
| 索引器 property | `methodTable` |
| Event | `methodTable`（event 子表） |
| Field | getter → `fieldGetterTable`；若可写 → `fieldSetterTable` |
| 无参 Property | 有 getter → `fieldGetterTable`；有 setter → `fieldSetterTable` |
| 基类继承 instance 成员 | **扁平写入**派生类 instance 三表（子类覆盖） |
| 基类 static 成员 | **扁平写入**派生类 static 三表（同 `TYPE_SYSTEM_SPEC.md` §5.1） |

**不注册** protected / private / explicit interface 实现（除非另有规格）。

构造函数、`_default`、数组 `__len` 等 **不**进入三表，按 `TYPE_SYSTEM_SPEC.md` 单独挂在 `T` / `IMT`。

---

## 8. 性能与对标

| 访问 | 路径 | 相对 xLua |
|------|------|-----------|
| `obj:Method()` 前先取 closure | Lua `__index` 帧 + 1× `rawget(methodTable)` | 与 xLua C `obj_indexer` method 分支同级 |
| `obj.field` | Lua `__index` 帧 + 2× `rawget` + getter call | 与 xLua getter 分支同级 |
| 旧 Mono C# `InstanceIndex` | P/Invoke + `tostring` + GC | **显著慢于本规格** |

代价：**method 与 field 均进入 Lua function 帧**（无 table-only `__index` 的 method 直查）。在 **不 patch VM** 前提下为务实最优；远期见 `ZLUA_VM_INDEX_SPEC.md`。

---

## 9. 错误消息约定

| 场景 | 消息（示意） |
|------|----------------|
| `__index` miss | `zlua: instance member not found: {key}` / `zlua: static member not found: {key}` |
| `__newindex` 无 setter | `zlua: instance member not writable: {key}` |
| getter 内部失败 | bridge 已有类型错误；保持 `zlua:` 前缀 |

---

## 10. 验收标准

- [ ] 已注册 method / field / property / event：hot path 无 C# `InstanceIndex` / `StaticTypeIndex`、无 `LuaDllExtension.tostring` 查表。
- [ ] 未注册键：`__index` / `__newindex` **strict error**，不返回 `nil`，不进反射。
- [ ] 静实例三表隔离；实例 userdata 不能隐式访问静态成员。
- [ ] 继承成员在 Bind 期扁平化；派生类覆盖基类同名键。
- [ ] 只读 property 写入、只写 property 读取均报错。
- [ ] `run_all.lua`、marshal、field access、enum、event、重载测试全绿。
- [ ] Mono 与 Il2Cpp **成员集合与报错语义**一致（实现路径可不同）。

---

## 11. 实现文件（Mono）

| 文件 | 职责 | 状态 |
|------|------|------|
| `TypeMemberLuaIndexer.cs` | Bootstrap + `BindInstanceMetatable` / `BindStaticMetatable` | ✅ |
| `TypeMethodRegistration.cs` | 写入 `methodTable` | ✅ |
| `TypeFieldRegistration.cs` | `BindGetterTable` / `BindSetterTable` + fallback closure | ✅ |
| `TypePropertyRegistration.cs` | 同上（property） | ✅ |
| `TypeEventRegistration.cs` | static → `methodTable`；instance → `BindInstanceEventsToMethodTable` | ✅ |
| `LuaManagerObject.cs` | 三表构建 + Lua indexer；已移除 C# `InstanceIndex` 等 | ✅ |

Il2Cpp 继续 `MetaBinding.cpp`；语义以本文 §3–§5 为准。

---

## 12. 修订记录

| 版本 | 日期 | 说明 |
|------|------|------|
| v1.0 | 2026-06-25 | 初版：三表 Lua obj_indexer、strict miss、静实例分离、Bootstrap |
