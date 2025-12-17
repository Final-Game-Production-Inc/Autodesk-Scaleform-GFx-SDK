/**************************************************************************

Filename    :   AS2_PlayerStringInvoke.cpp
Content     :   Deprecated string returning variants of 
Created     :   
Authors     :   Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "GFx/GFx_Player.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_Sprite.h"

namespace Scaleform { namespace GFx {

const char* Movie::GetVariable(const char* ppathToVar) const
{
    MovieImpl* _this = (MovieImpl*)this;
    if (!_this->pLevel0Movie) return 0;

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    Environment* penv = _this->pLevel0Movie->GetASEnvironment();
    ASString     path(penv->CreateString(ppathToVar));

    Value retVal;
    if (penv->GetVariable(path, &retVal))
    {
        _this->pRetValHolder->ResetPos();
        _this->pRetValHolder->ResizeStringArray(0);

        // Store string into a return value holder, so that it is not released till next call.
        // This is thread safe from the multiple MovieImpl point of view.
        unsigned pos = _this->pRetValHolder->StringArrayPos++;
        if (_this->pRetValHolder->StringArray.GetSize() < _this->pRetValHolder->StringArrayPos)
            _this->pRetValHolder->ResizeStringArray(_this->pRetValHolder->StringArrayPos);
        _this->pRetValHolder->StringArray[pos] = retVal.ToString(penv);

        return _this->pRetValHolder->StringArray[pos].ToCStr();
    }
    return NULL;
}

const wchar_t* Movie::GetVariableStringW(const char* ppathToVar) const
{
    MovieImpl* _this = (MovieImpl*)this;
    if (!_this->pLevel0Movie) return 0;

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    Environment* penv = _this->pLevel0Movie->GetASEnvironment();
    ASString     path(penv->CreateString(ppathToVar));

    Value retVal;
    if (penv->GetVariable(path, &retVal))
    {
        _this->pRetValHolder->ResetPos();
        _this->pRetValHolder->ResizeStringArray(0);

        ASString str = retVal.ToString(penv);
        unsigned len = str.GetLength();
        wchar_t* pdestBuf = (wchar_t*)_this->pRetValHolder->PreAllocateBuffer((len+1)*sizeof(wchar_t));

        UTF8Util::DecodeString(pdestBuf,str.ToCStr());
        return pdestBuf;
    }
    return NULL;
}

const char* Movie::Invoke(const char* pmethodName, const Value* pargs, unsigned numArgs)
{
    MovieImpl* _this = (MovieImpl*)this;
    if (!_this->pLevel0Movie) return 0;

#ifdef SF_AMP_SERVER
    AMP::ScopeFunctionTimer invokeTimer(_this->AdvanceStats, AMP::NativeCodeSwdHandle, AMP::Func_GFxMovieView_Invoke);
#endif

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    Value resultVal;
    Environment* penv = _this->pLevel0Movie->GetASEnvironment();
    SF_ASSERT(penv);

    // push arguments directly into environment
    for (int i = (int)numArgs - 1; i >= 0; --i)
    {
        SF_ASSERT(pargs); // should be checked only if numArgs > 0
        const Value& gfxVal = pargs[i];
        Value asval;
        _this->Value2ASValue(gfxVal, &asval);
        penv->Push(asval);
    }
    bool retVal;

    // try to resolve alias
    MovieImpl::InvokeAliasInfo* palias;
    if (_this->pInvokeAliases && (palias = _this->ResolveInvokeAlias(pmethodName)) != NULL)
        retVal = _this->InvokeAlias(pmethodName, *palias, &resultVal, numArgs);
    else
        retVal = _this->pLevel0Movie->Invoke(pmethodName, &resultVal, numArgs);
    penv->Drop(numArgs); // release arguments
    if (retVal)
    {
        _this->pRetValHolder->ResetPos();

        // Store string into a return value holder, so that it is not released till next call.
        // This is thread safe from the multiple MovieImpl point of view.
        unsigned pos = _this->pRetValHolder->StringArrayPos++;
        if (_this->pRetValHolder->StringArray.GetSize() < _this->pRetValHolder->StringArrayPos)
            _this->pRetValHolder->ResizeStringArray(_this->pRetValHolder->StringArrayPos);
        _this->pRetValHolder->StringArray[pos] = 
            resultVal.ToString(_this->pLevel0Movie->GetASEnvironment());

        return _this->pRetValHolder->StringArray[pos].ToCStr();
    }
    return NULL;    
}
const char*     Movie::Invoke(const char* pmethodName, const char* pargFmt, ...)
{
    MovieImpl* _this = (MovieImpl*)this;
    if (!_this->pLevel0Movie) return 0;

#ifdef SF_AMP_SERVER
    AMP::ScopeFunctionTimer invokeTimer(_this->AdvanceStats, AMP::NativeCodeSwdHandle, AMP::Func_GFxMovieView_Invoke);
#endif

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    Value resultVal;
    va_list args;
    va_start(args, pargFmt);

    bool retVal;
    // try to resolve alias
    MovieImpl::InvokeAliasInfo* palias;
    if (_this->pInvokeAliases && (palias = _this->ResolveInvokeAlias(pmethodName)) != NULL)
        retVal = _this->InvokeAliasArgs(pmethodName, *palias, &resultVal, pargFmt, args);
    else
        retVal = _this->pLevel0Movie->InvokeArgs(pmethodName, &resultVal, pargFmt, args);
    va_end(args);
    if (retVal)
    {
        _this->pRetValHolder->ResetPos();

        // Store string into a return value holder, so that it is not released till next call.
        // This is thread safe from the multiple MovieImpl point of view.
        unsigned pos = _this->pRetValHolder->StringArrayPos++;
        if (_this->pRetValHolder->StringArray.GetSize() < _this->pRetValHolder->StringArrayPos)
            _this->pRetValHolder->ResizeStringArray(_this->pRetValHolder->StringArrayPos);
        _this->pRetValHolder->StringArray[pos] = 
            resultVal.ToString(_this->pLevel0Movie->GetASEnvironment());

        return _this->pRetValHolder->StringArray[pos].ToCStr();
    }
    return NULL;    
}
const char*     Movie::InvokeArgs(const char* pmethodName, const char* pargFmt, va_list args)
{
    MovieImpl* _this = (MovieImpl*)this;
    if (_this->pLevel0Movie)
    {
#ifdef SF_AMP_SERVER
        AMP::ScopeFunctionTimer invokeTimer(_this->AdvanceStats, AMP::NativeCodeSwdHandle, AMP::Func_GFxMovieView_InvokeArgs);
#endif

        // Need to restore high precision mode of FPU for X86 CPUs.
        // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
        // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
        // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
        DoublePrecisionGuard dpg;

        Value resultVal;
        bool retVal;
        // try to resolve alias
        MovieImpl::InvokeAliasInfo* palias;
        if (_this->pInvokeAliases && (palias = _this->ResolveInvokeAlias(pmethodName)) != NULL)
            retVal = _this->InvokeAliasArgs(pmethodName, *palias, &resultVal, pargFmt, args);
        else
            retVal = _this->pLevel0Movie->InvokeArgs(pmethodName, &resultVal, pargFmt, args);
        if (retVal)
        {
            _this->pRetValHolder->ResetPos();

            // Store string into a return value holder, so that it is not released till next call.
            // This is thread safe from the multiple MovieImpl point of view.
            unsigned pos = _this->pRetValHolder->StringArrayPos++;
            if (_this->pRetValHolder->StringArray.GetSize() < _this->pRetValHolder->StringArrayPos)
                _this->pRetValHolder->ResizeStringArray(_this->pRetValHolder->StringArrayPos);
            _this->pRetValHolder->StringArray[pos] = 
                resultVal.ToString(_this->pLevel0Movie->GetASEnvironment());

            return _this->pRetValHolder->StringArray[pos].ToCStr();
        }
        return NULL;
    }
    return NULL;    
}

}}} // namespace Scaleform { namespace GFx {

