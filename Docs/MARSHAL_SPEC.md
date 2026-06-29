# C# 与 Lua 参数 Marshal 设计规范

本文档为 **参数编组总览**；具体类型规则见分册：

| 文档 | 类型 |
|------|------|
| `TYPE_SYSTEM_SPEC.md` | 类型解析、元表、成员访问、**枚举类型表** |
| `STRUCT_MARSHAL_SPEC.md` | struct（值类型）、**枚举 userdata 构造**、`StructStackScope` |
| `CLASS_MARSHAL_SPEC.md` | class、引用类型、数组元素 |
| `FUNCTION_MARSHAL_SPEC.md` | **Delegate、Lua 函数回调** |
| `METHOD_OVERLOAD_SPEC.md` | 重载与实参匹配 |

**平台原则：** Mono 与 Il2Cpp 的 Lua 可见编组语义一致；Il2Cpp 侧重零 GC 与生成代码快速路径。

**函数 / delegate：** Lua 调用 C# 方法时，delegate 形参接受 Lua function，由 `MethodBridge` 隐式 marshal，规则见 `FUNCTION_MARSHAL_SPEC.md` §4.0。

**覆盖标注：** 参数、返回值、方法或字段上的 `[LuaMarshalAs]` 可覆盖默认规则；须符合 **§6.2 合法集合**，否则 **§6.3** 回退 Default 并在 Editor 打错误日志。

---

## 1. 默认 Marshal 规则总览

未标注 `[LuaMarshalAs]`（或标注为 `LuaMarshalType.Default`）时，各类型在 **C# ↔ Lua** 双向调用中的默认编组如下。

| C# 类型 | C# → Lua | Lua → C# | 说明 |
|---------|----------|----------|------|
| `bool` | `boolean` | `boolean` | |
| `char` | **integer** / **number** | **integer** / **number** | 按 Unicode 码点（16 位） |
| `byte` | **integer** / **number** | **integer** / **number** | 见 §1.1 |
| `sbyte` | **integer** / **number** | **integer** / **number** | 见 §1.1 |
| `short` | **integer** / **number** | **integer** / **number** | 见 §1.1 |
| `ushort` | **integer** / **number** | **integer** / **number** | 见 §1.1 |
| `int` | **integer** / **number** | **integer** / **number** | 见 §1.1 |
| `uint` | **integer** / **number** | **integer** / **number** | 见 §1.1 |
| `long` | **integer** / **number** | **integer** / **number** | 见 §1.1 |
| `ulong` | **integer** / **number** | **integer** / **number** | 见 §1.1；须落在 Lua integer 可表示范围 |
| `float` | **number** | **number** | |
| `double` | **number** | **number** | |
| `IntPtr` | **integer** / **number** | **integer** / **number** | 指针 **数值**（`ToInt64` / `new IntPtr`）；与 §7.1 **非托管指针** 不同 |
| `UIntPtr` | **integer** / **number** | **integer** / **number** | 指针 **数值**（`ToUInt64` / `new UIntPtr`）；与 §7.1 不同 |
| `nint` / `nuint` | 同 `IntPtr` / `UIntPtr` | 同 `IntPtr` / `UIntPtr` | C# 本机整数别名；语义与对应指针宽度类型一致 |
| `T*`（非托管指针，如 `int*`、`void*`） | **Pointer**（lightuserdata） | **Pointer**（lightuserdata） | 仅 **透传**；见 §7.1 |
| 函数指针（如 `delegate*<int,int>`） | **Pointer**（lightuserdata） | **Pointer**（lightuserdata） | 仅 **透传**；见 §7.2 |
| `System.TypedReference` | **不支持** | **不支持** | 无互操作意义；见 §7.3 |
| `string` | **string** | **string** | |
| `byte[]` | **ArrayUserData** | **ArrayUserData** | 与 `T[]` 相同；`[LuaMarshalAs(Bytes)]` 时改为 ↔ **string**，见 §6 |
| `class` | **ClassUserData** | **ClassUserData** | 引用身份；`nil` ↔ `null` |
| `T[]`（一维 / szarray） | **ArrayUserData** | **ArrayUserData** | 见 `CLASS_MARSHAL_SPEC.md`、`TYPE_SYSTEM_SPEC.md` §7 |
| `T[,]` 等多维（mdarray） | **ArrayUserData** | **ArrayUserData** | 同上 |
| `enum` | **integer** / **number** | **integer** / **number** 或 **enum userdata** | 默认 **不** 推送 userdata；详见 §2 |
| `struct` | **OpaqueValue**（lightuserdata） | **StructUserData** 或 **table** | C#→Lua 默认 **StructHandle**（§4）；Lua→C# 亦接受 `_ctor` / `__call` 产物；table 规则见 `STRUCT_MARSHAL_SPEC.md` §6.4 |
| `Delegate` | **DelegateUserData** | **function** 或 **DelegateUserData** | Lua→C# 接受 Lua function 隐式 bridge；见 `FUNCTION_MARSHAL_SPEC.md` |
| `object` | 按 **运行时类型** Push | **boolean** / **number** / **string** / **userdata** | 多态装箱；见 §7.4 |
| `Nullable<T>` | 同 `T` 或 `nil` | 同 `T` 或 `nil` | `T` 为值类型时 `nil` ↔ `null`；见 §7.4 |
| `interface` | **ClassUserData** | **ClassUserData** | 与 class 相同，按 **实现对象** 编组 |
| `decimal` | **暂不支持**（默认） | **暂不支持**（默认） | 见 §7.4 |
| `ref struct`（如 `Span<T>`） | 见 `STRUCT_MARSHAL_SPEC.md` / `LIB_SPEC.md` | 见 `STRUCT_MARSHAL_SPEC.md` | 不能作为普通 by-val 形参默认传递；见 §7.4 |
| `void`（返回值） | （无） | — | |
| `null` / `nil` | `nil` | `nil` | 仅 **引用类型**、**Nullable**、delegate 等可空形态 |

**UserData 形态：** 上表中的 ClassUserData、ArrayUserData、StructUserData、enum userdata、DelegateUserData 均为带类型元表的 **full userdata**（`lua_newuserdata` + metatable），脚本侧经 `:` / `.` 访问成员；与 §4 的 **OpaqueValue**（无 metatable 的 lightuserdata）以及 §7.1 的 **Pointer**（无 metatable、仅透传的 lightuserdata）不同。

### 1.1 integer 与 number

- **Lua 5.4+**：整型基元、`char`、枚举底层整型、`IntPtr` / `UIntPtr` 数值优先使用 **integer**（`lua_pushinteger` / `lua_isinteger`）。
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

类型级或参数级可覆盖 §1 默认行为；`LuaMarshalType` 语义见 **§6**。struct 相关的双向配置（如 `StructUserData`、`ComposeFromStack`）另见 `STRUCT_MARSHAL_SPEC.md` §7。

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
| `zlua.to_user_data` | 本节 §4.4、`LIB_SPEC.md` |
| struct payload / scope | `STRUCT_MARSHAL_SPEC.md` §5、§6.2 |
| ref 引用类型 | `CLASS_MARSHAL_SPEC.md` §2 |
| delegate 不含 ref | `FUNCTION_MARSHAL_SPEC.md` §9 |

---

## 4. OpaqueValue（临时不透明参数）

本节定义 **C#→Lua marshal** 时，对 **栈上临时形参/局部** 的统一承载形态（原 `StructValueOps` 职责，现扩展并更名）。struct 默认路径（`StructHandle`）、以及需零拷贝暴露 **引用类型形参槽** 的场景，均走 OpaqueValue。参数/返回值标注 **`[LuaMarshalAs(LuaMarshalType.OpaqueLightUserData)]`** 时 **强制** 走本形态，见 **§6.3**。

### 4.1 Lua 可见形态

| 项 | 规则 |
|----|------|
| Lua 类型 | **lightuserdata** |
| 载荷 | **handleId**，类型为 **`uintptr_t`（`void*`）**，与 `lua_pushlightuserdata` / `lua_touserdata` 一致 |
| 平台 | **须兼容 32 位**：使用 `uintptr_t` / `intptr_t`，**不得**假定 `uint64_t` 或固定 64 位编码 |
| 不透明性 | Lua **不能**将其当作地址解引用；仅 C++ / 绑定层在 `Validate` 通过后解析 |

```cpp
// Push（概念）
void* handleId = EncodeOrRegister(paramSlotPtr);  // 见 §4.2
lua_pushlightuserdata(L, (void*)(uintptr_t)handleId);
```

**无 metatable、无 `__index` / `__newindex`、无字段/方法访问。** 脚本侧把它当作 **不可操作的临时令牌**。

### 4.2 handleId 与参数地址

`handleId` 在逻辑上映射到 **C# 调用栈上某一参数的存储地址**（`StructStackScope` 登记，见 `STRUCT_MARSHAL_SPEC.md` §3）：

| 元素类型 `T` | `stack[i]` / 登记地址含义 | `Resolve` 结果 |
|--------------|---------------------------|----------------|
| **struct**（值类型） | 指向 struct **实例字节** 的 `void*` | `T` 的栈上实例地址 |
| **class** 等引用类型 | 指向 **`Il2CppObject*` 槽** 的 `void*`（即 `Il2CppObject**`） | 引用槽地址；读 `*slot` 得对象指针 |

因此 OpaqueValue **不仅能表达 struct，也能表达 object 形参槽**；与长生命周期的 **ClassUserData**（`ObjectRegistry`）不同，此处 **不** 立即注册 GCHandle / userdata，仅暴露「本次调用栈上的参数位置」。

**校验（沿用现有设计）：** 任意使用前 `ValidateHandle(handleId)`——解码或查表确认 `generation` 仍有效、登记未撤销（`STRUCT_MARSHAL_SPEC.md` §3.4）。过期 → `luaL_error`（如 `zlua: opaque handle expired`）。

### 4.3 生命周期与禁止持久化

| 规则 | 说明 |
|------|------|
| 产生 | 仅 **C#→Lua** marshal 路径；**无** Lua API 伪造 handle |
| 有效域 | 与产生它的 **C# 函数形参/局部作用域** 及外层 **`StructStackScope`** 同步；`EndScope` 递增 `generation` 后集体失效 |
| 禁止保存 | **不得** 写入 upvalue、全局、表字段后在 **异步** 或 **后续 pcall** 中使用 |
| 失效后 | 再调用 `zlua.to_user_data` 或作为实参 Pop → **报错**（校验失败） |

```lua
local h = CS.Demo.GetPointHandle()   -- lightuserdata，同步链内有效
-- 若 pcall 返回后仍持有 h 并在下次调用中使用 → error
```

长生命周期、可字段访问的实例须转为 **StructUserData** 或 **ClassUserData**（§4.4）。

### 4.4 唯一脚本操作：`zlua.to_user_data`

OpaqueValue **不支持** 任何成员访问；**唯一** 支持的 Lua 操作是升级为正式 userdata：

```lua
local opaque = ...   -- lightuserdata from C#→Lua
local ud = zlua.to_user_data(opaque)
```

| 解析类型 | 产出 | 语义 |
|----------|------|------|
| struct 槽 | **StructUserData** | **拷贝** 栈上 struct 到 userdata payload（与旧 `ToUserData()` 相同） |
| `Il2CppObject**` 槽 | **ClassUserData** | 读 `*slot`，按 `ObjectRegistry` 规则 **PushConstructorInstance** |

- 拷贝/注册后，opaque 与 userdata **相互独立**（struct 为拷贝语义）。
- 未 `EndScope` 前，原 opaque 仍可继续使用（若仍需同步链内零拷贝）。
- Native：`OpaqueValue::ToUserData` / `__zlua_to_user_data`；详见 `LIB_SPEC.md`。

### 4.5 与 Lua→C# Pop 的关系

| 方向 | OpaqueValue 行为 |
|------|------------------|
| **C#→Lua** | 默认 struct `StructHandle` → Push OpaqueValue |
| **Lua→C#** | 若在 **同一同步链** 内将收到的 lightuserdata **原样传回** 对应形参，Pop 时 `Validate` + 按类型 `memcpy` 或绑定 ref 槽（见 `STRUCT_MARSHAL_SPEC.md` §6.1、§6.2.3） |
| **Lua→C#** | 期望 **StructUserData / ClassUserData** 的 API 不得依赖对 opaque 做 `:` / `.` 访问 |

### 4.6 实现模块（C++ / Mono）

| 模块 | 职责 |
|------|------|
| `StructStackScope` | 登记参数地址、`generation`、嵌套 scope（`STRUCT_MARSHAL_SPEC.md` §3） |
| **`OpaqueValue`** | `Validate` / `Resolve` / `ToUserData`；**不** 提供字段/方法 `__index` |
| `NotBlittableStructRegistry` / `ObjectRegistry` | `to_user_data` 的目标路径 |

Mono 与 Il2Cpp **Lua 可见语义一致**；Mono 可用 pinned / 临时 box 代替裸栈地址，但 lightuserdata + 校验 + `to_user_data` 行为须相同。

### 4.7 设计合理性（评估摘要）

| 维度 | 结论 |
|------|------|
| **`uintptr_t` 与 lightuserdata** | 与 Lua C API 一致，32/64 位自然适配；避免把 `uint64_t` 编码塞进指针在 32 位上截断或与 GC 混淆 |
| **统一 struct / object 槽** | 形参在栈上均为「某地址上的值」；引用类型用 `Il2CppObject**` 与 CLR `ref` 槽一致，减少两套临时 marshal |
| **禁止 opaque 上直接访问** | 避免脚本误把 **即将失效的栈副本** 当长生命周期对象；强制 `to_user_data` 显式升级 |
| **沿用 `StructStackScope` 校验** | 不增加新机制；异步/跨 pcall 误用仍被 `generation` 拦截 |
| **与 `StructUserData` / Class userdata 分工** | Opaque = 零拷贝临时；userdata = 可持有、可成员访问；边界清晰 |

---

## 5. 其他类型（索引）

| 类型 | 文档 |
|------|------|
| OpaqueValue / `StructHandle` | 本节 §4；`STRUCT_MARSHAL_SPEC.md` §3 |
| struct userdata | `STRUCT_MARSHAL_SPEC.md` §5 |
| class / 引用类型 | `CLASS_MARSHAL_SPEC.md` |
| delegate / Lua function | `FUNCTION_MARSHAL_SPEC.md` |
| 数组 | `CLASS_MARSHAL_SPEC.md`、`TYPE_SYSTEM_SPEC.md` §7 |
| 指针 / 函数指针 / TypedReference | 本节 §7 |
| `[LuaMarshalAs]` / `LuaMarshalType` | 本节 §6（含 §6.2 合法集合、§6.3 非法回退） |

---

## 6. `[LuaMarshalAs]` 与 `LuaMarshalType`

`LuaMarshalAsAttribute` 定义于 `ZLua.Common`，可标注于 **参数**、**返回值** 或 **方法**（方法级标注作用于该方法全部参数/返回值，除非被更细粒度标注覆盖）。

```csharp
public enum LuaMarshalFlags
{
    None = 0,
    OptionalField = 1,   // struct table 组装时缺键不报错；见 STRUCT_MARSHAL_SPEC.md §6.4
}

public enum LuaMarshalType
{
    Default,
    UserData,
    Bytes,
    OpaqueLightUserData,
}

[AttributeUsage(AttributeTargets.Parameter | AttributeTargets.ReturnValue | AttributeTargets.Method | AttributeTargets.Field)]
public sealed class LuaMarshalAsAttribute : Attribute
{
    public LuaMarshalType LuaMarshalType { get; }
    public LuaMarshalFlags Flags { get; set; }
    public LuaMarshalAsAttribute(LuaMarshalType luaMarshalType = LuaMarshalType.Default);
}
```

### 6.1 `LuaMarshalType` 枚举说明

| 值 | 适用方向 | 说明 |
|----|----------|------|
| **`Default`** | 双向 | 使用 **§1 默认 Marshal 规则**，不做额外转换。 |
| **`UserData`** | 双向 | **强制** 编组为 **full userdata**（带类型元表的 UserData 形态）。会 **改变** 下列类型的默认行为：<br>• **基元类型**（`bool`、`char`、整型、`float`/`double`、`IntPtr`/`UIntPtr` 等）：默认的 `boolean` / integer / number → **userdata**<br>• **`string`**：默认的 Lua **string** → **userdata**<br>• **`enum`**：默认的 integer/number → **enum userdata**（与显式 `_ctor` 构造形态一致）<br>Pop 时须传入与形参类型匹配的 userdata；不接受对应的裸 Lua 值（除非实现层另有兼容，以 Codegen / 反射路径为准）。 |
| **`Bytes`** | 双向 | **强制** 在 C# **`byte[]`** 与 Lua **`string`** 之间转换。会 **改变** 下列类型的默认行为：<br>• C# **`byte[]`**：默认的 ArrayUserData → Lua **string**（字节序列，非 UTF-8 文本语义时按原始 octet 处理）<br>• 标注于 **`byte[]` 形参/返回值** 时，Lua 侧须传 **string**；Pop 时 **不接受** ArrayUserData<br>• 若标注于 **`string`** 形参/返回值，则走 **`byte[]` ↔ string** 的对偶规则（按声明类型解析） |
| **`OpaqueLightUserData`** | **仅 C# → Lua** | **仅** 在 **C# 调用 Lua**（`[LuaInvoke]`、delegate bridge 的 C#→Lua push 等）路径生效。Push 到 Lua 的值为 **OpaqueValue**（**lightuserdata**，载荷为 `StructStackScope` 登记的 **handleId**），见 **§4**。<br>• **无** metatable，脚本 **不能** 直接访问字段/方法<br>• 须在同步调用链内使用，或通过 `zlua.to_user_data` 升级为 StructUserData / ClassUserData<br>• **Lua → C#** 方向 **无效**（标注于 Lua→C# 形参时不生效或报错，以实现为准） |

### 6.2 各类型的合法 `LuaMarshalType` 集合

每个 CLR 形参/返回值/字段类型仅允许绑定 **§6.1** 中与其语义相容的 `LuaMarshalType`（ **`Default` 对所有类型均合法** ）。Codegen / Mono 反射在 **解析标注时** 须先查表；**不在合法集合内** 的标注视为 **无效**，见 **§6.3**。

下表「合法集合」列出的为 **`Default` 之外** 可显式标注的值；未列出的 `LuaMarshalType` 对该类型 **非法**。

| C# 类型（分类） | 合法 `LuaMarshalType`（`Default` 除外） | 说明 |
|-----------------|----------------------------------------|------|
| **基元**（`bool`、`char`、`byte`…`ulong`、`float`、`double`） | `UserData` | 强制 full userdata，替代默认 boolean / integer / number |
| **`IntPtr` / `UIntPtr` / `nint` / `nuint`** | `UserData` | 强制 userdata，替代默认 integer / number |
| **`string`** | `UserData`、`Bytes` | `Bytes`：`string` ↔ Lua **string** 的 **原始 octet** 语义（与 `byte[]`+`Bytes` 对偶）；`UserData`：强制 userdata |
| **`byte[]`** | `Bytes`、`UserData` | `Bytes`：↔ Lua **string**；`UserData`：保持 ArrayUserData（显式标注时与默认相同） |
| **`T[]` / mdarray** | `UserData` | 默认已是 ArrayUserData；显式 `UserData` 等价强调 full userdata |
| **`enum`** | `UserData` | 强制 enum userdata，替代默认 integer / number |
| **`struct`**（普通值类型 struct） | `UserData`、`OpaqueLightUserData` | `UserData`：StructUserData；`OpaqueLightUserData`：**仅 C#→Lua**（§4）。另见 `STRUCT_MARSHAL_SPEC.md` §7 的 `StructUserData`、`ComposeFromStack`、`LuaStackFields` 等 **扩展枚举**（与本表 `UserData` / `OpaqueLightUserData` 正交） |
| **`class` / `interface`** | `UserData` | 默认已是 ClassUserData |
| **`Delegate` 及子类** | `UserData` | 默认已是 DelegateUserData 或 function bridge |
| **`object`** | `UserData` | 按运行时类型 Push 时强制 userdata 形态（实现层须与 §7.4 多态规则一致） |
| **`Nullable<T>`** | 同 **`T`** 的合法集合 | `T` 为上述任一行时继承该行；`T` 非法则 Nullable 亦非法 |
| **非托管指针**（`T*`、`void*` 等，§7.1） | （无） | 仅 `Default`（Pointer lightuserdata 透传） |
| **函数指针**（`delegate*<…>`，§7.2） | （无） | 仅 `Default` |
| **`TypedReference`** | （无） | 类型本身不支持互操作（§7.3）；标注无效 |
| **`decimal`** | （无） | v1 默认不支持（§7.4） |
| **`ref struct`**（`Span<T>` 等） | （无） | 不能作为普通 by-val 形参默认 marshal（§7.4） |

**方向过滤（与上表叠加）：**

| `LuaMarshalType` | 允许标注的方向 |
|------------------|----------------|
| `UserData`、`Bytes` | **双向**（Pop / Push 均可能生效，以形参/返回值方向为准） |
| `OpaqueLightUserData` | **仅 C# → Lua**（返回值、或 C# 调 Lua 时的 push 实参）；标注于 **纯 Lua→C# 形参** 时视为 **非法** |

**`LuaMarshalFlags`（如 `OptionalField`）** 仅作用于 **struct 字段** 的 table 组装，不改变上表对 `LuaMarshalType` 的合法性；见 `STRUCT_MARSHAL_SPEC.md` §6.4、§7.3。

### 6.3 非法标注：回退默认规则与 Editor 日志

当解析到的 `[LuaMarshalAs(LuaMarshalType.X)]`（`X ≠ Default`）**不满足** §6.2（类型不在合法集合、或 `OpaqueLightUserData` 用于不允许的方向）时：

| 行为 | 说明 |
|------|------|
| **编组** | **按 `Default` 处理**——与未标注 `[LuaMarshalAs]` 相同，走 **§1 默认规则**；**不** 因非法标注中断调用 |
| **日志** | **仅 Editor**（Unity Editor / `UNITY_EDITOR`、Mono 反射 Editor 路径）输出 **错误级** 日志，至少包含：**成员签名**（类型/方法/参数或返回值/字段）、**CLR 类型**、**非法 `LuaMarshalType` 值**、**回退为 Default** 的说明 |
| **Player / Il2Cpp 发布包** | **不** 打印上述日志（避免运行时开销与噪音）；仍静默回退 Default |

**示例（概念）：**

```text
[ZLua] Invalid LuaMarshalAs: ZLua.Tests.Fixtures.MarshalDefaultProbe.EchoInt(int value)
  parameter 'value' (System.Int32): LuaMarshalType.Bytes is not allowed; falling back to Default.
```

```text
[ZLua] Invalid LuaMarshalAs: MyGame.Point2D (struct field push)
  LuaMarshalType.OpaqueLightUserData is CSharpToLua-only; falling back to Default for LuaToCSharp.
```

**实现要点：**

- 校验时机：**首次** 绑定方法/字段/属性 marshal 策略时（Codegen 构建期或 Mono 首次反射注册时）各记录 **一次**；避免每次调用重复刷屏。
- Il2Cpp 与 Mono **回退语义一致**；差异仅在 Editor 是否打日志。
- 非法标注 **不** 抛异常到 Lua/C# 调用栈（与「配置错误但运行可继续」一致）；若需 CI 捕获，可另增 **Editor 专用** 校验工具或构建期 Weaver 报错（超出本节运行时范围）。

### 6.4 解析优先级（概念）

Codegen / Mono 反射在 Pop / Push 时按 **由细到粗** 解析：

1. **参数 / 返回值** 上的 `[LuaMarshalAs]`（若 `≠ Default`）
2. **方法** 上的 `[LuaMarshalAs]`（若 `≠ Default`）
3. **类型** 上的 `[LuaMarshalAs]` / XML 双向配置（见 `STRUCT_MARSHAL_SPEC.md` §7）
4. **§1 内置默认**

任意参数、返回值或方法级标注 **`≠ Default`** 时，Il2Cpp Codegen 对该方法生成 **专用** push/pcall/pop 代码，**不** 走泛型默认模板（见 `IL2CPP_DESIGN_SPEC.md` §5.2）。

解析过程中若标注 **非法**（§6.2），该条标注 **视为未设置**，继续向下一优先级查找；若全部为非法或未标注，则使用 **§1 默认**。

### 6.5 与 §1 / §4 的关系

| 场景 | 默认（§1） | `[LuaMarshalAs]` 覆盖 |
|------|------------|------------------------|
| C# 调 Lua，struct 形参 | 视上下文可能为 OpaqueValue 或 userdata | `OpaqueLightUserData` → 强制 OpaqueValue（§4） |
| C# 调 Lua，`int` 实参 | integer | `UserData` → full userdata |
| Lua 调 C#，`string` 形参 | string | `UserData` → userdata；`Bytes` 不适用 |
| Lua 调 C#，`byte[]` 形参 | ArrayUserData | `Bytes` → Lua string |
| 双向 enum | integer/number | `UserData` → enum userdata |

struct 专用的 `StructUserData`、`ComposeFromStack`、`LuaStackFields` 等扩展枚举见 `STRUCT_MARSHAL_SPEC.md` §7；与上述 **`UserData` / `OpaqueLightUserData`** 正交，可组合使用。

---

## 7. 指针、函数指针与不支持的 CLR 类型

本节补充 §1 总览表中 **未单独展开** 或 **明确不支持** 的 CLR 形态。与 **`IntPtr` / `UIntPtr`**（整型数值编组）严格区分。

### 7.1 非托管指针（`T*`、`void*` 等）

**范围：** CLR 中 `Type.IsPointer == true` 且元素为 **非托管** 类型的指针，例如 `int*`、`byte*`、`void*`、`MyStruct*`（`MyStruct` 为 unmanaged struct）。**不包含** `IntPtr` / `UIntPtr` / `nint` / `nuint`（它们走 §1 整型数值规则）。

| 方向 | 默认形态 | 说明 |
|------|----------|------|
| **C# → Lua** | **Pointer**（**lightuserdata**） | Push 指针 **地址值**（`uintptr_t` / 平台指针宽度）；**无** metatable |
| **Lua → C#** | **Pointer**（**lightuserdata**） | Pop 时须为 **同一 Pointer 令牌** 或绑定层认可的 lightuserdata；按声明的指针类型还原 |

**Lua 侧能力（刻意受限）：**

| 允许 | 禁止 |
|------|------|
| 作为实参 **原样传递** 给下一个 C# 调用（同步链内透传） | 解引用、读写指向内存 |
| 与 `nil` 区分（非 null 指针才有 Pointer） | `:` / `.` 成员访问、算术、`#`、`pairs` 等 |
| — | 写入全局 / 表 / upvalue 后在 **异步** 或 **跨 pcall** 使用（地址可能失效） |

**设计理由：** Lua 无法安全表达 C# 非托管指针的生命周期与别名；仅支持 **不透明令牌式透传**，供 native / 底层 API 衔接，不提供脚本级指针运算。

**与 `IntPtr` 对比：**

| 类型 | Lua 默认形态 | 脚本可当作整数运算 |
|------|--------------|-------------------|
| `IntPtr` / `UIntPtr` | integer / number | 可以（按数值） |
| `int*` / `void*` 等 | lightuserdata（Pointer） | **不可以**（仅透传） |

### 7.2 函数指针（function pointer）

**范围：** CLR 中 `Type.IsFunctionPointer == true` 的类型，例如 C# 9+ 的 `delegate*<int, int>`、`delegate*<void>`，以及 native 函数指针 typedef 映射到 CLR 的 function pointer 类型。

| 方向 | 默认形态 | 说明 |
|------|----------|------|
| **C# → Lua** | **Pointer**（**lightuserdata**） | Push 函数入口 **地址**；**无** metatable |
| **Lua → C#** | **Pointer**（**lightuserdata**） | Pop 还原为对应 function pointer 类型 |

**Lua 侧能力：** 与 §7.1 **相同**——**仅透传**，不能从 Lua 侧 **调用** 该地址（无 `:` 调用、不能当 `Delegate` / Lua function 使用）。

**与 `Delegate` 对比：**

| 类型 | Lua 默认形态 | Lua 侧可调用 |
|------|--------------|--------------|
| `Action` / `Func<...>` 等 **Delegate** | DelegateUserData 或 Lua function | **可以**（bridge） |
| `delegate*<...>` **函数指针** | lightuserdata（Pointer） | **不可以**（仅透传） |

### 7.3 `System.TypedReference`

| 方向 | 规则 |
|------|------|
| **C# → Lua** | **不支持** |
| **Lua → C#** | **不支持** |

**原因：** `TypedReference` 仅在受控反射 / 可变参数栈场景有意义，无法映射为稳定的 Lua 值，且无安全脚本用法。Codegen / 反射绑定遇到该形参或返回值时 **报错**（`NotSupportedException` / `luaL_error`），**不** 尝试按 pointer 或 object 降级。

### 7.4 其他 §1 未单独展开的类型

下列类型在 §1 总览中已 **简要列出** 或 **引用他册**；此处集中说明默认 marshal 与文档缺口，避免与 §7.1–§7.3 重复。

| C# 类型 | 默认 marshal（摘要） | 文档 / 实现状态 |
|---------|----------------------|-----------------|
| **`object`** | Push 按 **运行时实际类型**；Pop 接受 boolean / number / string / userdata | 多态 boxing；`ReadBoxedObjectValue` 路径；无单独分册 |
| **`Nullable<T>`** | 有值时同 `T`；`null` ↔ `nil` | `T` 为非 Nullable 值类型时 Pop 接受 `nil` |
| **`interface`** | 与 **class** 相同（ClassUserData） | 见 `CLASS_MARSHAL_SPEC.md` |
| **`decimal`** | **默认不支持** | Pop/Push 未纳入 v1 默认路径；需 `[LuaMarshalAs]` 或专用规则后再定 |
| **`ref struct`**（`Span<T>`、`ReadOnlySpan<T>` 等） | **不能** 作为普通 by-val 形参默认 marshal | 仅 `ref` / `zlua.new_ref` / OpaqueValue 等受控路径；见 `STRUCT_MARSHAL_SPEC.md`、`LIB_SPEC.md` §6 |
| **`params T[]`** | 视具体签名为 **数组** 或 **展开**；delegate bridge **可不支持** | `FUNCTION_MARSHAL_SPEC.md` §9 |
| **开放泛型形参**（如 `void M<T>(T x)` 且 `T` 未实例化） | 由 **调用时类型实参** 决定 | 见 `TYPE_SYSTEM_SPEC.md` 泛型解析 |
| **`dynamic`** | 编译期按 **`object`** 处理 | 无独立 Lua 形态 |
| **`string` 构建器 / 自定义 class`** | 同 **class** | `CLASS_MARSHAL_SPEC.md` |

**注册 / 暴露阶段即应拒绝的类型（非 marshal 规则，而是签名非法）：**

- 含 **`TypedReference`** 的参数或返回值（§7.3）
- **`[LuaInvoke]` / delegate bridge** 上的 **`ref` / `out` / `in`**（`FUNCTION_MARSHAL_SPEC.md` §9；普通 C# 方法的 ref 见 §3）
- 无法解析的 **byref 修饰符** 与 **ref struct by-val** 形参组合

### 7.5 Mono / Il2Cpp 一致性

| 项 | 要求 |
|----|------|
| Pointer / function pointer Push | lightuserdata，地址宽度 = 平台指针 |
| Pointer Pop | 仅接受 Pointer 形态；**不** 与 integer / full userdata 隐式互转 |
| TypedReference | 双向均报错，语义一致 |
| 错误消息 | 一致或等价（如 `unsupported arg type TypedReference`） |
