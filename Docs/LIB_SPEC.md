# ZLua 标准库（`zlua`）设计规范

本文档描述全局 **`zlua`** 表的 Lua API。源码位于：

`Packages/com.code-philosophy.zlua/Resources/zlua/zlualib.lua`

在 `lua_State` 初始化时由 native 嵌入执行（`dostring` / `BuiltinScripts::LoadZLuaLib`），与 `globals.lua` 一并加载。

**相关文档：**

| 文档 | 内容 |
|------|------|
| `TYPE_SYSTEM_SPEC.md` | `CSharp` 类型访问、元表、数组、泛型方法 |
| `METHOD_OVERLOAD_SPEC.md` | `signature` / `get_method` / `register_method` 语义 |
| `MARSHAL_SPEC.md` | 参数编组总览 |
| `FUNCTION_MARSHAL_SPEC.md` | Lua function ↔ C# delegate |
| `DESIGN_SPEC.md` | 双运行时与总体架构 |

**平台原则：** `zlua` 的 Lua 封装在 Mono 与 Il2Cpp 上**签名与语义一致**；耗时逻辑在 native `__zlua_*` 回调中实现，Lua 层保持薄封装。

---

## 1. 职责边界

| 层级 | 职责 |
|------|------|
| **`CSharp` 根表** | 程序集 / 类型懒加载；静态成员、`Type()` 构造（见 `TYPE_SYSTEM_SPEC.md`） |
| **`zlua` 库** | 类型构造辅助（泛型、数组）、`typeof`、重载显式绑定、常用类型常量 |
| **实例 userdata** | 经类型表 `__call` 或 `new_*` 创建；成员经元表访问，不经 `zlua` |

`zlua` **不**替代 `CSharp` 访问类型；二者配合使用。

---

## 2. 加载与全局对象

```lua
zlua = zlua or {}   -- 全局表，由 zlualib.lua 初始化
```

Il2Cpp 将脚本嵌入 `BuiltinScripts.cpp`；Mono 从 `Resources/zlua/zlualib.lua` 读取。内容应保持一致（或通过构建同步）。

---

## 3. 类型实参（`typeArg`）

多处 API 需要描述 C# 类型，统称 **类型实参** `typeArg`，以下形式等价（native 统一解析）：

| 形式 | 示例 |
|------|------|
| `zlua.types.*` | `zlua.types.int32` |
| `zlua.typeof(typeTable)` | `zlua.typeof(CSharp.AC.Demo)` |
| `CSharp` 类型表 | `CSharp.mscorlib['System.Int32']`（须已解析） |

`zlua.types.*` 见 §4.2：预置常用 corlib 类型的**类型全名**（如 `System.Int32`），可直接作为 `typeArg` 传入。

---

## 4. 类型查询与常量

### 4.1 `zlua.typeof`

```lua
zlua.typeof(typeTable) → typeDescriptor
```

| 参数 | 说明 |
|------|------|
| `typeTable` | `CSharp` 下的类型表（含 `__fullname` 等元数据） |

返回该类型的 **System.Type 等价物**（Mono：`System.Type`；Il2Cpp：携带 `Il2CppClass*` 的类型描述对象）。供需要显式类型对象的场景使用；多数 API 也可直接传 `typeArg`。

```lua
local t = zlua.typeof(CSharp.mscorlib['System.Int32'])
local demoType = zlua.typeof(CSharp.AC.Demo)   -- 无 namespace 全局类型
local panelType = zlua.typeof(CSharp.AC['MyGame.UI.Panel'])  -- 含 namespace
```

### 4.2 `zlua.types`

预置常用 corlib 类型全名，避免重复写长字符串：

```lua
zlua.types.int32      -- "System.Int32"
zlua.types.int64
zlua.types.single     -- System.Single
zlua.types.double
zlua.types.boolean
zlua.types.string
zlua.types.object
zlua.types.void       -- 若暴露；多用于签名场景
-- … 由 zlualib 或 native 初始化补全
```

可直接作为 `typeArg` 传入 `signature`、`make_generic_type`、`make_szarray_type` 等。

> **命名：** 旧称 `zlua.corlibtypes` 已废弃，统一为 `zlua.types`。

---

## 5. 泛型类型

### 5.1 `zlua.make_generic_type`

```lua
zlua.make_generic_type(genericBaseType, typeArg1, typeArg2, ...) → typeTable
```

| 参数 | 说明 |
|------|------|
| `genericBaseType` | 未闭合泛型定义的类型表；**含 namespace 时必须括号访问** |
| `genericParamType…` | 泛型实参；typeArg 规则见 `TYPE_SYSTEM_SPEC.md` §2.4 |

```lua
local ListInt = zlua.make_generic_type(
    CSharp.mscorlib['System.Collections.Generic.List'],
    zlua.types.int32
)
local list = ListInt()   -- 构造实例，见 TYPE_SYSTEM_SPEC §4.6
```

- 返回闭合泛型的**类型表**；相同实参多次调用应 **intern** 为同一表。
- 实参个数须与泛型定义一致，否则 `luaL_error`。

**Native：** `__zlua_make_generic_type`

---

## 6. ref 变量（`zlua.new_ref`）

创建 **带类型的 StructUserData**，作为 C# `ref` / `out` / `in` 形参的 **真 ref 实参**。规则见 `MARSHAL_SPEC.md` §3、`STRUCT_MARSHAL_SPEC.md` §6.2。

```lua
zlua.new_ref(ref_type [, value, ...]) → structUserdata
```

| 参数 | 说明 |
|------|------|
| `ref_type` | **typeArg**（§3）：须为 **值类型**（基元、enum、struct）；引用类型 → 报错 |
| `value` | 可选。省略或 `nil` → `default(T)` 写入 payload |
| `...` | struct / enum 时，额外参数同类型表 `_ctor` / `__call` 规则 |

**返回值：** `T` 的 StructUserData（挂接 `IMT`），payload 即 ref 目标存储；C# 经 `ref T` 修改后 **原地** 反映到该 userdata。

### 6.1 与 `_ctor` 的关系

| 创建方式 | 传给 `ref T` |
|----------|--------------|
| `zlua.new_ref(T, ...)` | 真 ref |
| `T._ctor(...)` / `T(...)` | **真 ref**（同一 StructUserData 模型） |

by-val 形参仍对 StructUserData 做 **拷贝**；仅 `ref`/`out`/`in` 绑定 payload 地址。

非 StructUserData 实参（如裸 `5`）传给 `ref int`：**拷贝语义**，调用成功但 **不回写** Lua 值（`MARSHAL_SPEC.md` §3.1）。

### 6.2 示例

```lua
-- ref 基元（须 new_ref 才有真 ref；裸 number 为拷贝语义）
local n = zlua.new_ref(zlua.types.int32, 5)
CS.Demo.Increment(n)

-- ref struct：_ctor 即真 ref
local p = Point2D(1, 2)
CS.Demo.Offset(p, 10, 20)

-- ref struct：new_ref + 默认 / 构造
local q = zlua.new_ref(Point2D)
local r = zlua.new_ref(Point2D, 3, 4)

-- out
local out = zlua.new_ref(zlua.types.int32)
CS.Demo.TryParse("42", out)
```

### 6.3 读取 payload（可选辅助）

实现可提供 `zlua.deref(refUserdata)` 或约定经 `IMT` 字段访问；MVP 以 struct **实例字段 API** 与 enum/基元 payload 读取为准。

**Native：** `__zlua_new_ref`

---

## 7. 数组类型与实例

与 `TYPE_SYSTEM_SPEC.md` §2.4–§2.6、§7 一致。

### 7.1 数组类型构造

```lua
zlua.make_szarray_type(typeArg) → szarrayTypeTable
zlua.make_mdarray_type(typeArg, rank) → mdarrayTypeTable
```

| API | 说明 |
|-----|------|
| `make_szarray_type` | 单维 0 基向量数组 `T[]`；`elementType` 见 `TYPE_SYSTEM_SPEC.md` §2.4 |
| `make_mdarray_type` | `rank` 维数组 `T[,…]`，`rank ≥ 1`；`elementType` 见 `TYPE_SYSTEM_SPEC.md` §2.4 |

```lua
local IntArray = zlua.make_szarray_type(zlua.types.int32)
local IntMatrix = zlua.make_mdarray_type(zlua.types.int32, 2)
```

### 7.2 数组实例创建

```lua
zlua.new_szarray_by_element_type(typeArg, length) → szarrayUserdata
zlua.new_szarray_by_szarray_type(szarrayTypeTable, length) → szarrayUserdata

zlua.new_mdarray_by_mdarray_type(mdarrayTypeTable, lowbounds, sizes) → mdarrayUserdata
zlua.new_mdarray_by_spec(typeArg, lowbounds, sizes) → mdarrayUserdata
```

| 参数 | 说明 |
|------|------|
| `length` | szarray 长度，`≥ 0` |
| `lowbounds` | 长度为 `rank` 的 Lua 表，每维下界 |
| `sizes` | 长度为 `rank` 的 Lua 表，每维元素个数 |

元素初始化为 `default(T)`。szarray 实例支持 `#arr`（`__len`，等价 `Length`）。

### 7.3 szarray 转换

#### `zlua.to_bytes`

```lua
zlua.to_bytes(szarray) → string
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
local bytes = zlua.to_bytes(int_arr)   -- #bytes == #int_arr * 4（int 为 4 字节）
```

元素类型不在白名单内时 `luaL_error`。

**Native：** `__zlua_to_bytes`

#### `zlua.to_table`

```lua
zlua.to_table(szarray) → table
```

将 szarray 转为 **等长** Lua 表；**对元素类型无限制**，每个元素按 `MARSHAL_SPEC.md` / `CLASS_MARSHAL_SPEC.md` 规则转为 Lua 值。

| 约束 | 说明 |
|------|------|
| 输入 | 必须是 szarray userdata |
| 输出长度 | `n = #szarray`；返回表在 `1 .. n` 上连续赋值 |
| 下标对应 | `t[i]` ↔ C# `arr[i - 1]`（Lua 1 基 ↔ C# 0 基） |

```lua
local t = zlua.to_table(obj_arr)
-- t[1] 对应 arr[0]，t[#t] 对应 arr[Length-1]
```

引用类型元素转为 userdata；值类型 struct 按 class marshal 规则处理。

**Native：** `__zlua_to_table`

---

**Native（§6.1–6.2）：** `__zlua_make_szarray_type`、`__zlua_new_szarray_*` 等（名称以实现为准，语义对齐本文档）。

---

## 8. 泛型方法实参

仅用于**方法自身带泛型参数**的情形（如 `void Foo<T>(T x)`），见 `TYPE_SYSTEM_SPEC.md` §6。

### 8.1 `zlua.make_generic_inst`

```lua
zlua.make_generic_inst(typeArg1, typeArg2, ...) → genericInst
```

构造泛型方法调用所需的 **`generic_inst`** 句柄；native 校验类型参数个数与约束。

### 8.2 调用约定

```lua
local inst = zlua.make_generic_inst(zlua.types.int32)
Type.Foo(inst, value)   -- 第一实参必须是 generic_inst
```

每个泛型方法在 native 维护 `inflatedMap`：`generic_inst` 指纹 → 单态化后的 bridge closure。

每个泛型方法在 native 维护 `inflatedMap`：`generic_inst` 指纹 → 单态化后的 bridge closure。

---

## 9. Delegate（可选显式 API）

**默认行为：** Lua 调用带 delegate 形参的 C# 方法时，直接传入 Lua function 即可，由 **方法参数 marshal** 隐式转换，与其它形参规则相同。详见 **`FUNCTION_MARSHAL_SPEC.md` §4.0**。

```lua
obj:RegisterCallback(function(v) print(v) end)   -- 无需 to_delegate
```

### 8.1 `zlua.to_delegate`（可选）

仅在需要先构造 delegate、再传递或缓存时使用（非常规路径）。

```lua
zlua.to_delegate(func, delegateTypeTable) → delegateUserdata
```

```lua
local d = zlua.to_delegate(function(a) return a end, closedFuncIntIntType)
obj:RegisterCallback(d)
```

| 参数 | 说明 |
|------|------|
| `func` | Lua function |
| `delegateTypeTable` | 已闭合的 delegate 类型表（无方法形参上下文时须显式指定） |

**Native：** `__zlua_to_delegate`（可选，待实现）

C# delegate 传入 Lua 后可直接 `d(...)`（`IMT.__call`），见 `FUNCTION_MARSHAL_SPEC.md` §3。

---

## 10. 方法重载辅助

完整语义见 `METHOD_OVERLOAD_SPEC.md`。`zlua` 仅提供薄封装。

### 9.1 `zlua.signature`

```lua
zlua.signature([typeArg1, typeArg2, ...]) → paramSignature
```

- **不包含** 方法名；仅参数类型列表。
- 无参：`()`；有参：`(System.Int32,…)`，使用 `Type.FullName` 规范格式。

```lua
local sig = zlua.signature(zlua.types.int32)           -- "(System.Int32)"
local sig2 = zlua.signature(zlua.types.int32, zlua.types.string)
```

**禁止**将返回值用作 `obj[sig]` 查表键；应配合 `get_method` 或模块级缓存。

**Native：** `__zlua_create_signature`（Lua 层不再暴露 `create_signature` 公共 API；若存在则视为 `signature` 别名并标记废弃）。

### 9.2 `zlua.get_method`

```lua
zlua.get_method(target, methodName, signature, is_static) → closure
```

| 参数 | 说明 |
|------|------|
| `target` | 实例 userdata 或类型表；用于解析声明类型 |
| `methodName` | C# 方法名 |
| `signature` | `zlua.signature(...)` 返回值 |
| `is_static` | `true` 查静态域；`false` 查实例域 |

```lua
local demo = CSharp.AC.Demo()
local run_i32 = zlua.get_method(demo, "Run", zlua.signature(zlua.types.int32), false)
run_i32(demo, 10)

local add = zlua.get_method(CSharp.AC.Demo, "Add",
    zlua.signature(zlua.types.int32, zlua.types.int32), true)
add(3, 5)
```

- 实例方法：点号调用并传入 `self`；静态方法：直接 `closure(...)`。
- `is_static == true` 时 `target` 可为实例，仅用于解析类型（见 `TYPE_SYSTEM_SPEC.md` §3.2）。

**Native：** `__zlua_get_method`（待实现）

### 9.3 `zlua.register_method`

```lua
zlua.register_method(aliasName, methodClosure) → void
```

将 `get_method` 返回的 closure 注册为类型元表别名：

```lua
zlua.register_method("run_i32", run_i32)
demo:run_i32(20)
```

| 参数 | 说明 |
|------|------|
| `aliasName` | 类内唯一 Lua 键（与 `[LuaAlias]` 规则一致，见 `METHOD_OVERLOAD_SPEC.md` §5） |
| `methodClosure` | 内含 `klass`、`is_static`；决定写入类型表或 `__instance_mt` |

**Native：** `__zlua_register_method`（待实现）

---

## 11. 与 `CSharp` 的配合示例

```lua
CSharp.AC = CSharp['Assembly-CSharp']

-- 类型解析（TYPE_SYSTEM_SPEC §2.2）
local Demo = CSharp.AC.Demo                      -- 无 namespace
local Panel = CSharp.AC['MyGame.UI.Panel']       -- 有 namespace：必须括号

-- typeof / types
local demo = Demo()
local sig = zlua.signature(zlua.types.int32)

-- 泛型
local ListInt = zlua.make_generic_type(
    CSharp.mscorlib['System.Collections.Generic.List'],
    zlua.types.int32
)

-- 数组
local IntArray = zlua.make_szarray_type(zlua.types.int32)
local arr = zlua.new_szarray_by_szarray_type(IntArray, 4)
print(#arr)
local bytes = zlua.to_bytes(arr)
local t = zlua.to_table(arr)

-- 重载
local run_i32 = zlua.get_method(demo, "Run", sig, false)
zlua.register_method("run_i32", run_i32)
```

---

## 12. Native 回调一览

| Lua API | Native 回调 | 状态（参考） |
|---------|-------------|--------------|
| `zlua.typeof` | `__zlua_typeof` | Mono / Il2Cpp MVP |
| `zlua.signature` | `__zlua_create_signature` | Mono（待对齐：仅 typeArg） |
| `zlua.make_generic_type` | `__zlua_make_generic_type` | Mono；Il2Cpp 占位 |
| `zlua.new_ref` | `__zlua_new_ref` | 待实现 |
| `zlua.make_szarray_type` | `__zlua_make_szarray_type` | 待实现 |
| `zlua.make_mdarray_type` | `__zlua_make_mdarray_type` | 待实现 |
| `zlua.new_szarray_*` | `__zlua_new_szarray_*` | 待实现 |
| `zlua.to_bytes` | `__zlua_to_bytes` | 待实现 |
| `zlua.to_table` | `__zlua_to_table` | 待实现 |
| `zlua.new_mdarray_*` | `__zlua_new_mdarray_*` | 待实现 |
| `zlua.make_generic_inst` | `__zlua_make_generic_inst` | 待实现 |
| `zlua.to_delegate` | `__zlua_to_delegate` | 可选，待实现 |
| `zlua.get_method` | `__zlua_get_method` | 待实现 |
| `zlua.register_method` | `__zlua_register_method` | 待实现 |

新增 native 回调在 `LuaInteropManager::RegisterZLuaApi`（Il2Cpp）/ `LuaManagerObject`（Mono）注册；Lua 侧仅 `function zlua.xxx(...) return __zlua_xxx(...) end`。

---

## 13. `zlualib.lua` 目标骨架

```lua
zlua = zlua or {}

function zlua.typeof(typeTable)
    return __zlua_typeof(typeTable)
end

function zlua.signature(...)
    return __zlua_create_signature(...)
end

function zlua.make_generic_type(genericBase, ...)
    return __zlua_make_generic_type(genericBase, ...)
end

function zlua.new_ref(ref_type, ...)
    return __zlua_new_ref(ref_type, ...)
end

function zlua.make_szarray_type(elementType)
    return __zlua_make_szarray_type(elementType)
end

function zlua.make_mdarray_type(elementType, rank)
    return __zlua_make_mdarray_type(elementType, rank)
end

function zlua.new_szarray_by_element_type(elementType, length)
    return __zlua_new_szarray_by_element_type(elementType, length)
end

function zlua.new_szarray_by_szarray_type(szarrayType, length)
    return __zlua_new_szarray_by_szarray_type(szarrayType, length)
end

function zlua.to_bytes(szarray)
    return __zlua_to_bytes(szarray)
end

function zlua.to_table(szarray)
    return __zlua_to_table(szarray)
end

function zlua.new_mdarray_by_mdarray_type(mdarrayType, lowbounds, sizes)
    return __zlua_new_mdarray_by_mdarray_type(mdarrayType, lowbounds, sizes)
end

function zlua.new_mdarray_by_spec(elementType, lowbounds, sizes)
    return __zlua_new_mdarray_by_spec(elementType, lowbounds, sizes)
end

function zlua.make_generic_inst(...)
    return __zlua_make_generic_inst(...)
end

function zlua.to_delegate(func, delegateType)
    return __zlua_to_delegate(func, delegateType)
end

function zlua.get_method(target, methodName, signature, is_static)
    return __zlua_get_method(target, methodName, signature, is_static)
end

function zlua.register_method(aliasName, methodClosure)
    return __zlua_register_method(aliasName, methodClosure)
end

zlua.types = zlua.types or {
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

当前仓库内 `zlualib.lua` 仍为早期原型（含 `create_signature(methodName, …)`、`get_method` 两参数等），**以实现清单为准逐步对齐本规范**。

---

## 14. 实现清单

- [ ] `zlua.new_ref` / `__zlua_new_ref`（`MARSHAL_SPEC.md` §3）
- [ ] `zlua.types` 初始化与 `corlibtypes` 迁移
- [ ] `signature` 仅接收 `typeArg`，去掉 methodName
- [ ] `get_method(target, methodName, signature, is_static)` native 实现
- [ ] `register_method(aliasName, closure)` native 实现
- [ ] 数组 `make_*` / `new_*` 全套 API
- [ ] `to_bytes` / `to_table`（szarray）
- [ ] `Marshaling::ReadDelegate` 隐式 marshal（`FUNCTION_MARSHAL_SPEC.md` §4.0，**必做**）
- [ ] （可选）`to_delegate` 显式 API
- [ ] `make_generic_inst` + 泛型方法 `inflatedMap`
- [ ] Mono / Il2Cpp `zlualib.lua` 内容同步（嵌入 vs Resources）
- [ ] `TYPE_SYSTEM_SPEC.md` §2.2 命名空间括号规则在类型解析回调中强制执行
