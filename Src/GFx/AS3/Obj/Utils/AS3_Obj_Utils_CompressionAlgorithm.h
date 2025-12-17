//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Utils_CompressionAlgorithm.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Utils_CompressionAlgorithm_H
#define INC_AS3_Obj_Utils_CompressionAlgorithm_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_utils
{
    extern const TypeInfo CompressionAlgorithmTI;
    extern const ClassInfo CompressionAlgorithmCI;
} // namespace fl_utils

namespace ClassTraits { namespace fl_utils
{
    class CompressionAlgorithm;
}}

namespace InstanceTraits { namespace fl_utils
{
    class CompressionAlgorithm;
}}

namespace Classes { namespace fl_utils
{
    class CompressionAlgorithm;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_utils
{
    class CompressionAlgorithm : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::CompressionAlgorithm"; }
#endif
    public:
        typedef Classes::fl_utils::CompressionAlgorithm ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        CompressionAlgorithm(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 2 };
        static const MemberInfo mi[MemberInfoNum];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_utils
{
    class CompressionAlgorithm : public Class
    {
        friend class ClassTraits::fl_utils::CompressionAlgorithm;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_utils::CompressionAlgorithmTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::CompressionAlgorithm"; }
#endif
    public:
        typedef CompressionAlgorithm SelfType;
        typedef CompressionAlgorithm ClassType;
        
    private:
        CompressionAlgorithm(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API members.
        const char* DEFLATE;
        const char* ZLIB;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

