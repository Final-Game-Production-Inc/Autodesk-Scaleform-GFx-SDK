/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_PlayerImpl.h
Content     :   MovieRoot and Definition classes
Created     :   
Authors     :   Michael Antonov, Artem Bolgar, Prasad Silva

Notes       :   This file contains class declarations used in
                GFxPlayerImpl.cpp only. Declarations that need to be
                visible by other player files should be placed
                in DisplayObjectBase.h.

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_PlayerImpl_H
#define INC_SF_GFX_PlayerImpl_H

#include "GFxConfig.h"
#include "GFx/GFx_Sprite.h"
#include "Render/Render_Math2D.h"
#include "Kernel/SF_File.h"

#include "GFx/GFx_DisplayList.h"
#include "GFx/GFx_LoaderImpl.h"
#include "GFx/GFx_FontResource.h"
#include "GFx/GFx_Shape.h"
//#include "GFx/AS2/AS2_StringManager.h"
#include "GFx/GFx_MediaInterfaces.h"

// Font managers are allocated in sprite root nodes.
#include "GFx/GFx_FontManager.h"

#include "Render/Text/Text_Core.h" // for Text::Allocator
#include "GFx/GFx_PlayerTasks.h"

// For now
#include "GFx/GFx_MovieDef.h"

#if defined(SF_OS_WIN32) && defined(GFX_ENABLE_BUILTIN_KOREAN_IME) && !defined(SF_NO_IME_SUPPORT)
#include "IME/GFxIMEImm32Dll.h"
#endif //defined(SF_OS_WIN32) && defined(GFX_ENABLE_BUILTIN_KOREAN_IME) && !defined(SF_NO_IME_SUPPORT)

// This is internal file that uses SF_HEAP_NEW.
#include "Kernel/SF_HeapNew.h"

#include "GFx/GFx_Input.h"


#include "GFx/GFx_ASMovieRootBase.h"
#include "GFx/AMP/Amp_ViewStats.h"

#include "Render/Render_Context.h"
#include "Render/Render_DrawableImage.h"
#include "GFx/GFx_DrawingContext.h"

#include "Render/Render_ScreenToWorld.h"

#if defined(SF_BUILD_CONSUMER)
#include "Render/Text/Text_FontManager.h"
#include "Render/Render_TreeNode.h"
#include "Render/Render_TreeText.h"
#include "Render/FontProvider/Render_FontProviderHUD.h"
#endif

#ifdef GFX_GESTURE_RECOGNIZE
#include "GFx/GFx_Gesture.h"
#endif

#define GFX_MAX_CONTROLLERS_SUPPORTED 16
#define DEFAULT_FLASH_FOV 55.0f

namespace Scaleform {
	
#ifdef GFX_ENABLE_SOUND
	namespace Sound
	{
		class   SoundRenderer;
	}
#endif
	
namespace GFx {

// ***** Declared Classes
class MovieDefImpl;
class MovieImpl;
class SpriteDef;
// Helpers
class ImportInfo;
class SwfEvent;
class LoadQueueEntry;
class DoublePrecisionGuard;
// Tag classes
class PlaceObjectTag;
class PlaceObject2Tag;
class PlaceObject3Tag;
class RemoveObjectTag;
class RemoveObject2Tag;
class SetBackgroundColorTag;
// class GASDoAction;           - in GFxAction.cpp
// class GFxStartSoundTag;      - in GFxSound.cpp

/*
#ifdef GFX_GESTURE_RECOGNIZE
	class GestureRecognizer;
#endif
	*/

// ***** External Classes
class Loader;

class IMECandidateListStyle;

#ifdef GFX_ENABLE_SOUND
class AudioBase;
#endif

//  ***** MovieDefRootNode

// MovieDefRootNode is maintained in MovieImpl for each MovieDefImpl; this
// node is referenced by every GFxSprite which has its own MovieDefImpl. We keep
// a distinction between imported and loadMovie based root sprites.
// 
// Purpose:
//  1) Allows us to cache temporary state of loading MovieDefs in the beginning of
//     Advance so that it is always consistent for all instances. If not done, it
//     would be possible to different instances of the same progressively loaded
//     file to be in different places in the same GfxMovieView frame.
//  2) Allows for FontManager to be shared among same-def movies.

class MovieDefRootNode : public ListNode<MovieDefRootNode>, public NewOverrideBase<StatMV_Other_Mem>
{
public:
    MovieDefRootNode(MovieDefImpl *pdefImpl, bool importFlag = 0)
        : SpriteRefCount(1), pDefImpl(pdefImpl), ImportFlag(importFlag)
    {  }
    virtual ~MovieDefRootNode() {}

    // The number of root sprites that are referencing this MovieDef. If this
    // number goes down to 0, the node is deleted.
    unsigned        SpriteRefCount;

    // MovieDef these root sprites use. No need to AddRef because GFxSprite does.
    MovieDefImpl*   pDefImpl;

    // Cache the number of frames that was loaded.
    unsigned        LoadingFrame;
    UInt32          BytesLoaded;
    // Imports don't get to rely on LoadingFrame because they are usually nested
    // and have independent frame count from import root. This LoadingFrame is
    // not necessary anyway because imports are guaranteed to be fully loaded.
    bool            ImportFlag;

    // The pointer to font manager used for a sprite. We need to keep font manager
    // here so that it can keep references to all MovieDefImpl instances for
    // all fonts obtained through GFxFontLib.
    Ptr<FontManager> pFontManager;
};

class TabIndexSortFunctor
{
public:
    inline bool operator()(const InteractiveObject* a, const InteractiveObject* b) const
    {
        return (a->GetTabIndex() < b->GetTabIndex());
    }
};

class AutoTabSortFunctor
{
    enum
    {
        Epsilon = 20
    };
public:
    inline bool operator()(const InteractiveObject* a, const InteractiveObject* b) const
    {
        DisplayObjectBase::Matrix ma = a->GetLevelMatrix();
        DisplayObjectBase::Matrix mb = b->GetLevelMatrix();
        RectF aRect  = ma.EncloseTransform(a->GetFocusRect());
        RectF bRect  = mb.EncloseTransform(b->GetFocusRect());

        //@DBG
        //printf("Comparing %s with %s\n", a->GetCharacterHandle()->GetNamePath().ToCStr(), b->GetCharacterHandle()->GetNamePath().ToCStr());

        PointF centerA = aRect.Center();
        PointF centerB = bRect.Center();

        if (Alg::Abs(aRect.y1 - bRect.y1) <= Epsilon ||
            Alg::Abs(aRect.y2 - bRect.y2) <= Epsilon ||
            Alg::Abs(centerA.y - centerB.y) <= Epsilon)
        {
            // same row
            //@DBG
            //printf ("   same row, less? %d, xA = %f, xB = %f\n", int(centerA.x < centerB.x), centerA.x , centerB.x);
            return centerA.x < centerB.x;
        }
        //@DBG
        //printf ("   less? %d, yA = %f, yB = %f\n", int(centerA.y < centerB.y), centerA.y , centerB.y);
        return centerA.y < centerB.y;
    }
};


class ASIntervalTimerIntf : public RefCountBase<ASIntervalTimerIntf, StatMV_ActionScript_Mem>
{
public:
    virtual ~ASIntervalTimerIntf() {}

    virtual void            Start(MovieImpl* proot) =0;
    // frameTime is in seconds
    virtual bool            Invoke(MovieImpl* proot, float frameTime) =0; 
    virtual bool            IsActive() const =0;
    virtual void            Clear() =0;
    virtual bool            ClearFor(MovieImpl* proot, MovieDefImpl*) =0;
    // returns time in microseconds
    virtual UInt64          GetNextInvokeTime() const =0; 
    virtual void            SetId(int id) =0;
    virtual int             GetId() const =0;
};

// Focus related structures
struct FocusGroupDescr
{
    Ptr<Render::TreeShape>              FocusRectNode;
    ArrayDH<Ptr<InteractiveObject>, StatMV_Other_Mem> TabableArray; 
    mutable WeakPtr<InteractiveObject>  LastFocused;
    Ptr<CharacterHandle>                ModalClip;
    UInt32                              LastFocusKeyCode;
    RectF                               LastFocusedRect;
    bool                                FocusRectShown;
    enum
    {
        TabableArray_Initialized        = 0x1,
        TabableArray_WithFocusEnabled   = 0x2
    };
    UInt8                               TabableArrayStatus;

    FocusGroupDescr(MemoryHeap* heap = NULL):
        TabableArray((!heap) ? Memory::GetHeapByAddress(GetThis()): heap), 
        LastFocusKeyCode(0), FocusRectShown(false), TabableArrayStatus(0) {}

    bool                IsFocused(const InteractiveObject* ch) const 
    { 
        Ptr<InteractiveObject> lch = LastFocused; return lch.GetPtr() == ch; 
    }
    void                ResetFocus() { LastFocused = NULL; }
    void                ResetFocusDirection() { LastFocusKeyCode = 0; }
    void                ResetTabableArray()
    {
        if (TabableArrayStatus & TabableArray_Initialized)
        {
            TabableArray.Resize(0);
            TabableArrayStatus = 0;
        }
    }
    Sprite* GetModalClip(MovieImpl* proot);

private:
    FocusGroupDescr* GetThis() { return this; }
};

struct ProcessFocusKeyInfo : public NewOverrideBase<StatMV_Other_Mem>
{
    FocusGroupDescr*        pFocusGroup;
    Ptr<InteractiveObject>  CurFocused;
    int                     CurFocusIdx;
    RectF                   Prev_aRect;
    UInt32                  PrevKeyCode;
    UInt32                  KeyCode;
    UInt8                   KeyboardIndex;
    UInt8                   KeysState;
    bool                    ManualFocus;
    bool                    InclFocusEnabled;
    bool                    Initialized;

    ProcessFocusKeyInfo():pFocusGroup(NULL), CurFocusIdx(-1),PrevKeyCode(0),KeyCode(0),
        KeyboardIndex(0), KeysState(0), ManualFocus(false),InclFocusEnabled(false),
        Initialized(false) {}
};

//
// ***** MovieImpl
//
// Global, shared root state for a GFxMovieSub and all its characters.
//

class MovieImpl : public Movie
{
    friend class Movie;

public:
    typedef Matrix2F Matrix;
    // Expose Value::ObjectInterface to other classes, to avoid protected.
    typedef Value::ObjectInterface ValueObjectInterface;

    // Storage for wide char conversions (VT_ConvertStringW). Stored per Value (if needed)
    struct WideStringStorage : public RefCountBase<WideStringStorage, StatMV_Other_Mem>
    {
        ASStringNode*   pNode;
        UByte           pData[1];

        WideStringStorage(ASStringNode* pnode, UPInt dataSz) : pNode(pnode) 
        {
            pNode->AddRef();
            // NOTE: pData must be guaranteed to have enough space for (wchar_t * strLen+1)
            UTF8Util::DecodeStringSafe((wchar_t*)pData, dataSz, pNode->pData, pNode->Size);
        }
        ~WideStringStorage()                { pNode->Release(); }
        SF_INLINE const wchar_t*  ToWStr()  { return (const wchar_t*)pData; }

    private:
        // Copying is prohibited
        WideStringStorage(const WideStringStorage&);
        const WideStringStorage& operator = (const WideStringStorage&);
    };
    
    struct LevelInfo
    {
        // Level must be >= 0. -1 is used to indicate lack of a level
        // elsewhere, but that is not allowed here.
        int                     Level;
        Ptr<InteractiveObject>  pSprite;
    };

    // Obtains cached states. The mat is only accessed if CachedStatesFlag is not
    // set, which meas we are outside of Advance and Display.
    Log*                 GetCachedLog() const
    {
         // Do not modify CachedLogFlag; that is only for Advance/Display.
        if (!G_IsFlagSet<Flag_CachedLogFlag>(Flags))
            pCachedLog = GetLog();
        return pCachedLog;
    }
    
#ifdef GFX_ENABLE_KEYBOARD
    // Keyboard

    const KeyboardState*    GetKeyboardState(unsigned keyboardIndex) const 
    { 
        if (keyboardIndex < GFX_MAX_KEYBOARD_SUPPORTED)
            return &KeyboardStates[keyboardIndex]; 
        return NULL;
    }
    KeyboardState*          GetKeyboardState(unsigned keyboardIndex) 
    { 
        if (keyboardIndex < GFX_MAX_KEYBOARD_SUPPORTED)
            return &KeyboardStates[keyboardIndex]; 
        return NULL;
    }
    void SetKeyboardListener(KeyboardState::IListener*);
#endif

    // Return value class - allocated after global context.
    // Used because GASString has no global or NewOverrideBase.
    struct ReturnValueHolder : public NewOverrideBase<StatMV_ActionScript_Mem>
    {
        char*     CharBuffer;
        unsigned  CharBufferSize;
        ArrayCC<ASString, StatMV_ActionScript_Mem>   StringArray;
        unsigned  StringArrayPos;

        ReturnValueHolder(ASStringManager* pmgr)
            : CharBuffer(0), CharBufferSize(0),
            StringArray(pmgr->CreateConstString("")),
            StringArrayPos(0) { }
        ~ReturnValueHolder() { if (CharBuffer) SF_FREE(CharBuffer); }

        SF_INLINE char* PreAllocateBuffer(unsigned size)
        {
            size = (size + 4095)&(~(4095));
            if (CharBufferSize < size || (CharBufferSize > size && (CharBufferSize - size) > 4096))
            {
                if (CharBuffer)
                    CharBuffer = (char*)SF_REALLOC(CharBuffer, size, StatMV_ActionScript_Mem);
                else
                    CharBuffer = (char*)SF_ALLOC(size, StatMV_ActionScript_Mem);
                CharBufferSize = size;
            }
            return CharBuffer;
        }
        SF_INLINE void ResetPos() { StringArrayPos = 0; }
        SF_INLINE void ResizeStringArray(unsigned n)
        {
            StringArray.Resize(Alg::Max(1u,n));
        }
    };

    class DragState
    {
    public:
        InteractiveObject*  pCharacter;
        InteractiveObject*  pTopmostEntity;
        bool                LockCenter;
        bool                Bound;
        // Bound coordinates
        PointF              BoundLT;
        PointF              BoundRB;
        // The difference between character origin and mouse location
        // at the time of dragStart, used and computed if LockCenter == 0.
        PointF              CenterDelta;
        unsigned            MouseIndex;

        DragState()
            : pCharacter(0), pTopmostEntity(0), LockCenter(0), Bound(0), 
              BoundLT(0.0), BoundRB(0.0), CenterDelta(0.0),
              MouseIndex(~0u)
            { }

        // Initializes lockCenter and mouse centering delta
        // based on the character.
        void InitCenterDelta(bool lockCenter, unsigned mouseIndex = 0);
    };

    // Sticky variable hash link node.
    struct StickyVarNode : public NewOverrideBase<StatMV_ActionScript_Mem>
    {
        ASString            Name;
        //void*             Value;
        StickyVarNode*      pNext;
        bool                Permanent;

        StickyVarNode(const ASString& name, bool permanent)
            : Name(name), pNext(0), Permanent(permanent) { }
        StickyVarNode(const StickyVarNode &node)
            : Name(node.Name), pNext(node.pNext), Permanent(node.Permanent) { }
        virtual ~StickyVarNode() {}

        virtual void Assign(const StickyVarNode &node)
        {
            //pNext = node.pNext; // do not assign pNext, only content
            Name = node.Name; Permanent = node.Permanent;
        }
    private:
        // use Assign instead!
        const StickyVarNode& operator = (const StickyVarNode &)
        {  return *this; }
    };

    enum FlagsType
    {
        // Set once the viewport has been specified explicitly.
        Flag_ViewportSet                    = 0x0001,

        // States are cached then this flag is set.
        Flag_CachedLogFlag                  = 0x0002,

        // Verbosity - assigned from ActionControl.
        Flag_VerboseAction                  = 0x0004,
        Flag_LogRootFilenames               = 0x0008,
        Flag_LogChildFilenames              = 0x0010,
        Flag_LogLongFilenames               = 0x0020,
        Flag_SuppressActionErrors           = 0x0040,    

        Flag_NeedMouseUpdate                = 0x0080,

        Flag_LevelClipsChanged              = 0x0100,
        // Set if Advance has not been called yet - generates warning on Display.
        Flag_AdvanceCalled                  = 0x0200,
        Flag_HasChanges                     = 0x0400,
        Flag_NoInvisibleAdvanceFlag         = 0x0800,
        Flag_SetCursorTypeFuncOverloaded    = 0x1000,

        // Flags for event handlers
        Flag_ContinueAnimation              = 0x2000,
        Flag_StageAutoOrients               = 0x4000,
        Flag_OnEventLoadProgressCalled      = 0x8000,

        Flag_DisableFocusAutoRelByClick     = 0x010000,

        Flag_BackgroundSetByTag             = 0x020000,
        Flag_MovieIsFocused                 = 0x040000,
        Flag_OptimizedAdvanceListInvalid    = 0x080000,

        Flag_Paused                         = 0x100000,
        Flag_ExitRequested                  = 0x200000,
        
        // CAN'T USE 0x400000, it is a part of DisableFocusAutoRelease!!!

        // Focus-related AS extension properties
        // Disables focus release when mouse moves
        Shift_DisableFocusAutoRelease       = 22, // 0xC00000

        // Enables moving focus by arrow keys even if _focusRect is set to false
        Shift_AlwaysEnableFocusArrowKeys    = 24,

        // Enables firing onPress/onRelease even if _focusRect is set to false
        Shift_AlwaysEnableKeyboardPress     = 26,

        // Disables firing onRollOver/Out if focus is changed by arrow keys
        Shift_DisableFocusRolloverEvent     = 28,

        // Disables default focus handling by arrow and tab keys
        Shift_DisableFocusKeys              = 30
    };
    enum Flags2Type
    {
        Flag2_AcceptAnimMovesWith3D          = 0x0001,
        Flag2_RegisteredFontsChanged         = 0x0002,
        Flag2_Restarting                     = 0x0004,
        Flag2_OptAdvListMarker               = 0x0008,
        Flag2_BidirectionalTextEnabled       = 0x0010
    };

    struct IndirectTransPair
    {
        Ptr<TreeNode>            TransformParent;
        Ptr<DisplayObjectBase>   Obj;
        Ptr<DisplayObjContainer> OriginalParent;
        mutable int              OrigParentDepth;

        IndirectTransPair():OrigParentDepth(-1) {}

        void Clear() { TransformParent = 0; Obj = 0; OriginalParent = 0; OrigParentDepth = 0; }
    };

#if defined(SF_OS_WIN32) && defined(GFX_ENABLE_BUILTIN_KOREAN_IME) && !defined(SF_NO_IME_SUPPORT)
    GFxIMEImm32Dll      Imm32Dll;
#endif

    // multitouch
    enum MultitouchInputMode
    {
        MTI_None        = MultitouchInterface::MTI_None,
        MTI_TouchPoint  = MultitouchInterface::MTI_TouchPoint,
        MTI_Gesture     = MultitouchInterface::MTI_Gesture,
        MTI_Mixed       = MultitouchInterface::MTI_Mixed
    };
    enum GestureMask
    {
        MTG_None        = MultitouchInterface::MTG_None,
        MTG_Pan         = MultitouchInterface::MTG_Pan,
        MTG_Zoom        = MultitouchInterface::MTG_Zoom,
        MTG_Rotate      = MultitouchInterface::MTG_Rotate,
        MTG_Swipe       = MultitouchInterface::MTG_Swipe
    };


    // *** Constructor / Destructor

    MovieImpl(MemoryHeap* pheap);
    ~MovieImpl();

    void SetASMovieRoot(ASMovieRootBase* pasmr);
    bool Init(MovieDefImpl* pmovieDef)
    {
        return pASMovieRoot->Init(pmovieDef);
    }

    ASStringManager*        GetStringManager() { return pASMovieRoot->GetStringManager(); }
    
    // This method returns the top most container for all stage objects. For AS2 this will be _root, for 
    // AS3 - stage.
    InteractiveObject*      GetMainContainer() const { return pMainMovie; }
    void                    SetMainContainer(InteractiveObject* p) { pMainMovie = p; }

    // This method returns the root display object container for the specified object. 
    // The "dobj" parameter might be NULL; in this case, for AS2 this method will return _root 
    // (same as GetMainContainer), for AS3 it will return "root".
    // If "dobj" is not NULL and it belongs to SWF loaded by loadMovie then the result is the
    // parent movie on which "loadMovie" was called. 
    DisplayObjContainer*    GetRootMovie(DisplayObject* dobj = NULL) const { return pASMovieRoot->GetRootMovie(dobj); }

    void                    RegisterAuxASClasses()  { pASMovieRoot->RegisterAuxASClasses(); }
    bool                    SetLevelMovie(int level, DisplayObjContainer *psprite);

    // Returns a movie at level, or null if no such movie exists.
    bool                    ReleaseLevelMovie(int level);

    void ClearDisplayList() { pASMovieRoot->ClearDisplayList(); }
    void ClearPlayList();

    // Non-virtual version of GetHeap(), used for efficiency.
    MemoryHeap*             GetMovieHeap() const { return pHeap; }

     
    // Finds a character given a global path. Path must 
    // include a _levelN entry as first component.
    SF_INLINE InteractiveObject*  FindTarget(const ASString& path) const;

    
    // Dragging support.
    void                SetDragState(const DragState& st);
    void                GetDragState(unsigned mouseIndex, DragState* st);
    void                StopAllDrags();
    void                StopDrag(unsigned mouseIndex);
    bool                IsDragging(unsigned mi) const;
    const InteractiveObject* GetDraggingCharacter(unsigned mouseIndex = 0) const
    {
        SF_ASSERT(mouseIndex < GFX_MAX_MICE_SUPPORTED);
        return CurrentDragStates[mouseIndex].pCharacter; 
    }
    InteractiveObject* GetDraggingCharacter(unsigned mouseIndex = 0)
    {
        SF_ASSERT(mouseIndex < GFX_MAX_MICE_SUPPORTED);
        return CurrentDragStates[mouseIndex].pCharacter; 
    }
    void                StopDragCharacter(const InteractiveObject* ch);
    bool                IsDraggingMouseIndex(unsigned mouseIndex) const;
    void                SetDragStateTopmostEntity(unsigned mouseIndex, InteractiveObject* topmostEntity);

	// Internal use by the AS3 VM to force Shapes into TreeShapes
	void				UpdateAllRenderNodes();

    // Internal use in characters, etc.
    // Use this to retrieve the last state of the mouse.
    virtual void        GetMouseState(unsigned mouseIndex, float* x, float* y, unsigned* buttons);

    const MouseState* GetMouseState(unsigned mouseIndex) const
    {
        if (mouseIndex >= GFX_MAX_MICE_SUPPORTED)
            return NULL;
        return &mMouseState[mouseIndex];
    }
    MouseState* GetMouseState(unsigned mouseIndex)
    {
        if (mouseIndex >= GFX_MAX_MICE_SUPPORTED)
            return NULL;
        return &mMouseState[mouseIndex];
    }
    void                SetMouseCursorCount(unsigned n)
    {
        MouseCursorCount = (n <= GFX_MAX_MICE_SUPPORTED) ? n : GFX_MAX_MICE_SUPPORTED;
    }
    unsigned            GetMouseCursorCount() const
    {
        return MouseCursorCount;
    }
    virtual void        SetControllerCount(unsigned n)
    {
        ControllerCount = (n <= GFX_MAX_KEYBOARD_SUPPORTED) ? n : GFX_MAX_KEYBOARD_SUPPORTED;
    }
    virtual unsigned    GetControllerCount() const
    {
        return ControllerCount;
    }
    bool                IsMouseSupportEnabled() const { return MouseCursorCount > 0; }
    
    // Return the size of a logical GFxMovieSub pixel as
    // displayed on-screen, with the current device
    // coordinates.
    float               GetPixelScale() const               { return PixelScale; }

    // Returns time elapsed since the first Advance, in seconds (with fraction part)
    Double              GetTimeElapsed() const              { return Double(TimeElapsed)/1000000.; }
    // Returns time elapsed since the first Advance, in milliseconds
    Double              GetTimeElapsedMs() const            { return Double(TimeElapsed)/1000.; }
    // Returns time elapsed since the first Advance, in microseconds
    UInt64              GetTimeElapsedMks() const           { return TimeElapsed; }

    float               GetFrameTime() const                { return FrameTime; }
    UInt64              GetStartTickMs() const              { return StartTickMs; }
    UInt64              GetASTimerMs() const;

    // Validation
    virtual bool        IsValid();
    
    // Create new instance names for unnamed objects.
    ASString           CreateNewInstanceName();

    // *** Load/Unload movie support
    
    // Head of load queue.
    LoadQueueEntry*     pLoadQueueHead;
    UInt32              LastLoadQueueEntryCnt;

    // Adds load queue entry and takes ownership of it.
    void                AddLoadQueueEntry(LoadQueueEntry *pentry);
    void                AddLoadQueueEntryMT(LoadQueueEntryMT* pentryMT);

    // Processes the load queue handling load/unload instructions.  
    void                ProcessLoadQueue();


    // *** Helpers for loading images.

    typedef Loader::FileFormatType FileFormatType;
        
    // Create a loaded image MovieDef based on image resource.
    // If load states are specified, they are used for bind states. Otherwise,
    // new states are created based on pStateBag and our loader.
    MovieDefImpl*       CreateImageMovieDef(ImageResource *pimageResource, bool bilinear,
                                            const char *purl, LoadStates *pls = 0);

    Ptr<ImageResource>  GetImageResourceByLinkageId(MovieDefImpl* md, const char* linkageId);

    // Fills in a file system path relative to _level0. The path
    // will contain a trailing '/' if not empty, so that relative
    // paths can be concatenated directly to it.
    // Returns 1 if the path is non-empty.
    bool                GetMainMoviePath(String *ppath) const;
    

    // *** Action List management

    // *** Movie implementation

    virtual void        SetViewport(const Viewport& viewDesc);
    virtual void        GetViewport(Viewport *pviewDesc) const;
    virtual void        SetViewScaleMode(ScaleModeType);
    virtual ScaleModeType   GetViewScaleMode() const                        { return ViewScaleMode; }
    virtual void        SetViewAlignment(AlignType);
    virtual AlignType   GetViewAlignment() const                            { return ViewAlignment; }
    virtual RectF       GetVisibleFrameRect() const                         { return TwipsToPixels(VisibleFrameRect); }
    const RectF&        GetVisibleFrameRectInTwips() const                  { return VisibleFrameRect; }

    virtual RectF       GetSafeRect() const                                 { return SafeRect; }
    virtual void        SetSafeRect(const RectF& rect)                      { SafeRect = rect; }

    virtual void        SetEdgeAAMode(Render::EdgeAAMode edgeAA) { pRenderRoot->SetEdgeAAMode(edgeAA); }
    virtual Render::EdgeAAMode  GetEdgeAAMode() const            { return pRenderRoot->GetEdgeAAMode(); }

    void                UpdateViewport();

    virtual void        SetVerboseAction(bool verboseAction)
    { 
#if !defined(GFX_AS2_VERBOSE) && !defined(GFX_AS3_VERBOSE)
        SF_DEBUG_WARNING(1, "VerboseAction is disabled by the GFX_AS2/AS3_VERBOSE macros in GFxConfig.h\n");
#endif
        G_SetFlag<Flag_VerboseAction>(Flags, verboseAction); 
    }
    // Turn off/on log for ActionScript errors..
    virtual void        SetActionErrorsSuppress(bool suppressActionErrors)  { G_SetFlag<Flag_SuppressActionErrors>(Flags, suppressActionErrors); }
    // Background color.
    virtual void        SetBackgroundColor(const Color color);
    virtual void        SetBackgroundAlpha(float alpha);
    virtual float       GetBackgroundAlpha() const                  { return BackgroundColor.GetAlpha() / 255.0f; }

    bool                IsBackgroundSetByTag() const                { return G_IsFlagSet<Flag_BackgroundSetByTag>(Flags); }
    void                SetBackgroundColorByTag(const Color color) 
    { 
        SetBackgroundColor(color); 
        G_SetFlag<Flag_BackgroundSetByTag>(Flags, true); 
    }

    // Actual execution and timeline control.
    virtual float       Advance(float deltaT, unsigned frameCatchUpCount, bool capture = true);

    virtual void        Capture(bool onChangeOnly = true);

    virtual const MovieDisplayHandle& GetDisplayHandle() const { return hDisplayRoot; }

    virtual void        SetCaptureThread(ThreadId captureThreadId) { RenderContext.SetCaptureThreadId(captureThreadId); }

    // ShutdownRendering support.
    virtual void    ShutdownRendering(bool wait) { RenderContext.Shutdown(wait); }
    virtual bool    IsShutdownRenderingComplete() const 
	{ 
		return (RenderContext.IsShutdownComplete() && (!DIContext || DIContext->IsShutdownComplete())); 
	}


    // An internal method that advances frames for movieroot's sprites
    void                AdvanceFrame(bool nextFrame, float framePos);
    void                InvalidateOptAdvanceList() { G_SetFlag<Flag_OptimizedAdvanceListInvalid>(Flags, true); }
    bool                IsOptAdvanceListInvalid() const { return G_IsFlagSet<Flag_OptimizedAdvanceListInvalid>(Flags); }
    bool                GetOptAdvListMarker() const { return (Flags2 & Flag2_OptAdvListMarker) != 0; }
#if defined(GFX_CHECK_PLAYLIST_DEBUG)
    // a safety counter to avoid hang up
    int                 _PLCheckCnt;
    
    // set this value to 100 if need to check playlist consistency more often per frame
    // setting it to 0 will still execute the consistency check once per frame.
    enum { PLCheckCnt_Limit_Per_Frame = 0 }; 

    void                CheckOptPlaylistConsistency(InteractiveObject* mustBeInList = 0,
                                                    InteractiveObject* mustBeOutOfList = 0,
                                                    bool enforce = false);
    void                CheckPlaylistConsistency(bool enforce = false);
#else
    void                CheckOptPlaylistConsistency(InteractiveObject* = 0,
                                                    InteractiveObject* = 0,
                                                    bool = false) {}
    void                CheckPlaylistConsistency(bool = false) {}
#endif

    void                ProcessUnloadQueue();
    // Releases list of unloaded characters
    void                ReleaseUnloadList();

    // Events.
    virtual unsigned    HandleEvent(const Event &event);
    virtual void        NotifyMouseState(float x, float y, unsigned buttons, unsigned mouseIndex = 0);
    virtual bool        HitTest(float x, float y, HitTestType testCond = HitTest_Shapes, unsigned controllerIdx = 0);

    /*
    virtual void        SetUserEventHandler(UserEventCallback phandler, void* puserData) 
    { 
        pUserEventHandler = phandler; 
        pUserEventHandlerData = puserData;
    }

    // FSCommand
    virtual void        SetFSCommandCallback (FSCommandCallback phandler)   { pFSCommandCallback = phandler; }
    */

    virtual void*       GetUserData() const                                 { return UserData; }
    virtual void        SetUserData(void* ud)                               { UserData = ud;  }

    // returns timerId to use with ClearIntervalTimer
    int                 AddIntervalTimer(ASIntervalTimerIntf *timer);  
    void                ClearIntervalTimer(int timerId);      
    void                ShutdownTimers();
    void                ShutdownTimersForMovieDef(MovieDefImpl* defimpl);
    bool                IsShutdowning() const { return !pRenderRoot.GetPtr() || G_IsFlagSet<Flag2_Restarting>(Flags2); }

#ifdef GFX_ENABLE_VIDEO
    void                AddVideoProvider(Video::VideoProvider*);
    void                RemoveVideoProvider(Video::VideoProvider*);
#endif
    
    // *** Movie implementation

    // Many of these methods delegate to pMovie methods of the same name; however,
    // they are hidden into the .cpp file because GFxSprite is not yet defined.
    
    virtual MovieDef*    GetMovieDef() const;
    
    SF_INLINE LoaderImpl*   GetLoaderImpl() const               { return pMainMovieDef->pLoaderImpl; }
    SF_INLINE MovieDefImpl* GetMovieDefImpl() const             { return pMainMovieDef; }     
    SF_INLINE void          SetMovieDefImpl(MovieDefImpl* p) { pMainMovieDef = p; }
    
    virtual unsigned    GetCurrentFrame() const;
    virtual bool        HasLooped() const;  
    virtual void        Restart(bool advance0 = true);  
    virtual void        GotoFrame(unsigned targetFrameNumber);
    virtual bool        GotoLabeledFrame(const char* label, int offset = 0);

    virtual void        SetPause(bool pause);
    virtual bool        IsPaused() const { return G_IsFlagSet<Flag_Paused>(Flags); }

    virtual void        SetPlayState(PlayState s);
    virtual PlayState   GetPlayState() const;
    virtual void        SetVisible(bool visible);
    virtual bool        GetVisible() const;
    
    void                AddStickyVariableNode(const ASString& path, StickyVarNode* pnode);
//    SF_INLINE void        AddStickyVariable(const GASString& fullPath, const Value &val, SetVarType setType);
//    SF_INLINE void        ResolveStickyVariables(InteractiveObject *pcharacter);
    void                ClearStickyVariables();

    InteractiveObject*  GetTopMostEntity(const PointF& mousePos, unsigned controllerIdx, 
        bool testAll, const InteractiveObject* ignoreMC = NULL);  

    // Profiling
    Ptr<AmpStats> AdvanceStats;
    SF_AMP_CODE(AMP::ViewStats& GetAdvanceStats() const { return *static_cast<AMP::ViewStats*>(AdvanceStats.GetPtr()); })

    // Obtains statistics for the movie view.
    virtual void        GetStats(StatBag* pbag, bool reset);


    virtual MemoryHeap* GetHeap() const { return pHeap; }

    // Forces to run garbage collection, if it is enabled. Does nothing otherwise.
    // 'gcFlags' parameter allows to control the speed of GC. GCF_Quick is fastest
    // but less memory is recovered, whereas GCF_Full is slowest but all possible 
    // memory is recovered.
    virtual void        ForceCollectGarbage(unsigned gcFlags = GCF_Full);

    // Additional GC control functions. 
    // SuspendGC suspends/resumes garbage collection. It is counted operation, meaning
    // if it was suspended N-times then it should be re-enabled N-times to restore normal operation.
    virtual void        SuspendGC(bool suspend);

    // Schedule garbage collection. Unlike ForceCollectGarbage it doesn't execute collection immediately;
    // instead, it will be executed when next Advance is called.
    // 'gcFlags' parameter allows to control the speed of GC. GCF_Quick is fastest
    // but less memory is recovered, whereas GCF_Full is slowest but all possible 
    // memory is recovered.
    virtual void        ScheduleGC(unsigned gcFlags = GCF_Full);

    // Prints out a report about objects and links between them.
    virtual void        PrintObjectsReport(UInt32 flags = 0, 
                                           Log* log = NULL,  
                                           const char* swfName = NULL,
                                           Ptr<AmpMovieObjectDesc>* root = NULL,
                                           MemoryHeap* heap = NULL);

    // ***** GFxStateBag implementation
    
    virtual StateBag* GetStateBagImpl() const   { return pStateBag.GetPtr(); }

    // Mark/unmark the movie as one that has "focus". This is user responsibility to set or clear
    // the movie focus. This is important for stuff like IME functions correctly.
    void                OnMovieFocus(bool set);
    virtual bool        IsMovieFocused() const { return G_IsFlagSet<Flag_MovieIsFocused>(Flags); }
    // Returns and optionally resets the "dirty flag" that indicates 
    // whether anything was changed on the stage (and need to be 
    // re-rendered) or not.
    virtual bool        GetDirtyFlag(bool doReset = true);
    //void                SetDirtyFlag() { G_SetFlag<Flag_DirtyFlag>(Flags, true); }

    // Forces render tree to be updated in order to apply all possible changes
    // related to images and/or textures.
    virtual void        ForceUpdateImages();

    virtual void        MakeAreaVisible(const Render::RectF& screenRect, 
                                        const Render::RectF& box,
                                        UInt32 flags = 0);
    // Resets ViewportMatrix and sets it on TreeRoot.
    virtual void        RestoreViewport();

    // Unlike RestoreViewport, this method just recalc the ViewportMatrix but does not
    // set it on treeroot.
    void                ResetViewportMatrix();

    void                SetNoInvisibleAdvanceFlag(bool f) { G_SetFlag<Flag_NoInvisibleAdvanceFlag>(Flags, f); }
    void                ClearNoInvisibleAdvanceFlag()     { SetNoInvisibleAdvanceFlag(false); }
    bool                IsNoInvisibleAdvanceFlagSet() const { return G_IsFlagSet<Flag_NoInvisibleAdvanceFlag>(Flags); }

    void                SetContinueAnimationFlag(bool f) { G_SetFlag<Flag_ContinueAnimation>(Flags, f); }
    void                ClearContinueAnimationFlag()     { SetContinueAnimationFlag(false); }
    bool                IsContinueAnimationFlagSet() const { return G_IsFlagSet<Flag_ContinueAnimation>(Flags); }

    void                SetBidirectionalTextEnabledFlag(bool f) { G_SetFlag<Flag2_BidirectionalTextEnabled>(Flags2, f); }
    void                ClearBidirectionalTextEnabledFlag()     { SetBidirectionalTextEnabledFlag(false); }
    bool                IsBidirectionalTextEnabled() const { return G_IsFlagSet<Flag2_BidirectionalTextEnabled>(Flags2); }

    bool                IsVerboseAction() const        { return G_IsFlagSet<Flag_VerboseAction>(Flags); }
    bool                IsSuppressActionErrors() const { return G_IsFlagSet<Flag_SuppressActionErrors>(Flags); }
    bool                IsLogRootFilenames() const  { return G_IsFlagSet<Flag_LogRootFilenames>(Flags); }
    bool                IsLogChildFilenames() const { return G_IsFlagSet<Flag_LogChildFilenames>(Flags); }
    bool                IsLogLongFilenames() const  { return G_IsFlagSet<Flag_LogLongFilenames>(Flags); }
    bool                IsAlwaysEnableKeyboardPress() const; 
    bool                IsAlwaysEnableKeyboardPressSet() const;
    void                SetAlwaysEnableKeyboardPress(bool f);
    bool                IsDisableFocusRolloverEvent() const;
    bool                IsDisableFocusRolloverEventSet() const;
    void                SetDisableFocusRolloverEvent(bool f);
    bool                IsDisableFocusAutoRelease() const;
    bool                IsDisableFocusAutoReleaseSet() const;
    void                SetDisableFocusAutoRelease(bool f);
    bool                IsDisableFocusAutoReleaseByMouseClick() const;
    void                SetDisableFocusAutoReleaseByMouseClick(bool f);
    bool                IsAlwaysEnableFocusArrowKeys() const;
    bool                IsAlwaysEnableFocusArrowKeysSet() const;
    void                SetAlwaysEnableFocusArrowKeys(bool f);
    bool                IsDisableFocusKeys() const;
    bool                IsDisableFocusKeysSet() const;
    void                SetDisableFocusKeys(bool f);

#if defined(SF_BUILD_CONSUMER)
    bool                IsValidEval() const;
#endif

    // Focus related functionality
    // Focus-related methods
    void                InitFocusKeyInfo(ProcessFocusKeyInfo* pfocusInfo, 
                                         const InputEventsQueue::QueueEntry::KeyEntry& keyEntry, 
                                         bool inclFocusEnabled,
                                         FocusGroupDescr* pfocusGroup = NULL);
    // Process keyboard input for focus
    void                ProcessFocusKey(Event::EventType event,
                                        const InputEventsQueue::QueueEntry::KeyEntry& keyEntry,
                                        ProcessFocusKeyInfo* pfocusInfo);
    void                FinalizeProcessFocusKey(ProcessFocusKeyInfo* pfocusInfo);

    // Internal methods that processes keyboard and mouse input
    void                ProcessInput();
    void                ProcessKeyboard(const InputEventsQueue::QueueEntry* qe, 
                                        ProcessFocusKeyInfo* focusKeyInfo);
    void                ProcessMouse(const InputEventsQueue::QueueEntry* qe, 
                                     UInt32* miceProceededMask, bool avm2);
#ifdef GFX_ENABLE_ANALOG_GAMEPAD
    void                ProcessGamePadAnalog(const InputEventsQueue::QueueEntry* qe);
#endif
#ifdef GFX_MULTITOUCH_SUPPORT_ENABLE
    // Returns a mouse index associated with the touchID. -1, if not found.
    int                 FindMouseStateIndexByTouchID(unsigned touchID);
    void                ProcessTouch(const InputEventsQueue::QueueEntry* qe, 
                                     ProcessFocusKeyInfo* focusKeyInfo);
    void                ProcessGesture(const InputEventsQueue::QueueEntry* qe);
#endif
    void                ProcessStatus(const InputEventsQueue::QueueEntry* qe);

    void                ProcessAccelerometer(const InputEventsQueue::QueueEntry* qe);
    void                ProcessGeolocation(const InputEventsQueue::QueueEntry* qe);

#ifdef SF_BUILD_LOGO
    Ptr<ImageInfo>    pDummyImage;
#endif

protected:
    void                FillTabableArray(const ProcessFocusKeyInfo* pfocusInfo);
    FocusGroupDescr&    GetFocusGroup(unsigned controllerIdx) 
    { 
        SF_ASSERT(controllerIdx < GFX_MAX_CONTROLLERS_SUPPORTED &&
            FocusGroupIndexes[controllerIdx] < GFX_MAX_CONTROLLERS_SUPPORTED);
        return FocusGroups[FocusGroupIndexes[controllerIdx]]; 
    }
    const FocusGroupDescr&  GetFocusGroup(unsigned controllerIdx) const 
    {    
        SF_ASSERT(controllerIdx < GFX_MAX_CONTROLLERS_SUPPORTED &&
            FocusGroupIndexes[controllerIdx] < GFX_MAX_CONTROLLERS_SUPPORTED);
        return FocusGroups[FocusGroupIndexes[controllerIdx]]; 
    }

    void                ResetMouseState();
    void                ResetKeyboardState();

    // update render nodes for focus rectangles
    void                UpdateFocusRectRenderNodes();

public:
    unsigned            GetFocusGroupIndex(unsigned controllerIdx) const
    {
        return FocusGroupIndexes[controllerIdx]; 
    }
    // returns a bit-mask where each bit represents a physical controller, 
    // associated with the specified focus group.
    UInt32              GetControllerMaskByFocusGroup(unsigned focusGroupIndex);
    
    // Hides yellow focus rectangle. This may happen if mouse moved.
    void                HideFocusRect(unsigned controllerIdx);
    // Returns the character currently with focus
    Ptr<InteractiveObject> GetFocusedCharacter(unsigned controllerIdx)
    { 
        return Ptr<InteractiveObject>(GetFocusGroup(controllerIdx).LastFocused); 
    }
    // Checks, is the specified item focused by the specified controller or not?
    bool                IsFocused(const InteractiveObject* ch, unsigned controllerIdx) const 
    { 
        return GetFocusGroup(controllerIdx).IsFocused(ch);
    }
    // Checks, if the 'ch' focused by ANY controller.
    bool                IsFocused(const InteractiveObject* ch) const;

    // Checks, is the specified item focused or not for keyboard input
    bool                IsKeyboardFocused(const InteractiveObject* ch, unsigned controllerIdx) const 
    { 
        return (IsFocused(ch, controllerIdx) && IsFocusRectShown(controllerIdx)); 
    }
    // Transfers focus to specified item. For movie clips and buttons the keyboard focus will not be set
    // to specified character, as well as focus rect will not be drawn.
    // return false, if focus change was prevented by NotifyOnFocusChange
    bool                SetFocusTo(InteractiveObject*, unsigned controllerIdx, FocusMovedType fmt);

    // Resets input focus the the specified controller. If a textField has a focus it will lose it.
    virtual void        ResetInputFocus(unsigned controllerIdx);

    // sets LastFocused to NULL
    void                ResetFocus(unsigned controllerIdx) 
    { 
        GetFocusGroup(controllerIdx).LastFocused = NULL; 
    }
    void                ResetFocusForChar(InteractiveObject* ch);
    
    // Queue up setting of focus. ptopMostCh is might be necessary for stuff like IME to 
    // determine its state; might be NULL, if not available.
    // return false, if focus change was prevented by NotifyOnFocusChange
    bool                QueueSetFocusTo
        (InteractiveObject* ch, InteractiveObject* ptopMostCh, unsigned controllerIdx, FocusMovedType fmt, ProcessFocusKeyInfo* = NULL);
    // Instantly transfers focus to specified item. For movie clips and buttons the keyboard focus will not be set
    // Instantly invokes pOldFocus->OnKillFocus, pNewFocus->OnSetFocus and Selection.bradcastMessage("onSetFocus").
    void                TransferFocus(InteractiveObject* pNewFocus, unsigned controllerIdx, FocusMovedType fmt);
    // Transfers focus to specified item. The keyboard focus transfered as well, as well as focus rect 
    // will be drawn (unless it is disabled).
    void                SetKeyboardFocusTo(InteractiveObject*, unsigned controllerIdx, FocusMovedType fmt = GFx_FocusMovedByKeyboard);
    // Returns true, if yellow focus rect CAN be shown at the time of call. This method will
    // return true, even if _focusrect = false and it is a time to show the rectangle.
    inline bool         IsFocusRectShown(unsigned controllerIdx) const
    { 
        return GetFocusGroup(controllerIdx).FocusRectShown; 
    }
    void                ResetTabableArrays();
    void                ResetFocusStates();

    void                ActivateFocusCapture(unsigned controllerIdx);
    // Sets modal movieclip. That means focus keys (TAB, arrow keys) will 
    // move focus only inside of this movieclip. To reset the modal clip set
    // to NULL.
    void                SetModalClip(Sprite* pmovie, unsigned controllerIdx);
    Sprite*             GetModalClip(unsigned controllerIdx);

    // associate a focus group with a controller.
    virtual bool        SetControllerFocusGroup(unsigned controllerIdx, unsigned logCtrlIdx);

    // returns focus group associated with a controller
    virtual unsigned    GetControllerFocusGroup(unsigned controllerIdx) const;

    unsigned            GetFocusGroupCount() const { return FocusGroupsCnt; }

    void                AddTopmostLevelCharacter(InteractiveObject*);
    void                RemoveTopmostLevelCharacter(DisplayObjectBase*);

    // Sets style of candidate list. Invokes OnCandidateListStyleChanged callback.
    void                SetIMECandidateListStyle(const IMECandidateListStyle& st);
    // Gets style of candidate list
    void                GetIMECandidateListStyle(IMECandidateListStyle* pst) const;

#if defined(SF_OS_WIN32) && defined(GFX_ENABLE_BUILTIN_KOREAN_IME) && !defined(SF_NO_IME_SUPPORT)
    // handle korean IME (core part)
    unsigned            HandleKoreanIME(const IMEEvent& imeEvent);
#endif // SF_NO_BUILTIN_KOREAN_IME

    Render::Text::Allocator*    GetTextAllocator();

    // Translates the point or rectangle in Flash coordinates to screen 
    // (window) coordinates. These methods takes into account the world matrix
    // of root, the viewport matrix and the user matrix from the renderer. 
    // Source coordinates should be in root coordinate space, in pixels.
    virtual PointF      TranslateToScreen(const PointF& p, Matrix2F* puserMatrix = 0);
    virtual RectF       TranslateToScreen(const RectF& p, Matrix2F* puserMatrix = 0);

    // Translates the point in the character's coordinate space to the point on screen (window).
    // pathToCharacter - path to a movieclip, textfield or button, i.e. "_root.hud.mc";
    // pt is in pixels, in coordinate space of the character, specified by the pathToCharacter
    // returning value is in pixels of screen.
    virtual bool        TranslateLocalToScreen(const char* pathToCharacter, 
                                               const PointF& pt, 
                                               PointF* presPt, 
                                               Matrix2F* userMatrix = 0);
    // Changes the shape of the mouse cursor. If Mouse.setCursorType is overriden by ActionScript
    // when it will be invoked. This function should be used instead of GASMouseCtorFunc::SetCursorType
    // to provide callback in user's ActionScript.
    void                ChangeMouseCursorType(unsigned mouseIdx, unsigned newCursorType);

    // finds first available font manager (by traversing through root nodes).
    // If pdefImpl is not NULL it will search for font manager associated with the particular
    // defImpl. 
    FontManager*        FindFontManager(MovieDefImpl* pdefImpl = NULL);

    // Read variables data to the pdata. Data is in format var1="value"&var2="value2"
    static bool         ReadTextData(String* pdata, File* pfile, int* pfileLen, bool urlEncoded);

    // Read binary data to the pdata.
    static bool         ReadBinaryData(ArrayPOD<UByte>* pdata, File* pfile, int* pfileLen);

    // Checks if exit was requested by ActionScript.
    virtual bool        IsExitRequested() const { return G_IsFlagSet<Flag_ExitRequested>(Flags); }
    void                RequestExit() { G_SetFlag<Flag_ExitRequested>(Flags, true); }

    // Checks if timeline animation is allowed to run when setting 3D properties, generally not for AS3
    virtual bool        AcceptAnimMovesWith3D() const { return G_IsFlagSet<Flag2_AcceptAnimMovesWith3D>(Flags2); }
    void                SetAcceptAnimMovesWith3D(bool b) { G_SetFlag<Flag2_AcceptAnimMovesWith3D>(Flags2, b); }

    // This function is similar to GFxMovieDefImpl::GetExportedResource but besides searching in local
    // it looks also in all imported moviedefs starting from level0 def.
    bool                FindExportedResource(MovieDefImpl* localDef, ResourceBindData *pdata, const String& symbol);

    Render::Context&    GetRenderContext() { return RenderContext; }
    Render::DrawableImageContext* GetDrawableImageContext();
    void                SetThreadCommandQueue(Render::ThreadCommandQueue* p) { pRTCommandQueue = p; }
    Render::TreeRoot*   GetRenderRoot()    { return pRenderRoot; }

    DrawingContext*     CreateDrawingContext();

    void                AddMovieDefToKillList(MovieDefImpl*);
    void                ProcessMovieDefToKillList();

#ifdef SF_ENABLE_ANE					
    // Extension context
    bool                Call(const char* extensionID, const char* contextID, const char* methodName, unsigned argc, const Value* const argv, Scaleform::GFx::Value* const result);
    const               char* GetExtensionDirectory(const char* extensionID);
    void                FinalizeExtensionContext(const char* extensionID, const char* contextID);
    void                InitializeExtensionContext(const char* extensionID, const char* contextID);
	GFx::Value*			GetActionScriptData(const char* extensionID, const char* contextID);
	void				SetActionScriptData(const char* extensionID, const char* contextID, GFx::Value* data);
#endif

    bool                DoesStageAutoOrients() const { return G_IsFlagSet<Flag_StageAutoOrients>(Flags); }
    virtual void        SetStageAutoOrients(bool v = true)
    {
        G_SetFlag<Flag_StageAutoOrients>(Flags, v); 
    }
    OrientationEvent::OrientationType GetDeviceOrientation() const { return DeviceOrientation; }
    void                SetDeviceOrientation(OrientationEvent::OrientationType o) { DeviceOrientation = o; }

    ReturnValueHolder*  GetRetValHolder();
private:
    void                CheckMouseCursorType(unsigned mouseIdx, InteractiveObject* ptopMouseCharacter);
    // iterates through all drawing contexts and re-create renderer treenodes, if necessary.
    void                UpdateAllDrawingContexts();
    // clears the list of drawing context with zeroing pNext/pPrev in each node.
    void                ClearDrawingContextList();
    // update 3D view and projection after viewport changes
    void                UpdateViewAndPerspective();   

public:
    // multitouch related methods
    MultitouchInputMode GetMultitouchInputMode() const;
    void                SetMultitouchInputMode(MultitouchInputMode);
    unsigned            GetMaxTouchPoints() const;
    UInt32              GetSupportedGesturesMask() const; // ret combination of MTG_* flags

    // accelerometer
    bool                RegisterAccelerometer(int accelerometerId);
    bool                UnregisterAccelerometer(int accelerometerId);
    bool                IsAccelerometerMuted() const;
    bool                IsAccelerometerSupported() const;
    void                SetAccelerometerInterval(int accelerometerId, int interval);

    // geolocation
    bool                RegisterGeolocation(int geolocationId);
    bool                UnregisterGeolocation(int geolocationId);
    bool                IsGeolocationMuted() const;
    bool                IsGeolocationSupported() const;
    void                SetGeolocationInterval(int geolocationId, int interval);

    // 3D related methods
    static void MakeViewAndPersp3D(Matrix3F *matView, Matrix4F *matPersp, 
        const RectF &vfr,                           // ex. visible frame rect
        const PointF &projCenter,                   // ex. perspective center of projection, in TWIPS
        float fieldOfView=DEFAULT_FLASH_FOV,        // perspective field of view, in degrees, default flash FOV is 55
        float focalLength=0,                        // this is normally computed automatically, but the computed value can be overridden here 
        bool bInvertY=false);                       // if the Y axis should be inverted

    // 3D view matrix for movie
    virtual void                GetViewMatrix3D(Matrix3F *);
    virtual void                SetViewMatrix3D(const Matrix3F& m);

    // 3D projection matrix for movie
    virtual void                GetProjectionMatrix3D(Matrix4F *);
    virtual void                SetProjectionMatrix3D(const Matrix4F& m);

    // helper function for 3D screen to world
    void                        SetNormalizedScreenCoords(const Render::PointF &pt);

    // does the movie contain a 3D sprite
    bool                        Is3D() const;

    // saves indirect transform pair of nodes, where transformParent is the actual
    // node where the transform should be taken from; 'node' is the node that should
    // use indirect transfrom from 'transformParent' (it should be inserted in the 
    // render tree somewhere).
    void                AddIndirectTransformPair(DisplayObjContainer* origParent, 
        TreeNode* transformParent, DisplayObjectBase* obj);
    MovieImpl::IndirectTransPair RemoveIndirectTransformPair(DisplayObjectBase* obj);
    void                ClearIndirectTransformPairs();
    void                UpdateTransformParent(DisplayObjectBase* obj, DisplayObjectBase* transfParent);

    bool                AreRegisteredFontsChanged() const { return G_IsFlagSet<Flag2_RegisteredFontsChanged>(Flags2); }
    void                SetRegisteredFontsChanged(bool v = true)
    {
        G_SetFlag<Flag2_RegisteredFontsChanged>(Flags2, v); 
    }

    // Functionality for AS3 Font.registerFont.
    // Register a single font resource. Used by AS3 Font.registerFont.
    bool                RegisterFont(MovieDef* md, FontResource* fontRes);

    // find registered font by name and flags (see FontManager)
    FontResource*       FindRegisteredFont(const char* pfontName, 
                                           unsigned matchFontFlags,
                                           MovieDef** ppsrcMovieDef) const;

    // Unregister fonts from the pdefImpl, cleaning fontmanagers' cache.
    void                UnregisterFonts(MovieDefImpl* pdefImpl);

    // Adds all registered fonts into the hash 'fonts'.
    void                LoadRegisteredFonts(HashSet<Ptr<Render::Font> >& fonts);

public: // data

    // *** Complex object interface support

    // VM specific interface used to access complex objects (Object, etc.)
    Value::ObjectInterface*     pObjectInterface;

    // Heap used for all allocations within MovieImpl.
    MemoryHeap*                 pHeap;

    // Convenience pointer to _level0's Def (root in AS3). Keep this around even if _level0 unloads,
    // to avoid external crashes.
    Ptr<MovieDefImpl>           pMainMovieDef;
    // Pointer to sprite in level0 or null. Stored for convenient+efficient access.
    InteractiveObject*          pMainMovie;

    // Sorted array of currently loaded movie levels. Level 0 has
    // special significance, since it dictates viewport scale, etc. 
    // For AS3 this array always has only 1 element with Level = 0
    ArrayLH<LevelInfo, StatMV_Other_Mem>  MovieLevels;

    // A list of root MovieDefImpl objects used by all root sprites; these objects
    // can be loaded progressively in the background.
    List<MovieDefRootNode>      RootMovieDefNodes;

    Ptr<StateBagImpl>           pStateBag;

    // Renderer
    Ptr<Render::TreeRoot>       pRenderRoot;
    MovieDisplayHandle          hDisplayRoot;
    Ptr<Render::TreeContainer>  pTopMostRoot;

    GFx::Viewport               mViewport;
    float                       PixelScale;
    // View scale values, used to adjust input mouse coordinates
    // map viewport -> movie coordinates.
    float                       ViewScaleX, ViewScaleY;
    float                       ViewOffsetX, ViewOffsetY; // in stage pixels (not viewport's ones)
    ScaleModeType               ViewScaleMode;
    AlignType                   ViewAlignment;
    RectF                       VisibleFrameRect; // rect, in swf coords (twips), visible in curr viewport
    RectF                       SafeRect;         // storage for user-define safe rect
    Matrix                      ViewportMatrix; 
    Render::ScreenToWorld       ScreenToWorld;

#ifdef GFX_ENABLE_VIDEO
    HashSet<Ptr<Video::VideoProvider> > VideoProviders;
#endif

    // *** States cached in Advance()
    
    mutable Ptr<Log>            pCachedLog;

    // Handler pointer cached during advance.
    Ptr<UserEventHandler>       pUserEventHandler;
    Ptr<FSCommandHandler>       pFSCommandHandler;
    Ptr<ExternalInterface>      pExtIntfHandler;

#ifdef SF_ENABLE_ANE					
    Ptr<ExtensionContextInterface>  pExtContextHandler;
#endif

    Ptr<FontManagerStates>      pFontManagerStates;

#ifdef GFX_ENABLE_SOUND
    // We don't keep them here as Ptr<> to avoid dependances from SoundRenderer
    // in another parts of GFx (like IME). We hold these pointer here only for caching propose
    AudioBase*                  pAudio;
    Sound::SoundRenderer*       pSoundRenderer;
#endif

    // *** Special reference for XML object manager
    ExternalLibPtr*             pXMLObjectManager;

    // Amount of time that has elapsed since start of playback. Used for
    // reporting in action instruction and setTimeout/setInterval. In microseconds.
    UInt64                      TimeElapsed;
    // Time remainder from previous advance, updated every frame.
    float                       TimeRemainder;
    // Cached seconds per frame; i.e., 1.0f / FrameRate.
    float                       FrameTime;

    unsigned                    ForceFrameCatchUp;

    GFx::InputEventsQueue			InputEventsQueue;

	/*
#ifdef GFX_GESTURE_RECOGNIZE
	GFx::GestureRecognizer			GestureRecognizer;
#endif
	*/

    Color                       BackgroundColor;
    MouseState                  mMouseState[GFX_MAX_MICE_SUPPORTED];
    unsigned                    MouseCursorCount;
    unsigned                    ControllerCount;

    void*                       UserData;

#ifdef GFX_ENABLE_KEYBOARD
    // Keyboard
    KeyboardState               KeyboardStates[GFX_MAX_KEYBOARD_SUPPORTED];
#endif

protected:
    // use GetRetValHolder() method instead.
    ReturnValueHolder*          pRetValHolder;
public:
    // Instance name assignment counter.
    UInt32                      InstanceNameCount;

    DragState                   CurrentDragStates[GFX_MAX_MICE_SUPPORTED];

    // Sticky variable clip hash table.
    ASStringHash<StickyVarNode*> StickyVariables;


    ArrayLH<Ptr<InteractiveObject>, StatMV_Other_Mem> TopmostLevelCharacters;

    UInt64                                  StartTickMs, PauseTickMs;

    // interval timer stuff
    ArrayLH<Ptr<ASIntervalTimerIntf> ,StatMV_Other_Mem> IntervalTimers;
    int                                     LastIntervalTimerId;

    // Focus management stuff
    Ptr<Render::TreeContainer>              FocusRectContainerNode;
    FocusGroupDescr                         FocusGroups[GFX_MAX_CONTROLLERS_SUPPORTED];
    unsigned                                FocusGroupsCnt;
    // Map controller index to focus group
    UInt8                                   FocusGroupIndexes[GFX_MAX_CONTROLLERS_SUPPORTED];
    bool                                    FocusRectChanged;

    // The head of the playlist.
    // Playlist head, the first child for events or frame actions execution.
    // "Playable" children characters (ASCharacters) are linked by InteractiveObject::pPlayNext
    // and InteractiveObject::pPlayPrev pointers. If pPlayPrev == NULL, then this is the first
    // element in playlist, if pPlayNext == NULL - the last one.
    InteractiveObject*                      pPlayListHead;
    InteractiveObject*                      pPlayListOptHead;
    // List of all unloaded but not destroyed yet characters.
    InteractiveObject*                      pUnloadListHead;

    UInt32                                  Flags;
    UInt32                                  Flags2;

    IMECandidateListStyle*                  pIMECandidateListStyle; // stored candidate list style
#if defined(SF_OS_WIN32) && defined(GFX_ENABLE_BUILTIN_KOREAN_IME) && defined(GFX_ENABLE_IME)
    GFxIMEImm32Dll                          Imm32Dll;
#endif

    LoadQueueEntryMT*                       pLoadQueueMTHead;

    struct FontDesc
    {
        Ptr<MovieDef>       pMovieDef;
        Ptr<FontResource>   pFont;
    };
    // Keeps registered fonts (by AS3 Font.registerFont).
    ArrayLH<FontDesc>                       RegisteredFonts;

#if defined(SF_BUILD_CONSUMER)
    Ptr<Render::TreeText>       pWMLicenseText;
    Ptr<Render::TreeText>       pWMDateText;
    Ptr<Render::FontProvider>   pWMFontProvider;
    Ptr<Render::Font>           pWMFont;
#endif

private:
    // A list of all drawing context to be updated at the end of Advance
    List<DrawingContext>                    DrawingContextList;

    // an array of moviedefs to be released, if Render::Context allows. When movie unloads,
    // its moviedefimpl should be put in this array and the array will release the def later, 
    // in Advance.
    struct MDKillListEntry
    {
        UInt64              KillFrameId;
        Ptr<MovieDefImpl>   pMovieDef;
    };
    // it must be in global heap, since its reallocation may be called
    // from OnExceedLimit.
    Array<MDKillListEntry, StatMV_Other_Mem> MovieDefKillList;

    // This extra smart pointer is necessary to perform correct shutdown sequence in
    // a destructor and it should come BEFORE RenderContext (so, its dtor will be
    // called AFTER RenderContext's dtor). The Movie::pASMovieRoot should be nullified first.
    Ptr<ASMovieRootBase>                    pSavedASMovieRoot;
    // should be defined last to make sure its dtor is called first.
    Render::Context                         RenderContext; 
    char                                    PreviouslyCaptured;
    Ptr<Render::DrawableImageContext>       DIContext;
    Render::ThreadCommandQueue*             pRTCommandQueue;

    // Multitouch
    Ptr<MultitouchInterface>                MultitouchHAL;
    MultitouchInputMode                     MultitouchMode;
    Ptr<InteractiveObject>                  GestureTopMostChar;

    OrientationEvent::OrientationType       DeviceOrientation;

    ArrayLH<IndirectTransPair>              IndirectTransformPairs;
};

// ** Inline Implementation
SF_INLINE bool MovieImpl::IsAlwaysEnableKeyboardPress() const 
{
    return G_Is3WayFlagTrue<Shift_AlwaysEnableKeyboardPress>(Flags);
}
SF_INLINE bool MovieImpl::IsAlwaysEnableKeyboardPressSet() const 
{
    return G_Is3WayFlagSet<Shift_AlwaysEnableKeyboardPress>(Flags);
}
SF_INLINE void MovieImpl::SetAlwaysEnableKeyboardPress(bool f) 
{
    G_Set3WayFlag<Shift_AlwaysEnableKeyboardPress>(Flags, f);
}

SF_INLINE bool MovieImpl::IsDisableFocusRolloverEvent() const 
{
    return G_Is3WayFlagTrue<Shift_DisableFocusRolloverEvent>(Flags);
}
SF_INLINE bool MovieImpl::IsDisableFocusRolloverEventSet() const 
{
    return G_Is3WayFlagSet<Shift_DisableFocusRolloverEvent>(Flags);
}
SF_INLINE void MovieImpl::SetDisableFocusRolloverEvent(bool f) 
{
    G_Set3WayFlag<Shift_DisableFocusRolloverEvent>(Flags, f);
}

SF_INLINE bool MovieImpl::IsDisableFocusAutoRelease() const
{
    return G_Is3WayFlagTrue<Shift_DisableFocusAutoRelease>(Flags);
}
SF_INLINE bool MovieImpl::IsDisableFocusAutoReleaseSet() const
{
    return G_Is3WayFlagSet<Shift_DisableFocusAutoRelease>(Flags);
}
SF_INLINE void MovieImpl::SetDisableFocusAutoRelease(bool f) 
{
    G_Set3WayFlag<Shift_DisableFocusAutoRelease>(Flags, f);
}

SF_INLINE bool MovieImpl::IsDisableFocusAutoReleaseByMouseClick() const
{
    return G_IsFlagSet<Flag_DisableFocusAutoRelByClick>(Flags);
}
SF_INLINE void MovieImpl::SetDisableFocusAutoReleaseByMouseClick(bool f) 
{
    G_SetFlag<Flag_DisableFocusAutoRelByClick>(Flags, f);
}

SF_INLINE bool MovieImpl::IsAlwaysEnableFocusArrowKeys() const
{
    return G_Is3WayFlagTrue<Shift_AlwaysEnableFocusArrowKeys>(Flags);
}
SF_INLINE bool MovieImpl::IsAlwaysEnableFocusArrowKeysSet() const
{
    return G_Is3WayFlagSet<Shift_AlwaysEnableFocusArrowKeys>(Flags);
}
SF_INLINE void MovieImpl::SetAlwaysEnableFocusArrowKeys(bool f) 
{
    G_Set3WayFlag<Shift_AlwaysEnableFocusArrowKeys>(Flags, f);
}

SF_INLINE bool MovieImpl::IsDisableFocusKeys() const
{
    return G_Is3WayFlagTrue<Shift_DisableFocusKeys>(Flags);
}
SF_INLINE bool MovieImpl::IsDisableFocusKeysSet() const
{
    return G_Is3WayFlagSet<Shift_DisableFocusKeys>(Flags);
}
SF_INLINE void MovieImpl::SetDisableFocusKeys(bool f) 
{
    G_Set3WayFlag<Shift_DisableFocusKeys>(Flags, f);
}

/*SF_INLINE void MovieImpl::AddStickyVariable(const GASString& fullPath, const Value &val, SetVarType setType)
{
    pASMovieRoot->AddStickyVariable(fullPath, val, setType);
}

SF_INLINE void MovieImpl::ResolveStickyVariables(InteractiveObject *pcharacter)
{
    pASMovieRoot->ResolveStickyVariables(pcharacter);
}*/

SF_INLINE InteractiveObject* MovieImpl::FindTarget(const ASString& path) const
{
    return pASMovieRoot->FindTarget(path);
}

// helper function for 3D screen to world
SF_INLINE     void MovieImpl::SetNormalizedScreenCoords(const Render::PointF &mousePos)
{
    float nsx = 2.f * ((mousePos.x - PixelsToTwips(ViewOffsetX)) / VisibleFrameRect.Width()) - 1.0f;
    float nsy = 2.f * ((mousePos.y - PixelsToTwips(ViewOffsetY)) / VisibleFrameRect.Height()) - 1.0f;
    ScreenToWorld.SetNormalizedScreenCoords(nsx, nsy);
}

// does the movie contain a 3D sprite
SF_INLINE   bool MovieImpl::Is3D() const
{
    for (unsigned movieIndex = 0; movieIndex < MovieLevels.GetSize(); movieIndex++)
    {
        if (MovieLevels[movieIndex].pSprite->Is3D())
            return true;
    }
    return false;
}
// ** End Inline Implementation


//!AB: This class restores high precision mode of FPU for X86 CPUs.
// Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
// leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
// be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
class DoublePrecisionGuard
{
#if defined (SF_CC_MSVC) && defined(SF_CPU_X86)
    unsigned    fpc;
    //short       fpc;
#endif

public:

    DoublePrecisionGuard ()
    {
#if defined (SF_CC_MSVC) && defined(SF_CPU_X86)
  #if (SF_CC_MSVC >= 1400)
        // save precision mode (control word)
        _controlfp_s(&fpc, 0, 0);
        // set 53 bit precision
        unsigned _fpc;
        _controlfp_s(&_fpc, _PC_53, _MCW_PC);
  #else
        // save precision mode (control word)
        fpc = _controlfp(0,0);
        // set 53 bit precision
        _controlfp(_PC_53, _MCW_PC);
  #endif
        /*
        // save precision mode (only for X86)
        GASM fstcw fpc;
        //short _fpc = (fpc & ~0x300) | 0x300;  // 64-bit mantissa (REAL10)
        short _fpc = (fpc & ~0x300) | 0x200;  // 53-bit mantissa (REAL8)
        // set 53 bit precision
        GASM fldcw _fpc;
        */
#endif
    }

    ~DoublePrecisionGuard ()
    {
#if defined (SF_CC_MSVC) && defined (SF_CPU_X86)
  #if (SF_CC_MSVC >= 1400)
        // restore precision mode
        unsigned _fpc;
        _controlfp_s(&_fpc, fpc, _MCW_PC);
  #else
        _controlfp(fpc, _MCW_PC);
  #endif
        /*
        // restore precision mode (only for X86)
        GASM fldcw fpc;
        */
#endif
    }
};

}} // namespace Scaleform::GFx

#endif // INC_SF_GFX_PlayerImpl_H
