//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_System_Capabilities.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_System_Capabilities_H
#define INC_AS3_Obj_System_Capabilities_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_system
{
    extern const TypeInfo CapabilitiesTI;
    extern const ClassInfo CapabilitiesCI;
} // namespace fl_system
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
    extern const TypeInfo StringTI;
    extern const ClassInfo StringCI;
    extern const TypeInfo NumberTI;
    extern const ClassInfo NumberCI;
} // namespace fl

namespace ClassTraits { namespace fl_system
{
    class Capabilities;
}}

namespace InstanceTraits { namespace fl_system
{
    class Capabilities;
}}

namespace Classes { namespace fl_system
{
    class Capabilities;
}}

//##protect##"forward_declaration"
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_system
{
    class Capabilities : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Capabilities"; }
#endif
    public:
        typedef Classes::fl_system::Capabilities ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Capabilities(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { ThunkInfoNum = 27 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[27];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_system
{
    class Capabilities : public Class
    {
        friend class ClassTraits::fl_system::Capabilities;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_system::CapabilitiesTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Capabilities"; }
#endif
    public:
        typedef Capabilities SelfType;
        typedef Capabilities ClassType;
        
    private:
        Capabilities(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"

        void getOSName(ASString& result);

//##protect##"class_$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_avHardwareDisableGet, 
            mid_hasAccessibilityGet, 
            mid_hasAudioGet, 
            mid_hasAudioEncoderGet, 
            mid_hasEmbeddedVideoGet, 
            mid_hasIMEGet, 
            mid_hasMP3Get, 
            mid_hasPrintingGet, 
            mid_hasScreenBroadcastGet, 
            mid_hasScreenPlaybackGet, 
            mid_hasStreamingAudioGet, 
            mid_hasStreamingVideoGet, 
            mid_hasTLSGet, 
            mid_hasVideoEncoderGet, 
            mid_isDebuggerGet, 
            mid_languageGet, 
            mid_localFileReadDisableGet, 
            mid_manufacturerGet, 
            mid_osGet, 
            mid_pixelAspectRatioGet, 
            mid_playerTypeGet, 
            mid_screenColorGet, 
            mid_screenDPIGet, 
            mid_screenResolutionXGet, 
            mid_screenResolutionYGet, 
            mid_serverStringGet, 
            mid_versionGet, 
        };
        void avHardwareDisableGet(bool& result);
        void hasAccessibilityGet(bool& result);
        void hasAudioGet(bool& result);
        void hasAudioEncoderGet(bool& result);
        void hasEmbeddedVideoGet(bool& result);
        void hasIMEGet(bool& result);
        void hasMP3Get(bool& result);
        void hasPrintingGet(bool& result);
        void hasScreenBroadcastGet(bool& result);
        void hasScreenPlaybackGet(bool& result);
        void hasStreamingAudioGet(bool& result);
        void hasStreamingVideoGet(bool& result);
        void hasTLSGet(bool& result);
        void hasVideoEncoderGet(bool& result);
        void isDebuggerGet(bool& result);
        void languageGet(ASString& result);
        void localFileReadDisableGet(bool& result);
        void manufacturerGet(ASString& result);
        void osGet(ASString& result);
        void pixelAspectRatioGet(Value::Number& result);
        void playerTypeGet(ASString& result);
        void screenColorGet(ASString& result);
        void screenDPIGet(Value::Number& result);
        void screenResolutionXGet(Value::Number& result);
        void screenResolutionYGet(Value::Number& result);
        void serverStringGet(ASString& result);
        void versionGet(ASString& result);

        // C++ friendly wrappers for AS3 methods.
        bool avHardwareDisableGet()
        {
            bool result;
            avHardwareDisableGet(result);
            return result;
        }
        bool hasAccessibilityGet()
        {
            bool result;
            hasAccessibilityGet(result);
            return result;
        }
        bool hasAudioGet()
        {
            bool result;
            hasAudioGet(result);
            return result;
        }
        bool hasAudioEncoderGet()
        {
            bool result;
            hasAudioEncoderGet(result);
            return result;
        }
        bool hasEmbeddedVideoGet()
        {
            bool result;
            hasEmbeddedVideoGet(result);
            return result;
        }
        bool hasIMEGet()
        {
            bool result;
            hasIMEGet(result);
            return result;
        }
        bool hasMP3Get()
        {
            bool result;
            hasMP3Get(result);
            return result;
        }
        bool hasPrintingGet()
        {
            bool result;
            hasPrintingGet(result);
            return result;
        }
        bool hasScreenBroadcastGet()
        {
            bool result;
            hasScreenBroadcastGet(result);
            return result;
        }
        bool hasScreenPlaybackGet()
        {
            bool result;
            hasScreenPlaybackGet(result);
            return result;
        }
        bool hasStreamingAudioGet()
        {
            bool result;
            hasStreamingAudioGet(result);
            return result;
        }
        bool hasStreamingVideoGet()
        {
            bool result;
            hasStreamingVideoGet(result);
            return result;
        }
        bool hasTLSGet()
        {
            bool result;
            hasTLSGet(result);
            return result;
        }
        bool hasVideoEncoderGet()
        {
            bool result;
            hasVideoEncoderGet(result);
            return result;
        }
        bool isDebuggerGet()
        {
            bool result;
            isDebuggerGet(result);
            return result;
        }
        ASString languageGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            languageGet(result);
            return result;
        }
        bool localFileReadDisableGet()
        {
            bool result;
            localFileReadDisableGet(result);
            return result;
        }
        ASString manufacturerGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            manufacturerGet(result);
            return result;
        }
        ASString osGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            osGet(result);
            return result;
        }
        Value::Number pixelAspectRatioGet()
        {
            Value::Number result;
            pixelAspectRatioGet(result);
            return result;
        }
        ASString playerTypeGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            playerTypeGet(result);
            return result;
        }
        ASString screenColorGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            screenColorGet(result);
            return result;
        }
        Value::Number screenDPIGet()
        {
            Value::Number result;
            screenDPIGet(result);
            return result;
        }
        Value::Number screenResolutionXGet()
        {
            Value::Number result;
            screenResolutionXGet(result);
            return result;
        }
        Value::Number screenResolutionYGet()
        {
            Value::Number result;
            screenResolutionYGet(result);
            return result;
        }
        ASString serverStringGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            serverStringGet(result);
            return result;
        }
        ASString versionGet()
        {
            ASString result(GetStringManager().CreateEmptyString());
            versionGet(result);
            return result;
        }

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

