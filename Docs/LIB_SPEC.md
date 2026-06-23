# NovaLua 标准库（`novalua`）设计规范

本文档描述全局 **`novalua`** 表的 Lua API。源码位于：

`Packages/com.code-philosophy.novalua/Resources/novalua/novalualib.lua`

在 `lua_State` 初始化时由 native 嵌入执行（`dostring` / `BuiltinScripts::LoadNovaLuaLib`），与 `globals.lua` 一并加载。

**相关文档：**

| 文档 | 内容 |
|------|------|
| `TYPE_SYSTEM_SPEC.md` | `CSharp` 类型访问、元表、数组、泛型方法 |
| `METHOD_OVERLOAD_SPEC.md` | `signature` / `get_method` / `register_method` 语义 |
| `MARSHAL_SPEC.md` | 参数编组总览 |
| `DESIGN_SPEC.md` | 双运行时与总体架构 |

**平台原则：** `novalua` 的 Lua 封装在 Mono 与 Il2Cpp 上**签名与语义一致**；耗时逻辑在 native `__novalua_*` 回调中实现，Lua 层保持薄封装。

---

## 1. 职责边界

| 层级 | 职责 |
|------|------|
| **`CSharp` 根表** | 程序集 / 类型懒加载；静态成员、`Type()` 构造（见 `TYPE_SYSTEM_SPEC.md`） |
| **`novalua` 库** | 类型构造辅助（泛型、数组）、`typeof`、重载显式绑定、常用类型常量 |
| **实例 userdata** | 经类型表 `__call` 或 `new_*` 创建；成员经元表访问，不经 `novalua` |

`novalua` **不**替代 `CSharp` 访问类型；二者配合使用。

---

## 2. 加载与全局对象

```lua
novalua = novalua or {}   -- 全局表，由 novalualib.lua 初始化
```

Il2Cpp 将脚本嵌入 `BuiltinScripts.cpp`；Mono 从 `Resources/novalua/novalualib.lua` 读取。内容应保持一致（或通过构建同步）。

---

## 3. 类型实参（`typeArg`）

多处 API 需要描述 C# 类型，统称 **类型实参** `typeArg`，以下形式等价（native 统一解析）：

| 形式 | 示例 |
|------|------|
| `novalua.types.*` | `novalua.types.int32` |
| `novalua.typeof(typeTable)` | `novalua.typeof(CSharp.AC.Demo)` |
| `CSharp` 类型表 | `CSharp.mscorlib['System.Int32']`（须已解析） |

`novalua.types.*` 见 §4.2：预置常用 corlib 类型的**类型全名**（如 `System.Int32`），可直接作为 `typeArg` 传入。

---

## 4. 类型查询与常量

### 4.1 `novalua.typeof`

```lua
novalua.typeof(typeTable) → typeDescriptor
```

| 参数 | 说明 |
|------|------|
| `typeTable` | `CSharp` 下的类型表（含 `__fullname` 等元数据） |

返回该类型的 **System.Type 等价物**（Mono：`System.Type`；Il2Cpp：携带 `Il2CppClass*` 的类型描述对象）。供需要显式类型对象的场景使用；多数 API 也可直接传 `typeArg`。

```lua
local t = novalua.typeof(CSharp.mscorlib['System.Int32'])
local demoType = novalua.typeof(CSharp.AC.Demo)   -- 无 namespace 全局类型
local panelType = novalua.typeof(CSharp.AC['MyGame.UI.Panel'])  -- 含 namespace
```

### 4.2 `novalua.types`

预置常用 corlib 类型全名，避免重复写长字符串：

```lua
novalua.types.int32      -- "System.Int32"
novalua.types.int64
novalua.types.single     -- System.Single
novalua.types.double
novalua.types.boolean
novalua.types.string
novalua.types.object
novalua.types.void       -- 若暴露；多用于签名场景
-- … 由 novalualib 或 native 初始化补全
```

可直接作为 `typeArg` 传入 `signature`、`make_generic_type`、`make_szarray_type` 等。

> **命名：** 旧称 `novalua.corlibtypes` 已废弃，统一为 `novalua.types`。

---

## 5. 泛型类型

### 5.1 `novalua.make_generic_type`

```lua
novalua.make_generic_type(genericBaseType, typeArg1, typeArg2, ...) → typeTable
```

| 参数 | 说明 |
|------|------|
| `genericBaseType` | 未闭合泛型定义的类型表；**含 namespace 时必须括号访问** |

```lua
local ListInt = novalua.make_generic_type(
    CSharp.mscorlib['System.Collections.Generic.List'],
    novalua.types.int32
)
local list = ListInt()   -- 构造实例，见 TYPE_SYSTEM_SPEC §4.6
```

- 返回闭合泛型的**类型表**；相同实参多次调用应 **intern** 为同一表。
- 实参个数须与泛型定义一致，否则 `luaL_error`。

**Native：** `__novalua_make_generic_type`

---

## 6. 数组类型与实例

与 `TYPE_SYSTEM_SPEC.md` §2.5、§7 一致。

### 6.1 数组类型构造

```lua
novalua.make_szarray_type(typeArg) → szarrayTypeTable
novalua.make_mdarray_type(typeArg, rank) → mdarrayTypeTable
```

| API | 说明 |
|-----|------|
| `make_szarray_type` | 单维 0 基向量数组 `T[]` |
| `make_mdarray_type` | `rank` 维数组 `T[,…]`，`rank ≥ 1` |

```lua
local IntArray = novalua.make_szarray_type(novalua.types.int32)
local IntMatrix = novalua.make_mdarray_type(novalua.types.int32, 2)
```

### 6.2 数组实例创建

```lua
novalua.new_szarray_by_element_type(typeArg, length) → szarrayUserdata
novalua.new_szarray_by_szarray_type(szarrayTypeTable, length) → szarrayUserdata

novalua.new_mdarray_by_mdarray_type(mdarrayTypeTable, lowbounds, sizes) → mdarrayUserdata
novalua.new_mdarray_by_spec(typeArg, lowbounds, sizes) → mdarrayUserdata
```

| 参数 | 说明 |
|------|------|
| `length` | szarray 长度，`≥ 0` |
| `lowbounds` | 长度为 `rank` 的 Lua 表，每维下界 |
| `sizes` | 长度为 `rank` 的 Lua 表，每维元素个数 |

元素初始化为 `default(T)`。szarray 实例支持 `#arr`（`__len`，等价 `Length`）。

### 6.3 szarray 转换

#### `novalua.to_bytes`

```lua
novalua.to_bytes(szarray) → string
```

将 **szarray** 实例的底层元素内存按顺序拷贝为 Lua **二进制字符串**（`string` 每字节对应一字节，可含 `\0`）。

| 约束 | 说明 |
|------|------|
| 输入 | 必须是 szarray userdata（**不支持** mdarray） |
| 元素类型 | 仅 **blittable 基元**：`bool`、`byte`、`sbyte`、`char`、`short`、`ushort`、`int`、`uint`、`long`、`ulong`、`float`、`double` |
| 不支持 | `string`、`object`、`decimal` 及任意引用类型、非 blittable struct |

**布局：**

- 按 C# 数组下标 `0 .. Length-1` 顺序拼接；无额外长度头。
- 多字节数值使用 **平台原生字节序**（Il2Cpp 目标平台通常为 **little-endian**），与 `Buffer.BlockCopy` / 内存逐元素布局一致。
- `bool` 按 **1 字节** 存储：`0` / `非 0`（与 Il2Cpp 布尔数组内存布局一致）。

```lua
local bytes = novalua.to_bytes(int_arr)   -- #bytes == #int_arr * 4（int 为 4 字节）
```

元素类型不在白名单内时 `luaL_error`。

**Native：** `__novalua_to_bytes`

#### `novalua.to_table`

```lua
novalua.to_table(szarray) → table
```

将 szarray 转为 **等长** Lua 表；**对元素类型无限制**，每个元素按 `MARSHAL_SPEC.md` / `CLASS_MARSHAL_SPEC.md` 规则转为 Lua 值。

| 约束 | 说明 |
|------|------|
| 输入 | 必须是 szarray userdata |
| 输出长度 | `n = #szarray`；返回表在 `1 .. n` 上连续赋值 |
| 下标对应 | `t[i]` ↔ C# `arr[i - 1]`（Lua 1 基 ↔ C# 0 基） |

```lua
local t = novalua.to_table(obj_arr)
-- t[1] 对应 arr[0]，t[#t] 对应 arr[Length-1]
```

引用类型元素转为 userdata；值类型 struct 按 class marshal 规则处理。

**Native：** `__novalua_to_table`

---

**Native（§6.1–6.2）：** `__novalua_make_szarray_type`、`__novalua_new_szarray_*` 等（名称以实现为准，语义对齐本文档）。

---

## 7. 泛型方法实参

仅用于**方法自身带泛型参数**的情形（如 `void Foo<T>(T x)`），见 `TYPE_SYSTEM_SPEC.md` §6。

### 7.1 `novalua.make_generic_inst`

```lua
novalua.make_generic_inst(typeArg1, typeArg2, ...) → genericInst
```

构造泛型方法调用所需的 **`generic_inst`** 句柄；native 校验类型参数个数与约束。

### 7.2 调用约定

```lua
local inst = novalua.make_generic_inst(novalua.types.int32)
Type.Foo(inst, value)   -- 第一实参必须是 generic_inst
```

每个泛型方法在 native 维护 `inflatedMap`：`generic_inst` 指纹 → 单态化后的 bridge closure。

---

## 8. 方法重载辅助

完整语义见 `METHOD_OVERLOAD_SPEC.md`。`novalua` 仅提供薄封装。

### 8.1 `novalua.signature`

```lua
novalua.signature([typeArg1, typeArg2, ...]) → paramSignature
```

- **不包含** 方法名；仅参数类型列表。
- 无参：`()`；有参：`(System.Int32,…)`，使用 `Type.FullName` 规范格式。

```lua
local sig = novalua.signature(novalua.types.int32)           -- "(System.Int32)"
local sig2 = novalua.signature(novalua.types.int32, novalua.types.string)
```

**禁止**将返回值用作 `obj[sig]` 查表键；应配合 `get_method` 或模块级缓存。

**Native：** `__novalua_create_signature`（Lua 层不再暴露 `create_signature` 公共 API；若存在则视为 `signature` 别名并标记废弃）。

### 8.2 `novalua.get_method`

```lua
novalua.get_method(target, methodName, signature, is_static) → closure
```

| 参数 | 说明 |
|------|------|
| `target` | 实例 userdata 或类型表；用于解析声明类型 |
| `methodName` | C# 方法名 |
| `signature` | `novalua.signature(...)` 返回值 |
| `is_static` | `true` 查静态域；`false` 查实例域 |

```lua
local demo = CSharp.AC.Demo()
local run_i32 = novalua.get_method(demo, "Run", novalua.signature(novalua.types.int32), false)
run_i32(demo, 10)

local add = novalua.get_method(CSharp.AC.Demo, "Add",
    novalua.signature(novalua.types.int32, novalua.types.int32), true)
add(3, 5)
```

- 实例方法：点号调用并传入 `self`；静态方法：直接 `closure(...)`。
- `is_static == true` 时 `target` 可为实例，仅用于解析类型（见 `TYPE_SYSTEM_SPEC.md` §3.2）。

**Native：** `__novalua_get_method`（待实现）

### 8.3 `novalua.register_method`

```lua
novalua.register_method(aliasName, methodClosure) → void
```

将 `get_method` 返回的 closure 注册为类型元表别名：

```lua
novalua.register_method("run_i32", run_i32)
demo:run_i32(20)
```

| 参数 | 说明 |
|------|------|
| `aliasName` | 类内唯一 Lua 键（与 `[LuaAlias]` 规则一致，见 `METHOD_OVERLOAD_SPEC.md` §5） |
| `methodClosure` | 内含 `klass`、`is_static`；决定写入类型表或 `__instance_mt` |

**Native：** `__novalua_register_method`（待实现）

---

## 9. 与 `CSharp` 的配合示例

```lua
CSharp.AC = CSharp['Assembly-CSharp']

-- 类型解析（TYPE_SYSTEM_SPEC §2.2）
local Demo = CSharp.AC.Demo                      -- 无 namespace
local Panel = CSharp.AC['MyGame.UI.Panel']       -- 有 namespace：必须括号

-- typeof / types
local demo = Demo()
local sig = novalua.signature(novalua.types.int32)

-- 泛型
local ListInt = novalua.make_generic_type(
    CSharp.mscorlib['System.Collections.Generic.List'],
    novalua.types.int32
)

-- 数组
local IntArray = novalua.make_szarray_type(novalua.types.int32)
local arr = novalua.new_szarray_by_szarray_type(IntArray, 4)
print(#arr)
local bytes = novalua.to_bytes(arr)
local t = novalua.to_table(arr)

-- 重载
local run_i32 = novalua.get_method(demo, "Run", sig, false)
novalua.register_method("run_i32", run_i32)
```

---

## 10. Native 回调一览

| Lua API | Native 回调 | 状态（参考） |
|---------|-------------|--------------|
| `novalua.typeof` | `__novalua_typeof` | Mono / Il2Cpp MVP |
| `novalua.signature` | `__novalua_create_signature` | Mono（待对齐：仅 typeArg） |
| `novalua.make_generic_type` | `__novalua_make_generic_type` | Mono；Il2Cpp 占位 |
| `novalua.make_szarray_type` | `__novalua_make_szarray_type` | 待实现 |
| `novalua.make_mdarray_type` | `__novalua_make_mdarray_type` | 待实现 |
| `novalua.new_szarray_*` | `__novalua_new_szarray_*` | 待实现 |
| `novalua.to_bytes` | `__novalua_to_bytes` | 待实现 |
| `novalua.to_table` | `__novalua_to_table` | 待实现 |
| `novalua.new_mdarray_*` | `__novalua_new_mdarray_*` | 待实现 |
| `novalua.make_generic_inst` | `__novalua_make_generic_inst` | 待实现 |
| `novalua.get_method` | `__novalua_get_method` | 待实现 |
| `novalua.register_method` | `__novalua_register_method` | 待实现 |

新增 native 回调在 `LuaInteropManager::RegisterNovaLuaApi`（Il2Cpp）/ `LuaManagerObject`（Mono）注册；Lua 侧仅 `function novalua.xxx(...) return __novalua_xxx(...) end`。

---

## 11. `novalualib.lua` 目标骨架

```lua
novalua = novalua or {}

function novalua.typeof(typeTable)
    return __novalua_typeof(typeTable)
end

function novalua.signature(...)
    return __novalua_create_signature(...)
end

function novalua.make_generic_type(genericBase, ...)
    return __novalua_make_generic_type(genericBase, ...)
end

function novalua.make_szarray_type(elementType)
    return __novalua_make_szarray_type(elementType)
end

function novalua.make_mdarray_type(elementType, rank)
    return __novalua_make_mdarray_type(elementType, rank)
end

function novalua.new_szarray_by_element_type(elementType, length)
    return __novalua_new_szarray_by_element_type(elementType, length)
end

function novalua.new_szarray_by_szarray_type(szarrayType, length)
    return __novalua_new_szarray_by_szarray_type(szarrayType, length)
end

function novalua.to_bytes(szarray)
    return __novalua_to_bytes(szarray)
end

function novalua.to_table(szarray)
    return __novalua_to_table(szarray)
end

function novalua.new_mdarray_by_mdarray_type(mdarrayType, lowbounds, sizes)
    return __novalua_new_mdarray_by_mdarray_type(mdarrayType, lowbounds, sizes)
end

function novalua.new_mdarray_by_spec(elementType, lowbounds, sizes)
    return __novalua_new_mdarray_by_spec(elementType, lowbounds, sizes)
end

function novalua.make_generic_inst(...)
    return __novalua_make_generic_inst(...)
end

function novalua.get_method(target, methodName, signature, is_static)
    return __novalua_get_method(target, methodName, signature, is_static)
end

function novalua.register_method(aliasName, methodClosure)
    return __novalua_register_method(aliasName, methodClosure)
end

novalua.types = novalua.types or {
    void    = "System.Void",
    boolean = "System.Boolean",
    int32   = "System.Int32",
    int64   = "System.Int64",
    single  = "System.Single",
    double  = "System.Double",
    string  = "System.String",
    object  = "System.Object",
}
```

当前仓库内 `novalualib.lua` 仍为早期原型（含 `create_signature(methodName, …)`、`get_method` 两参数等），**以实现清单为准逐步对齐本规范**。

---

## 12. 实现清单

- [ ] `novalua.types` 初始化与 `corlibtypes` 迁移
- [ ] `signature` 仅接收 `typeArg`，去掉 methodName
- [ ] `get_method(target, methodName, signature, is_static)` native 实现
- [ ] `register_method(aliasName, closure)` native 实现
- [ ] 数组 `make_*` / `new_*` 全套 API
- [ ] `to_bytes` / `to_table`（szarray）
- [ ] `make_generic_inst` + 泛型方法 `inflatedMap`
- [ ] Mono / Il2Cpp `novalualib.lua` 内容同步（嵌入 vs Resources）
- [ ] `TYPE_SYSTEM_SPEC.md` §2.2 命名空间括号规则在类型解析回调中强制执行
