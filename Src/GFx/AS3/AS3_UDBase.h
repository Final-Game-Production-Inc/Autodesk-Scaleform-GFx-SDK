/**************************************************************************

Filename    :   AS3_UDBase.h
Content     :   
Created     :   Feb, 2011
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_AS3_UDBase_H
#define INC_AS3_UDBase_H

#include "AS3_Instance.h"

namespace Scaleform { namespace GFx { namespace AS3 
{

namespace Classes
{
    class UDBase : public Class
    {
    public:
        typedef UDBase Self;

    public:
        UDBase(ClassTraits::Traits& t);
        virtual ~UDBase();

    public:
        virtual CheckResult PreInit(const Value& _this) const;
        //virtual void PostInit(const Value& _this, unsigned argc, const Value* argv) const;

        virtual void InitPrototype(AS3::Object& obj) const;
    };
} // namespace Classes

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_UDBase_H

