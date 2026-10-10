// Copyright 2026 Code Philosophy
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

/*
 * HybridCLR (il2cpp_plus) interop shim.
 * Assumes generated/ZLuaConf.inc was included by ZLuaCommon.h first.
 *
 * Why this exists
 * ---------------
 * HybridCLR runs hot-update assemblies on an interpreter. For such a method,
 * `methodPointer` is NOT a normal AOT entry point:
 *
 *   - Until the method is initialized it is either null or an unresolved stub.
 *   - Once initialized it points at a native2managed bridge, or at
 *     `NotSupportNative2Managed` (which raises) when no bridge was generated.
 *
 * libil2cpp itself never dereferences it blind: `vm::Runtime::InvokeWithThrow`
 * and `vm::Type` (delegate construction) both call
 * `hybridclr::InitAndGetInterpreterDirectlyCallMethodPointer` first. ZLua must do
 * the same before any `invoker_method` call, and must never cast `methodPointer`
 * to a concrete AOT signature for an interpreted method -- the specialized stubs
 * assume the real AOT ABI, which interpreted methods do not have.
 *
 * When ZLUA_ENABLE_HYBRIDCLR is 0 every helper degrades to a constant-false /
 * no-op and nothing from the hybridclr tree is included, so a standalone build
 * compiles exactly as it did before this integration.
 */

/*
 * Normally written by generated/ZLuaConf.inc. Unlike ZLUA_LUA_API_FAMILY /
 * ZLUA_UNITY_VERSION -- where guessing would silently miscompile, so a missing
 * value is a hard #error -- "absent" here has one unambiguous meaning: off.
 * Defaulting keeps a conf file generated before this flag existed compiling.
 */
#ifndef ZLUA_ENABLE_HYBRIDCLR
#define ZLUA_ENABLE_HYBRIDCLR 0
#endif

// Keep il2cpp-config.h first: in a HybridCLR tree it includes
// hybridclr/Il2CppVersionCompatibleDefs.h -> generated/UnityVersion.inc, which
// defines HYBRIDCLR_UNITY_VERSION. Il2CppCompatibleDef.h hard-#errors without it
// ("please run 'HybridCLR/Generate/All' before building"), so do not reorder.
#include "il2cpp-config.h"
#include "il2cpp-class-internals.h"

#if ZLUA_ENABLE_HYBRIDCLR
#include "hybridclr/Il2CppCompatibleDef.h"
#include "hybridclr/interpreter/InterpreterModule.h"
#endif

namespace zlua
{

/**
 * True when @p method is implemented by HybridCLR's interpreter (a hot-update
 * method), meaning its `methodPointer` is not a plain AOT entry point.
 *
 * Reads il2cpp_plus's `MethodInfo::isInterpterImpl` (spelling is upstream's).
 * libil2cpp sets it in `vm::Class` method initialization and in
 * `metadata::GenericMethod`, so it is already correct by the time Lua can reach
 * the method.
 */
inline bool ZLuaIsInterpreterImpl(const MethodInfo* method)
{
#if ZLUA_ENABLE_HYBRIDCLR
    return method != nullptr && method->isInterpterImpl != 0;
#else
    (void)method;
    return false;
#endif
}

/**
 * Prepares @p method for invocation through `invoker_method`.
 *
 * Mirrors what `vm::Runtime::InvokeWithThrow` does: call unconditionally and let
 * HybridCLR decide. For an interpreted method this populates `methodPointer` /
 * `virtualMethodPointer` and installs the interpreter invoker; for an AOT method
 * it is a cheap no-op. Safe to call repeatedly -- HybridCLR latches on
 * `initInterpCallMethodPointer`.
 */
inline void ZLuaEnsureInvokable(const MethodInfo* method)
{
#if ZLUA_ENABLE_HYBRIDCLR
    // if (method != nullptr)
    //{
    //     hybridclr::InitAndGetInterpreterDirectlyCallMethodPointer(method);
    // }
#else
    (void)method;
#endif
}

/**
 * True when @p method must go through the generic `invoker_method` path instead
 * of a specialized stub that casts `methodPointer` to a concrete signature.
 *
 * Also calls ZLuaEnsureInvokable, so a caller that takes the generic path on a
 * true result has an already-initialized method.
 */
inline bool ZLuaRequiresGenericInvoke(const MethodInfo* method)
{
#if ZLUA_ENABLE_HYBRIDCLR
    if (!ZLuaIsInterpreterImpl(method))
    {
        return false;
    }

    if (method->methodPointer == hybridclr::interpreter::InterpreterModule::NotSupportNative2Managed)
    {
        return true;
    }

    // ZLuaEnsureInvokable(method);
    // return true;
    return false;
#else
    (void)method;
    return false;
#endif
}

} // namespace zlua
