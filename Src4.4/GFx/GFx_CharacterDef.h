/**************************************************************************

PublicHeader:   None
Filename    :   GFx_CharacterDef.h
Content     :   Defines abstract base character and playlist
                data classes.
Created     :   
Authors     :   Michael Antonov, TU

Notes       :   More implementation-specific classes are
                defined in the GFxPlayerImpl.h

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_CHARACTERDEF_H
#define INC_SF_GFX_CHARACTERDEF_H

#include "Kernel/SF_RefCount.h"
#include "Render/Render_Types2D.h"
#include "Render/Render_Matrix2x4.h"

#include "GFx/GFx_Types.h"
#include "GFx/GFx_Resource.h"

namespace Scaleform { 

namespace GFx {

// ***** Declared Classes


// ***** External Classes

class DisplayContext;
class ExecuteTag;
class MovieDataDef;
class MovieDefImpl;
class LogState;

#ifdef GFX_ENABLE_SOUND
class SoundStreamDef;
#endif

class DisplayObjectBase;
class InteractiveObject;
class TimelineDef;

// "newable" Scale9Grid rectangle.
struct Scale9Grid : public NewOverrideBase<StatMD_CharDefs_Mem>
{
    RectF   Rect;

    Scale9Grid() : Rect(0) {}
    Scale9Grid(const RectF& r) : Rect(r) {}

    Scale9Grid& operator=(const RectF& r) { Rect = r; return *this; }
};

// ***** CharacterDef

// A CharacterDef is the immutable data representing the template of a movie element.
// This is not a public interface.  It's here so it can be mixed into MovieDefinition,
// MovieDefinitionSub, and SpriteDef, without using multiple inheritance.
class CharacterDef : public Resource
{
    ResourceId        Id;
public:
    enum CharacterDefType
    {
        Unknown,
        Shape,
        MovieData, // MainTimeLine
        Sprite, // MovieClip with Def 
        TextField,
        StaticText,
        Button,
        Video,
        Bitmap,

        EmptySprite // sprite, created by AS
    };

    // Identifiers for code-defined characters (ok since Flash chars are <= 0xFFFF).
    // These can be passed directly to GetCharDef.
    enum {
        CharId_EmptyMovieClip               = ResourceId::IdType_InternalConstant | 1,
        CharId_EmptyTextField               = ResourceId::IdType_InternalConstant | 2,
        CharId_EmptyButton                  = ResourceId::IdType_InternalConstant | 3,
        CharId_EmptyShape                   = ResourceId::IdType_InternalConstant | 4,
        CharId_EmptyVideo                   = ResourceId::IdType_InternalConstant | 5,

        // These IDs are defined for image file loaded MovieDefs.
        // The constant are with
        CharId_ImageMovieDef_ImageResource  = 0,
        CharId_ImageMovieDef_ShapeDef       = 1
    };

    CharacterDef()
        : Id(ResourceId(ResourceId::InvalidId))
    {  }

    virtual ~CharacterDef() { }

    virtual CharacterDefType GetType() const =0;

    virtual void        Display(DisplayContext &context, DisplayObjectBase* pinstanceInfo);

    // Determine if the specified point falls into a shape.
    virtual bool        DefPointTestLocal(const PointF &pt, bool testShape = 0,
                                          const DisplayObjectBase *pinst = 0) const 
    { SF_UNUSED3(pt, testShape, pinst); return false; }  

    // Obtains character bounds in local coordinate space.
    virtual RectF      GetBoundsLocal(float morphRatio = 0) const
    { SF_UNUSED(morphRatio); return RectF(0); }   

    virtual RectF      GetRectBoundsLocal(float morphRatio = 0) const
    { SF_UNUSED(morphRatio); return RectF(0); }   
       
    virtual unsigned    GetVersion() const
    { SF_ASSERT(0); return 0; }

    // Creates a new instance of a character, ownership is passed to creator (character is
    // not automatically added to parent as a child; that is responsibility of the caller).
    // Default behavior is to create a GenericCharacter.
    //  - pbindingImpl argument provides the MovieDefImpl object with binding tables
    //    necessary for the character instance to function. Since in the new implementation
    //    loaded data is separated from instances, CharacterDef will not know what
    //    what MovieDefImpl it is associated with, so this data must be passed.
    //    This value is significant when creating GFxSprite objects taken from an import,
    //    since their MovieDefImpl is different from pparent->GetResourceMovieDef().
//    virtual DisplayContext*       CreateCharacterInstance(InteractiveObject* pparent, ResourceId rid,
//                                                        MovieDefImpl *pbindingImpl);

    
    // Id access - necessary for attachMobie
    // MA TBD: attachMovie should obtain an Id in some other way, such as
    // export table traversal and lookup.
    ResourceId               GetId() const           { return Id; }
    void                     SetId(ResourceId id) { Id = id; }    
};



// *****  TimelineDef - Character with frame Playlist support


class TimelineDef : public CharacterDef
{
public:

  
    struct Frame
    {
        // NOTE: Tag and tag array memory is responsible for
        //
        ExecuteTag**    pTagPtrList;
        unsigned        TagCount;

        Frame()
            : pTagPtrList(0), TagCount(0) { }       
        Frame(const Frame &src)
            : pTagPtrList(src.pTagPtrList), TagCount(src.TagCount) { }

        // Frames are returned by value because playlist array
        // can be reallocated on a loading thread in MovieDataDef.
        Frame& operator = (const Frame &src)
        { pTagPtrList = src.pTagPtrList; TagCount = src.TagCount; return *this; }

        // Calls destructors on all of the tag objects.
        void    DestroyTags();

        unsigned       GetTagCount() const      { return TagCount; }
        ExecuteTag* GetTag(unsigned index) const { return pTagPtrList[index]; }
    };


    // *** Playlist Access
    
    virtual unsigned            GetFrameCount() const                                   = 0;

    virtual const Frame         GetPlaylist(int frameNumber) const                      = 0;
    virtual bool                GetInitActions(Frame* pframe, int frameNumber) const    = 0;
    
    virtual bool                GetLabeledFrame(const char* label, unsigned* frameNumber, 
                                                bool translateNumbers = 1) const        = 0;
    virtual const String*       GetFrameLabel(unsigned frameNumber, unsigned* exactFrameNumberForLabel = NULL) const =0;
    // fills array of labels for the passed frame. One frame may have multiple labels.
    virtual Array<String>*      GetFrameLabels(unsigned frameNumber, Array<String>* destArr) const =0;

#ifdef GFX_ENABLE_SOUND
    virtual SoundStreamDef*     GetSoundStream() const                                  = 0;
    virtual void                SetSoundStream(SoundStreamDef*)                      = 0;
    virtual unsigned            GetLoadingFrame() const                                 = 0;
#endif
};


// TimelineDef with loading support. This is a base class for SpriteDef; however,
// it is used to avoid the inclusion of GFxSprite class in GFxLoadProcess.h.
class TimelineIODef : public TimelineDef
{
public:

    // *** Playlist loading

    // Adds frames to list. Frame tag / tag pointer list ownership is passed to TimelineDef.
    // TimeDef is responsible for destructing tags, however, memory belongs to MovieDataDef.
    virtual void                SetLoadingPlaylistFrame(const Frame& frame, LogState *plog)       = 0;
    virtual void                AddFrameName(const String& name, LogState *plog)  = 0;

    // Not supported by SpriteDef and thus not included here.
    // virtual void                SetLoadingInitActionFrame(const Frame& frame, LogState *plog) = 0;
};


}} // namespace Scaleform::GFx
#endif // INC_SF_GFX_CHARACTERDEF_H
