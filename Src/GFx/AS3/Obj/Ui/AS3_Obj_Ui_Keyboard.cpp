//##protect##"disclaimer"
/**************************************************************************

Filename    :   AS3_Obj_Ui_Keyboard.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
//##protect##"disclaimer"

#include "AS3_Obj_Ui_Keyboard.h"
#include "../../AS3_VM.h"
#include "../../AS3_Marshalling.h"
//##protect##"includes"
#include "../AS3_Obj_Array.h"
#include "GFx/AS3/AS3_MovieRoot.h"
//##protect##"includes"


namespace Scaleform { namespace GFx { namespace AS3 
{

//##protect##"methods"
//##protect##"methods"

namespace Classes { namespace fl_ui
{
    Keyboard::Keyboard(ClassTraits::Traits& t)
    : Class(t)
    , A(65)
    , ALTERNATE(18)
    , B(66)
    , BACKQUOTE(192)
    , BACKSLASH(220)
    , BACKSPACE(8)
    , C(67)
    , CAPS_LOCK(20)
    , COMMA(188)
    , COMMAND(15)
    , CONTROL(17)
    , D(68)
    , DELETE(46)
    , DOWN(40)
    , E(69)
    , END(35)
    , ENTER(13)
    , EQUAL(187)
    , ESCAPE(27)
    , F(70)
    , F1(112)
    , F10(121)
    , F11(122)
    , F12(123)
    , F13(124)
    , F14(125)
    , F15(126)
    , F2(113)
    , F3(114)
    , F4(115)
    , F5(116)
    , F6(117)
    , F7(118)
    , F8(119)
    , F9(120)
    , G(71)
    , H(72)
    , HOME(36)
    , I(73)
    , INSERT(45)
    , J(74)
    , K(75)
    , KEYNAME_BEGIN("Begin")
    , KEYNAME_BREAK("Break")
    , KEYNAME_CLEARDISPLAY("ClrDsp")
    , KEYNAME_CLEARLINE("ClrLn")
    , KEYNAME_DELETE("Delete")
    , KEYNAME_DELETECHAR("DelChr")
    , KEYNAME_DELETELINE("DelLn")
    , KEYNAME_DOWNARROW("Down")
    , KEYNAME_END("End")
    , KEYNAME_EXECUTE("Exec")
    , KEYNAME_F1("F1")
    , KEYNAME_F10("F10")
    , KEYNAME_F11("F11")
    , KEYNAME_F12("F12")
    , KEYNAME_F13("F13")
    , KEYNAME_F14("F14")
    , KEYNAME_F15("F15")
    , KEYNAME_F16("F16")
    , KEYNAME_F17("F17")
    , KEYNAME_F18("F18")
    , KEYNAME_F19("F19")
    , KEYNAME_F2("F2")
    , KEYNAME_F20("F20")
    , KEYNAME_F21("F21")
    , KEYNAME_F22("F22")
    , KEYNAME_F23("F23")
    , KEYNAME_F24("F24")
    , KEYNAME_F25("F25")
    , KEYNAME_F26("F26")
    , KEYNAME_F27("F27")
    , KEYNAME_F28("F28")
    , KEYNAME_F29("F29")
    , KEYNAME_F3("F3")
    , KEYNAME_F30("F30")
    , KEYNAME_F31("F31")
    , KEYNAME_F32("F32")
    , KEYNAME_F33("F33")
    , KEYNAME_F34("F34")
    , KEYNAME_F35("F35")
    , KEYNAME_F4("F4")
    , KEYNAME_F5("F5")
    , KEYNAME_F6("F6")
    , KEYNAME_F7("F7")
    , KEYNAME_F8("F8")
    , KEYNAME_F9("F9")
    , KEYNAME_FIND("Find")
    , KEYNAME_HELP("Help")
    , KEYNAME_HOME("Home")
    , KEYNAME_INSERT("Insert")
    , KEYNAME_INSERTCHAR("InsChr")
    , KEYNAME_INSERTLINE("InsLn")
    , KEYNAME_LEFTARROW("Left")
    , KEYNAME_MENU("Menu")
    , KEYNAME_MODESWITCH("ModeSw")
    , KEYNAME_NEXT("Next")
    , KEYNAME_PAGEDOWN("PgDn")
    , KEYNAME_PAGEUP("PgUp")
    , KEYNAME_PAUSE("Pause")
    , KEYNAME_PREV("Prev")
    , KEYNAME_PRINT("Print")
    , KEYNAME_PRINTSCREEN("PrntScrn")
    , KEYNAME_REDO("Redo")
    , KEYNAME_RESET("Reset")
    , KEYNAME_RIGHTARROW("Right")
    , KEYNAME_SCROLLLOCK("ScrlLck")
    , KEYNAME_SELECT("Select")
    , KEYNAME_STOP("Stop")
    , KEYNAME_SYSREQ("SysReq")
    , KEYNAME_SYSTEM("Sys")
    , KEYNAME_UNDO("Undo")
    , KEYNAME_UPARROW("Up")
    , KEYNAME_USER("User")
    , L(76)
    , LEFT(37)
    , LEFTBRACKET(219)
    , M(77)
    , MINUS(189)
    , N(78)
    , NUMBER_0(48)
    , NUMBER_1(49)
    , NUMBER_2(50)
    , NUMBER_3(51)
    , NUMBER_4(52)
    , NUMBER_5(53)
    , NUMBER_6(54)
    , NUMBER_7(55)
    , NUMBER_8(56)
    , NUMBER_9(57)
    , NUMPAD(21)
    , NUMPAD_0(96)
    , NUMPAD_1(97)
    , NUMPAD_2(98)
    , NUMPAD_3(99)
    , NUMPAD_4(100)
    , NUMPAD_5(101)
    , NUMPAD_6(102)
    , NUMPAD_7(103)
    , NUMPAD_8(104)
    , NUMPAD_9(105)
    , NUMPAD_ADD(107)
    , NUMPAD_DECIMAL(110)
    , NUMPAD_DIVIDE(111)
    , NUMPAD_ENTER(108)
    , NUMPAD_MULTIPLY(106)
    , NUMPAD_SUBTRACT(109)
    , O(79)
    , P(80)
    , PAGE_DOWN(34)
    , PAGE_UP(33)
    , PERIOD(190)
    , Q(81)
    , QUOTE(222)
    , R(82)
    , RIGHT(39)
    , RIGHTBRACKET(221)
    , S(83)
    , SEMICOLON(186)
    , SHIFT(16)
    , SLASH(191)
    , SPACE(32)
    , STRING_BEGIN("\xEF\x9C\xAA")
    , STRING_BREAK("\xEF\x9C\xB2")
    , STRING_CLEARDISPLAY("\xEF\x9C\xBA")
    , STRING_CLEARLINE("\xEF\x9C\xB9")
    , STRING_DELETE("\xEF\x9C\xA8")
    , STRING_DELETECHAR("\xEF\x9C\xBE")
    , STRING_DELETELINE("\xEF\x9C\xBC")
    , STRING_DOWNARROW("\xEF\x9C\x81")
    , STRING_END("\xEF\x9C\xAB")
    , STRING_EXECUTE("\xEF\x9D\x82")
    , STRING_F1("\xEF\x9C\x84")
    , STRING_F10("\xEF\x9C\x8D")
    , STRING_F11("\xEF\x9C\x8E")
    , STRING_F12("\xEF\x9C\x8F")
    , STRING_F13("\xEF\x9C\x90")
    , STRING_F14("\xEF\x9C\x91")
    , STRING_F15("\xEF\x9C\x92")
    , STRING_F16("\xEF\x9C\x93")
    , STRING_F17("\xEF\x9C\x94")
    , STRING_F18("\xEF\x9C\x95")
    , STRING_F19("\xEF\x9C\x96")
    , STRING_F2("\xEF\x9C\x85")
    , STRING_F20("\xEF\x9C\x97")
    , STRING_F21("\xEF\x9C\x98")
    , STRING_F22("\xEF\x9C\x99")
    , STRING_F23("\xEF\x9C\x9A")
    , STRING_F24("\xEF\x9C\x9B")
    , STRING_F25("\xEF\x9C\x9C")
    , STRING_F26("\xEF\x9C\x9D")
    , STRING_F27("\xEF\x9C\x9E")
    , STRING_F28("\xEF\x9C\x9F")
    , STRING_F29("\xEF\x9C\xA0")
    , STRING_F3("\xEF\x9C\x86")
    , STRING_F30("\xEF\x9C\xA1")
    , STRING_F31("\xEF\x9C\xA2")
    , STRING_F32("\xEF\x9C\xA3")
    , STRING_F33("\xEF\x9C\xA4")
    , STRING_F34("\xEF\x9C\xA5")
    , STRING_F35("\xEF\x9C\xA6")
    , STRING_F4("\xEF\x9C\x87")
    , STRING_F5("\xEF\x9C\x88")
    , STRING_F6("\xEF\x9C\x89")
    , STRING_F7("\xEF\x9C\x8A")
    , STRING_F8("\xEF\x9C\x8B")
    , STRING_F9("\xEF\x9C\x8C")
    , STRING_FIND("\xEF\x9D\x85")
    , STRING_HELP("\xEF\x9D\x86")
    , STRING_HOME("\xEF\x9C\xA9")
    , STRING_INSERT("\xEF\x9C\xA7")
    , STRING_INSERTCHAR("\xEF\x9C\xBD")
    , STRING_INSERTLINE("\xEF\x9C\xBB")
    , STRING_LEFTARROW("\xEF\x9C\x82")
    , STRING_MENU("\xEF\x9C\xB5")
    , STRING_MODESWITCH("\xEF\x9D\x87")
    , STRING_NEXT("\xEF\x9D\x80")
    , STRING_PAGEDOWN("\xEF\x9C\xAD")
    , STRING_PAGEUP("\xEF\x9C\xAC")
    , STRING_PAUSE("\xEF\x9C\xB0")
    , STRING_PREV("\xEF\x9C\xBF")
    , STRING_PRINT("\xEF\x9C\xB8")
    , STRING_PRINTSCREEN("\xEF\x9C\xAE")
    , STRING_REDO("\xEF\x9D\x84")
    , STRING_RESET("\xEF\x9C\xB3")
    , STRING_RIGHTARROW("\xEF\x9C\x83")
    , STRING_SCROLLLOCK("\xEF\x9C\xAF")
    , STRING_SELECT("\xEF\x9D\x81")
    , STRING_STOP("\xEF\x9C\xB4")
    , STRING_SYSREQ("\xEF\x9C\xB1")
    , STRING_SYSTEM("\xEF\x9C\xB7")
    , STRING_UNDO("\xEF\x9D\x83")
    , STRING_UPARROW("\xEF\x9C\x80")
    , STRING_USER("\xEF\x9C\xB6")
    , T(84)
    , TAB(9)
    , U(85)
    , UP(38)
    , V(86)
    , W(87)
    , X(88)
    , Y(89)
    , Z(90)
    {
//##protect##"class_::Keyboard::Keyboard()"
//##protect##"class_::Keyboard::Keyboard()"
    }
    void Keyboard::capsLockGet(bool& result)
    {
//##protect##"class_::Keyboard::capsLockGet()"
        SF_UNUSED1(result);
#ifdef GFX_ENABLE_KEYBOARD
        ASVM& vm = static_cast<ASVM&>(GetVM());
        // only zero keyboard is supported for now
        KeyboardState* ks = vm.GetMovieImpl()->GetKeyboardState(0); 
        KeyModifiers sks = ks->GetKeyModifiers();
        result = sks.IsCapsToggled();
#else
        WARN_NOT_IMPLEMENTED("Keyboard::capsLockGet()");
        result = false;
#endif
//##protect##"class_::Keyboard::capsLockGet()"
    }
    void Keyboard::numLockGet(bool& result)
    {
//##protect##"class_::Keyboard::numLockGet()"
#ifdef GFX_ENABLE_KEYBOARD
        ASVM& vm = static_cast<ASVM&>(GetVM());
        // only zero keyboard is supported for now
        KeyboardState* ks = vm.GetMovieImpl()->GetKeyboardState(0); 
        KeyModifiers sks = ks->GetKeyModifiers();
        result = sks.IsNumToggled();
#else
        WARN_NOT_IMPLEMENTED("Keyboard::numLockGet()");
        result = false;
#endif
//##protect##"class_::Keyboard::numLockGet()"
    }
    void Keyboard::isAccessible(bool& result)
    {
//##protect##"class_::Keyboard::isAccessible()"
        SF_UNUSED1(result);
        result = false; //@TODO
//##protect##"class_::Keyboard::isAccessible()"
    }
//##protect##"class_$methods"
//##protect##"class_$methods"

}} // namespace Classes

typedef ThunkFunc0<Classes::fl_ui::Keyboard, Classes::fl_ui::Keyboard::mid_capsLockGet, bool> TFunc_Classes_Keyboard_capsLockGet;
typedef ThunkFunc0<Classes::fl_ui::Keyboard, Classes::fl_ui::Keyboard::mid_numLockGet, bool> TFunc_Classes_Keyboard_numLockGet;
typedef ThunkFunc0<Classes::fl_ui::Keyboard, Classes::fl_ui::Keyboard::mid_isAccessible, bool> TFunc_Classes_Keyboard_isAccessible;

template <> const TFunc_Classes_Keyboard_capsLockGet::TMethod TFunc_Classes_Keyboard_capsLockGet::Method = &Classes::fl_ui::Keyboard::capsLockGet;
template <> const TFunc_Classes_Keyboard_numLockGet::TMethod TFunc_Classes_Keyboard_numLockGet::Method = &Classes::fl_ui::Keyboard::numLockGet;
template <> const TFunc_Classes_Keyboard_isAccessible::TMethod TFunc_Classes_Keyboard_isAccessible::Method = &Classes::fl_ui::Keyboard::isAccessible;

namespace ClassTraits { namespace fl_ui
{
    const MemberInfo Keyboard::mi[Keyboard::MemberInfoNum] = {
        {"A", NULL, OFFSETOF(Classes::fl_ui::Keyboard, A), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"ALTERNATE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, ALTERNATE), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"B", NULL, OFFSETOF(Classes::fl_ui::Keyboard, B), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"BACKQUOTE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, BACKQUOTE), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"BACKSLASH", NULL, OFFSETOF(Classes::fl_ui::Keyboard, BACKSLASH), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"BACKSPACE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, BACKSPACE), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"C", NULL, OFFSETOF(Classes::fl_ui::Keyboard, C), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"CAPS_LOCK", NULL, OFFSETOF(Classes::fl_ui::Keyboard, CAPS_LOCK), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"CharCodeStrings", NULL, OFFSETOF(Classes::fl_ui::Keyboard, CharCodeStrings), Abc::NS_Public, SlotInfo::BT_ObjectCpp, 1},
        {"COMMA", NULL, OFFSETOF(Classes::fl_ui::Keyboard, COMMA), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"COMMAND", NULL, OFFSETOF(Classes::fl_ui::Keyboard, COMMAND), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"CONTROL", NULL, OFFSETOF(Classes::fl_ui::Keyboard, CONTROL), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"D", NULL, OFFSETOF(Classes::fl_ui::Keyboard, D), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"DELETE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, DELETE), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"DOWN", NULL, OFFSETOF(Classes::fl_ui::Keyboard, DOWN), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"E", NULL, OFFSETOF(Classes::fl_ui::Keyboard, E), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"END", NULL, OFFSETOF(Classes::fl_ui::Keyboard, END), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"ENTER", NULL, OFFSETOF(Classes::fl_ui::Keyboard, ENTER), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"EQUAL", NULL, OFFSETOF(Classes::fl_ui::Keyboard, EQUAL), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"ESCAPE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, ESCAPE), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"F", NULL, OFFSETOF(Classes::fl_ui::Keyboard, F), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"F1", NULL, OFFSETOF(Classes::fl_ui::Keyboard, F1), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"F10", NULL, OFFSETOF(Classes::fl_ui::Keyboard, F10), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"F11", NULL, OFFSETOF(Classes::fl_ui::Keyboard, F11), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"F12", NULL, OFFSETOF(Classes::fl_ui::Keyboard, F12), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"F13", NULL, OFFSETOF(Classes::fl_ui::Keyboard, F13), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"F14", NULL, OFFSETOF(Classes::fl_ui::Keyboard, F14), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"F15", NULL, OFFSETOF(Classes::fl_ui::Keyboard, F15), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"F2", NULL, OFFSETOF(Classes::fl_ui::Keyboard, F2), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"F3", NULL, OFFSETOF(Classes::fl_ui::Keyboard, F3), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"F4", NULL, OFFSETOF(Classes::fl_ui::Keyboard, F4), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"F5", NULL, OFFSETOF(Classes::fl_ui::Keyboard, F5), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"F6", NULL, OFFSETOF(Classes::fl_ui::Keyboard, F6), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"F7", NULL, OFFSETOF(Classes::fl_ui::Keyboard, F7), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"F8", NULL, OFFSETOF(Classes::fl_ui::Keyboard, F8), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"F9", NULL, OFFSETOF(Classes::fl_ui::Keyboard, F9), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"G", NULL, OFFSETOF(Classes::fl_ui::Keyboard, G), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"H", NULL, OFFSETOF(Classes::fl_ui::Keyboard, H), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"HOME", NULL, OFFSETOF(Classes::fl_ui::Keyboard, HOME), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"I", NULL, OFFSETOF(Classes::fl_ui::Keyboard, I), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"INSERT", NULL, OFFSETOF(Classes::fl_ui::Keyboard, INSERT), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"J", NULL, OFFSETOF(Classes::fl_ui::Keyboard, J), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"K", NULL, OFFSETOF(Classes::fl_ui::Keyboard, K), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"KEYNAME_BEGIN", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_BEGIN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_BREAK", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_BREAK), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_CLEARDISPLAY", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_CLEARDISPLAY), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_CLEARLINE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_CLEARLINE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_DELETE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_DELETE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_DELETECHAR", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_DELETECHAR), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_DELETELINE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_DELETELINE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_DOWNARROW", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_DOWNARROW), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_END", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_END), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_EXECUTE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_EXECUTE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F1", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F1), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F10", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F10), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F11", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F11), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F12", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F12), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F13", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F13), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F14", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F14), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F15", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F15), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F16", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F16), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F17", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F17), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F18", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F18), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F19", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F19), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F2", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F2), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F20", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F20), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F21", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F21), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F22", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F22), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F23", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F23), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F24", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F24), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F25", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F25), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F26", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F26), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F27", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F27), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F28", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F28), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F29", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F29), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F3", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F3), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F30", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F30), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F31", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F31), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F32", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F32), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F33", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F33), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F34", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F34), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F35", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F35), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F4", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F4), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F5", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F5), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F6", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F6), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F7", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F7), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F8", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F8), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_F9", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_F9), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_FIND", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_FIND), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_HELP", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_HELP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_HOME", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_HOME), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_INSERT", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_INSERT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_INSERTCHAR", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_INSERTCHAR), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_INSERTLINE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_INSERTLINE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_LEFTARROW", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_LEFTARROW), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_MENU", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_MENU), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_MODESWITCH", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_MODESWITCH), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_NEXT", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_NEXT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_PAGEDOWN", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_PAGEDOWN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_PAGEUP", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_PAGEUP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_PAUSE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_PAUSE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_PREV", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_PREV), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_PRINT", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_PRINT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_PRINTSCREEN", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_PRINTSCREEN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_REDO", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_REDO), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_RESET", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_RESET), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_RIGHTARROW", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_RIGHTARROW), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_SCROLLLOCK", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_SCROLLLOCK), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_SELECT", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_SELECT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_STOP", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_STOP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_SYSREQ", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_SYSREQ), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_SYSTEM", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_SYSTEM), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_UNDO", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_UNDO), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_UPARROW", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_UPARROW), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"KEYNAME_USER", NULL, OFFSETOF(Classes::fl_ui::Keyboard, KEYNAME_USER), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"L", NULL, OFFSETOF(Classes::fl_ui::Keyboard, L), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"LEFT", NULL, OFFSETOF(Classes::fl_ui::Keyboard, LEFT), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"LEFTBRACKET", NULL, OFFSETOF(Classes::fl_ui::Keyboard, LEFTBRACKET), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"M", NULL, OFFSETOF(Classes::fl_ui::Keyboard, M), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"MINUS", NULL, OFFSETOF(Classes::fl_ui::Keyboard, MINUS), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"N", NULL, OFFSETOF(Classes::fl_ui::Keyboard, N), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMBER_0", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMBER_0), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMBER_1", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMBER_1), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMBER_2", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMBER_2), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMBER_3", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMBER_3), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMBER_4", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMBER_4), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMBER_5", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMBER_5), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMBER_6", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMBER_6), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMBER_7", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMBER_7), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMBER_8", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMBER_8), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMBER_9", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMBER_9), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMPAD", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMPAD), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMPAD_0", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMPAD_0), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMPAD_1", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMPAD_1), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMPAD_2", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMPAD_2), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMPAD_3", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMPAD_3), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMPAD_4", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMPAD_4), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMPAD_5", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMPAD_5), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMPAD_6", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMPAD_6), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMPAD_7", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMPAD_7), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMPAD_8", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMPAD_8), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMPAD_9", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMPAD_9), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMPAD_ADD", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMPAD_ADD), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMPAD_DECIMAL", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMPAD_DECIMAL), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMPAD_DIVIDE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMPAD_DIVIDE), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMPAD_ENTER", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMPAD_ENTER), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMPAD_MULTIPLY", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMPAD_MULTIPLY), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"NUMPAD_SUBTRACT", NULL, OFFSETOF(Classes::fl_ui::Keyboard, NUMPAD_SUBTRACT), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"O", NULL, OFFSETOF(Classes::fl_ui::Keyboard, O), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"P", NULL, OFFSETOF(Classes::fl_ui::Keyboard, P), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"PAGE_DOWN", NULL, OFFSETOF(Classes::fl_ui::Keyboard, PAGE_DOWN), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"PAGE_UP", NULL, OFFSETOF(Classes::fl_ui::Keyboard, PAGE_UP), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"PERIOD", NULL, OFFSETOF(Classes::fl_ui::Keyboard, PERIOD), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"Q", NULL, OFFSETOF(Classes::fl_ui::Keyboard, Q), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"QUOTE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, QUOTE), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"R", NULL, OFFSETOF(Classes::fl_ui::Keyboard, R), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"RIGHT", NULL, OFFSETOF(Classes::fl_ui::Keyboard, RIGHT), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"RIGHTBRACKET", NULL, OFFSETOF(Classes::fl_ui::Keyboard, RIGHTBRACKET), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"S", NULL, OFFSETOF(Classes::fl_ui::Keyboard, S), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"SEMICOLON", NULL, OFFSETOF(Classes::fl_ui::Keyboard, SEMICOLON), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"SHIFT", NULL, OFFSETOF(Classes::fl_ui::Keyboard, SHIFT), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"SLASH", NULL, OFFSETOF(Classes::fl_ui::Keyboard, SLASH), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"SPACE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, SPACE), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"STRING_BEGIN", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_BEGIN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_BREAK", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_BREAK), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_CLEARDISPLAY", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_CLEARDISPLAY), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_CLEARLINE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_CLEARLINE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_DELETE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_DELETE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_DELETECHAR", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_DELETECHAR), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_DELETELINE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_DELETELINE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_DOWNARROW", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_DOWNARROW), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_END", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_END), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_EXECUTE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_EXECUTE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F1", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F1), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F10", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F10), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F11", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F11), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F12", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F12), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F13", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F13), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F14", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F14), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F15", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F15), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F16", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F16), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F17", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F17), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F18", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F18), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F19", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F19), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F2", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F2), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F20", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F20), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F21", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F21), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F22", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F22), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F23", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F23), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F24", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F24), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F25", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F25), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F26", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F26), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F27", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F27), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F28", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F28), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F29", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F29), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F3", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F3), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F30", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F30), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F31", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F31), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F32", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F32), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F33", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F33), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F34", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F34), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F35", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F35), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F4", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F4), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F5", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F5), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F6", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F6), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F7", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F7), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F8", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F8), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_F9", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_F9), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_FIND", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_FIND), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_HELP", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_HELP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_HOME", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_HOME), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_INSERT", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_INSERT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_INSERTCHAR", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_INSERTCHAR), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_INSERTLINE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_INSERTLINE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_LEFTARROW", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_LEFTARROW), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_MENU", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_MENU), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_MODESWITCH", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_MODESWITCH), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_NEXT", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_NEXT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_PAGEDOWN", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_PAGEDOWN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_PAGEUP", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_PAGEUP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_PAUSE", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_PAUSE), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_PREV", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_PREV), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_PRINT", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_PRINT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_PRINTSCREEN", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_PRINTSCREEN), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_REDO", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_REDO), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_RESET", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_RESET), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_RIGHTARROW", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_RIGHTARROW), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_SCROLLLOCK", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_SCROLLLOCK), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_SELECT", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_SELECT), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_STOP", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_STOP), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_SYSREQ", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_SYSREQ), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_SYSTEM", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_SYSTEM), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_UNDO", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_UNDO), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_UPARROW", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_UPARROW), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"STRING_USER", NULL, OFFSETOF(Classes::fl_ui::Keyboard, STRING_USER), Abc::NS_Public, SlotInfo::BT_ConstChar, 1},
        {"T", NULL, OFFSETOF(Classes::fl_ui::Keyboard, T), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"TAB", NULL, OFFSETOF(Classes::fl_ui::Keyboard, TAB), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"U", NULL, OFFSETOF(Classes::fl_ui::Keyboard, U), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"UP", NULL, OFFSETOF(Classes::fl_ui::Keyboard, UP), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"V", NULL, OFFSETOF(Classes::fl_ui::Keyboard, V), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"W", NULL, OFFSETOF(Classes::fl_ui::Keyboard, W), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"X", NULL, OFFSETOF(Classes::fl_ui::Keyboard, X), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"Y", NULL, OFFSETOF(Classes::fl_ui::Keyboard, Y), Abc::NS_Public, SlotInfo::BT_UInt, 1},
        {"Z", NULL, OFFSETOF(Classes::fl_ui::Keyboard, Z), Abc::NS_Public, SlotInfo::BT_UInt, 1},
    };

    // const UInt16 Keyboard::tito[Keyboard::ThunkInfoNum] = {
    //    0, 1, 2, 
    // };
    const TypeInfo* Keyboard::tit[3] = {
        &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
        &AS3::fl::BooleanTI, 
    };
    const ThunkInfo Keyboard::ti[Keyboard::ThunkInfoNum] = {
        {TFunc_Classes_Keyboard_capsLockGet::Func, &Keyboard::tit[0], "capsLock", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_Keyboard_numLockGet::Func, &Keyboard::tit[1], "numLock", NULL, Abc::NS_Public, CT_Get, 0, 0, 0, 0, NULL},
        {TFunc_Classes_Keyboard_isAccessible::Func, &Keyboard::tit[2], "isAccessible", NULL, Abc::NS_Public, CT_Method, 0, 0, 0, 0, NULL},
    };

    Keyboard::Keyboard(VM& vm, const ClassInfo& ci)
    : fl::Object(vm, ci)
    {
//##protect##"ClassTraits::Keyboard::Keyboard()"
//##protect##"ClassTraits::Keyboard::Keyboard()"

    }

    Pickable<Traits> Keyboard::MakeClassTraits(VM& vm)
    {
        MemoryHeap* mh = vm.GetMemoryHeap();
        Pickable<Traits> ctr(SF_HEAP_NEW_ID(mh, StatMV_VM_CTraits_Mem) Keyboard(vm, AS3::fl_ui::KeyboardCI));

        Pickable<InstanceTraits::Traits> itr(SF_HEAP_NEW_ID(mh, StatMV_VM_ITraits_Mem) InstanceTraitsType(vm, AS3::fl_ui::KeyboardCI));
        ctr->SetInstanceTraits(itr);

        // There is no problem with Pickable not assigned to anything here. Class constructor takes care of this.
        Pickable<Class> cl(SF_HEAP_NEW_ID(mh, StatMV_VM_Class_Mem) ClassType(*ctr));

        return ctr;
    }
//##protect##"ClassTraits$methods"
//##protect##"ClassTraits$methods"

}} // namespace ClassTraits

namespace fl_ui
{
    const TypeInfo KeyboardTI = {
        TypeInfo::CompileTime | TypeInfo::Final,
        sizeof(ClassTraits::fl_ui::Keyboard::InstanceType),
        ClassTraits::fl_ui::Keyboard::ThunkInfoNum,
        ClassTraits::fl_ui::Keyboard::MemberInfoNum,
        0,
        0,
        "Keyboard", "flash.ui", &fl::ObjectTI,
        TypeInfo::None
    };

    const ClassInfo KeyboardCI = {
        &KeyboardTI,
        ClassTraits::fl_ui::Keyboard::MakeClassTraits,
        ClassTraits::fl_ui::Keyboard::ti,
        ClassTraits::fl_ui::Keyboard::mi,
        NULL,
        NULL,
    };
}; // namespace fl_ui


}}} // namespace Scaleform { namespace GFx { namespace AS3

