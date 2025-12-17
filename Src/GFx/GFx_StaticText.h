/**************************************************************************

Filename    :   GFx_StaticText.h
Content     :   Static text field character implementation
Created     :   May, 2007
Authors     :   Artem Bolgar, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFX_StaticText_H
#define INC_SF_GFX_StaticText_H

//#include "GFxConfig.h"

#include "GFx/GFx_DisplayObject.h"
#include "GFx/GFx_TextFieldDef.h"
#include "Render/Text/Text_LineBuffer.h"
#include "Render/Text/Text_FilterDesc.h"

namespace Scaleform {
namespace GFx {

using Render::Text::TextFilter;

//////////////////////////////////////////////////////////////////////////
// StaticTextCharacter
//
class StaticTextCharacter : public DisplayObject
{
    friend class StaticTextSnapshotData;

    Ptr<StaticTextDef>  pDef;
    Matrix2F            OrigMatrix;
public:
    enum
    {
        Flags_FontsResolved = 1,
        Flags_NextFrame     = 2
    };
    typedef Render::Text::Highlighter   Highlighter;
    typedef Render::Text::LineBuffer    LineBuffer;

    LineBuffer      TextGlyphRecords;
    TextFilter      Filter;

    class HighlightDesc : public NewOverrideBase<StatMV_Text_Mem>
    {
        Highlighter   HighlightManager;
    public:
        Highlighter& GetManager()                       { return HighlightManager; }
        bool         IsEmpty() const                    { return true; } //@REN BackgroundDrawing.IsEmpty(); }
        void         Invalidate()                       { HighlightManager.Invalidate(); }
        void         Validate()                         { HighlightManager.Validate(); }
        bool         IsValid() const                    { return HighlightManager.IsValid(); }
        void         SetSelectColor(const Color& color) { HighlightManager.SetSelectColor(color); }

    } *pHighlight;

    UInt8                   Flags;

    StaticTextCharacter(StaticTextDef* pdef, MovieDefImpl *pbindingDefImpl, 
        ASMovieRootBase* pasRoot, InteractiveObject* parent, ResourceId id);

    virtual ~StaticTextCharacter();

    virtual void    AdvanceFrame(bool nextFrame, float framePos)
    {
        SF_UNUSED(framePos);
        if (nextFrame)
            Flags |= Flags_NextFrame;
        else
            Flags &= ~Flags_NextFrame;
    }
    virtual CharacterDef::CharacterDefType GetType() const { return CharacterDef::StaticText; }

    virtual const Matrix&   GetMatrix() const;
    virtual void            SetMatrix(const Matrix& m);

    virtual void    SetFilters(const FilterSet* filters);
    virtual void    SetTextFilters(const TextFilter& f) 
    {
        Filter = f;
        RecreateVisibleTextLayout();
    }


    bool            HasHighlights()     { return pHighlight != NULL; }

    HighlightDesc*  CreateTextHighlighter();
    HighlightDesc*  GetTextHighlighter()        { return pHighlight; }

    StaticTextDef*  GetStaticTextDef() const 
    { 
        return pDef; 
    }

    virtual CharacterDef* GetCharacterDef() const
    {
        return pDef;
    }

//    virtual void    Display(DisplayContext &context);

    // These are used for finding bounds, width and height.
    virtual RectF  GetBounds(const Matrix &transform) const
    {       
        return transform.EncloseTransform(pDef->GetBoundsLocal());
    }   

    virtual RectF  GetRectBounds(const Matrix &transform) const
    {       
        return transform.EncloseTransform(pDef->GetRectBoundsLocal());
    }   

    virtual bool    PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask = 0) const
    {
        return pDef->DefPointTestLocal(pt, hitTestMask & HitTest_TestShape, this);
    }

    // Override this to hit-test shapes and make Button-Mode sprites work.
    virtual TopMostResult GetTopMostMouseEntity(const Render::PointF &pt, TopMostDescr* pdescr)
    {
        pdescr->pResult = GetTopMostMouseEntityDef(pDef, pt, pdescr->TestAll, pdescr->pIgnoreMC);
        if (pdescr->pResult)
            return TopMost_Found;
        return TopMost_FoundNothing;
    }
protected:
    virtual Ptr<Render::TreeNode>   CreateRenderNode(Render::Context& context) const;
    void                            RecreateVisibleTextLayout();
};


#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT

//
// Data storage for static text character snapshot
//
// See AS2_TextSnapshot.h for usage
//
class StaticTextSnapshotData : public NewOverrideBase<StatMV_Text_Mem>
{
public:
    typedef Render::Text::LineBuffer                LineBuffer;
    typedef Render::Text::HighlighterRangeIterator  HighlighterRangeIterator;
    typedef Render::Text::HighlightDesc             HighlightDesc;

    class GlyphVisitor
    {
        friend class StaticTextSnapshotData;

    public:
        virtual ~GlyphVisitor() {}
        virtual void OnVisit() = 0;

        SF_INLINE const char*       GetFontName()   { return pFont->GetName(); }
        SF_INLINE UPInt             GetRunIndex()   { return RunIdx; }
        SF_INLINE const Color&      GetColor()      { return ColorValue; }
        SF_INLINE float             GetHeight()     { return Height; }
        SF_INLINE const Render::Matrix2F&    GetMatrix()     { return Matrix; }
        SF_INLINE const RectF&      GetCorners()    { return Corners; }
        SF_INLINE bool              IsSelected()    { return bSelected; }

    private:      
        Render::Matrix2F                Matrix;
        RectF                           Corners;
        LineBuffer::GlyphEntry*         pGlyph;
        Font*                           pFont;
        UPInt                           RunIdx;
        float                           Height;
	Render::Color                       ColorValue;
        bool                            bSelected;
    };


    StaticTextSnapshotData();

    void            Add(StaticTextCharacter* pstChar);

    UPInt           GetCharCount() const;

    String          GetSubString(UPInt start, UPInt end, bool binclNewLines) const;
    String          GetSelectedText(bool binclNewLines) const;
    int             FindText(UPInt start, const char* query, bool bcaseSensitive) const;
    bool            IsSelected(UPInt start, UPInt end) const;
    void            SetSelected(UPInt start, UPInt end, bool bselect) const;
    void            SetSelectColor(const Color& color);
    int             HitTestTextNearPos(float x, float y, float closedist) const;

    void            Visit(GlyphVisitor* pvisitor, UPInt start, UPInt end) const;
        
private:

    struct CharRef
    {
        Ptr<StaticTextCharacter>    pChar;
        UPInt                       CharCount;
    };

    ArrayLH<CharRef>       StaticTextCharRefs;
    StringLH               SnapshotString;
    Color                  SelectColor;
};


#endif  // SF_NO_FXPLAYER_AS_TEXTSNAPSHOT

}} // namespace Scaleform::GFx

#endif  // INC_SF_GFX_StaticText_H
