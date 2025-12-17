/**************************************************************************

Filename    :   SF_Locale.cpp
Content     :   Language-specific formatting information (locale)
Created     :   January 26, 2009
Authors     :   Sergey Sikorskiy, Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "SF_Locale.h"
#if !defined(SF_OS_PSVITA)
    #include <locale.h>
#endif
#ifdef SF_OS_WIN32
    #include <malloc.h>
#endif

namespace Scaleform {

////////////////////////////////////////////////////////////////////////////////
#if defined(SF_OS_WIN32) && !defined(SF_OS_WINMETRO)

inline
UInt32 GetLocaleInfoWChar(LCID lcid, LCTYPE lctype)
{
	UInt32 wbuf[2];

	wbuf[0] = 0;
	wbuf[1] = 0;

	if(!::GetLocaleInfoW(lcid, lctype, (WCHAR *)wbuf, 2))
    {
        SF_ASSERT(false);
    }

    return wbuf[0];
}

static
String GetLocaleInfoStr(LCID lcid, LCTYPE lctype)
{
    // Get size of required buffer.
    int    res_size = ::GetLocaleInfoW(lcid, lctype, (WCHAR *)NULL, 0);
    // Memory allocated with _alloca will be automatically freed when function exits.
    WCHAR* wbuf = (WCHAR*)_alloca(res_size * sizeof(WCHAR));

    String result;

    res_size = ::GetLocaleInfoW(lcid, lctype, wbuf, res_size);
	
    SF_ASSERT(res_size != 0);

    if(res_size)
    {
        result.AppendString(wbuf, res_size);
    }

    return result;
}

#endif

////////////////////////////////////////////////////////////////////////////////
Locale::Locale()
    : GroupSeparator(0), DecimalSeparator(0), PositiveSign(0), NegativeSign(0)
    , ExponentialSign(0)
{
}


////////////////////////////////////////////////////////////////////////////////
SystemLocale::SystemLocale()
{
#if defined(SF_OS_WIN32) && !defined(SF_OS_WINMETRO)

    // const LCID lcid = GetLanguageLCID(lang_);
    const LCID lcid = LOCALE_USER_DEFAULT;

    SetEnglishName(GetLocaleInfoStr(lcid, LOCALE_SENGLANGUAGE));
    SetNativeName(GetLocaleInfoStr(lcid, LOCALE_SNATIVELANGNAME));

    SetGrouping(GetLocaleInfoStr(lcid, LOCALE_SGROUPING));

    SetGroupSeparator(GetLocaleInfoWChar(lcid, LOCALE_STHOUSAND));
    SetDecimalSeparator(GetLocaleInfoWChar(lcid, LOCALE_SDECIMAL));
    SetPositiveSign(GetLocaleInfoWChar(lcid, LOCALE_SPOSITIVESIGN));
    SetNegativeSign(GetLocaleInfoWChar(lcid, LOCALE_SNEGATIVESIGN));

    const char* exp_sign = "e";
    SetExponentialSign(UTF8Util::DecodeNextChar(&exp_sign));

#elif defined(SF_OS_ANDROID) || defined(SF_OS_PSVITA)

    SetGroupSeparator(',');
    SetDecimalSeparator('.');
    SetPositiveSign('+');
    SetNegativeSign('-');

#else // POSIX

    if (setlocale(LC_ALL, NULL)) 
    {
        SetEnglishName(setlocale(LC_ALL, NULL));
        SetNativeName(GetEnglishName());

        const struct lconv* lc = localeconv();

        if (lc)
        {
            //lc->grouping - controls thousands grouping
            // SetGrouping();

            if (lc->thousands_sep)
            {
    	        SetGroupSeparator(UTF8Util::GetCharAt(0, lc->thousands_sep, 1));
            }

            if (lc->decimal_point)
            {
	            SetDecimalSeparator(UTF8Util::GetCharAt(0, lc->decimal_point, 1));
            }

            if (lc->positive_sign)
            {
                SetPositiveSign(UTF8Util::GetCharAt(0, lc->positive_sign, 1));
            }

            if (lc->negative_sign)
            {
                SetNegativeSign(UTF8Util::GetCharAt(0, lc->negative_sign, 1));
            }
        }
    }

#endif
}

} // Scaleform

