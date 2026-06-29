# ZLua VM 成员索引规格（v1.0）

Lua 侧访问 C# 对象（userdata / 类型表）时的 **metatable 索引优化** 策略。目标：在 PUC-Rio Lua 5.4 上通过 **极简 VM patch** 超越 xLua `gen_obj_indexer`；LuaJIT 分支退守 C/Lua indexer，不 patch VM。

**关联文档：** `OPTIMIZATION.md` §4.8、`META_TABLE_SPEC.md`（Mono Lua obj_indexer）、`TYPE_SYSTEM_SPEC.md`、`IL2CPP_DESIGN_SPEC.md`

**语义锚点（Il2Cpp 已实现）：** `ZLua~/libil2cpp-2022/zlua/MetaBinding.cpp` → `DispatchIndex` / `DispatchNewIndex`

---

## 1. 问题与目标

### 1.1 根本矛盾

| 访问 | 期望 | `__index` 表中的存储 | 查表后 |
|------|------|---------------------|--------|
| `obj.Method` | 可调用 closure | closure | **直接返回** |
| `obj.field` | 字段**值** | getter closure | **必须 invoke** |

method 与 field 语义不同：field 读天然比 method 多一次 getter 调用，**无法消除 invoke**；能优化的是 **lookup 路径**（查表 + 分派 + 是否经 C# / Lua function 帧）。

### 1.2 xLua 现状（对标）

xLua `obj_indexer`（`3rd/xLua/build/xlua.c`）为 **C closure `__index`**：

1. method 表 `gettable` → 命中则返回  
2. getter 表 `gettable` → 命中则 `call(obj)`  
3. 数组 / csindexer / 基类链 / fallback  

ZLua 当前 Mono 方案（`META_TABLE_SPEC.md`）：Lua function `__index` + 三表 upvalue，已去掉 C# hot path，但仍整段走 **Lua function 帧**。

### 1.3 目标

- **PUC-Rio Lua 5.4（主战场）：** VM 内一次完成 method 返回或 field invoke；**不**影响普通 table / 第三方 metatable。  
- **LuaJIT（兼容分支）：** 不 patch VM；Mono 用 Lua indexer，Il2Cpp 用 C indexer（与 xLua 同级或略优）。  
- **语义统一：** 所有运行时共用同一套 `MemberKind` 与 miss fallback 规则。

---

## 2. 双轨 VM 策略

```
                    ┌──────────────────────────────────────┐
  PUC-Rio 5.4       │ VM patch                             │
  (Mono + Il2Cpp)   │ OP_GETFIELD userdata 快路径          │  ← 降维打击 xLua
                    │ + 可选 finishget tagged getter 兜底   │
                    └──────────────────────────────────────┘

                    ┌──────────────────────────────────────┐
  LuaJIT            │ 不 patch VM                          │
                    │ Mono: META_TABLE_SPEC Lua indexer      │  ← 控维护成本
                    │ Il2Cpp: MetaBinding C __index        │
                    └──────────────────────────────────────┘
```

| 维度 | PUC-Rio 5.4 | LuaJIT |
|------|-------------|--------|
| VM 改动 | `lvm.c` 少量 `#ifdef ZLUA` | 无 |
| 热路径 | `OP_GETFIELD` → native 分派 | C / Lua `__index` |
| 维护 | merge 5.4.x 小版本，1–2 处 patch | 与 xLua luajit 版同级 |
| 5.4 特性 | integer、utf8 等完整支持 | API 停留 5.1/5.2 语义，非 ZLua 主目标 |

**路线：** spec 以 **Lua 5.4** 为准；5.1 / 5.3 视为同 patch **可移植**，非首发。

---

## 3. PUC-Rio Lua 5.4：VM patch 设计

### 3.1 原则

1. **仅 ZLua 绑定的 full userdata（及 static type table）** 走快路径；普通 table、第三方 `mt.__index = {}` **零行为变化**。  
2. **method**：查表命中 → 原样返回 closure（与官方 table `__index` 同速）。  
3. **field / property getter**：查表命中 tagged getter → VM 内 invoke，结果写入目标槽。  
4. **miss**：走现有 C fallback（Mono `InstanceIndex` 等 / Il2Cpp `DispatchIndex` 返回 0 后的逻辑）。

### 3.2 分层（推荐实现顺序）

#### 档 A — `OP_GETFIELD` userdata 快路径（必做）

在 `lvm.c` 的 `OP_GETFIELD`（可选 `OP_GETTABLE`）中：

```c
vmcase(OP_GETFIELD) {
  TValue *rb = vRB(i);
  TString *key = tsvalue(KC(i));
  if (ttisfulluserdata(rb) && zlua_is_bound_ud(rb)) {
    if (zlua_fast_getfield(L, rb, key, ra))
      vmbreak;
  }
  /* 原有 fastget + luaV_finishget */
}
```

`zlua_fast_getfield` 语义对齐 Il2Cpp `DispatchIndex`：

| MemberKind | 行为 |
|------------|------|
| Method | push registry closure，return true |
| Field | 调 native getter(obj / offset)，return true |
| Property | pcall getter closure 或 inline getter，return true |
| Event | push event 表，return true |
| miss | return false → 回落官方 metamethod 链 |

**影响面：** 仅 ZLua userdata；普通 userdata / table **不进入** `zlua_is_bound_ud`。

#### 档 B — `luaV_finishget` tagged getter（可选，单表 `__index` 模型）

当 `__index` 为 **单表**（method + tagged getter 混放）时，在 `fastget` 命中后：

```c
const TValue *obj = t0;  /* 循环前保存原始被索引对象 */

if (luaV_fastget(L, t, key, slot, luaH_get)) {
  if (zlua_is_getter(slot)) {
    zlua_invoke_getter(L, obj, slot, val);
    return;
  }
  setobj2s(L, val, slot);
  return;
}
```

- `zlua_is_getter(slot)`：`ttisCclosure` 且 `f == zlua_getter_trampoline`（或 upvalue 标记）。  
- 普通 closure / 非 getter 值：**一次指针比较**，与官方行为一致。  
- **不**对非 ZLua 对象做额外 string 或字典查找。

#### 档 C — blittable field offset 直读（Il2Cpp 优先，可选）

对编译期已知 layout 的 field，在 `zlua_fast_getfield` 内直接 `*(T*)(ud + offset)`，跳过 getter CCall。仅适用于 Field kind + 固定 offset；Property / boxing 类型仍走 getter。

### 3.3 注册期数据结构（与 Il2Cpp 对齐）

Il2Cpp 已有：

```cpp
// MetaBinding.cpp
enum class MetaKind { Method, Field, Property, Event, ... };
struct MetaInfo { MetaKind kind; ... };
using NameMetaMap = std::unordered_map<std::string, MetaInfo>;
```

Mono / VM 侧应对齐为 **typeId → MemberMap**（注册期构建，运行时 O(1) 或 hash）：

| 字段 | 说明 |
|------|------|
| `kind` | Method / Field / Property / Event |
| `closureRef` | Method / Property getter registry ref |
| `getterFn` | Field 原生 getter C 函数指针 |
| `offset` | Il2Cpp instance field offset（Mono 可选 GCHandle 路径） |
| `fallbackRef` | miss 时 C# / C closure |

**metatable 布局（5.4 patch 落地后）：**

- userdata metatable 挂 **`__zlua_member_map`**（lightuserdata 或 registry ref），VM 快路径只读此 map。  
- **不再**依赖运行时 Lua function `__index`（`TypeMemberLuaIndexer` 退役）。  
- `__index` / `__newindex` 可设为 nil 或仅作 fallback（miss 时官方 metamethod 调 C closure）。

### 3.4 写路径（`__newindex` 对称）

| 路径 | 行为 |
|------|------|
| VM `OP_SETFIELD` 快路径 | `zlua_fast_setfield`，对齐 `DispatchNewIndex` |
| tagged setter | 同 getter，finishset 或快路径内 invoke |
| miss | C# `InstanceNewIndex` / C `DispatchNewIndex` |

### 3.5 源码改动清单（5.4）

| 文件 | 改动 |
|------|------|
| `lvm.c` | `OP_GETFIELD` / `OP_SETFIELD` ZLua 分支；可选 `luaV_finishget` / `luaV_finishset` |
| `lapi.c` | 若需导出 `zlua_bind_userdata` 等 |
| `zlua.c`（新增） | getter 标记、MemberMap 注册、fallback |
| `luaconf.h` | `#define ZLUA` 开关 |

预计 **1–2 个逻辑点 + 薄封装**，升级 5.4.x 时携带 patch hunk。

### 3.6 不影响普通 mt 的证明

| 场景 | 是否进入 ZLua 分支 |
|------|-------------------|
| 普通 table `t[k]` | 否（table fastget 成功或无 ZLua ud） |
| 普通 userdata + 标准 metatable | 否（无 `__zlua` 标记） |
| `mt.__index = methods` 类模式 | 否（除非 obj 为 ZLua bound ud） |
| ZLua C# 实例 userdata | 是 |

---

## 4. LuaJIT 分支（不 patch VM）

### 4.1 原因

- 需同时改 `lj_meta.c`、trace recorder、IR、汇编后端，**非** 5.4 级「一两处 patch」。  
- 互调场景 userdata + metamethod 常导致 trace abort，luajit 峰值优势有限。  
- xLua luajit 版在 C# 成员访问上同样走 C `obj_indexer`，未 JIT 进 getter。

### 4.2 实现

| 运行时 | `__index` / `__newindex` | 说明 |
|--------|--------------------------|------|
| Mono | `TypeMemberLuaIndexer` | Lua closure，getters/setters 为 upvalue |
| Il2Cpp | `MetaBinding::InstanceIndex` 等 | C closure + `NameMetaMap` |

性能预期：**与 xLua luajit 档相当**；不作为降维打击主路径。

---

## 5. 与现有实现的关系

### 5.1 过渡方案（当前，P1 · 4.8）

| 组件 | 状态 |
|------|------|
| `TypeMemberLuaIndexer.cs` | Mono 5.4 **过渡**；VM patch 落地后退役 |
| `MetaBinding.cpp` `DispatchIndex` | Il2Cpp **语义参考**；VM patch 为其 VM 内联版 |
| C# `InstanceIndex` 等 | **仅 fallback**（miss / 反射） |

### 5.2 落地后目标架构

```
obj.field  (PUC-Rio 5.4)
  OP_GETFIELD
    → zlua_is_bound_ud? 
         → zlua_fast_getfield (MemberMap)
              Method  → push closure
              Field   → native getter / offset 直读
              miss    → C fallback (InstanceIndex)
    → 官方 luaV_finishget
```

### 5.3 验收标准

- [ ] 已注册成员：`obj.field` / `obj:Method()` 无 Lua function `__index` 帧（Profiler 可证）。  
- [ ] 已注册 field hot path 无 `lua_tostring` → C# string（Mono fallback 除外）。  
- [ ] 普通 Lua OOP / 第三方库 metatable 行为与官方 Lua 5.4 一致。  
- [ ] `run_all.lua`、marshal、field access、enum、只读属性测试全绿。  
- [ ] Mono（5.4 patch）与 Il2Cpp（同 MemberKind 规则）语义一致。  
- [ ] LuaJIT 分支不 patch VM，测试通过即可。

---

## 6. 相对 xLua 的差异（降维点）

| 项目 | xLua | ZLua（本 spec 落地后） |
|------|------|------------------------|
| indexer 形态 | C closure `obj_indexer` | 5.4：**VM 内分派**，无 indexer 函数帧 |
| method 查表 | method 表 gettable | MemberMap O(1) / 单表 fastget |
| field 查表 | getter 表 gettable + call | 同一次查表 + VM 内 invoke |
| 普通 mt 影响 | 无 | 无（userdata 守卫） |
| LuaJIT | C `obj_indexer` | 同左（刻意不 patch） |
| Il2Cpp field | 视生成档而定 | offset 直读 + VM/C 双路径 |

---

## 7. 实施路线

1. **Spec 冻结（本文档）** — MemberKind、MemberMap、快路径伪代码与 Il2Cpp 对齐。  
2. **Il2Cpp `DispatchIndex` 对照表** — 列出 Mono 侧等价 MemberMap 构建（注册期已有 field/method/property 绑定）。  
3. **5.4 patch 档 A** — `OP_GETFIELD` + `zlua_fast_getfield`；Mono Editor 验证。  
4. **退役 `TypeMemberLuaIndexer`** — 5.4 Mono/Il2Cpp 均走 VM 路径。  
5. **可选档 B/C** — 单表 tagged getter、offset 直读。  
6. **LuaJIT** — 维持 §4，文档标注为非主性能路径。

---

## 8. 修订记录

| 版本 | 日期 | 说明 |
|------|------|------|
| v1.0 | 2026-06-25 | 初版：双轨策略、5.4 VM patch、LuaJIT fallback、与 MetaBinding 对齐 |
