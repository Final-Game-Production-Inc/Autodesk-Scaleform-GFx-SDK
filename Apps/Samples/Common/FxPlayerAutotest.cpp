/**********************************************************************

PublicHeader:   Render
Filename    :   FxPlayerAutotest.cpp
Content     :   
Created     :   2012/07/11
Authors     :   Bart Muzzin

Copyright   :   Copyright 2012 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

***********************************************************************/

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_MsgFormat.h"
#include "FxPlayerAutotest.h"
#include "FxPlayerAppBase.h"

#if defined (SF_OS_ANDROID) && !defined (SF_ANDROID_NDK_BUILD)
extern "C" {
      extern void *__dso_handle __attribute__((__visibility__ ("hidden")));
        void *__dso_handle;
}
#endif

// *** XML Prototypes
namespace Scaleform {

#if defined(GFX_ENABLE_XML)
void WriteXML( File* file, XML::Document* xmldoc);
void AddAttributeInt(XML::ObjectManager& mgr, XML::ElementNode& node, const char* name, int value);
void AddAttributeUInt64(XML::ObjectManager& mgr, XML::ElementNode& node, const char* name, UInt64 value);
void AddAttributeFloat(XML::ObjectManager& mgr, XML::ElementNode& node, const char* name, float value);
void AddAttributeString(XML::ObjectManager& mgr, XML::ElementNode& node, const char* name, const char* value);
KeyModifiers ComputeModifiers(const char* buffer, const StringHash<unsigned>& modMap);
void ComputeModifierString(char* buffer, const KeyModifiers& mods, const StringHash<unsigned>& modMap);
#endif // GFX_ENABLE_XML

// *** Timers
class TickResultListNode : public ListNode<TickResultListNode>
{
public:
    TickResultListNode(UInt64 rawTicks, UInt64 rawFreq, UInt32 ticksMs) : 
      RawTicks(rawTicks), RawFreq(rawFreq), TicksMs(ticksMs) { }

    UInt64 RawTicks;
    UInt64 RawFreq;
    UInt32 TicksMs;
};

// This timer is used when playing back automated tests. This basically ignores calculated values,
// and returns values from the autotest instructions. This is only true on the Advance thread however,
// the Render thread may query timings (for profiling reasons, etc.), these simply return the calculated
// values.
class PlaybackTimerOverride : public Timer::TimerOverride
{
public:
    PlaybackTimerOverride() :
      LastRawTicks(1), LastRawFreq(1), LastTicksMs(1) 
    {
        AdvanceThreadId = Scaleform::GetCurrentThreadId();
    }
    virtual UInt32 GetTicksMs(UInt32 measured)
    {
        return Scaleform::GetCurrentThreadId() == AdvanceThreadId ? LastTicksMs : measured;
    }
    virtual UInt64 GetRawTicks(UInt64 measured)
    {
        return Scaleform::GetCurrentThreadId() == AdvanceThreadId ? LastRawTicks : measured;
    }
    virtual UInt64 GetRawFrequency(UInt64 measured)
    {
        return Scaleform::GetCurrentThreadId() == AdvanceThreadId ? LastRawFreq : measured;
    }
    void SetTickValue(UInt64 rawTicks, UInt64 rawFreq, UInt32 ticksMs)
    {
        LastRawTicks = rawTicks;
        LastRawFreq  = rawFreq;
        LastTicksMs  = ticksMs;
    }
private:
    ThreadId AdvanceThreadId;
    UInt64 LastRawTicks;
    UInt64 LastRawFreq;
    UInt32 LastTicksMs;

} PlaybackTimer;

#if defined(GFX_ENABLE_XML)
// This timer is used when recording automated tests. It essentially just records queries, and 
// passes back the original measured timings.
class RecordTimerOverride : public Timer::TimerOverride
{
public:
    RecordTimerOverride() : ObjectMgr(0), RootNode(0), LastRawTicks(1), LastRawFreq(1), LastTicksMs(1) { }
    void SetRecordObjects(XML::ObjectManager* objMgr, XML::ElementNode* rootNode)
    {
        AdvanceThreadId = Scaleform::GetCurrentThreadId();
        ObjectMgr = objMgr;
        RootNode = rootNode;
    }
    void SetBeforeAdvanceValues(UInt64 rawTicks, UInt64 rawFreq, UInt32 ticksMs)
    {
        recordValues(rawTicks, rawFreq, ticksMs);
    }
    virtual UInt32 GetTicksMs(UInt32)
    {
        return LastTicksMs;
    }
    virtual UInt64 GetRawTicks(UInt64)
    {
        return LastRawTicks;
    }
    virtual UInt64 GetRawFrequency(UInt64)
    {
        return LastRawFreq;
    }
private:
    void recordValues(UInt64 rawTicks, UInt64 rawFreq, UInt32 ticksMs)
    {
        SF_DEBUG_ASSERT(Scaleform::GetCurrentThreadId() == AdvanceThreadId, "Shouldn't be recording timing from thread other than Advance.");
        if (RootNode && ObjectMgr)
        {
            XML::ObjectManager& mgr = *ObjectMgr;
            Ptr<XML::ElementNode> instruction = *mgr.CreateElementNode(mgr.CreateString("Instruction"));
            AddAttributeString(mgr, *instruction, "name", "Timer");
            AddAttributeUInt64(mgr, *instruction, "rawticks", rawTicks);
            // Frequency will likely never change, so, if it doesn't, just put it in the file once.
            if (rawFreq != LastRawFreq)
                AddAttributeUInt64(mgr, *instruction, "rawfreq",  rawFreq);
            AddAttributeInt(mgr, *instruction,    "ticksMs",  ticksMs);
            RootNode->AppendChild(instruction);

            LastRawTicks = rawTicks;
            LastRawFreq  = rawFreq;
            LastTicksMs  = ticksMs;
            return;
        }
        SF_DEBUG_WARNONCE(1,"Attempt to record timer value, but no XML::ObjectManager and/or XML::ElementNode provided.");
    }
    XML::ObjectManager* ObjectMgr;
    XML::ElementNode* RootNode;
    UInt64 LastRawTicks;
    UInt64 LastRawFreq;
    UInt32 LastTicksMs;
    ThreadId AdvanceThreadId;

} RecordTimer;

#endif // GFX_ENABLE_XML

ScopedAutoTestTimerReplacement::ScopedAutoTestTimerReplacement(FxPlayerAppBase* app) : 
    ReplacedTimer(false),
    App(app)
{
#if defined(GFX_ENABLE_XML)
    if (!app || !app->AutoTest)
        return;

    ReplacedTimer = true;
    if (app->AutoTest->IsPlayback())
        Timer::SetTimerOverride(&PlaybackTimer);
    else
    {
        UInt64 rawTick = Timer::GetRawTicks();
        UInt64 rawFreq = Timer::GetRawFrequency();
        UInt32 msTicks = Timer::GetTicksMs();
        RecordTimer.SetBeforeAdvanceValues(rawTick, rawFreq, msTicks);

        Timer::SetTimerOverride(&RecordTimer);
    }
#endif
}

ScopedAutoTestTimerReplacement::~ScopedAutoTestTimerReplacement()
{
    if (!ReplacedTimer)
        return;

    //if (!App->AutoTest->IsPlayback())
    //{
    //    UInt64 rawTick = Timer::GetRawTicks();
    //    UInt64 rawFreq = Timer::GetRawFrequency();
    //    UInt32 msTicks = Timer::GetTicksMs();
    //    RecordTimer.SetAfterAdvanceValues(rawTick, rawFreq, msTicks);
    //}
    Timer::SetTimerOverride(0);
}

#if defined(GFX_ENABLE_XML)

// *** FxPlayerCommands - used only in automated testing.
class FxPlayerCommandAutoAdvance : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown, XML::ElementNode* node) const
    {
        SF_UNUSED2(controllerIdx, keyDown);

        // 'delta' attribute is the number of seconds to advance (defaults to 0).
        double advanceDelta = 0.0;
        if ( node )
        {
            for (XML::Attribute* attr = node->FirstAttribute; attr != NULL; attr = attr->Next)
            {
                String attrName(attr->Name.ToCStr());
                if ( attrName.CompareNoCase("delta") == 0 )
                    advanceDelta = SFstrtod(attr->Value.ToCStr(), 0);
            }        
        }

        // Make sure we don't do a negative advance.
        advanceDelta = Alg::Max<double>(0.0,advanceDelta);
        app->AdvanceMovie((float)advanceDelta);
    }
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        Execute(app, controllerIdx, keyDown, 0);
    }

    virtual String GetHelpString() const
    {
        return "Advances the movie a certain number of frames.";
    }
    static void Record(FxPlayerAppBase* app, float delta)
    {
        if (!app->AutoTest || app->AutoTest->IsPlayback())
            return;

        XML::ObjectManager& mgr = *app->AutoTest->GetObjManager();
        Ptr<XML::ElementNode> node = *mgr.CreateElementNode(mgr.CreateString("Instruction"));
        AddAttributeString(mgr, *node, "name", "AutoAdvance");
        AddAttributeFloat(mgr, *node, "delta", delta);

        XML::ElementNode* autoNode = (XML::ElementNode*)app->AutoTest->GetInstructions()->FirstChild.GetPtr();
        autoNode->AppendChild(node);
    }

    FXPLAYERCOMMAND_GETCOMMANDNAME(AutoAdvance);
};

class FxPlayerCommandRandom : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown, XML::ElementNode* node) const
    {
        SF_UNUSED3(app, controllerIdx, keyDown);

        // 'seed' attribute is the seed to set the random to.
        UInt32 seed = 13;
        if ( node )
        {
            for (XML::Attribute* attr = node->FirstAttribute; attr != NULL; attr = attr->Next)
            {
                String attrName(attr->Name.ToCStr());
                if ( attrName.CompareNoCase("seed") == 0 )
                    seed = (SInt32)SFatoq(attr->Value.ToCStr());
            }        
        }
        Alg::Random::SeedRandom(seed);
    }
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        Execute(app, controllerIdx, keyDown, 0);
    }

    virtual String GetHelpString() const
    {
        return "Sets the random seed to a known value.";
    }
    static void Record(FxPlayerAppBase* app, SInt32 seed);

    FXPLAYERCOMMAND_GETCOMMANDNAME(Random);

};

class FxPlayerCommandTimer : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown, XML::ElementNode* node) const
    {
        SF_UNUSED3(app, controllerIdx, keyDown);

        // 'ticks' attribute the ticks to set the timer to, for the next query.
        UInt64 rawTicks = 1;
        UInt64 rawFreq  = 1;
        UInt32 ticksMs  = 1;
        if ( node )
        {
            for (XML::Attribute* attr = node->FirstAttribute; attr != NULL; attr = attr->Next)
            {
                String attrName(attr->Name.ToCStr());
                if ( attrName.CompareNoCase("rawticks") == 0 )
                    rawTicks = SFatouq(attr->Value.ToCStr());
                if ( attrName.CompareNoCase("rawfreq") == 0 )
                    rawFreq  = SFatouq(attr->Value.ToCStr());
                if ( attrName.CompareNoCase("ticksMs") == 0 )
                    ticksMs  = (UInt32)SFatouq(attr->Value.ToCStr());
            }        
        }

        PlaybackTimer.SetTickValue(rawTicks, rawFreq, ticksMs);
    }
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        Execute(app, controllerIdx, keyDown, 0);
    }

    virtual String GetHelpString() const
    {
        return "Sets the next query of the timer to be the given value (automated testing).";
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(Timer);

};

class FxPlayerCommandKeyPress : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown, XML::ElementNode* node) const
    {
        Key::Code code = Key::None;
        unsigned int wcharCode = 0;
        KeyModifiers modifiers = 0;

        if ( node )
        {
            // 'key' attribute is the string representing the keycode to push (see Key::Code).
            // 'modifier' is a *single* modifier attribtue for the keypress (see KeyModifiers).
            for (XML::Attribute* attr = node->FirstAttribute; attr != NULL; attr = attr->Next)
            {
                String attrName(attr->Name.ToCStr());
                if ( attrName.CompareNoCase("key") == 0 )
                {
                    const KeyCode* pc = app->AutoTest->GetKeyCodeMap().GetCaseInsensitive(attr->Value.ToCStr());
                    if ( !pc )
                        fprintf(stderr, "Keycode not found for '%s'\n", attr->Value.ToCStr());
                    else
                        code = *pc;
                }
                else if ( attrName.CompareNoCase("code") == 0)
                {
                    code = (Key::Code)SFatoq(attr->Value.ToCStr());
                }
                else if ( attrName.CompareNoCase("wcharCode") == 0 )
                {
                    wcharCode = (UInt32)SFatoq(attr->Value.ToCStr());
                }
                else if ( attrName.CompareNoCase("modifier") == 0)
                {
                    modifiers = ComputeModifiers(attr->Value.ToCStr(), app->AutoTest->GetKeyModifierMap());
                }
            }
            app->OnKey(controllerIdx, code, wcharCode, keyDown, modifiers );
        }
    }
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        Execute(app, controllerIdx, keyDown, 0);
    }

    virtual String GetHelpString() const
    {
        return "Simulates pressing an arbitrary key";
    }

    static void Record(FxPlayerAppBase* app, unsigned controllerIndex, Key::Code keyCode, unsigned wcharCode, 
        bool downFlag, KeyModifiers mods)
    {
        if (!app->AutoTest || app->AutoTest->IsPlayback())
            return;

        char buffer[256];
        XML::ObjectManager& mgr = *app->AutoTest->GetObjManager();
        Ptr<XML::ElementNode> node = *mgr.CreateElementNode(mgr.CreateString("Instruction"));
        AddAttributeString(mgr, *node, "name", "KeyPress");
        AddAttributeInt(mgr, *node, "controllerIndex", controllerIndex);
        AddAttributeInt(mgr, *node, "keyDown", downFlag);
        AddAttributeInt(mgr, *node, "code", keyCode);
        AddAttributeInt(mgr, *node, "wcharCode", wcharCode);
        ComputeModifierString(buffer, mods, app->AutoTest->GetKeyModifierMap());
        AddAttributeString(mgr, *node, "modifier", buffer);

        XML::ElementNode* autoNode = (XML::ElementNode*)app->AutoTest->GetInstructions()->FirstChild.GetPtr();
        autoNode->AppendChild(node);
    }

    FXPLAYERCOMMAND_GETCOMMANDNAME(KeyPress);
};

class FxPlayerCommandMouseButton : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown, XML::ElementNode* node) const
    {
        KeyModifiers modifiers = 0;
        Point<int> pt;
        int button = 0;

        if ( node )
        {
            // 'key' attribute is the string representing the keycode to push (see Key::Code).
            // 'modifier' is a *single* modifier attribtue for the keypress (see KeyModifiers).
            // 'button' is the index of the button to push
            // 'x' and 'y' are the movie coordinates to move the mouse to.
            for (XML::Attribute* attr = node->FirstAttribute; attr != NULL; attr = attr->Next)
            {
                String attrName(attr->Name.ToCStr());
                if ( attrName.CompareNoCase("modifier") == 0)
                    modifiers = ComputeModifiers(attr->Value.ToCStr(), app->AutoTest->GetKeyModifierMap());
                else if (attrName.CompareNoCase("x") == 0 )
                    pt.x = (int)SFatoq(attr->Value.ToCStr());
                else if (attrName.CompareNoCase("y") == 0 )
                    pt.y = (int)SFatoq(attr->Value.ToCStr());
                else if (attrName.CompareNoCase("rx") == 0 )
                    pt.x = (int)(app->MViewSize.Width * SFatof(attr->Value.ToCStr()));
                else if (attrName.CompareNoCase("ry") == 0 )
                    pt.y = (int)(app->MViewSize.Height * SFatof(attr->Value.ToCStr()));
                else if (attrName.CompareNoCase("button") == 0)
                    button = (int)SFatoq(attr->Value.ToCStr());
            }
        }


        pt = app->AdjustMovieDefToViewPort(pt);
        app->OnMouseButton(controllerIdx, button, keyDown, pt, (KeyModifiers)modifiers);
    }
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        Execute(app, controllerIdx, keyDown, 0);
    }

    virtual String GetHelpString() const
    {
        return "Simulates clicking the mouse.";
    }

    static void Record(FxPlayerAppBase* app, unsigned mouseIndex, unsigned button, bool downFlag, 
        const PointF& mousePos, KeyModifiers mods)
    {
        if (!app->AutoTest || app->AutoTest->IsPlayback())
            return;

        char buffer[256];
        XML::ObjectManager& mgr = *app->AutoTest->GetObjManager();
        Ptr<XML::ElementNode> node = *mgr.CreateElementNode(mgr.CreateString("Instruction"));
        AddAttributeString(mgr, *node, "name", "MouseButton");
        AddAttributeInt(mgr, *node, "controllerIndex", mouseIndex);
        AddAttributeInt(mgr, *node, "keyDown", downFlag);
        AddAttributeInt(mgr, *node, "button", button);
        AddAttributeFloat(mgr, *node, "x", mousePos.x);
        AddAttributeFloat(mgr, *node, "y", mousePos.y);
        ComputeModifierString(buffer, mods, app->AutoTest->GetKeyModifierMap());
        AddAttributeString(mgr, *node, "modifier", buffer);

        XML::ElementNode* autoNode = (XML::ElementNode*)app->AutoTest->GetInstructions()->FirstChild.GetPtr();
        autoNode->AppendChild(node);
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(MouseButton);
};

class FxPlayerCommandMouseWheel : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool, XML::ElementNode* node) const
    {
        KeyModifiers modifiers;
        Point<int> pt;
        float zdelta = 0.0f;

        if ( node )
        {
            // 'key' attribute is the string representing the keycode to push (see Key::Code).
            // 'modifier' is a *single* modifier attribtue for the keypress (see KeyModifiers).
            // 'button' is the index of the button to push
            // 'x' and 'y' are the movie coordinates to move the mouse to.
            for (XML::Attribute* attr = node->FirstAttribute; attr != NULL; attr = attr->Next)
            {
                String attrName(attr->Name.ToCStr());
                if ( attrName.CompareNoCase("modifier") == 0)
                    modifiers = ComputeModifiers(attr->Value.ToCStr(), app->AutoTest->GetKeyModifierMap());
                else if (attrName.CompareNoCase("x") == 0 )
                    pt.x = (int)SFatoq(attr->Value.ToCStr());
                else if (attrName.CompareNoCase("y") == 0 )
                    pt.y = (int)SFatoq(attr->Value.ToCStr());
                else if (attrName.CompareNoCase("rx") == 0 )
                    pt.x = (int)(app->MViewSize.Width * SFatof(attr->Value.ToCStr()));
                else if (attrName.CompareNoCase("ry") == 0 )
                    pt.y = (int)(app->MViewSize.Height * SFatof(attr->Value.ToCStr()));
                else if (attrName.CompareNoCase("zdelta") == 0)
                    zdelta = (float)SFatof(attr->Value.ToCStr());
            }
        }

        pt = app->AdjustMovieDefToViewPort(pt);
        app->OnMouseWheel(controllerIdx, zdelta, pt, (KeyModifiers)modifiers);
    }
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        Execute(app, controllerIdx, keyDown, 0);
    }

    virtual String GetHelpString() const
    {
        return "Simulates turning the mouse wheel.";
    }
    static void Record(FxPlayerAppBase* app, unsigned mouseIndex, float zdelta, 
        const PointF& mousePos, KeyModifiers mods)
    {
        if (!app->AutoTest || app->AutoTest->IsPlayback())
            return;

        char buffer[256];
        XML::ObjectManager& mgr = *app->AutoTest->GetObjManager();
        Ptr<XML::ElementNode> node = *mgr.CreateElementNode(mgr.CreateString("Instruction"));
        AddAttributeString(mgr, *node, "name", "MouseButton");
        AddAttributeInt(mgr, *node, "controllerIndex", mouseIndex);
        AddAttributeFloat(mgr, *node, "zdelta", zdelta);
        AddAttributeFloat(mgr, *node, "x", mousePos.x);
        AddAttributeFloat(mgr, *node, "y", mousePos.y);
        ComputeModifierString(buffer, mods, app->AutoTest->GetKeyModifierMap());
        AddAttributeString(mgr, *node, "modifier", buffer);

        XML::ElementNode* autoNode = (XML::ElementNode*)app->AutoTest->GetInstructions()->FirstChild.GetPtr();
        autoNode->AppendChild(node);
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(MouseWheel);
};

class FxPlayerCommandMouseMove : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool, XML::ElementNode* node) const
    {
        KeyModifiers modifiers;
        Point<int> pt;

        if ( node ) 
        {
            // 'key' attribute is the string representing the keycode to push (see Key::Code).
            // 'modifier' is a *single* modifier attribtue for the keypress (see KeyModifiers).
            // 'x' and 'y' are the movie coordinates to move the mouse to.
            for (XML::Attribute* attr = node->FirstAttribute; attr != NULL; attr = attr->Next)
            {
                String attrName(attr->Name.ToCStr());
                if ( attrName.CompareNoCase("modifier") == 0)
                    modifiers = ComputeModifiers(attr->Value.ToCStr(), app->AutoTest->GetKeyModifierMap());
                else if (attrName.CompareNoCase("x") == 0 )
                    pt.x = (int)SFatoq(attr->Value.ToCStr());
                else if (attrName.CompareNoCase("y") == 0 )
                    pt.y = (int)SFatoq(attr->Value.ToCStr());
                else if (attrName.CompareNoCase("rx") == 0 )
                    pt.x = (int)(app->MViewSize.Width * SFatof(attr->Value.ToCStr()));
                else if (attrName.CompareNoCase("ry") == 0 )
                    pt.y = (int)(app->MViewSize.Height * SFatof(attr->Value.ToCStr()));
            }
        }

        pt = app->AdjustMovieDefToViewPort(pt);
        app->OnMouseMove(controllerIdx, pt, (KeyModifiers)modifiers);
    }
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        Execute(app, controllerIdx, keyDown, 0);
    }

    virtual String GetHelpString() const
    {
        return "Simulates moving the mouse.";
    }

    static void Record(FxPlayerAppBase* app, unsigned mouseIndex, const PointF& mousePos, KeyModifiers mods)
    {
        if (!app->AutoTest || app->AutoTest->IsPlayback())
            return;

        char buffer[256];
        XML::ObjectManager& mgr = *app->AutoTest->GetObjManager();
        Ptr<XML::ElementNode> node = *mgr.CreateElementNode(mgr.CreateString("Instruction"));
        AddAttributeString(mgr, *node, "name", "MouseMove");
        AddAttributeInt(mgr, *node, "controllerIndex", mouseIndex);
        AddAttributeFloat(mgr, *node, "x", mousePos.x);
        AddAttributeFloat(mgr, *node, "y", mousePos.y);
        ComputeModifierString(buffer, mods, app->AutoTest->GetKeyModifierMap());
        AddAttributeString(mgr, *node, "modifier", buffer);

        XML::ElementNode* autoNode = (XML::ElementNode*)app->AutoTest->GetInstructions()->FirstChild.GetPtr();
        autoNode->AppendChild(node);
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(MouseMove);
};

class FxPlayerCommandFocus : public FxPlayerCommand
{
public:
    virtual void Execute(FxPlayerAppBase* app, unsigned, bool, XML::ElementNode* node) const
    {
        KeyModifiers modifiers = 0;
        bool set = true;
        if ( node ) 
        {
            for (XML::Attribute* attr = node->FirstAttribute; attr != NULL; attr = attr->Next)
            {
                String attrName(attr->Name.ToCStr());
                if ( attrName.CompareNoCase("modifier") == 0)
                    modifiers = ComputeModifiers(attr->Value.ToCStr(), app->AutoTest->GetKeyModifierMap() );
                else if (attrName.CompareNoCase("set") == 0 )
                    set = SFatoq(attr->Value.ToCStr()) != 0;
            }
        }

        app->OnFocus(set, modifiers);
    }
    virtual void Execute(FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown) const
    {
        Execute(app, controllerIdx, keyDown, 0);
    }

    virtual String GetHelpString() const
    {
        return "Simulates focusing the movie.";
    }

    static void Record(FxPlayerAppBase* app, bool set, KeyModifiers mods)
    {
        if (!app->AutoTest || app->AutoTest->IsPlayback())
            return;

        char buffer[256];
        XML::ObjectManager& mgr = *app->AutoTest->GetObjManager();
        Ptr<XML::ElementNode> node = *mgr.CreateElementNode(mgr.CreateString("Instruction"));
        AddAttributeString(mgr, *node, "name", "Focus");
        AddAttributeInt(mgr, *node, "set", set ? 1 : 0);
        ComputeModifierString(buffer, mods, app->AutoTest->GetKeyModifierMap());
        AddAttributeString(mgr, *node, "modifier", buffer);

        XML::ElementNode* autoNode = (XML::ElementNode*)app->AutoTest->GetInstructions()->FirstChild.GetPtr();
        autoNode->AppendChild(node);
    }
    FXPLAYERCOMMAND_GETCOMMANDNAME(Focus);
};
#endif // GFX_ENABLE_XML

}; // namespace Scaleform

#if defined(GFX_ENABLE_XML)

// ** FxPlayerCommand recording functions. These are intentionally outside the Scaleform namespace.
void FxPlayerCommand::RecordAndExecute( FxPlayerAppBase* app, unsigned controllerIdx, bool keyDown, GFx::XML::ElementNode* parameters /*= 0*/ ) const
{
    // Record.
    SF_DEBUG_ASSERT(app && app->AutoTest, "Expected app, and app to be in AutoTesting mode.");
    FxPlayerAutoTest& test = *app->AutoTest;
    SF_DEBUG_ASSERT(test.GetObjManager(), "Expected XML ObjectManager instance in app.");
    XML::ObjectManager& mgr = *test.GetObjManager();
    SF_DEBUG_ASSERT(test.GetInstructions() && test.GetInstructions()->FirstChild && test.GetInstructions()->FirstChild->Type == 1, 
        "Expected XML structure to be created.");
    const char* name = GetCommandName();
    Ptr<XML::ElementNode> newNode = *mgr.CreateElementNode(mgr.CreateString("Instruction"));
    AddAttributeString(mgr, *newNode, "name", name);
    AddAttributeInt(mgr, *newNode, "keyDown", keyDown ? 1 : 0);
    AddAttributeInt(mgr, *newNode, "controllerIdx", controllerIdx);

    if (parameters)
    {
        SF_DEBUG_WARNONCE2(newNode->Value == parameters->Value, 
            "Command name and parameter nodes have different names (cmd=%s, params=%s)", 
            newNode->Value.ToCStr(), parameters->Value.ToCStr());

        // Clone the attributes on the parameter node.
        XML::Attribute * attr = parameters->FirstAttribute;
        while (attr)
        {
            XML::Attribute* newAttr = mgr.CreateAttribute(attr->Name, attr->Value);
            newNode->AddAttribute(newAttr);
            attr = attr->Next;
        }
    }
    XML::ElementNode* autoNode = (XML::ElementNode*)test.GetInstructions()->FirstChild.GetPtr();
    autoNode->AppendChild(newNode);

    // now, Execute
    Execute(app, controllerIdx, keyDown, parameters);
}
#endif // GFX_ENABLE_XML

namespace Scaleform {

FxPlayerAutoTest::FxPlayerAutoTest(FxPlayerAppBase* app, bool playback, const char* autoFile, const char* autoOutDir ) :
    App(app),
    Playback(playback),
    AutoFilename(autoFile),
    AutoOutputDir(autoOutDir)
{
}

FxPlayerAutoTest::~FxPlayerAutoTest()
{
#if defined(GFX_ENABLE_XML)
    // If we are recording, write out the file now.
    if (!IsPlayback())
    {
        Ptr<SysFile> recordFile =  *SF_NEW SysFile(AutoFilename,File::Open_Truncate|File::Open_Create|File::Open_Write);
        WriteXML(recordFile, AutoInstructions);
        recordFile->Close();
        AutoInstructions = 0;
        AutoObjManager = 0;
    }
#endif
}

bool FxPlayerAutoTest::OnInit(Platform::ViewConfig& config)
{
    KeyCodeMap.SetCaseInsensitive("A", Key::A);              
    KeyCodeMap.SetCaseInsensitive("B", Key::B );
    KeyCodeMap.SetCaseInsensitive("C", Key::C );
    KeyCodeMap.SetCaseInsensitive("D", Key::D );
    KeyCodeMap.SetCaseInsensitive("E", Key::E );
    KeyCodeMap.SetCaseInsensitive("F", Key::F );
    KeyCodeMap.SetCaseInsensitive("G", Key::G );
    KeyCodeMap.SetCaseInsensitive("H", Key::H );
    KeyCodeMap.SetCaseInsensitive("I", Key::I );
    KeyCodeMap.SetCaseInsensitive("J", Key::J );
    KeyCodeMap.SetCaseInsensitive("K", Key::K );
    KeyCodeMap.SetCaseInsensitive("L", Key::L );
    KeyCodeMap.SetCaseInsensitive("M", Key::M );
    KeyCodeMap.SetCaseInsensitive("N", Key::N );
    KeyCodeMap.SetCaseInsensitive("O", Key::O );
    KeyCodeMap.SetCaseInsensitive("P", Key::P );
    KeyCodeMap.SetCaseInsensitive("Q", Key::Q );
    KeyCodeMap.SetCaseInsensitive("R", Key::R );
    KeyCodeMap.SetCaseInsensitive("S", Key::S );
    KeyCodeMap.SetCaseInsensitive("T", Key::T );
    KeyCodeMap.SetCaseInsensitive("U", Key::U );
    KeyCodeMap.SetCaseInsensitive("V", Key::V );
    KeyCodeMap.SetCaseInsensitive("W", Key::W );
    KeyCodeMap.SetCaseInsensitive("X", Key::X );
    KeyCodeMap.SetCaseInsensitive("Y", Key::Y );
    KeyCodeMap.SetCaseInsensitive("Z", Key::Z );
    KeyCodeMap.SetCaseInsensitive("Num0", Key::Num0);           
    KeyCodeMap.SetCaseInsensitive("Num1", Key::Num1 );
    KeyCodeMap.SetCaseInsensitive("Num2", Key::Num2 );
    KeyCodeMap.SetCaseInsensitive("Num3", Key::Num3 );
    KeyCodeMap.SetCaseInsensitive("Num4", Key::Num4 );
    KeyCodeMap.SetCaseInsensitive("Num5", Key::Num5 );
    KeyCodeMap.SetCaseInsensitive("Num6", Key::Num6 );
    KeyCodeMap.SetCaseInsensitive("Num7", Key::Num7 );
    KeyCodeMap.SetCaseInsensitive("Num8", Key::Num8 );
    KeyCodeMap.SetCaseInsensitive("Num9", Key::Num9 );
    KeyCodeMap.SetCaseInsensitive("KP_0", Key::KP_0);           
    KeyCodeMap.SetCaseInsensitive("KP_1", Key::KP_1 );
    KeyCodeMap.SetCaseInsensitive("KP_2", Key::KP_2 );
    KeyCodeMap.SetCaseInsensitive("KP_3", Key::KP_3 );
    KeyCodeMap.SetCaseInsensitive("KP_4", Key::KP_4 );
    KeyCodeMap.SetCaseInsensitive("KP_5", Key::KP_5 );
    KeyCodeMap.SetCaseInsensitive("KP_6", Key::KP_6 );
    KeyCodeMap.SetCaseInsensitive("KP_7", Key::KP_7 );
    KeyCodeMap.SetCaseInsensitive("KP_8", Key::KP_8 );
    KeyCodeMap.SetCaseInsensitive("KP_9", Key::KP_9 );
    KeyCodeMap.SetCaseInsensitive("KP_Multiply", Key::KP_Multiply );
    KeyCodeMap.SetCaseInsensitive("KP_Add", Key::KP_Add );
    KeyCodeMap.SetCaseInsensitive("KP_Enter", Key::KP_Enter );
    KeyCodeMap.SetCaseInsensitive("KP_Subtract", Key::KP_Subtract );
    KeyCodeMap.SetCaseInsensitive("KP_Decimal", Key::KP_Decimal );
    KeyCodeMap.SetCaseInsensitive("KP_Divide", Key::KP_Divide );
    KeyCodeMap.SetCaseInsensitive("F1", Key::F1);             
    KeyCodeMap.SetCaseInsensitive("F2", Key::F2 );
    KeyCodeMap.SetCaseInsensitive("F3", Key::F3 );
    KeyCodeMap.SetCaseInsensitive("F4", Key::F4 );
    KeyCodeMap.SetCaseInsensitive("F5", Key::F5 );
    KeyCodeMap.SetCaseInsensitive("F6", Key::F6 );
    KeyCodeMap.SetCaseInsensitive("F7", Key::F7 );
    KeyCodeMap.SetCaseInsensitive("F8", Key::F8 );
    KeyCodeMap.SetCaseInsensitive("F9", Key::F9 );
    KeyCodeMap.SetCaseInsensitive("F10", Key::F10 );
    KeyCodeMap.SetCaseInsensitive("F11", Key::F11 );
    KeyCodeMap.SetCaseInsensitive("F12", Key::F12 );
    KeyCodeMap.SetCaseInsensitive("F13", Key::F13 );
    KeyCodeMap.SetCaseInsensitive("F14", Key::F14 );
    KeyCodeMap.SetCaseInsensitive("F15", Key::F15 );
    KeyCodeMap.SetCaseInsensitive("Backspace", Key::Backspace);      
    KeyCodeMap.SetCaseInsensitive("Tab", Key::Tab );
    KeyCodeMap.SetCaseInsensitive("Clear", Key::Clear);          
    KeyCodeMap.SetCaseInsensitive("Return", Key::Return );
    KeyCodeMap.SetCaseInsensitive("Shift", Key::Shift);          
    KeyCodeMap.SetCaseInsensitive("Control", Key::Control );
    KeyCodeMap.SetCaseInsensitive("Alt", Key::Alt );
    KeyCodeMap.SetCaseInsensitive("Pause", Key::Pause );
    KeyCodeMap.SetCaseInsensitive("CapsLock", Key::CapsLock);       
    KeyCodeMap.SetCaseInsensitive("Escape", Key::Escape);         
    KeyCodeMap.SetCaseInsensitive("Space", Key::Space);          
    KeyCodeMap.SetCaseInsensitive("PageUp", Key::PageUp );
    KeyCodeMap.SetCaseInsensitive("PageDown", Key::PageDown );
    KeyCodeMap.SetCaseInsensitive("End", Key::End);            
    KeyCodeMap.SetCaseInsensitive("Home", Key::Home );
    KeyCodeMap.SetCaseInsensitive("Left", Key::Left );
    KeyCodeMap.SetCaseInsensitive("Up", Key::Up );
    KeyCodeMap.SetCaseInsensitive("Right", Key::Right );
    KeyCodeMap.SetCaseInsensitive("Down", Key::Down );
    KeyCodeMap.SetCaseInsensitive("Insert", Key::Insert);         
    KeyCodeMap.SetCaseInsensitive("Delete", Key::Delete );
    KeyCodeMap.SetCaseInsensitive("Help", Key::Help );
    KeyCodeMap.SetCaseInsensitive("NumLock", Key::NumLock);        
    KeyCodeMap.SetCaseInsensitive("ScrollLock", Key::ScrollLock);     

    KeyCodeMap.SetCaseInsensitive("Semicolon", Key::Semicolon);      
    KeyCodeMap.SetCaseInsensitive("Equal", Key::Equal);          
    KeyCodeMap.SetCaseInsensitive("Comma", Key::Comma);          
    KeyCodeMap.SetCaseInsensitive("Minus", Key::Minus);          
    KeyCodeMap.SetCaseInsensitive("Period", Key::Period);         
    KeyCodeMap.SetCaseInsensitive("Slash", Key::Slash);          
    KeyCodeMap.SetCaseInsensitive("Bar", Key::Bar);            
    KeyCodeMap.SetCaseInsensitive("BracketLeft", Key::BracketLeft);    
    KeyCodeMap.SetCaseInsensitive("Backslash", Key::Backslash);      
    KeyCodeMap.SetCaseInsensitive("BracketRight", Key::BracketRight);   
    KeyCodeMap.SetCaseInsensitive("Quote", Key::Quote);          

    KeyCodeMap.SetCaseInsensitive("OEM_AX", Key::OEM_AX);         
    KeyCodeMap.SetCaseInsensitive("OEM_102", Key::OEM_102);        
    KeyCodeMap.SetCaseInsensitive("ICO_HELP", Key::ICO_HELP);       
    KeyCodeMap.SetCaseInsensitive("ICO_00", Key::ICO_00);         

    KeyModifierMap.SetCaseInsensitive("ShiftPressed", KeyModifiers::Key_ShiftPressed); 
    KeyModifierMap.SetCaseInsensitive("CtrlPressed", KeyModifiers::Key_CtrlPressed);  
    KeyModifierMap.SetCaseInsensitive("AltPressed", KeyModifiers::Key_AltPressed);   
    KeyModifierMap.SetCaseInsensitive("CapsToggled", KeyModifiers::Key_CapsToggled);  
    KeyModifierMap.SetCaseInsensitive("NumToggled", KeyModifiers::Key_NumToggled);   
    KeyModifierMap.SetCaseInsensitive("ScrollToggled", KeyModifiers::Key_ScrollToggled);
    KeyModifierMap.SetCaseInsensitive("ExtendedKey", KeyModifiers::Key_ExtendedKey);  

#if defined(GFX_ENABLE_XML)
    // Create all of our mappings. Some will need to be created inside the App itself, because its FxPlayerCommands
    // might not be visible externally.
    Ptr<FxPlayerCommand> cmd;
    AddCommand(SF_NEW FxPlayerCommandAutoAdvance());
    AddCommand(SF_NEW FxPlayerCommandRandom());
    AddCommand(SF_NEW FxPlayerCommandTimer());
    AddCommand(SF_NEW FxPlayerCommandKeyPress());
    AddCommand(SF_NEW FxPlayerCommandMouseButton());
    AddCommand(SF_NEW FxPlayerCommandMouseWheel());
    AddCommand(SF_NEW FxPlayerCommandMouseMove());
    AddCommand(SF_NEW FxPlayerCommandFocus());

    // Important: Seed the random number generator if doing the autotest.
    // Otherwise, AS that uses the random functions will appear differently
    // in the output, and cause the tests to fail.
    Alg::Random::SeedRandom(13);

    // Turn off VSync, it may interfere with screenshot timing.
    config.SetFlag(Platform::View_VSync, true);

    Ptr<XML::SupportBase> pxmlSupport = App->mLoader.GetXMLSupport();
    if (IsPlayback())
    {
        // Attempt to load the 'instructions'.
        XML::DOMBuilder domBuilder(pxmlSupport);
        AutoInstructions = domBuilder.ParseFile(AutoFilename.ToCStr(), App->mLoader.GetFileOpener());       

        // Construct a default test, which just takes a screenshot, advances one second, and takes another.
        if ( !AutoInstructions || !AutoInstructions->FirstChild ) 
        {
            fprintf(stderr, "Failed load autotest instructions from '%s'. Using default instructions.\n", AutoFilename.ToCStr());
            String defaultInstructions = 
                "<AutoTest>\n"
                "    <Instruction name=\"MouseMove\" x=\"0\" y=\"0\"/>\n"
                "    <Instruction name=\"TakeScreenshot\"/>\n"
                "    <Instruction name=\"AutoAdvance\" delta=\"1.0\"/>\n"
                "    <Instruction name=\"TakeScreenshot\"/>\n"
                "</AutoTest>";
            AutoInstructions = domBuilder.ParseString(defaultInstructions.ToCStr(), defaultInstructions.GetLength());
            if (!AutoInstructions || !AutoInstructions->FirstChild )
            {
                fprintf(stderr, "Failed to parse default instructions.\n" );
                return false;
            }
        }

        // Iterate directly to the instructions.
        NextAutoInstruction = AutoInstructions->FirstChild;
        while ( NextAutoInstruction && NextAutoInstruction->Type != 1 )
            NextAutoInstruction = NextAutoInstruction->NextSibling;
        if (NextAutoInstruction )
        {
            NextAutoInstruction = ((XML::ElementNode*)NextAutoInstruction.GetPtr())->FirstChild;
            while ( NextAutoInstruction && NextAutoInstruction->Type != 1 )
                NextAutoInstruction = NextAutoInstruction->NextSibling;
        } 
        SF_ASSERT(!NextAutoInstruction || SFstricmp(NextAutoInstruction->Value.ToCStr(), "Instruction" ) == 0 );

        // Playback any instructions immediately, which are not Advance commands.
        PerformPlayback(true);
    }
    else // Recording
    {
        // Create the XML structure.
        AutoObjManager = *new XML::ObjectManager();

        XML::DOMBuilder domBuilder(pxmlSupport);
        String defaultInstructions = 
            "<AutoTest>\n"
            "</AutoTest>";
        AutoInstructions = domBuilder.ParseString(defaultInstructions.ToCStr(), defaultInstructions.GetLength(), AutoObjManager);
        if (!AutoInstructions || !AutoInstructions->FirstChild )
        {
            fprintf(stderr, "Failed to parse default instructions.\n" );
            return false;
        }

        RecordTimer.SetRecordObjects(AutoObjManager, (XML::ElementNode*)AutoInstructions->FirstChild.GetPtr());
    }
    return true;

#else // !GFX_ENABLE_XML
    SF_UNUSED(config);
    fprintf(stderr, "Error: GFX_ENABLE_XML must be defined to record or playback automated tests.\n");
    return false;
#endif

}

void FxPlayerAutoTest::AddCommand(FxPlayerCommand* _cmd)
{
    Ptr<FxPlayerCommand> cmd = *_cmd;
    SF_DEBUG_ASSERT(cmd, "Can't add a NULL command");
    SF_DEBUG_ASSERT1(AutoCommandMap.GetCaseInsensitive(cmd->GetCommandName()) == 0, 
        "Adding a command with the same name (case insensitive). Name = %s", cmd->GetCommandName());
    AutoCommandMap.Add(cmd->GetCommandName(), cmd);
}

void FxPlayerAutoTest::PerformPlayback(bool untilAdvanceOnly)
{
#ifdef GFX_ENABLE_XML
    // Make sure that the movie has the mouse and keyboard count set, regardless of platform.
    // Because tests can be created on every platform, their 'capabilities' should be equivalent
    // when autotesting, so the output is also equivalent.
    if ( App->pMovie )
    {
        App->pMovie->SetMouseCursorCount(1);
        App->pMovie->SetControllerCount(1);
        App->OnFocus(true, 0);
    }

    while ( NextAutoInstruction )
    {
        if ( NextAutoInstruction->Type != 1)
        {
            NextAutoInstruction = NextAutoInstruction->NextSibling;
            continue;
        }
        XML::ElementNode* element= (XML::ElementNode*)(NextAutoInstruction.GetPtr());

        // Standard FxPlayerCommand::Execute parameters (scan XML for them).
        String commandName("");
        bool keyDown = true;
        int controllerIdx = 0;

        // Interpret the default parameter attributes from the XML
        for (XML::Attribute* attr = element->FirstAttribute; attr != NULL; attr = attr->Next)
        {
            String attrName(attr->Name.ToCStr());
            if ( attrName.CompareNoCase("name") == 0 )
                commandName = attr->Value.ToCStr();
            else if ( attrName.CompareNoCase("controllerIdx") == 0 )
                controllerIdx = (int)SFatoq(attr->Value.ToCStr());
            else if ( attrName.CompareNoCase("keyDown") == 0 )
                keyDown = SFatoq(attr->Value.ToCStr()) != 0;                
        }

        // If we are only executing until an advance instruction, quit if we find one.
        if (untilAdvanceOnly && commandName == "AutoAdvance")
            break;

        // Find the command and try executing it.
        Ptr<FxPlayerCommand>* command = AutoCommandMap.GetCaseInsensitive(commandName);
        if ( !command )
            fprintf(stderr, "Could not locate autotest command: %s\n", commandName.ToCStr());
        else
            (*command)->Execute(App, controllerIdx, keyDown, element);

        NextAutoInstruction = NextAutoInstruction->NextSibling;
    }
#else
    SF_UNUSED(untilAdvanceOnly);
#endif
}

void FxPlayerAutoTest::GetScreenshotFilename(String& filename, unsigned index) const
{
    // If doing an autotest, put the screenshot in the AutoOutputDir, and append the test filename as a directory.
    String testURL = AutoFilename;
    String testFile = testURL.GetFilename();
    Format(filename, "{0}/{1}/{2}_{3}", AutoOutputDir, testURL, testFile.StripExtension(), index);
}

void FxPlayerAutoTest::OnAdvanceMovie(float delta)
{
#if defined(GFX_ENABLE_XML)
    FxPlayerCommandAutoAdvance::Record(App, delta);
#else
    SF_UNUSED(delta);
#endif
}

void FxPlayerAutoTest::OnKeyPress(unsigned controllerIndex, Key::Code keyCode, unsigned wcharCode, bool downFlag, KeyModifiers mods)
{
#if defined(GFX_ENABLE_XML)
    FxPlayerCommandKeyPress::Record(App, controllerIndex, keyCode, wcharCode, downFlag, mods);
#else
    SF_UNUSED5(controllerIndex, keyCode, wcharCode, downFlag, mods);
#endif
}

void FxPlayerAutoTest::OnMouseButton(unsigned mouseIndex, unsigned button, bool downFlag, const PointF& mousePos, KeyModifiers mods)
{
#if defined(GFX_ENABLE_XML)
    FxPlayerCommandMouseButton::Record(App, mouseIndex, button, downFlag, mousePos, mods);
#else
    SF_UNUSED5(mouseIndex, button, downFlag, mousePos, mods);
#endif
}

void FxPlayerAutoTest::OnMouseWheel(unsigned mouseIndex, float zdelta, const PointF& mousePos, KeyModifiers mods)
{
#if defined(GFX_ENABLE_XML)
    FxPlayerCommandMouseWheel::Record(App, mouseIndex, zdelta, mousePos, mods);
#else
    SF_UNUSED4(mouseIndex, zdelta, mousePos, mods);
#endif
}

void FxPlayerAutoTest::OnMouseMove(unsigned mouseIndex, const PointF& mousePos, KeyModifiers mods)
{
#if defined(GFX_ENABLE_XML)
    FxPlayerCommandMouseMove::Record(App, mouseIndex, mousePos, mods);
#else
    SF_UNUSED3(mouseIndex, mousePos, mods);
#endif
}

void FxPlayerAutoTest::OnFocus(bool set, KeyModifiers mods)
{
#if defined(GFX_ENABLE_XML)
    FxPlayerCommandFocus::Record( App, set, mods);
#else
    SF_UNUSED2(set, mods);
#endif
}

#if defined(GFX_ENABLE_XML)
// Poor man's version of writing out XML files, since expat doesn't support it directly.
void WriteXMLNode(File& file, XML::Node* node)
{
    if (!node)
        return;

    switch(node->Type)
    {
    case 1: 
        {
            // Write the element tag.
            file.Write((const UByte*)"<", 1);
            XML::ElementNode* enode = (XML::ElementNode*)node;
            file.Write((const UByte*)enode->Value.ToCStr(), enode->Value.GetSize());
            file.Write((const UByte*)" ", 1);
            // Write attributes
            XML::Attribute* pattr = enode->FirstAttribute;
            while (pattr)
            {
                file.Write((const UByte*)pattr->Name.ToCStr(), pattr->Name.GetSize());
                file.Write((const UByte*)"=\"", 2);
                file.Write((const UByte*)pattr->Value.ToCStr(), pattr->Value.GetSize());
                file.Write((const UByte*)"\" ", 2);
                pattr = pattr->Next;
            }
            if (enode->FirstChild == 0)
            {
                file.Write((const UByte*)"/>\n", 3);
            }
            else
            {
                file.Write((const UByte*)">\n", 2);

                // Write children.
                XML::Node* child = enode->FirstChild;
                while (child)
                {
                    WriteXMLNode(file, child);
                    child = child->NextSibling;
                }

                // Write closing tag.
                file.Write((const UByte*)"</", 2);
                XML::ElementNode* xenode = (XML::ElementNode*)node;
                file.Write((const UByte*)xenode->Value.ToCStr(), xenode->Value.GetSize());
                file.Write((const UByte*)">\n", 3);
            }
            break;
        }

    case 3: // Text
        {
            XML::TextNode* tnode = (XML::TextNode*)node;        
            file.Write((const UByte*)tnode->Value.ToCStr(), tnode->Value.GetSize());
            break;
        }

    default: 
        {
            SF_DEBUG_MESSAGE1(1, "XMLNode with type: %d, unrecognized.", node->Type);
            break;
        }
    }
}
void WriteXML( File* file, XML::Document* xmldoc)
{
    if (!file || !xmldoc)
        return;

    Ptr<XML::Node> node = xmldoc->FirstChild;
    while (node)
    {
        WriteXMLNode(*file, node);
        node = node->NextSibling;
    }

}

void AddAttributeInt(XML::ObjectManager& mgr, XML::ElementNode& node, const char* name, int value)
{
    char buffer[256];
    SFsprintf(buffer, 256, "%d", value);
    XML::Attribute* attr = mgr.CreateAttribute(mgr.CreateString(name, SFstrlen(name)), mgr.CreateString(buffer, SFstrlen(buffer)));
    node.AddAttribute(attr);
}

void AddAttributeUInt64(XML::ObjectManager& mgr, XML::ElementNode& node, const char* name, UInt64 value)
{
    char buffer[256];
    SFsprintf(buffer, 256, "%llu", value);
    XML::Attribute* attr = mgr.CreateAttribute(mgr.CreateString(name, SFstrlen(name)), mgr.CreateString(buffer, SFstrlen(buffer)));
    node.AddAttribute(attr);
}

void AddAttributeFloat(XML::ObjectManager& mgr, XML::ElementNode& node, const char* name, float value)
{
    char buffer[256];
    SFsprintf(buffer, 256, "%.8f", value);
    XML::Attribute* attr = mgr.CreateAttribute(mgr.CreateString(name, SFstrlen(name)), mgr.CreateString(buffer, SFstrlen(buffer)));
    node.AddAttribute(attr);
}

void AddAttributeString(XML::ObjectManager& mgr, XML::ElementNode& node, const char* name, const char* value)
{
    XML::Attribute* attr = mgr.CreateAttribute(mgr.CreateString(name, SFstrlen(name)), mgr.CreateString(value, SFstrlen(value)));
    node.AddAttribute(attr);
}

void ComputeModifierString(char* buffer, const KeyModifiers& mods, const StringHash<unsigned>& modMap)
{
    buffer[0] = 0;
    unsigned bit = 1;
    bool first = true;
    do 
    {
        if (mods.States & bit)
        {
            StringHash<unsigned>::ConstIterator it = modMap.Begin();
            while (!it.IsEnd())
            {
                if (it->Second == bit)
                {
                    if (!first)
                        SFstrcat(buffer, 256, ";");
                    first = false;
                    SFstrcat(buffer, 256, it->First.ToCStr());
                    break;
                }
                ++it;
            }
        }
        bit <<= 1;
    } while (bit < KeyModifiers::Initialized_Bit);
}

KeyModifiers ComputeModifiers(const char* buffer, const StringHash<unsigned>& modMap)
{
    char substr[256];
    KeyModifiers mods;
    const char* nextStart = 0;

    do 
    {
        const char* nextStartBuffer = SFstrchr(buffer,';');
        if (nextStartBuffer)
            SFstrncpy(substr, 256, buffer, nextStartBuffer -buffer);
        else
            SFstrcpy(substr, 256, buffer);
        buffer = nextStartBuffer +1;

        unsigned val;
        if (modMap.Get(substr, &val))
        {
            mods.States |= val;
        }
    } while (nextStart);
    return mods;
}

#endif // defined(GFX_ENABLE_XML)

}; // Scaleform
