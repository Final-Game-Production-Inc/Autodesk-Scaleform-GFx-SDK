#include "GFx/AS2/AS2_MovieRoot.h"
#include "Kernel/SF_MsgFormat.h"
#include "GFx/AS2/AS2_AvmCharacter.h"
#include "GFx/AS2/AS2_ArrayObject.h"
#include "GFx/AS2/AS2_AvmSprite.h"
#include "GFx/AS2/AS2_BitmapFilter.h"
#include "GFx/GFx_ASUtils.h"
#include "Render/Render_TreeLog.h"
#include "Render/Render_TreeShape.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** AvmCharacter

// Constructor.
AvmCharacter::AvmCharacter(InteractiveObject* pdispObj) 
: pDispObj(pdispObj)
{
    pdispObj->BindAvmObj(this);
}

AvmCharacter::~AvmCharacter()
{
}

// ASEnvironment - sometimes necessary for strings.
Environment* AvmCharacter::GetASEnvironment()
{
    InteractiveObject* pparent = GetParent();
    while(pparent && !pparent->IsSprite())
        pparent = pparent->GetParent();
    if (!pparent)
        return 0;
    return GFx::AS2::ToAvmSprite(pparent)->GetASEnvironment();
}

const Environment* AvmCharacter::GetASEnvironment() const
{
    // Call non-const version. Const-ness really only matters for GFxSprite.
    return const_cast<AvmCharacter*>(this)->GetASEnvironment();
}

MovieRoot* AvmCharacter::GetAS2Root() const 
{ 
    return static_cast<MovieRoot*>(pDispObj->GetASMovieRoot()); 
}

// MovieImpl*           AvmCharacter::GetMovieImpl() const 
// { 
//     return GetAS2Root()->pMovieImpl; 
// }

void AvmCharacter::OnEventUnload()
{
}

// Determines the absolute path of the character.
const char* AvmCharacter::GetAbsolutePath(String *ppath) const
{
    SF_ASSERT(ppath);
    InteractiveObject* pparent = GetParent();
    if (pparent)
    {
        SF_ASSERT(pparent != pDispObj);
        pparent->GetAbsolutePath(ppath);
        *ppath += ".";
        *ppath += GetName().ToCStr();
    }
    else
    {
        if (IsSprite())
            Format(*ppath, "_level{0}", ToAvmSprite()->GetLevel());
        else
        {
            // Non-sprite characters must have parents.
            SF_ASSERT(0);
            ppath->Clear();
        }
    }
    return ppath->ToCStr();
}

void AvmCharacter::CloneInternalData(const InteractiveObject* src)
{
    SF_ASSERT(src);
    EventHandlers = GFx::AS2::ToAvmCharacter(src)->EventHandlers;
}

GlobalContext* AvmCharacter::GetGC() const
{
    return GetAS2Root()->pGlobalContext;
}


// *** Shared ActionScript methods.

// Depth implementation - same in MovieClip, Button, TextField.
void    AvmCharacter::CharacterGetDepth(const FnCall& fn)
{
    InteractiveObject* pcharacter = fn.ThisPtr->ToCharacter();
    if (!pcharacter)    
         pcharacter = fn.Env->GetTarget();  
    SF_ASSERT(pcharacter);

    // Depth always starts at -16384,
    // probably to allow user assigned depths to start at 0.
    fn.Result->SetInt(pcharacter->GetDepth() - 16384);
}

// *** Standard member support


AvmCharacter::MemberTableType AvmCharacter::MemberTable[] =
{
    { "_x",             AvmCharacter::M_x,            true },  
    { "_y",             AvmCharacter::M_y,            true },
    { "_xscale",        AvmCharacter::M_xscale,       true },
    { "_yscale",        AvmCharacter::M_yscale,       true },
    { "_currentframe",  AvmCharacter::M_currentframe, true },
    { "_totalframes",   AvmCharacter::M_totalframes,  true },
    { "_alpha",         AvmCharacter::M_alpha,        true },
    { "_visible",       AvmCharacter::M_visible,      true },
    { "_width",         AvmCharacter::M_width,        true },
    { "_height",        AvmCharacter::M_height,       true },
    { "_rotation",      AvmCharacter::M_rotation,     true },
    { "_target",        AvmCharacter::M_target,       true },
    { "_framesloaded",  AvmCharacter::M_framesloaded, true },
    { "_name",          AvmCharacter::M_name,         true },
    { "_droptarget",    AvmCharacter::M_droptarget,   true },
    { "_url",           AvmCharacter::M_url,          true },
    { "_highquality",   AvmCharacter::M_highquality,  true },
    { "_focusrect",     AvmCharacter::M_focusrect,    true },
    { "_soundbuftime",  AvmCharacter::M_soundbuftime, true },
    // SWF 5+.
    { "_quality",       AvmCharacter::M_quality,  true },
    { "_xmouse",        AvmCharacter::M_xmouse,   true },
    { "_ymouse",        AvmCharacter::M_ymouse,   true }, 

    // Extra shared properties which can have a default implementation.
    { "_parent",        AvmCharacter::M_parent,       false },     
    { "blendMode",      AvmCharacter::M_blendMode,    false },
    { "cacheAsBitmap",  AvmCharacter::M_cacheAsBitmap, false },
    { "filters",        AvmCharacter::M_filters,      false },
    { "enabled",        AvmCharacter::M_enabled,      false },
    { "trackAsMenu",    AvmCharacter::M_trackAsMenu,  false },
    { "_lockroot",      AvmCharacter::M_lockroot,     false },
    { "tabEnabled",     AvmCharacter::M_tabEnabled,   false },
    { "tabIndex",       AvmCharacter::M_tabIndex,     false },
    { "useHandCursor",  AvmCharacter::M_useHandCursor, false },  

    { "edgeaaMode",     AvmCharacter::M_edgeaaMode, false },
    
    // Not shared.
    { "menu",           AvmCharacter::M_menu, false },

    // MovieClip custom.
    { "focusEnabled",   AvmCharacter::M_focusEnabled, false },
    { "tabChildren",    AvmCharacter::M_tabChildren, false },
    { "transform",      AvmCharacter::M_transform, false },
    { "scale9Grid",     AvmCharacter::M_scale9Grid, false },
    { "hitArea",        AvmCharacter::M_hitArea, false },
    { "scrollRect",     AvmCharacter::M_scrollRect, false },

    // TextField custom.
    { "text",           AvmCharacter::M_text, false },
    { "textWidth",      AvmCharacter::M_textWidth, false },
    { "textHeight",     AvmCharacter::M_textHeight, false },
    { "textColor",      AvmCharacter::M_textColor, false },
    { "length",         AvmCharacter::M_length, false },
    { "html",           AvmCharacter::M_html, false },
    { "htmlText",       AvmCharacter::M_htmlText, false },
    { "styleSheet",     AvmCharacter::M_styleSheet, false },
    { "autoSize",       AvmCharacter::M_autoSize, false },
    { "wordWrap",       AvmCharacter::M_wordWrap, false },
    { "multiline",      AvmCharacter::M_multiline, false },
    { "border",         AvmCharacter::M_border, false },
    { "variable",       AvmCharacter::M_variable, false },
    { "selectable",     AvmCharacter::M_selectable, false },
    { "embedFonts",     AvmCharacter::M_embedFonts, false },
    { "antiAliasType",  AvmCharacter::M_antiAliasType, false },
    { "hscroll",        AvmCharacter::M_hscroll, false },
    { "scroll",         AvmCharacter::M_scroll, false },
    { "maxscroll",      AvmCharacter::M_maxscroll, false },
    { "maxhscroll",     AvmCharacter::M_maxhscroll, false },
    { "background",     AvmCharacter::M_background, false },
    { "backgroundColor",AvmCharacter::M_backgroundColor, false },
    { "borderColor",    AvmCharacter::M_borderColor, false },
    { "bottomScroll",   AvmCharacter::M_bottomScroll, false },
    { "type",           AvmCharacter::M_type, false },
    { "maxChars",       AvmCharacter::M_maxChars, false },
    { "condenseWhite",  AvmCharacter::M_condenseWhite, false },
    { "mouseWheelEnabled", AvmCharacter::M_mouseWheelEnabled, false },
    { "password",       AvmCharacter::M_password, false },
    
    // GFx extensions
    { "shadowStyle",    AvmCharacter::M_shadowStyle, false },
    { "shadowColor",    AvmCharacter::M_shadowColor, false },
    { "hitTestDisable", AvmCharacter::M_hitTestDisable, false },
    { "noTranslate",    AvmCharacter::M_noTranslate, false },
    { "caretIndex",     AvmCharacter::M_caretIndex, false },
    { "numLines",       AvmCharacter::M_numLines, false   },
    { "verticalAutoSize",AvmCharacter::M_verticalAutoSize, false },
    { "fontScaleFactor", AvmCharacter::M_fontScaleFactor, false },
    { "verticalAlign",  AvmCharacter::M_verticalAlign, false },
    { "textAutoSize",   AvmCharacter::M_textAutoSize, false },
    { "useRichTextClipboard", AvmCharacter::M_useRichTextClipboard, false },
    { "alwaysShowSelection",  AvmCharacter::M_alwaysShowSelection, false },
    { "selectionBeginIndex",  AvmCharacter::M_selectionBeginIndex, false },
    { "selectionEndIndex",    AvmCharacter::M_selectionEndIndex, false },
    { "selectionBkgColor",    AvmCharacter::M_selectionBkgColor, false },
    { "selectionTextColor",   AvmCharacter::M_selectionTextColor, false },
    { "inactiveSelectionBkgColor",  AvmCharacter::M_inactiveSelectionBkgColor, false },
    { "inactiveSelectionTextColor", AvmCharacter::M_inactiveSelectionTextColor, false },
    { "noAutoSelection", AvmCharacter::M_noAutoSelection, false },
    { "disableIME",      AvmCharacter::M_disableIME, false },
    { "topmostLevel",    AvmCharacter::M_topmostLevel, false },
    { "noAdvance",       AvmCharacter::M_noAdvance, false },
    { "focusGroupMask",      AvmCharacter::M_focusGroupMask, false },
    { "bidirectionalTextEnabled", AvmCharacter::M_bidirectionalTextEnabled, false },

    // Dynamic Text
    { "autoFit",        AvmCharacter::M_autoFit, false },
    { "blurX",          AvmCharacter::M_blurX, false },
    { "blurY",          AvmCharacter::M_blurY, false },
    { "blurStrength",   AvmCharacter::M_blurStrength, false },
    { "outline",        AvmCharacter::M_outline, false },
    { "fauxBold",       AvmCharacter::M_fauxBold, false },
    { "fauxItalic",     AvmCharacter::M_fauxItalic, false },
    { "restrict",       AvmCharacter::M_restrict, false },
    
    // Dynamic Text Shadow
    { "shadowAlpha",      AvmCharacter::M_shadowAlpha, false },
    { "shadowAngle",      AvmCharacter::M_shadowAngle, false },
    { "shadowBlurX",      AvmCharacter::M_shadowBlurX, false },
    { "shadowBlurY",      AvmCharacter::M_shadowBlurY, false },
    { "shadowDistance",   AvmCharacter::M_shadowDistance, false },
    { "shadowHideObject", AvmCharacter::M_shadowHideObject, false },
    { "shadowKnockOut",   AvmCharacter::M_shadowKnockOut, false },
    { "shadowQuality",    AvmCharacter::M_shadowQuality, false },
    { "shadowStrength",   AvmCharacter::M_shadowStrength, false },
    { "shadowOutline",    AvmCharacter::M_shadowOutline, false },

    { "_z",               AvmCharacter::M_z, true },
    { "_zscale",          AvmCharacter::M_zscale, true },
    { "_xrotation",       AvmCharacter::M_xrotation, true },
    { "_yrotation",       AvmCharacter::M_yrotation, true },
    { "_matrix3d",        AvmCharacter::M_matrix3d, true },
    { "_perspfov",        AvmCharacter::M_fov, true },

    { "$version",    AvmCharacter::M__version, false },

    // Done.
    { 0,  AvmCharacter::M_InvalidMember, false }
};

void    AvmCharacter::InitStandardMembers(GlobalContext *pcontext)
{
    SF_COMPILER_ASSERT( (sizeof(MemberTable)/sizeof(MemberTable[0]))
                        == M_StandardMemberCount + 1);

    // Add all standard members.
    MemberTableType* pentry;
    ASStringManager* pstrManager = pcontext->GetStringManager()->GetStringManager();

    pcontext->StandardMemberMap.SetCapacity(M_StandardMemberCount);  

    for (pentry = MemberTable; pentry->pName; pentry++)
    {
        ASString name(pstrManager->CreateConstString(pentry->pName, strlen(pentry->pName),
                                                      ASString::Flag_StandardMember |
                                                      (pentry->CaseInsensitive?ASString::Flag_CaseInsensitive:0)));
        pcontext->StandardMemberMap.Add(name, (SByte)pentry->Id);
    }
}

bool             AvmCharacter::IsStandardMember(const ASString& memberName, ASString* pcaseInsensitiveName)
{
    if (memberName.IsStandardMember())
        return true;
    // now need to check a special case. Flash 7+ uses case sensitive names
    // for every variable/member but some exceptions. Some standard members
    // still might be referenced case insensitively.
    if (memberName.GetLength() > 0 && memberName.GetCharAt(0) == '_')
    {
        ASString lowerCase = memberName.ToLower();
        if (lowerCase.IsCaseInsensitive())
        {
            if (pcaseInsensitiveName)
                *pcaseInsensitiveName = lowerCase;
            return true;
        }
    }
    return false;
}

// Looks up a standard member and returns M_InvalidMember if it is not found.
AvmCharacter::StandardMember  AvmCharacter::GetStandardMemberConstant(const ASString& memberName) const
{
    SByte   memberConstant = M_InvalidMember; // Has to be signed or conversion is incorrect!
    ASString lowerCaseName = GetGC()->GetStringManager()->GetBuiltin(ASBuiltin_empty_);
    if (IsStandardMember(memberName, &lowerCaseName))
    {
        GlobalContext* pcontext = GetGC();
        bool caseSensitive = lowerCaseName.IsEmpty();
        pcontext->StandardMemberMap.GetCaseCheck(memberName, &memberConstant, caseSensitive);
    }        
    
    SF_ASSERT((memberConstant != M_InvalidMember) ? 
        (memberName.IsStandardMember() || (lowerCaseName.IsCaseInsensitive() && lowerCaseName.IsStandardMember())) : 1);
    return (StandardMember) memberConstant;
}

// BlendMode lookup table.
// Should be moved elsewhere so that it can apply to buttons, etc.
static const char * GFx_BlendModeNames[1+ 14] =
{
    "normal",   // 0?
    "normal",   
    "layer",
    "multiply",
    "screen",
    "lighten",
    "darken",
    "difference",
    "add",
    "subtract",
    "invert",
    "alpha",
    "erase",
    "overlay",
    "hardlight"
};

// Handles built-in members. Return 0 if member is not found or not supported.
bool    AvmCharacter::SetStandardMember(StandardMember member, const Value& val, bool opcodeFlag)
{   
    SF_ASSERT(pDispObj);

    if (opcodeFlag && ((member < M_BuiltInProperty_Begin) || (member > M_BuiltInProperty_End)))
    {
        pDispObj->LogScriptError("Invalid SetProperty request, property number %d", member);
        return 0;
    }
    // Make sure that this character class supports the constant.
    if (member == M_InvalidMember)
        return 0;
    if (!((member <= M_SharedPropertyEnd) && (GetStandardMemberBitMask() & (1<<member))))
        return 0;
    Environment* pEnv = GetASEnvironment ();

    switch(member)
    {
    case M_x:
        {
//             // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
             if (val.IsUndefined())
                 return 1;
             Number xval = val.ToNumber(pEnv);
//             if (ASNumberUtil::IsNEGATIVE_INFINITY(xval) || ASNumberUtil::IsPOSITIVE_INFINITY(xval))
//                 xval = 0;
//             SetAcceptAnimMoves(0);
//             SF_ASSERT(pGeomData);
// 
//             Matrix  m = GetMatrix();
//             pGeomData->X = int(floor(PixelsToTwips(xval)));
//             m.Tx() = (float) pGeomData->X;
//             if (m.IsValid())
//                 SetMatrix(m);
            pDispObj->SetX(xval);
            return 1;
        }

    case M_y:
        {
            // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
            if (val.IsUndefined())
                return 1;
            Number yval = val.ToNumber(pEnv);
            pDispObj->SetY(yval);
            return 1;
        }

    case M_xscale:
        {
            // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
            if (val.IsUndefined())
                return 1;
            Double newXScale = val.ToNumber(pEnv);
            pDispObj->SetXScale(newXScale);
            return 1;
        }

    case M_yscale:
        {
            // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
            if (val.IsUndefined())
                return 1;
            Double newYScale = val.ToNumber(pEnv);
            pDispObj->SetYScale(newYScale);
            return 1;
        }

    case M_rotation:
        {
            // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
            if (val.IsUndefined())
                return 1;
            Number rval = val.ToNumber(pEnv);
            pDispObj->SetRotation(rval);
            return 1;
        }

    case M_width:
        {
            if (val.IsUndefined())
                return 1;
            // MA: Width/Height modification in Flash is unusual in that it performs
            // relative axis scaling in the x local axis (width scaling) and y local
            // axis (height scaling). The resulting width after scaling does not
            // actually equal the original, instead, it is related to rotation!
            // AB: I am second on that! Looks like a bug in Flash.

            // NOTE: Although it works for many cases, this is still not correct. Modification 
            // of width seems very strange (if not buggy) in Flash.
            Number wval = val.ToNumber(pEnv);
            pDispObj->SetWidth(wval);
            return 1;
        }

    case M_height:
        {
            if (val.IsUndefined())
                return 1;
            Number hval = val.ToNumber(pEnv);
            pDispObj->SetHeight(hval);
            return 1;
        }
    

    case M_alpha:
        {
            if (val.IsUndefined())
                return 1;
            // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
            Number aval = val.ToNumber(pEnv);
            pDispObj->SetAlpha(aval);
            return 1;
        }

    case M_visible:
        {
            // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
            pDispObj->SetVisible(val.ToBool(pEnv));           
            // Setting visibility does not affect AnimMoves.
            return 1;
        }

    case M_blendMode:
        {
            // NOTE: Setting "blendMode" in Flash does NOT disconnect object from time-line,
            // so just setting the value is the correct behavior.
            if (val.GetType() == Value::STRING)
            {
                ASString  asstr(val.ToString(pEnv));
                String  str = asstr.ToCStr();
                for (unsigned i=1; i<(sizeof(GFx_BlendModeNames)/sizeof(GFx_BlendModeNames[0])); i++)
                {
                    if (str == GFx_BlendModeNames[i])
                    {
                        pDispObj->SetBlendMode((DisplayObject::BlendType) i);
                        return 1;
                    }
                }
            }
            else
            {
                int mode = (int)val.ToNumber(pEnv);
                mode = Alg::Max<int>(Alg::Min<int>(mode,14),1);
                pDispObj->SetBlendMode((DisplayObject::BlendType) mode);
            }
            return 1;
        }


    case M_name:
        {
            pDispObj->SetName(val.ToString(pEnv));
            return 1;
        }
        
    case M_enabled:
        {
            pDispObj->SetEnabledFlag(val.ToBool(pEnv));
            return 1;
        }       

    case M_trackAsMenu:
        {
            pDispObj->SetTrackAsMenuFlag(val.ToBool(pEnv));
            return 1;
        }

    // Read-only properties.
    case M_droptarget:
    case M_target:
    case M_url:
    case M_xmouse:
    case M_ymouse:  
    case M_parent: // TBD: _parent is not documented as read only. Can it be changed ??

    // These are only implemented for MovieClip. Technically, they should not be here;
    // however, they are Flash built-ins. Since they are read-only, we can handle them here.
    case M_currentframe:
    case M_totalframes:
    case M_framesloaded:        

        pEnv->LogScriptWarning("Attempt to write read-only property %s.%s, ignored",
            pDispObj->GetName().ToCStr(), AvmCharacter::MemberTable[member].pName);
        return 1;

    case M_tabEnabled:
        if (!val.IsUndefined())
            pDispObj->SetTabEnabledFlag(val.ToBool(GetASEnvironment()));
        else
            pDispObj->UndefineTabEnabledFlag();
        return 1;

    case M_tabIndex:
        pDispObj->SetTabIndex((int)val.ToNumber(GetASEnvironment()));
        return 1;

    case M_focusrect:
        if (!val.IsUndefined())
            pDispObj->SetFocusRectFlag(val.ToBool(GetASEnvironment()));
        else
            pDispObj->UndefineTabEnabledFlag();
        pDispObj->SetDirtyFlag();
        return 1;

    // Un-implemented properties:
    case M_soundbuftime:
    case M_highquality:
    case M_quality:
        // We have a default get below, so return 1.
        return 1;

    case M_useHandCursor:
        if (!val.IsUndefined())
            pDispObj->SetUseHandCursorFlag(val.ToBool(GetASEnvironment()));
        else
            pDispObj->UndefineUseHandCursorFlag();
        return 1;

    case M_filters:
        {
#ifdef GFX_AS2_ENABLE_FILTERS
            Object* pobj = val.ToObject(pEnv);
            if (pobj && pobj->InstanceOf(pEnv, pEnv->GetPrototype(ASBuiltin_Array)))
            {
                ArrayObject* parrobj = static_cast<ArrayObject*>(pobj);
                Ptr<FilterSet> newfilters = *SF_NEW FilterSet();
                for (int i=0; i < parrobj->GetSize(); ++i)
                {
                    Value* arrval = parrobj->GetElementPtr(i);
                    if (arrval)
                    {
                        Object* pavobj = arrval->ToObject(pEnv);
                        if (pavobj && pavobj->InstanceOf(pEnv, pEnv->GetPrototype(ASBuiltin_BitmapFilter)))
                        {
                            BitmapFilterObject* pfilterobj = static_cast<BitmapFilterObject*>(pavobj);
                            newfilters->AddFilter(pfilterobj->GetFilter());
                        }
                    }
                }
                SetFilters(newfilters);

                pDispObj->SetDirtyFlag();
                pDispObj->SetAcceptAnimMoves(false);
            }
#endif
            return 1;
        }
        break;

    case M_cacheAsBitmap:
        pDispObj->SetCacheAsBitmap(val.ToBool(GetASEnvironment()));
        return 1;

    case M_lockroot:
        // Allow at least property assignment in map for now.
        return 0;

    case M_edgeaaMode:
        {
            // Pass through
            EdgeAAMode edgeAA = EdgeAA_Inherit;
            GASNumberValue mode = val.ToInt32(GetASEnvironment());
            if (mode == 3) 
                edgeAA = EdgeAA_Disable;
            else
            {
                if (mode == 1)      edgeAA = EdgeAA_On;
                else if (mode == 2)  edgeAA = EdgeAA_Off;
            }
            pDispObj->GetRenderNode()->SetEdgeAAMode(edgeAA);
            return 1;
        }

    default:
        // Property not handled, fall out.
        return 0;
    }
}

bool    AvmCharacter::GetStandardMember(StandardMember member, Value* val, bool opcodeFlag) const
{
    SF_ASSERT(pDispObj);

    if (opcodeFlag && ((member < M_BuiltInProperty_Begin) || (member > M_BuiltInProperty_End)))
    {
        GetASEnvironment()->LogScriptError("Invalid GetProperty query, property number %d", member);
        return 0;
    }

    // Make sure that this character class supports the constant.
    if (member == M_InvalidMember)
        return 0;
    if (!((member <= M_SharedPropertyEnd) && (GetStandardMemberBitMask() & (1<<member))))
        return 0;

    switch(member)
    {
    case M_x:
        {           
            // NOTE: If updating this logic, also update Value::ObjectInterface::GetDisplayInfo
            val->SetNumber(pDispObj->GetX());
            return 1;
        }
    case M_y:       
        {   
            // NOTE: If updating this logic, also update Value::ObjectInterface::GetDisplayInfo
            val->SetNumber(pDispObj->GetY());
            return 1;
        }

    case M_xscale:      
        {           
            // NOTE: If updating this logic, also update Value::ObjectInterface::GetDisplayInfo
            val->SetNumber(pDispObj->GetXScale());   // result in percent
            return 1;
        }
    case M_yscale:      
        {           
            // NOTE: If updating this logic, also update Value::ObjectInterface::GetDisplayInfo
            DisplayObject::GeomDataType geomData;
            val->SetNumber(pDispObj->GetYScale());   // result in percent
            return 1;
        }

    case M_width:   
        {
            //RectF    boundRect = GetLevelBounds();
            //!AB: width and height of nested movieclips returned in the coordinate space of its parent!
            val->SetNumber(pDispObj->GetWidth());
            return 1;
        }
    case M_height:      
        {
            //RectF    boundRect = GetLevelBounds();
            //!AB: width and height of nested movieclips returned in the coordinate space of its parent!
            val->SetNumber(pDispObj->GetHeight());
            return 1;
        }

    case M_rotation:
        {
            // NOTE: If updating this logic, also update Value::ObjectInterface::GetDisplayInfo
            // Verified against Macromedia player using samples/TestRotation.Swf
            val->SetNumber(pDispObj->GetRotation());
            return 1;
        }

    case M_alpha:   
        {
            // NOTE: If updating this logic, also update Value::ObjectInterface::GetDisplayInfo
            // Alpha units are in percent.
            val->SetNumber(pDispObj->GetAlpha());
            return 1;
        }

    case M_visible: 
        {
            // NOTE: If updating this logic, also update Value::ObjectInterface::GetDisplayInfo
            val->SetBool(pDispObj->GetVisible());
            return 1;
        }

    case M_blendMode:
        {
            val->SetString(GetASEnvironment()->CreateString(GFx_BlendModeNames[pDispObj->GetBlendMode()]));
            // Note: SWF 8 can sometimes report "undefined". TBD.
            return 1;
        }

    case M_name:
        {
            val->SetString(pDispObj->GetName());
            return 1;
        }
        
    case M_enabled:
        {           
            val->SetBool(pDispObj->GetEnabled());
            return 1;
        }   

    case M_trackAsMenu:
        {
            val->SetBool(pDispObj->GetTrackAsMenu());
            return 1;
        }

    case M_target:
        {
            // Full path to this object; e.G. "/sprite1/sprite2/ourSprite"
            StringBuffer      name;
            Ptr<InteractiveObject> root = GetTopParent();

            for (const InteractiveObject* p = pDispObj; p != 0 && p != root; p = p->GetParent())
            {
                const ASString& cname = p->GetName();
                name.Insert (cname.ToCStr(), 0);
                name.Insert ("/", 0);
            }
            val->SetString(GetASEnvironment()->CreateString(name.ToCStr(),name.GetSize()));
            return 1;
        }

    case M_parent:
        {
            if (pDispObj->GetParent())
                val->SetAsCharacter(pDispObj->GetParent());
            else
                val->SetUndefined();
            return 1;
        }

    case M_droptarget:
        {
            // Absolute path in slash syntax where we were last Dropped (?)
            // @@ TODO
            //val->SetString(GetASEnvironment()->GetBuiltin(ASBuiltin__root));
            //return 1;

            // Crude implementation..
            val->SetUndefined();
            MovieImpl* proot = GetASEnvironment()->GetMovieImpl();
            Render::PointF mousePos = proot->GetMouseState(0)->GetLastPosition();
            InteractiveObject* ptopCh = proot->GetTopMostEntity(mousePos, 0 /* controller */, true, pDispObj);
            StringBuffer      name;
            for (const InteractiveObject* p = ptopCh; p != 0; p = p->GetParent())
            {
                const ASString& cname = p->GetName();
                name.Insert (cname.ToCStr(), 0);
                name.Insert ("/", 0);
            }
            val->SetString(GetASEnvironment()->CreateString(name.ToCStr(),name.GetSize()));
            return 1;
        }

    case M_url: 
        {
            // our URL.
            Array<char>        urlArray;
            const char*         purl = pDispObj->GetResourceMovieDef()->GetFileURL();
            UPInt               urlSize = purl ? SFstrlen(purl) : 0;
            urlArray.Resize(urlSize + 1);
                   
            for (UPInt i = 0; i < urlSize; ++i)
                urlArray[i] = (purl[i] == '\\') ? '/' : purl[i];
            urlArray[urlSize] = 0;
            
            String urlStr;
            ASUtils::EscapePath(&urlArray[0], urlSize, &urlStr);
            val->SetString(GetASEnvironment()->CreateString(urlStr));
            return 1;
        }

    case M_highquality: 
        {
            // Whether we're in high quality mode or not.
            val->SetBool(true);
            return 1;
        }
    case M_quality:
        {
            val->SetString(GetASEnvironment()->CreateString("HIGH"));
            return 1;
        }

    case M_focusrect:   
        {
            // Is a yellow rectangle visible around a focused AvmCharacter Clip (?)
            if (pDispObj->IsFocusRectFlagDefined())
                val->SetBool(pDispObj->IsFocusRectFlagTrue());
            else
                val->SetNull();
            return 1;
        }

    case M_soundbuftime:        
        {
            // Number of seconds before sound starts to Stream.
            val->SetNumber(0.0);
            return 1;
        }

    case M_xmouse:  
        {
            // Local coord of mouse IN PIXELS.
            val->SetNumber(pDispObj->GetMouseX());
            return 1;
        }

    case M_ymouse:  
        {
            // Local coord of mouse IN PIXELS.
            val->SetNumber(pDispObj->GetMouseY());
            return 1;
        }

    case M_tabEnabled:
        if (pDispObj->IsTabEnabledFlagDefined())
            val->SetBool(pDispObj->IsTabEnabledFlagTrue());
        else
            val->SetUndefined();
        return 1;

    case M_tabIndex:
        val->SetNumber((Number)pDispObj->GetTabIndex());
        return 1;

    case M_useHandCursor:
        if (!pDispObj->IsUseHandCursorFlagDefined())
            return 0;
        val->SetBool(pDispObj->IsUseHandCursorFlagTrue());
        return 1;

    case M_filters:
        {
#ifdef GFX_AS2_ENABLE_FILTERS
            Environment* penv = const_cast<Environment*>(GetASEnvironment());
            ASStringContext* psc = penv->GetSC();
            Ptr<ArrayObject> arrayObj = *SF_HEAP_NEW(psc->GetHeap()) ArrayObject(psc);
            const FilterSet* pFilters = GetFilters();
            if (pFilters)
            {
                for (unsigned i = 0 ; i < pFilters->GetFilterCount(); i++)
                {
                    const Filter* f = pFilters->GetFilter(i);
                    if ( !f )
                        continue;
                    BitmapFilterObject* pfilterobj = BitmapFilterObject::CreateFromDesc(penv, *f);
                    if (pfilterobj)
                    {
                        arrayObj->PushBack(pfilterobj);
                        pfilterobj->Release();
                    }
                }
            }
            val->SetAsObject(arrayObj);
#endif
        }
        return 1;

    case M_cacheAsBitmap:
        // NOTE: If a display object has a filter pointer, it always returns that CaB is enabled,
        // regardless of whether the property is set or not (pDispObj->GetCacheAsBitmap will return
        // whether the flag is set).
        val->SetBool(pDispObj && pDispObj->GetRenderNode() && pDispObj->GetRenderNode()->GetFilters());
        return 1;

    // Un-implemented properties:
    case M_lockroot:
        // Allow at least property assignment in map for now.
        return 0;

    // These are only implemented for MovieClip. Technically, they should not be here;
    // however, they are Flash built-ins. Return 0 so that GFxSprite properly handles them.
    case M_currentframe:
    case M_totalframes:
    case M_framesloaded:
        return 0;   

    case M_edgeaaMode:
        {
            switch (pDispObj->GetRenderNode()->GetEdgeAAMode())
            {
            case EdgeAA_On:         val->SetNumber(1);      break;
            case EdgeAA_Off:        val->SetNumber(2);      break;
            case EdgeAA_Disable:    val->SetNumber(3);      break;
            default:                val->SetNumber(0);      break;
            }
            return 1;
        }

    default:
        break;
    }

    return 0;
}



// Duplicate the object with the specified name and add it with a new name at a new depth.
InteractiveObject* AvmCharacter::CloneDisplayObject(const ASString& newname, int depth, const ObjectInterface *psource)
{
    Sprite* pparent = GetParent()->CharToSprite();
    if (!pparent)
        return 0;
    if ((depth < 0) || (depth > (2130690045 + 16384)))
        return 0;

    // Clone us.    
    CharPosInfo pos( pDispObj->GetId(), depth,
                        1, pDispObj->GetCxform(), 1, pDispObj->GetMatrix(), NULL, /* PPS: NULL for className? */
                        pDispObj->GetRatio(), pDispObj->GetClipDepth());

    // true: replace if depth is occupied
    DisplayObjectBase* pnewCh = pparent->AddDisplayObject(
        pos, newname, 0, psource, 
        SF_MAX_UINT, DisplayList::Flags_ReplaceIfDepthIsOccupied, 0, pDispObj);
    if (pnewCh)
        return pnewCh->CharToInteractiveObject();
    return NULL;
}

// Remove the object with the specified name.
void    AvmCharacter::RemoveDisplayObject()
{
    if (!GetParent())
        return;
    Sprite* pparent = GetParent()->CharToSprite();
    if (!pparent)
        return; 
    pparent->RemoveDisplayObject(pDispObj->GetDepth(), pDispObj->GetId());
}


void    AvmCharacter::CopyPhysicalProperties(const InteractiveObject *poldChar)
{
    // Copy physical properties, used by loadMovie().
    EventHandlers = GFx::AS2::ToAvmCharacter(poldChar)->EventHandlers;
}


static SF_INLINE EventId GFx_TreatEventId(const EventId& id)
{
    // for keyDown/keyUp search by id only (w/o KeyCode/AsciiCode)
    if (id.Id == EventId::Event_KeyDown || id.Id == EventId::Event_KeyUp)
        return EventId(id.Id); 
    return id;
}

bool    AvmCharacter::HasClipEventHandler(const EventId& id) const
{ 
    const EventsArray* evts = EventHandlers.Get(GFx_TreatEventId(id));
    return evts != 0; 
}

bool    AvmCharacter::InvokeClipEventHandlers(Environment* penv, const EventId& id)
{ 
    const EventsArray* evts = EventHandlers.Get(GFx_TreatEventId(id));
    if (evts)
    {
        for (UPInt i = 0, n = evts->GetSize(); i < n; ++i)
        {
            const Value& method = (*evts)[i];
            GAS_Invoke0(method, NULL, this, penv);
        }
        return true;
    }
    return false; 
}

void    AvmCharacter::SetSingleClipEventHandler(const EventId& id, const Value& method)
{ 
    SF_ASSERT(id.GetEventsCount() == 1);
    EventsArray* evts = EventHandlers.Get(GFx_TreatEventId(id));
    if (!evts)
    {
        // Local arrays need to be allocated for the right heap.
        EventsArray *pea = SF_HEAP_AUTO_NEW(this) EventsArray;
        pea->PushBack(method);               
        EventHandlers.Set(id, *pea); 
        delete pea;
    }
    else
    {
        evts->PushBack(method);
    }
}

void    AvmCharacter::SetClipEventHandlers(const EventId& id, const Value& method)
{ 
    unsigned numOfEvents = id.GetEventsCount();
    SF_ASSERT(numOfEvents > 0);
    if (numOfEvents == 1)
    {
        SetSingleClipEventHandler(id, method);
    }
    else
    {
        // need to create multiple event handlers with the same method. This is
        // necessary when "on(release, releaseOutside)" kind of handlers is used.
        for (unsigned i = 0, mask = 0x1; i < numOfEvents; mask <<= 1)
        {
            if (id.Id & mask)
            {
                ++i;
                EventId copied = id;
                copied.Id = mask;
                SetSingleClipEventHandler(copied, method);
            }
        }
    }
}


// Execute this even immediately (called for processing queued event actions).
bool    AvmCharacter::ExecuteEvent(const EventId& id)
{
    SF_ASSERT(pDispObj);

    // Keep Environment alive during any method calls!
    Ptr<InteractiveObject> thisPtr(GetDispObj());
    Environment* env = GetASEnvironment();
    // Keep target of Environment alive during any method calls!
    // note, that env->GetTarget() is not always equal to this (for buttons, for example)
    Ptr<InteractiveObject> targetPtr(env->GetTarget());

#ifdef GFX_ENABLE_KEYBOARD
    if (env && (id.Id == EventId::Event_KeyDown || id.Id == EventId::Event_KeyUp))
    {
        // We need to update listeners of KeyboardState (actually, there is only one
        // listener - GASKeyAsObject). The UpdateListeners method updates last ascii and
        // key codes in the singleton GASKeyAsObject, thus, ActionScript's Key.getAscii()
        // and Key.getCode() will return actual values when they are being used inside the
        // onClipEvent(keyDown / keyUp) event handlers.
        MovieImpl* proot = env->GetMovieImpl();
        if (proot)
        {
            KeyboardState* keyboardState = proot->GetKeyboardState(id.KeyboardIndex);
            SF_ASSERT(keyboardState);
            keyboardState->UpdateListeners(id);
        }
    }
#endif //#ifdef GFX_ENABLE_KEYBOARD

    int         handlerFoundCnt = 0;
    Value    method;

    // First, check for built-in onClipEvent() handler.
    if (HasClipEventHandler(id))
    {
        if (id.RollOverCnt == 0)
        {
            // Dispatch.
            InvokeClipEventHandlers(env, id);
            ++handlerFoundCnt;
        }
    }

    // Check for member function, it is called after onClipEvent(). 
    // In ActionScript 2.0, event method names are CASE SENSITIVE.
    // In ActionScript 1.0, event method names are CASE INSENSITIVE.    
    ASString    methodName(EventId_GetFunctionName(env->GetSC()->GetStringManager(), id));
    if (methodName.GetSize() > 0)
    {           
        if (GetMemberRaw(env->GetSC(), methodName, &method))
        {
            if (method.IsProperty())
            {
                Value actualMethod;
                method.GetPropertyValue(env, this, &actualMethod);
                method = actualMethod;
            }
            if (!method.IsNull())
            {
                // check, do we need to pass mouse index as a parameter
                if (env->CheckExtensions())
                {
                    unsigned nArgs = 0;
                    bool     handlerFound = true;
                    if (id.RollOverCnt != 0)
                    {
                        // we need to check do we need to invoke nested drag/roll/Over/Out
                        // for multiple mice. If the corresponding handler function has 2
                        // or more parameters, then it is assumed to be ready for handling
                        // multiple rollovers, so, onRollOver/Out (and onDrag...) will be
                        // invoked for each mouse. If there are less than two parameters 
                        // (or, most likely, no parameters at all) then onRollOver will be
                        // invoked only once when the first mouse cursor appears above the clip
                        // and onRollOut will be invoked once when the last cursor leave the clip.
                        FunctionRef mfref = method.ToFunction(env);
                        if (!mfref.IsNull())
                        {
                            if (mfref->GetNumArgs() < 2)
                                handlerFound = false;
                        }
                    }

                    if (handlerFound)
                    {
                        ++handlerFoundCnt;
                        if (env->IsVerboseAction())
                            env->LogAction("\n!!! ExecuteEvent started '%s' = %p for %s\n", methodName.ToCStr(), 
                            method.ToFunction(env).GetObjectPtr(), pDispObj->GetCharacterHandle()->GetNamePath().ToCStr());

                        // Enable button index for applicable events (including all aux events)
                        if ((id.Id & EventId::Event_AuxEventMask) || id.Id == EventId::Event_DragOver || 
                            id.Id == EventId::Event_DragOut || id.Id == EventId::Event_ReleaseOutside || 
                            id.Id == EventId::Event_Release || id.Id == EventId::Event_Press)
                        {
                            const ButtonEventId& btnEvtId = static_cast<const ButtonEventId&>(id);
                            env->Push(Value(btnEvtId.ButtonId));
                            ++nArgs;
                        }

                        if (id.Id == EventId::Event_RollOver || id.Id == EventId::Event_RollOut ||
                            id.Id == EventId::Event_DragOver || id.Id == EventId::Event_DragOut ||
                            id.Id == EventId::Event_DragOverAux || id.Id == EventId::Event_DragOutAux)
                        {
                            env->Push(Value(id.RollOverCnt));
                            ++nArgs;
                        }
                        else if (id.Id == EventId::Event_Press || id.Id == EventId::Event_Release ||
                            id.Id == EventId::Event_PressAux || id.Id == EventId::Event_ReleaseAux)
                        {
                            // push a negative number if keyboard was used; 0, if mouse.
                            if (id.KeyCode == Key::None)
                                env->Push(Value(0));
                            else
                                env->Push(Value(-1));
                            ++nArgs;
                        }
                        if (id.MouseIndex >= 0 || nArgs > 0)
                        {
                            env->Push(Value(id.MouseIndex));
                            ++nArgs;
                        }
                        GAS_Invoke(method, NULL, this, env, nArgs, env->GetTopIndex(), methodName.ToCStr());
                        if (nArgs > 0)
                            env->Drop(nArgs);

                        if (env->IsVerboseAction())
                            env->LogAction("!!! ExecuteEvent finished '%s' = %p for %s\n\n", methodName.ToCStr(), 
                            method.ToFunction(env).GetObjectPtr(), pDispObj->GetCharacterHandle()->GetNamePath().ToCStr());
                    }
                }
                else
                {
                    if (id.RollOverCnt == 0)
                    {
                        ++handlerFoundCnt;
                        if (env->IsVerboseAction())
                            env->LogAction("\n!!! ExecuteEvent started '%s' = %p for %s\n", methodName.ToCStr(), 
                            method.ToFunction(env).GetObjectPtr(), pDispObj->GetCharacterHandle()->GetNamePath().ToCStr());

                        GAS_Invoke0(method, NULL, this, env);

                        if (env->IsVerboseAction())
                            env->LogAction("!!! ExecuteEvent finished '%s' = %p for %s\n\n", methodName.ToCStr(), 
                            method.ToFunction(env).GetObjectPtr(), pDispObj->GetCharacterHandle()->GetNamePath().ToCStr());
                    }
                }
            }
        }
    }

    return handlerFoundCnt != 0;
}

bool    AvmCharacter::ExecuteFunction(const FunctionRef& function, const ValueArray& params)
{
    if (function.GetObjectPtr() != 0)
    {
        Value result;
        Environment* penv = GetASEnvironment();
        SF_ASSERT(penv);

        int nArgs = (int)params.GetSize();
        if (nArgs > 0)
        {
            for (int i = nArgs - 1; i >= 0; --i)
                penv->Push(params[i]);
        }
        function.Invoke(FnCall(&result, this, penv, nArgs, penv->GetTopIndex()));
        if (nArgs > 0)
        {
            penv->Drop(nArgs);
        }
        return true;
    }
    return false;
}

bool    AvmCharacter::ExecuteCFunction(const CFunctionPtr function, const ValueArray& params)
{
    if (function != 0)
    {
        Value result;
        Environment* penv = GetASEnvironment();
        SF_ASSERT(penv);

        int nArgs = (int)params.GetSize();
        if (nArgs > 0)
        {
            for (int i = nArgs - 1; i >= 0; --i)
                penv->Push(params[i]);
        }
        function(FnCall(&result, this, penv, nArgs, penv->GetTopIndex()));
        if (nArgs > 0)
        {
            penv->Drop(nArgs);
        }
        return true;
    }
    return false;
}

// set this.__proto__ = psrcObj->prototype
void    AvmCharacter::SetProtoToPrototypeOf(ObjectInterface* psrcObj)
{
    Value prototype;
    ASStringContext *psc = GetASEnvironment()->GetSC();
    if (psrcObj->GetMemberRaw(psc, psc->GetBuiltin(ASBuiltin_prototype), &prototype))
        Set__proto__(psc, prototype.ToObject(NULL));
}

void    AvmCharacter::VisitMembers(ASStringContext *psc, MemberVisitor *pvisitor, unsigned visitFlags,
                                   const ObjectInterface*) const
{
    Ptr<Object> asObj = GetASObject();
    if (asObj) 
        asObj->VisitMembers(psc, pvisitor, visitFlags, this); 
}


// Delete a member field, if not read-only. Return true if deleted.
bool    AvmCharacter::DeleteMember(ASStringContext *psc, const ASString& name)
{
    SF_ASSERT(pDispObj);
    if (IsStandardMember(name))
    {    
        StandardMember member = GetStandardMemberConstant(name);
        if (member != M_InvalidMember && (member <= M_SharedPropertyEnd))
        {
            if (GetStandardMemberBitMask() & (1<<member))
            {
                switch(member)
                {
                    case M_useHandCursor:
                        pDispObj->UndefineUseHandCursorFlag();
                        return true;

                    default:
                        return false;
                }
            }
        }
    }

    Ptr<Object> asObj = GetASObject();
    if (asObj) 
    {
        if (asObj->DeleteMember(psc, name))
            return true;
    }
    return false;
}

bool    AvmCharacter::SetMemberFlags(ASStringContext *psc, const ASString& name, const UByte flags)
{
    Ptr<Object> asObj = GetASObject();
    if (asObj) 
        return asObj->SetMemberFlags(psc, name, flags); 
    //TODO: Standard members?... (AB)
    return false;
}

bool    AvmCharacter::SetMember(Environment* penv, const ASString& name, const Value& val, const PropFlags& flags)
{    
    if (IsStandardMember(name))
    {
        StandardMember member = GetStandardMemberConstant(name);
        if (SetStandardMember(member, val, 0))
            return true;

        switch(member)
        {
        case M_topmostLevel:
            if (GetASEnvironment()->CheckExtensions())
            {
                pDispObj->SetTopmostLevelFlag(val.ToBool(GetASEnvironment())); 
                if (pDispObj->IsTopmostLevelFlagSet())
                    GetMovieImpl()->AddTopmostLevelCharacter(pDispObj);
                else
                    GetMovieImpl()->RemoveTopmostLevelCharacter(pDispObj);
            }
            break; // do not return - need to save it to members too

        case M_noAdvance:
            if (GetASEnvironment()->CheckExtensions())
            {
                bool noAdv = val.ToBool(GetASEnvironment());
                if (noAdv != pDispObj->IsNoAdvanceLocalFlagSet())
                {
                    pDispObj->SetNoAdvanceLocalFlag(noAdv);
                    pDispObj->ModifyOptimizedPlayList();
                    InteractiveObject* pparent = GetParent();
                    if (pparent && !pparent->IsNoAdvanceLocalFlagSet())
                        pDispObj->PropagateNoAdvanceLocalFlag();
                }
            }
            break; // do not return - need to save it to members too

        case M_fov:
            if (GetASEnvironment()->CheckExtensions())
            {
                pDispObj->SetFOV(val.ToNumber(GetASEnvironment()));
            }
            break;
        case M_z:
            if (GetASEnvironment()->CheckExtensions())
            {
                pDispObj->SetZ(val.ToNumber(GetASEnvironment()));
            }
            break;
        case M_zscale:
            if (GetASEnvironment()->CheckExtensions())
            {
                pDispObj->SetZScale(val.ToNumber(GetASEnvironment()));
            }
            break;
        case M_matrix3d:
            if (GetASEnvironment()->CheckExtensions())
            {
                Object *pObj = val.ToObject(penv);
                if (pObj == NULL)
                    pDispObj->Clear3D();
                else
                {
                    if (pObj->GetObjectType() != ObjectInterface::Object_Array)
                        break;
                    ObjectInterface *poi = val.ToObjectInterface(penv);
                    ArrayObject *arrayObj = static_cast<ArrayObject*>(poi);
                    SF_ASSERT(arrayObj->GetSize()==16);
                    Matrix4F m4;
                    for(int i = 0, n = arrayObj->GetSize(); i < n; ++i)
                    {
                        Value* elem = arrayObj->GetElementPtr(i);
                        if (elem && elem->IsNumber())
                            m4.Data()[i] = (float)elem->ToNumber(penv); 
                    }
                    m4.Transpose();
                    Matrix3F m3(m4);
                    pDispObj->SetMatrix3D(m3);
                }
            }
            break;
        case M_xrotation:
            if (GetASEnvironment()->CheckExtensions())
            {
                pDispObj->SetXRotation(val.ToNumber(GetASEnvironment()));
            }
            break;
        case M_yrotation:
            if (GetASEnvironment()->CheckExtensions())
            {
                pDispObj->SetYRotation(val.ToNumber(GetASEnvironment()));
            }
            break;
        case M_focusGroupMask:
            if (GetASEnvironment()->CheckExtensions())
            {
                if (val.IsUndefined())
                {
                    break;
                }
                // NOTE: If updating this logic, also update GFxValue::ObjectInterface::SetDisplayInfo
                UInt32 rval = val.ToUInt32(GetASEnvironment());
                pDispObj->PropagateFocusGroupMask(rval);
            }
            break;

        default:
            break;
        }
    }

    // a special case for setting __proto__ to a movieclip, button or textfield
    // need to set it into the character, not to the AS object, since
    // Get__proto__ is no more virtual.
    if (penv->IsCaseSensitive())
    {
        if (name == penv->GetBuiltin(ASBuiltin___proto__))
        {   
            if (val.IsSet())
                Set__proto__(penv->GetSC(), val.ToObject(NULL));
        }
    }
    else
    {
        name.ResolveLowercase();
        if (penv->GetBuiltin(ASBuiltin___proto__).CompareBuiltIn_CaseInsensitive_Unchecked(name))
        {   
            if (val.IsSet())
                Set__proto__(penv->GetSC(), val.ToObject(NULL));
        }
    }
    // Note that MovieClipObject will also track setting of button
    // handlers, i.e. 'onPress', etc.
    Object* asObj = GetASObject();
    if (asObj) 
        return asObj->SetMember(penv, name, val, flags);
    return false;
}

bool AvmCharacter::HasMember(ASStringContext *psc, const ASString& name, bool inclPrototypes)
{
    if (IsStandardMember(name))
    {
        StandardMember member = GetStandardMemberConstant(name);
        if (member != M_InvalidMember && (member <= M_SharedPropertyEnd))
        {
            if (GetStandardMemberBitMask() & (1<<member))
                return true;
        }
    }    

    Ptr<Object> asObj = GetASObject();
    if (asObj) 
        return asObj->HasMember(psc, name, inclPrototypes); 
    return false;
}

bool AvmCharacter::FindMember(ASStringContext *psc, const ASString& name, Member* pmember)
{
    Ptr<Object> obj = GetASObject();
    return (obj) ? obj->FindMember(psc, name, pmember) : false;
}

void            AvmCharacter::Set__proto__(ASStringContext *psc, Object* protoObj)
{
    Ptr<Object> obj = GetASObject();
    if (obj)
    {
        obj->Set__proto__(psc, protoObj);
    }
    pProto = protoObj;
}

/*Object*      AvmCharacter::Get__proto__()
{
    Ptr<Object> obj = GetASObject();
    return (obj) ? obj->Get__proto__() : NULL;
}*/

bool AvmCharacter::InstanceOf(Environment* penv, const Object* prototype, bool inclInterfaces) const
{
    Ptr<Object> obj = GetASObject();
    return (obj) ? obj->InstanceOf(penv, prototype, inclInterfaces) : false;
}

bool AvmCharacter::Watch(ASStringContext *psc, const ASString& prop, const FunctionRef& callback, const Value& userData)
{
    Ptr<Object> obj = GetASObject();
    return (obj) ? obj->Watch(psc, prop, callback, userData) : false;
}

bool AvmCharacter::Unwatch(ASStringContext *psc, const ASString& prop)
{
    Ptr<Object> obj = GetASObject();
    return (obj) ? obj->Unwatch(psc, prop) : false;
}

void AvmCharacter::OnFocus
    (InteractiveObject::FocusEventType event, 
    InteractiveObject* oldOrNewFocusCh, 
    unsigned controllerIdx,
    FocusMovedType)
{
    Value focusMethodVal;
    
    Environment* penv = GetASEnvironment();
    if (!penv) // penv == NULL if object is removed from its parent's display list
        return;
    ASStringContext* psc = penv->GetSC();
    ASString   focusMethodName(psc->GetBuiltin((event == InteractiveObject::SetFocus) ?
                                ASBuiltin_onSetFocus : ASBuiltin_onKillFocus));   
    if (GetMemberRaw(psc, focusMethodName, &focusMethodVal))
    {
        FunctionRef focusMethod = focusMethodVal.ToFunction(NULL);
        if (!focusMethod.IsNull())
        {
            unsigned nargs = 1;
            if (penv->CheckExtensions())
            {
                penv->Push(Number(controllerIdx));
                ++nargs;
            }
            if (oldOrNewFocusCh)
                penv->Push(Value(oldOrNewFocusCh));
            else
                penv->Push(Value::NULLTYPE);
            Value result;
            focusMethod.Invoke(FnCall(&result, Value(pDispObj), penv, nargs, penv->GetTopIndex()));
            penv->Drop(nargs);
        }
    }
}

InteractiveObject* AvmCharacter::GetTopParent(bool ignoreLockRoot) const
{
    InteractiveObject* pparent = GetParent();
    if (!pparent || !pparent->IsAVM1())
        return pDispObj; // already top parent
    return pparent->GetTopParent(ignoreLockRoot);
}

AvmCharacter* AvmCharacter::GetAvmTopParent(bool ignoreLockRoot) const
{
    if (!GetParent()->IsAVM1())
        return NULL;
    return static_cast<AvmCharacter*>(
        GetParent()->GetTopParent(ignoreLockRoot)->GetAvmObjImpl());
}

bool    AvmCharacter::IsTabable() const
{
    SF_ASSERT(pDispObj);

    if (!pDispObj->GetVisible()) return false;
    if (!pDispObj->IsTabEnabledFlagDefined())
    {
        Object* pproto = Get__proto__();
        if (pproto)
        {
            // check prototype for tabEnabled
            Value val;
            const Environment* penv = GetASEnvironment();
            if (pproto->GetMemberRaw(penv->GetSC(), penv->CreateConstString("tabEnabled"), &val))
            {
                if (!val.IsUndefined())
                    return val.ToBool(penv);
            }
        }
        return (pDispObj->ActsAsButton() || pDispObj->GetTabIndex() > 0);
    }
    else
        return pDispObj->IsTabEnabledFlagTrue();
}

// ***** GenericDisplayObj

GenericDisplayObj::GenericDisplayObj
(CharacterDef* pdef, ASMovieRootBase* pasRoot, InteractiveObject* pparent, ResourceId id)
 : DisplayObjectBase(pasRoot, pparent, id),
   pDef(static_cast<ShapeBaseCharacterDef*>(pdef))
{
    SF_ASSERT(pDef);
    SF_ASSERT(pDef->GetType() == CharacterDef::Shape);
}


DisplayObjectBase::TopMostResult 
GenericDisplayObj::GetTopMostMouseEntity(const Render::PointF &pt, TopMostDescr* pdescr) 
{   
    pdescr->pResult = GetTopMostMouseEntityDef(pDef, pt, pdescr->TestAll, pdescr->pIgnoreMC);
    if (pdescr->pResult)
        return TopMost_Found;
    return TopMost_FoundNothing;
}

bool GenericDisplayObj::PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask) const
{     
    return pDef->DefPointTestLocal(pt, hitTestMask & HitTest_TestShape, this);
}

Ptr<Render::TreeNode> GenericDisplayObj::CreateRenderNode(Render::Context& context) const
{
    return pDef->CreateTreeShape(context, GetResourceMovieDef());
}

float GenericDisplayObj::GetRatio() const
{
    if (pRenNode)
    {
        if (pRenNode->GetReadOnlyData()->GetType() == Render::TreeNode::NodeData::ET_Shape)
        {
            return static_cast<Render::TreeShape*>(pRenNode.GetPtr())->GetMorphRatio();
        }
    }
    return 0.f; 
}

void  GenericDisplayObj::SetRatio(float f)
{
    Render::TreeNode* n = GetRenderNode();
    if (n && n->GetReadOnlyData()->GetType() == Render::TreeNode::NodeData::ET_Shape)
    {
        static_cast<Render::TreeShape*>(pRenNode.GetPtr())->SetMorphRatio(f);
    }
}

}}} // SF::GFx::AS2
