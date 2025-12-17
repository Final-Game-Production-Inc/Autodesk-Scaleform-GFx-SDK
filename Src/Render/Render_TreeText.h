/**************************************************************************

PublicHeader:   Render
Filename    :   Render_TreeText.h
Content     :   TreeText represents a TextField
Created     :   2010
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef SF_Render_TreeText_H
#define SF_Render_TreeText_H

#include "Kernel/SF_Array.h"
#include "Kernel/SF_List.h"
#include "Render_TreeNode.h"
#include "Render_TextMeshProvider.h"
#include "Render_TextLayout.h"
#include "Text/Text_DocView.h"

namespace Scaleform { namespace Render {

class TreeText : public TreeNode
{
public:

    class NodeData : public ContextData_ImplMixin<NodeData, TreeNode::NodeData>
    {
        typedef ContextData_ImplMixin<NodeData, TreeNode::NodeData> BaseClass;

    public:
        Ptr<Text::DocView>  pDocView;
        Ptr<TextLayout>     pLayout;   // pLayout should come after DocView, so ~DocView called after ~Layout
        enum
        {
            TextFlags_RebuildLayout = 0x01
        };
        UInt8               TextFlags;

        NodeData();
        NodeData(NonlocalCloneArg<NodeData> src);
        ~NodeData();

        // Textfield data should be stored here,
        // with main-thread data being separated.

        virtual bool            PropagateUp(Entry* entry) const;

        virtual TreeCacheNode*  updateCache(TreeCacheNode* pparent, TreeCacheNode* pinsert,
                                            TreeNode* pnode, UInt16 depth) const;

        virtual TreeNode*       CloneCreate(Context* context) const;
    };

    SF_RENDER_CONTEXT_ENTRY_INLINES(NodeData)

    void SetLayout(const TextLayout::Builder& b);
    const TextLayout* GetLayout() const
    {
        return GetReadOnlyData()->pLayout;
    }
    Text::DocView* GetDocView() const;

    // creates new docview
    void    Init(Text::Allocator*, Text::FontManagerBase*, Log* log = NULL);
    // init with existing docview
    void    Init(Text::DocView*);

    void    SetBounds(const RectF& r);
    RectF   GetBounds() const;

    // Set color of border around the text. If alpha is set to 0 then border is 
    // not drawing (default behavior).
    void    SetBorderColor(const Color& borderColor);
    void    SetBorderColor(UInt32 borderColor);

    // Returns currently set border color. If alpha is 0 then border is turned off.
    Color   GetBorderColor() const;

    // Set color of background under the text. If alpha is set to 0 then background is 
    // not drawing (default behavior).
    void    SetBackgroundColor(const Color& bkgColor);
    void    SetBackgroundColor(UInt32 color);

    // Returns currently set background color. If alpha is 0 then background is turned off.
    Color   GetBackgroundColor() const;

    enum Alignment
    {
        Align_Left,
        Align_Default = Align_Left,
        Align_Right,
        Align_Center,
        Align_Justify
    };
    // Sets horizontal text alignment (right, left, center)
    void        SetAlignment(Alignment);
    // Returns horizontal text alignment (right, left, center)
    Alignment   GetAlignment() const;

    enum VAlignment
    {
        VAlign_Top,
        VAlign_Default = VAlign_Top,
        VAlign_Center,
        VAlign_Bottom
    };
    // Sets vertical text alignment (top, bottom, center)
    void        SetVAlignment(VAlignment);
    // Returns vertical text alignment (top, bottom, center)
    VAlignment  GetVAlignment() const;

    // SetText sets UTF-8, UCS-2 or String text value to the text object. The optional 
    // parameter "lengthInBytes" specifies number of bytes in the UTF-8 string; 
    // "lengthInChars" specifies number of characters in wide character string.
    // If these parameters are not specified then
    // the UTF-8 and UCS-2 strings should be null-terminated.
    void    SetText(const char* putf8Str, UPInt lengthInBytes = UPInt(-1));
    void    SetText(const wchar_t* pstr,  UPInt lengthInChars = UPInt(-1));
    void    SetText(const String& str);
    // Returns currently set text in UTF-8 format. It returns plain text value;
    // even if HTML is used then it returns the string with all HTML tags stripped out.
    String  GetText() const;

    // SetHtmlText parses UTF-8, UCS-2 or String encoded HTML and initializes the text object
    // by the parsed HTML text.
    // The optional parameter "lengthInBytes" specifies number of bytes in the UTF-8 string; 
    // "lengthInChars" specifies number of characters in wide character string.
    // If these parameters are not specified then
    // the UTF-8 and UCS-2 strings should be null-terminated.
    void    SetHtmlText(const char* putf8Str, 
                        UPInt lengthInBytes = UPInt(-1), 
                        Text::StyledText::HTMLImageTagInfoArray* pimgInfoArr = NULL);
    void    SetHtmlText(const wchar_t* pstr,  
                        UPInt lengthInChars = UPInt(-1),
                        Text::StyledText::HTMLImageTagInfoArray* pimgInfoArr = NULL);
    void    SetHtmlText(const String& str, 
                        Text::StyledText::HTMLImageTagInfoArray* pimgInfoArr = NULL);
    // Returns currently set text in HTML format. If plain text is used with setting formatting 
    // by calling methods, such as SetColor, SetFont, etc, then this text will be converted to 
    // appropriate HTML format by this method.
    String  GetHtmlText() const ;

    // Sets color (R, G, B, A) to whole text or to the part of text in interval [startPos..endPos].
    // Both "startPos" and "endPos" parameters are optional.
    void    SetColor(Color c, UPInt startPos = 0, UPInt endPos = UPInt(-1));
    // Sets font to whole text or to the part of text in interval [startPos..endPos].
    // Both "startPos" and "endPos" parameters are optional.
    void    SetFont (const char* pfontName, UPInt startPos = 0, UPInt endPos = UPInt(-1));
    // Sets font size to whole text or to the part of text in interval [startPos..endPos].
    // Both "startPos" and "endPos" parameters are optional.
    void    SetFontSize(float fontSize, UPInt startPos = 0, UPInt endPos = UPInt(-1));
    // Sets letter spacing to whole text or to the part of text in interval [startPos..endPos].
    // Both "startPos" and "endPos" parameters are optional.
    void    SetLetterSpacing(float letterSpacing, UPInt startPos = 0, UPInt endPos = UPInt(-1));

    enum FontStyle
    {
        Normal,
        Bold,
        Italic,
        BoldItalic,
        ItalicBold = BoldItalic
    };
    // Sets font style to whole text or to the part of text in interval [startPos..endPos].
    // Both "startPos" and "endPos" parameters are optional.
    void    SetFontStyle(FontStyle, UPInt startPos = 0, UPInt endPos = UPInt(-1));
    // Sets or clears underline to whole text or to the part of text in interval [startPos..endPos].
    // Both "startPos" and "endPos" parameters are optional.
    void    SetUnderline(bool underline, UPInt startPos = 0, UPInt endPos = UPInt(-1));

    // Sets multiline (parameter 'multiline' is set to true) or singleline (false) type of the text.
    void    SetMultiline(bool multiline);
    // Returns 'true' if the text is multiline; 'false' otherwise.
    bool    IsMultiline() const;

    // Turns wordwrapping on/off
    void    SetWordWrap(bool wordWrap);
    // Returns state of wordwrapping.
    bool    IsWordWrap() const;

    enum AAMode
    {
        AA_Animation,
        AA_Readability
    };
    // Sets anti-aliasing mode - AA_Readibility for readability and AA_Animation for animation.
    // The default setting is anti-aliasing for animation.
    void    SetAAMode(AAMode);
    // Returns currently set anti-aliasing mode.
    AAMode  GetAAMode() const;

    // Enumeration of supported filters
    enum FilterType
    {
        Filter_Unknown      = 0,
        Filter_DropShadow   = 1,
        Filter_Blur         = 2,
        Filter_Glow         = 3

        // not supported
        //Filter_Bevel        = 3,
        //Filter_GradientGlow = 4,
        //Filter_Convolution  = 5,
        //Filter_AdjustColor  = 6,
        //Filter_GradientBevel= 7
    };
    // Bit flags for filters
    enum FilterFlagsType
    {
        FilterFlag_KnockOut   = 0x20,
        FilterFlag_HideObject = 0x40,
        FilterFlag_FineBlur   = 0x80
    };

    // A structure defining a filter.
    struct Filter
    {
        unsigned Type; // see FilterType
        union
        {
            struct
            {
                float BlurX;
                float BlurY;
                float Strength; // in percents
            } Blur;

            struct
            {
                float BlurX;
                float BlurY;
                float Strength; // in percents
                UInt32 Color;
                UInt8 Flags;    // bit FilterFlag_*
            } Glow;

            struct
            {
                float BlurX;
                float BlurY;
                float Strength; // in percents
                UInt32 Color;
                UInt8 Flags;    // bit FilterFlag_*

                float Angle;    // in degrees
                float Distance; // in pixels
            } DropShadow;
        };

        Filter() : Type(Filter_Unknown)  { InitByDefaultValues(); }
        Filter(FilterType ft) : Type(ft) { InitByDefaultValues(); }

        // Sets filter type. Useful, if initializing the filter as 
        // an element of array.
        void SetFilterType(FilterType ft) { Type = ft; }

        // Resets all data members to its default values.
        void InitByDefaultValues();

        // Sets "knockout" flag on
        void SetKnockOut()   { Glow.Flags |= FilterFlag_KnockOut; }
        // Clears "knockout" flag
        void ClearKnockOut() { Glow.Flags &= ~FilterFlag_KnockOut; }
        // Sets "hideobject" flag on
        void SetHideObject()   { Glow.Flags |= FilterFlag_HideObject; }
        // Clears "hideobject" flag 
        void ClearHideObject() { Glow.Flags &= ~FilterFlag_HideObject; }
    };

    // Sets filters on the text. More than one filter may be applied to one
    // text: "blur" filter might be combined with either "glow" or "dropshadow" filter.
    void    SetFilters(const Filter* filters, UPInt filtersCnt = 1);

    // Fills filtersBuf by the filters set on the node. 
    // 'filtersCntInBuf' should specify a max number of elements in 'filtersBuf'.
    // Returns a number of filled elements in 'filtersBuf'.
    UPInt   GetFilters(Filter* filtersBuf, UPInt filtersCntInBuf);

    // Remove all filters from the text
    void    ClearFilters();

    // Returns size (width and height) of the text.
    SizeF   GetTextSize();

    // Set a flag to rebuild layout and add to propagation.
    void    NotifyLayoutChanged();
private:
    void    UpdateDefaultTextFormat(Text::DocView*, const Text::TextFormat& fmt);
    void    UpdateDefaultParagraphFormat(Text::DocView*, const Text::ParagraphFormat& fmt);
};


}} // Scaleform::Render

#endif
