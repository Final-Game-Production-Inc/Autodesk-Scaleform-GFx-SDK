//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Ui_Keyboard.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#ifndef INC_AS3_Obj_Ui_Keyboard_H
#define INC_AS3_Obj_Ui_Keyboard_H

#include "../AS3_Obj_Object.h"
//##protect##"includes"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{
// Forward declaration.
namespace fl_ui
{
    extern const TypeInfo KeyboardTI;
    extern const ClassInfo KeyboardCI;
} // namespace fl_ui
namespace fl
{
    extern const TypeInfo BooleanTI;
    extern const ClassInfo BooleanCI;
} // namespace fl

namespace ClassTraits { namespace fl_ui
{
    class Keyboard;
}}

namespace InstanceTraits { namespace fl_ui
{
    class Keyboard;
}}

namespace Classes { namespace fl_ui
{
    class Keyboard;
}}

//##protect##"forward_declaration"
#ifdef ALTERNATE
#undef ALTERNATE
#endif

#ifdef DELETE
#undef DELETE
#endif
//##protect##"forward_declaration"
    
namespace ClassTraits { namespace fl_ui
{
    class Keyboard : public fl::Object
    {
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "ClassTraits::Keyboard"; }
#endif
    public:
        typedef Classes::fl_ui::Keyboard ClassType;
        typedef InstanceTraits::fl::Object InstanceTraitsType;
        typedef InstanceTraitsType::InstanceType InstanceType;

    public:
        Keyboard(VM& vm, const ClassInfo& ci);
        static Pickable<Traits> MakeClassTraits(VM& vm);
        enum { MemberInfoNum = 244 };
        static const MemberInfo mi[MemberInfoNum];
        enum { ThunkInfoNum = 3 };
        static const ThunkInfo ti[ThunkInfoNum];
        // static const UInt16 tito[ThunkInfoNum];
        static const TypeInfo* tit[3];
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

    };
}}

namespace Classes { namespace fl_ui
{
    class Keyboard : public Class
    {
        friend class ClassTraits::fl_ui::Keyboard;
        static const TypeInfo& GetTypeInfo() { return AS3::fl_ui::KeyboardTI; }
        
#ifdef GFX_AS3_VERBOSE
    private:
        virtual const char* GetAS3ObjectType() const { return "Classes::Keyboard"; }
#endif
    public:
        typedef Keyboard SelfType;
        typedef Keyboard ClassType;
        
    private:
        Keyboard(ClassTraits::Traits& t);
       
    private:
        SelfType& GetSelf()
        {
            return *this;
        }

//##protect##"class_$methods"
//##protect##"class_$methods"

    public:
        // AS3 API methods.
        enum MethodID {
            mid_capsLockGet, 
            mid_numLockGet, 
            mid_isAccessible, 
        };
        void capsLockGet(bool& result);
        void numLockGet(bool& result);
        void isAccessible(bool& result);

        // C++ friendly wrappers for AS3 methods.
        bool capsLockGet()
        {
            bool result;
            capsLockGet(result);
            return result;
        }
        bool numLockGet()
        {
            bool result;
            numLockGet(result);
            return result;
        }
        bool isAccessible()
        {
            bool result;
            isAccessible(result);
            return result;
        }

    public:
        // AS3 API members.
        const UInt32 A;
        const UInt32 ALTERNATE;
        const UInt32 B;
        const UInt32 BACKQUOTE;
        const UInt32 BACKSLASH;
        const UInt32 BACKSPACE;
        const UInt32 C;
        const UInt32 CAPS_LOCK;
        SPtr<Instances::fl::Array> CharCodeStrings;
        const UInt32 COMMA;
        const UInt32 COMMAND;
        const UInt32 CONTROL;
        const UInt32 D;
        const UInt32 DELETE;
        const UInt32 DOWN;
        const UInt32 E;
        const UInt32 END;
        const UInt32 ENTER;
        const UInt32 EQUAL;
        const UInt32 ESCAPE;
        const UInt32 F;
        const UInt32 F1;
        const UInt32 F10;
        const UInt32 F11;
        const UInt32 F12;
        const UInt32 F13;
        const UInt32 F14;
        const UInt32 F15;
        const UInt32 F2;
        const UInt32 F3;
        const UInt32 F4;
        const UInt32 F5;
        const UInt32 F6;
        const UInt32 F7;
        const UInt32 F8;
        const UInt32 F9;
        const UInt32 G;
        const UInt32 H;
        const UInt32 HOME;
        const UInt32 I;
        const UInt32 INSERT;
        const UInt32 J;
        const UInt32 K;
        const char* KEYNAME_BEGIN;
        const char* KEYNAME_BREAK;
        const char* KEYNAME_CLEARDISPLAY;
        const char* KEYNAME_CLEARLINE;
        const char* KEYNAME_DELETE;
        const char* KEYNAME_DELETECHAR;
        const char* KEYNAME_DELETELINE;
        const char* KEYNAME_DOWNARROW;
        const char* KEYNAME_END;
        const char* KEYNAME_EXECUTE;
        const char* KEYNAME_F1;
        const char* KEYNAME_F10;
        const char* KEYNAME_F11;
        const char* KEYNAME_F12;
        const char* KEYNAME_F13;
        const char* KEYNAME_F14;
        const char* KEYNAME_F15;
        const char* KEYNAME_F16;
        const char* KEYNAME_F17;
        const char* KEYNAME_F18;
        const char* KEYNAME_F19;
        const char* KEYNAME_F2;
        const char* KEYNAME_F20;
        const char* KEYNAME_F21;
        const char* KEYNAME_F22;
        const char* KEYNAME_F23;
        const char* KEYNAME_F24;
        const char* KEYNAME_F25;
        const char* KEYNAME_F26;
        const char* KEYNAME_F27;
        const char* KEYNAME_F28;
        const char* KEYNAME_F29;
        const char* KEYNAME_F3;
        const char* KEYNAME_F30;
        const char* KEYNAME_F31;
        const char* KEYNAME_F32;
        const char* KEYNAME_F33;
        const char* KEYNAME_F34;
        const char* KEYNAME_F35;
        const char* KEYNAME_F4;
        const char* KEYNAME_F5;
        const char* KEYNAME_F6;
        const char* KEYNAME_F7;
        const char* KEYNAME_F8;
        const char* KEYNAME_F9;
        const char* KEYNAME_FIND;
        const char* KEYNAME_HELP;
        const char* KEYNAME_HOME;
        const char* KEYNAME_INSERT;
        const char* KEYNAME_INSERTCHAR;
        const char* KEYNAME_INSERTLINE;
        const char* KEYNAME_LEFTARROW;
        const char* KEYNAME_MENU;
        const char* KEYNAME_MODESWITCH;
        const char* KEYNAME_NEXT;
        const char* KEYNAME_PAGEDOWN;
        const char* KEYNAME_PAGEUP;
        const char* KEYNAME_PAUSE;
        const char* KEYNAME_PREV;
        const char* KEYNAME_PRINT;
        const char* KEYNAME_PRINTSCREEN;
        const char* KEYNAME_REDO;
        const char* KEYNAME_RESET;
        const char* KEYNAME_RIGHTARROW;
        const char* KEYNAME_SCROLLLOCK;
        const char* KEYNAME_SELECT;
        const char* KEYNAME_STOP;
        const char* KEYNAME_SYSREQ;
        const char* KEYNAME_SYSTEM;
        const char* KEYNAME_UNDO;
        const char* KEYNAME_UPARROW;
        const char* KEYNAME_USER;
        const UInt32 L;
        const UInt32 LEFT;
        const UInt32 LEFTBRACKET;
        const UInt32 M;
        const UInt32 MINUS;
        const UInt32 N;
        const UInt32 NUMBER_0;
        const UInt32 NUMBER_1;
        const UInt32 NUMBER_2;
        const UInt32 NUMBER_3;
        const UInt32 NUMBER_4;
        const UInt32 NUMBER_5;
        const UInt32 NUMBER_6;
        const UInt32 NUMBER_7;
        const UInt32 NUMBER_8;
        const UInt32 NUMBER_9;
        const UInt32 NUMPAD;
        const UInt32 NUMPAD_0;
        const UInt32 NUMPAD_1;
        const UInt32 NUMPAD_2;
        const UInt32 NUMPAD_3;
        const UInt32 NUMPAD_4;
        const UInt32 NUMPAD_5;
        const UInt32 NUMPAD_6;
        const UInt32 NUMPAD_7;
        const UInt32 NUMPAD_8;
        const UInt32 NUMPAD_9;
        const UInt32 NUMPAD_ADD;
        const UInt32 NUMPAD_DECIMAL;
        const UInt32 NUMPAD_DIVIDE;
        const UInt32 NUMPAD_ENTER;
        const UInt32 NUMPAD_MULTIPLY;
        const UInt32 NUMPAD_SUBTRACT;
        const UInt32 O;
        const UInt32 P;
        const UInt32 PAGE_DOWN;
        const UInt32 PAGE_UP;
        const UInt32 PERIOD;
        const UInt32 Q;
        const UInt32 QUOTE;
        const UInt32 R;
        const UInt32 RIGHT;
        const UInt32 RIGHTBRACKET;
        const UInt32 S;
        const UInt32 SEMICOLON;
        const UInt32 SHIFT;
        const UInt32 SLASH;
        const UInt32 SPACE;
        const char* STRING_BEGIN;
        const char* STRING_BREAK;
        const char* STRING_CLEARDISPLAY;
        const char* STRING_CLEARLINE;
        const char* STRING_DELETE;
        const char* STRING_DELETECHAR;
        const char* STRING_DELETELINE;
        const char* STRING_DOWNARROW;
        const char* STRING_END;
        const char* STRING_EXECUTE;
        const char* STRING_F1;
        const char* STRING_F10;
        const char* STRING_F11;
        const char* STRING_F12;
        const char* STRING_F13;
        const char* STRING_F14;
        const char* STRING_F15;
        const char* STRING_F16;
        const char* STRING_F17;
        const char* STRING_F18;
        const char* STRING_F19;
        const char* STRING_F2;
        const char* STRING_F20;
        const char* STRING_F21;
        const char* STRING_F22;
        const char* STRING_F23;
        const char* STRING_F24;
        const char* STRING_F25;
        const char* STRING_F26;
        const char* STRING_F27;
        const char* STRING_F28;
        const char* STRING_F29;
        const char* STRING_F3;
        const char* STRING_F30;
        const char* STRING_F31;
        const char* STRING_F32;
        const char* STRING_F33;
        const char* STRING_F34;
        const char* STRING_F35;
        const char* STRING_F4;
        const char* STRING_F5;
        const char* STRING_F6;
        const char* STRING_F7;
        const char* STRING_F8;
        const char* STRING_F9;
        const char* STRING_FIND;
        const char* STRING_HELP;
        const char* STRING_HOME;
        const char* STRING_INSERT;
        const char* STRING_INSERTCHAR;
        const char* STRING_INSERTLINE;
        const char* STRING_LEFTARROW;
        const char* STRING_MENU;
        const char* STRING_MODESWITCH;
        const char* STRING_NEXT;
        const char* STRING_PAGEDOWN;
        const char* STRING_PAGEUP;
        const char* STRING_PAUSE;
        const char* STRING_PREV;
        const char* STRING_PRINT;
        const char* STRING_PRINTSCREEN;
        const char* STRING_REDO;
        const char* STRING_RESET;
        const char* STRING_RIGHTARROW;
        const char* STRING_SCROLLLOCK;
        const char* STRING_SELECT;
        const char* STRING_STOP;
        const char* STRING_SYSREQ;
        const char* STRING_SYSTEM;
        const char* STRING_UNDO;
        const char* STRING_UPARROW;
        const char* STRING_USER;
        const UInt32 T;
        const UInt32 TAB;
        const UInt32 U;
        const UInt32 UP;
        const UInt32 V;
        const UInt32 W;
        const UInt32 X;
        const UInt32 Y;
        const UInt32 Z;

//##protect##"class_$data"
//##protect##"class_$data"

    };
}}

//##protect##"methods"
//##protect##"methods"

}}} // namespace Scaleform { namespace GFx { namespace AS3

#endif

