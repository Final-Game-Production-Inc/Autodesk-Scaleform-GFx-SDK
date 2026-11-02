/**************************************************************************

Filename    :   Render_DebugEnv.h
Content     :   Helpers for reading debug-only environment variables.
Created     :   August 17, 2009
Authors     :   FINALZ-27980

Notes       :   MSVC flags plain getenv() with C4996 ("This function or variable may
                be unsafe") and asks for _dupenv_s. _dupenv_s allocates the value with
                malloc(), so every call site would otherwise have to free() it - easy to
                leak on an early return, which is exactly what these debug probes do.
                Both helpers below hide that detail. Semantics match getenv(): a variable
                counts as "set" even when its value is an empty string.

**************************************************************************/

#ifndef INC_SF_Render_DebugEnv_H
#define INC_SF_Render_DebugEnv_H

#include "Kernel/SF_Types.h"

#include <stdlib.h>
#include <string.h>

namespace Scaleform { namespace Render {

// Returns true when the variable is defined (empty value still counts as defined).
inline bool IsDebugEnvSet(const char* name)
{
#if defined(SF_OS_WIN32) && defined(_MSC_VER)
    char*  pValue = 0;
    size_t valueSize = 0;
    if (_dupenv_s(&pValue, &valueSize, name) != 0 || !pValue)
        return false;
    free(pValue);
    return true;
#else
    return getenv(name) != 0;
#endif
}

// Copies the variable's value into pOut, always NUL-terminated (truncated if needed).
// Returns false when the variable is not defined; pOut[0] is set to 0 in that case.
inline bool GetDebugEnvValue(const char* name, char* pOut, UPInt outSize)
{
    if (!pOut || outSize == 0)
        return false;
    pOut[0] = 0;

#if defined(SF_OS_WIN32) && defined(_MSC_VER)
    char*  pValue = 0;
    size_t valueSize = 0;
    if (_dupenv_s(&pValue, &valueSize, name) != 0 || !pValue)
        return false;

    UPInt length = (valueSize > 0) ? (UPInt)(valueSize - 1) : 0;
    if (length >= outSize)
        length = outSize - 1;
    if (length)
        memcpy(pOut, pValue, length);
    pOut[length] = 0;
    free(pValue);
    return true;
#else
    const char* pValue = getenv(name);
    if (!pValue)
        return false;

    UPInt length = (UPInt)strlen(pValue);
    if (length >= outSize)
        length = outSize - 1;
    memcpy(pOut, pValue, length);
    pOut[length] = 0;
    return true;
#endif
}

}} // namespace Scaleform::Render

#endif // INC_SF_Render_DebugEnv_H
