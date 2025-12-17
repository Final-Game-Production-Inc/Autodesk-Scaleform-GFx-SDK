//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Net_URLRequestDefaults.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Net_URLRequestDefaults_H
#define INC_AS3_Obj_Net_URLRequestDefaults_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_net
{
    extern const TypeInfo URLRequestDefaultsTI;
    extern const ClassInfo URLRequestDefaultsCI;
} // namespace fl_net
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
} // namespace fl

namespace ClassTraits { namespace fl_net
{
    class URLRequestDefaults;
}}

namespace InstanceTraits { namespace fl_net
{
    class URLRequestDefaults;
}}

namespace Classes { namespace fl_net
{
    class URLRequestDefaults;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_net
{
    class URLRequestDefaults : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::URLRequestDefaults"; }
#endif
    public:
        typedef Classes::fl_net::URLRequestDefaults ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        URLRequestDefaults(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_net
{
    class URLRequestDefaults : public Class
    {
        friend class ClassTraits::fl_net::URLRequestDefaults;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_net::URLRequestDefaultsTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::URLRequestDefaults"; }
#endif
    public:
        typedef URLRequestDefaults SelfType;
        typedef URLRequestDefaults ClassType;
        
    private:
        URLRequestDefaults(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

