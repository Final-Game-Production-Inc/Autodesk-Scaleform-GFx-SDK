/**************************************************************************

Filename    :   Video_VideoCharacter.h
Content     :   GFx video: VideoCharacterDef and VideoCharacter classes
Created     :   May, 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_GFX_VIDEOCHAR_H
#define INC_GFX_VIDEOCHAR_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "GFx/GFx_MediaInterfaces.h"
#include "GFx/GFx_CharacterDef.h"
#include "GFx/GFx_LoadProcess.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_InteractiveObject.h"

#include "Render/Render_Context.h"
#include "Render/Render_TreeNode.h"

#include "Video/Video_VideoImage.h"

namespace Scaleform { namespace GFx {

namespace AS2 {
class VideoObject;
class VideoProto;
class VideoCtorFunction;
class AvmVideoCharacter;
}

namespace AS3 {
class AvmVideoCharacter;
}

//////////////////////////////////////////////////////////////////////////
//

class AvmVideoCharacterBase : public AvmInteractiveObjBase
{
public:
    AvmVideoCharacterBase() : AvmInteractiveObjBase()
    {
        PictureWidth = PictureHeight = Deblocking = 0;
        Smoothing = true;
    }

    int  PictureWidth;
    int  PictureHeight;
    int  Deblocking;
    bool Smoothing;
};

namespace Video {

class VideoCharacter;
class VideoCharacterDef;

//////////////////////////////////////////////////////////////////////////
//

class VideoCharacterDef : public CharacterDef
{
public:
    VideoCharacterDef() : CharacterDef() { InitEmptyDef(); }
    virtual ~VideoCharacterDef() {}

    void InitEmptyDef()
    {
        NumFrames = 0;
        Width = 0; Height = 0;
        Deblocking = 0;
        Smoothing = false;
        CodecId = 2;
    }
    void Read(LoadProcess* p, TagType tagType);

    virtual unsigned GetResourceTypeCode() const { return MakeTypeCode(Resource::RT_VideoDef); }
    virtual CharacterDefType GetType() const     { return CharacterDef::Video; }

    unsigned NumFrames;
    unsigned Width;
    unsigned Height;
    UInt8    Deblocking;
    bool     Smoothing;
    UInt8    CodecId;
};

//////////////////////////////////////////////////////////////////////////
//

class VideoCharacter : public InteractiveObject
{
public:
    VideoCharacter(VideoCharacterDef* def, MovieDefImpl *pbindingDefImpl, ASMovieRootBase* pasroot,
                   InteractiveObject* parent, ResourceId id);
    ~VideoCharacter() {}

    void AttachVideoProvider(VideoProvider* pprovider);
    void CreateTexture();
    void ReleaseTexture();

    virtual CharacterDef* GetCharacterDef() const { return pDef; }
    virtual CharacterDef::CharacterDefType GetType() const { return CharacterDef::Video; }
    virtual Render::RectF GetBounds(const Render::Matrix2F &transform) const
                                                  { return transform.EncloseTransform(ViewRect); }

    SF_INLINE AvmVideoCharacterBase* GetAvmVideoCharacter()
    {
        return GetAvmObjImpl()->ToAvmVideoCharacterBase();
    }
    SF_INLINE const AvmVideoCharacterBase* GetAvmVideoCharacter() const
    {
        return GetAvmObjImpl()->ToAvmVideoCharacterBase();
    }

    Render::RectF ViewRect;

protected:
    virtual Ptr<Render::TreeNode> CreateRenderNode(Render::Context& context) const;
    Render::TreeNode* RecreateRenderNode() const;

    VideoCharacterDef*      pDef;
    WeakPtr<VideoProvider>  pVideoProvider;
    Ptr<VideoImage>         pTextureImage;
};

}}} // Scaleform::GFx::Video

#endif // GFX_ENABLE_VIDEO

#endif // INC_GFX_VIDEOCHAR_H
