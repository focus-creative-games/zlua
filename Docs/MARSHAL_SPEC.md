# C# 与 Lua 参数 Marshal 设计规范

本文档为 **参数编组总览**；具体类型规则见分册：

| 文档 | 类型 |
|------|------|
| `TYPE_SYSTEM_SPEC.md` | 类型解析、元表、成员访问、**枚举类型表** |
| `STRUCT_MARSHAL_SPEC.md` | struct（值类型）、**枚举 userdata 构造** |
| `CLASS_MARSHAL_SPEC.md` | class、引用类型、数组元素 |
| `FUNCTION_MARSHAL_SPEC.md` | **Delegate、Lua 函数回调** |
| `METHOD_OVERLOAD_SPEC.md` | 重载与实参匹配 |

**平台原则：** Mono 与 Il2Cpp 的 Lua 可见编组语义一致；Il2Cpp 侧重零 GC 与生成代码快速路径。

**函数 / delegate：** Lua 调用 C# 方法时，delegate 形参接受 Lua function，由 `MethodBridge` 隐式 marshal，规则见 `FUNCTION_MARSHAL_SPEC.md` §4.0。

---

## 1. 基元与字符串（概要）

| C# 类型 | Lua 形态（默认） | 说明 |
|---------|------------------|------|
| `bool` | `boolean` | |
| 整型基元（`byte`…`ulong`） | **integer**（Lua 5.4+ 优先）或 **number** | 见 §1.1 |
| `float` / `double` | `number` | |
| `string` | `string` | |
| `char` | integer / number | 按 Unicode 码点 |

### 1.1 integer 与 number

- **Lua 5.4+**：整型基元及枚举底层整型优先使用 **integer**（`lua_pushinteger` / `lua_isinteger`）。
- **不支持 integer 的 Lua 版本**：退化为 **number**，须为整数值（无小数部分）。
- Il2Cpp Codegen 与 Mono 反射路径的 **可见语义一致**；仅实现层 API 不同。

（class、struct、数组等详细规则见各分册。）

---

## 2. 枚举（enum）

枚举在 C# 中为 **值类型**，底层为单一整型字段。Lua 侧 **默认不按 userdata 传递**，而按 **integer / number** 编组；需要 boxed 形态时通过类型表 `_ctor` / `__call` 构造 userdata（见 `STRUCT_MARSHAL_SPEC.md` §5.4、`TYPE_SYSTEM_SPEC.md` §3.5）。

### 2.1 默认规则（C# ↔ Lua）

| 方向 | 默认形态 | 说明 |
|------|----------|------|
| **C# → Lua** | **integer**（优先）或 **number** | 推送枚举的 **底层整数值**，不推送 userdata |
| **Lua → C#** | **integer** / **number** | 接受整型 Lua 值，按目标枚举 **底层类型** 转换并 `Enum.ToObject` / 等价路径 |
| **Lua → C#**（备选） | **enum userdata** | 从 userdata payload 读出底层整型再转换（见 `STRUCT_MARSHAL_SPEC.md` §5.4） |

**不接受**（除非 `[LuaMarshalAs]` 另行规定）：将枚举默认编组为 **string**（枚举名）、**boolean**、或普通 **table**。

### 2.2 底层类型与范围

Codegen / 反射须读取枚举 **underlying type**（`System.Int32`、`System.Byte` 等）：

- Pop 时校验 Lua 整型值是否落在底层类型可表示范围内；越界 → `luaL_error`。
- Push 时使用与底层类型宽度一致的 integer/number 语义。

| 底层类型 | Push 优先 | Pop 接受 |
|----------|-----------|----------|
| `sbyte` … `ulong` | integer / number | integer / number（整型） |
| 非整型底层（罕见） | number | number |

### 2.3 与类型表常量字段的关系

`CSharp[assembly][EnumType].MemberName` 在类型表上暴露为 **同名 integer/number 字段**（非 userdata），其值等于该枚举常量的底层整型值。详见 `TYPE_SYSTEM_SPEC.md` §3.5。

下列写法在作为 **enum 形参** 时等价（默认 marshal）：

```lua
local e = CSharp.AC['MyGame.Color'].Red   -- integer/number 常量
foo(e)
foo(CSharp.AC['MyGame.Color'].Red)
foo(1)   -- 裸整型，须能转换为该 enum
```

### 2.4 userdata 形态（非默认，显式构造）

当脚本需要 **enum 实例 userdata**（长生命周期持有、与 struct 相同的 `:` 实例 API、或 `[LuaMarshalAs]` 强制 userdata）时：

```lua
local boxed = CSharp.AC['MyGame.Color'](CSharp.AC['MyGame.Color'].Red)
-- 或
local boxed = CSharp.AC['MyGame.Color']._ctor(CSharp.AC['MyGame.Color'].Blue)
```

构造语义与 **blittable struct** 的 `_ctor` → userdata 相同，见 `STRUCT_MARSHAL_SPEC.md` §5.4。

作为 **enum 形参** 传入 C# 时，userdata 与 integer/number **均接受**（默认规则 §2.1）。

### 2.5 `[LuaMarshalAs]` 扩展

类型级或参数级可覆盖默认行为（如强制 userdata、string 名等）；未标注时适用本节默认规则。配置语法见 `STRUCT_MARSHAL_SPEC.md` §7。

### 2.6 Mono / Il2Cpp 一致性

| 项 | 要求 |
|----|------|
| 默认 Push / Pop | integer/number ↔ 底层整型 |
| 常量字段 | 类型表上为 integer/number |
| `_ctor` / `__call` | 单整型参数 → enum userdata |
| 错误消息 | 一致或等价 |

---

## 3. `ref` / `out` / `in`（Lua → C#）

**范围：** 仅 **Lua 调用 C#** 方法/构造时的形参；`[LuaInvoke]`、delegate bridge **不支持** ref/out（见 `FUNCTION_MARSHAL_SPEC.md` §9）。

**统一规则：** Lua 侧 **不区分** `ref` / `out` / `in`，均按 **ref 语义** 处理；C# 侧仍保留各自 CLR 语义（`in` 只读等）。

### 3.1 核心原则：StructUserData = 真 ref，否则 = 拷贝

| Lua 实参 | C# `ref`/`out`/`in` 形参 Pop 行为 | C# 修改后 Lua 侧 |
|----------|-----------------------------------|------------------|
| **StructUserData**（见 §3.2）且类型与 `T` 一致 | 绑定 payload 地址 / box，**真 ref** | userdata 内值/字段已更新 |
| **其他可转换形态**（number、string、table、nil…） | 按 **by-val** 规则读入 **临时 ref 槽**，**拷贝语义** | **不变**；**不报错** |

```lua
local x = 5
CS.Demo.Increment(x)   -- 正常执行；C# 内 ref 参数已变，Lua 的 x 仍为 5

local n = zlua.new_ref(zlua.types.int32, 5)
CS.Demo.Increment(n)   -- 真 ref；zlua.deref(n) 或读 payload 得新值
```

**设计理由：** 裸 Lua 值无 lvalue，无法回写 local；与 C# 将 by-val 实参隐式拷入临时变量再取 ref 的行为一致，脚本不应因此失败。

### 3.2 何谓 StructUserData（ref 变量）

满足以下条件的 userdata 视为 **ref 变量**（真 ref 实参）：

| 来源 | 说明 |
|------|------|
| `zlua.new_ref(ref_type [, value, ...])` | 显式 ref 槽；见 `LIB_SPEC.md` §6 |
| 值类型 **`_ctor` / `__call`** 产物 | 如 `Point2D(1, 2)`；与 `new_ref` 共用 payload 存储，传给 `ref Point2D` 为 **真 ref** |
| enum **`_ctor` / `__call`** 产物 | 如 `Color(1)`；传给 `ref Color` 为真 ref |
| C#→Lua 推送的 struct **StructUserData** | 长生命周期 userdata；Lua 再传入 `ref T` 为真 ref |

**类型校验：** Pop 时 `userdata` 绑定类型须与形参元素类型 `T`（`parameterType.GetElementType()`）**精确匹配**；不匹配 → `luaL_error`。

**非 ref 变量：** 基元/enum 的 integer、string、table 等 **不是** StructUserData，走 §3.1 拷贝分支。

### 3.3 按元素类型 `T` 的分支

| `T` | 真 ref（StructUserData） | 拷贝语义（非 StructUserData） |
|----|--------------------------|-------------------------------|
| 基元 / enum | payload 原地更新 | 临时槽；Lua 裸值不变 |
| struct | payload / box 原地更新；字段经 `IMT` 可见 | 临时 struct 副本；原 Lua 值不变 |
| class / string / array / delegate | 共享对象引用；**C# 对 ref 重新赋值不回 Lua** | 同左；临时槽丢弃 rebind |

引用类型 **重新绑定** ref（`refParam = other`）**永不**写回 Lua；**可变对象原地修改**（如 `ref StringBuilder`）仍通过共享引用可见。详见 `CLASS_MARSHAL_SPEC.md` §2。

### 3.4 `out` 与缺省 / `nil`

| 实参 | 拷贝分支 | StructUserData 分支 |
|------|----------|---------------------|
| 省略 / `nil` | 临时 `default(T)`；Invoke 后 **丢弃** | `new_ref(T)` / `new_ref(T, nil)` → `default(T)`；Invoke 后 payload 更新 |

若要取得 `out` 结果，须传入 **`zlua.new_ref(T)`**（或已存在的同类型 StructUserData）。

### 3.5 桥接流程（概念）

```text
PopRefArgument(luaIndex, T, isOut):
  if IsStructUserData(luaIndex) && userdata.Type == T:
      return BindRef(&payload)              // 真 ref；Invoke 后 Lua 已可见
  value = PopByValue(luaIndex, T)           // 与 by-val 相同
  if isOut && (missing || nil): value = default(T)
  temp = AllocTempRefSlot(T, value)         // 仅本次 Invoke 有效
  return BindRef(temp)                      // Invoke 后丢弃，不写回 Lua 栈/local
```

Il2Cpp：`temp` 在 `MethodBridge` 栈帧；StructUserData 为 `lua_newuserdata` payload 指针。  
Mono：可先 GCHandle box；**可观察语义**与 Il2Cpp 一致。

### 3.6 相关 API 与文档

| 项 | 文档 |
|----|------|
| `zlua.new_ref` | `LIB_SPEC.md` §6 |
| struct payload / Handle | `STRUCT_MARSHAL_SPEC.md` §5、§6.2 |
| ref 引用类型 | `CLASS_MARSHAL_SPEC.md` §2 |
| delegate 不含 ref | `FUNCTION_MARSHAL_SPEC.md` §9 |

---

## 4. 其他类型（索引）

| 类型 | 文档 |
|------|------|
| struct | `STRUCT_MARSHAL_SPEC.md` |
| class / 引用类型 | `CLASS_MARSHAL_SPEC.md` |
| delegate / Lua function | `FUNCTION_MARSHAL_SPEC.md` |
| 数组 | `CLASS_MARSHAL_SPEC.md`、`TYPE_SYSTEM_SPEC.md` §7 |
