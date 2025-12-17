/**************************************************************************

PublicHeader:   Render
Filename    :   Render_TextMeshProvider.h
Content     :   
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_TextMeshProvider_H
#define INC_SF_Render_TextMeshProvider_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_List.h"
#include "Render_Types2D.h"
#include "Render_Containers.h"
#include "Render_GlyphParam.h"
#include "Render_MatrixPool.h"
#include "Render_Primitive.h"
#include "Render_MeshCache.h"
#include "Render_TextLayout.h"
#include "Render_Font.h"
#include "Render_TessDefs.h"
#include "Render_Matrix4x4.h"
#include "Render_MeshKey.h"

namespace Scaleform { namespace Render {


class Texture;
class TextMeshProvider;
struct TextMeshEntry;
class GlyphCache;
struct GlyphNode;
class TextPrimitiveBundle;
class BundleEntry;
struct TextNotifier;


//------------------------------------------------------------------------
struct VectorGlyphKey
{
    FontCacheHandle* pFont;
    UInt16           GlyphIndex;
    UInt8            HintedVector;
    UInt8            HintedRaster;
    UInt16           Flags;
    UInt16           Outline;

    VectorGlyphKey() : 
        pFont(0), GlyphIndex(0), HintedVector(0), HintedRaster(0), Flags(0), Outline(0)
    {}

    VectorGlyphKey(FontCacheHandle* font, unsigned glyphIndex, 
                   unsigned hintedVector, unsigned hintedRaster, 
                   unsigned flags, unsigned outline) :
        pFont(font), GlyphIndex((UInt16)glyphIndex), 
        HintedVector((UInt8)hintedVector), HintedRaster((UInt8)hintedRaster), 
        Flags((UInt16)flags & (Font::FF_Bold | Font::FF_Italic)),
        Outline((UInt16)outline){}

    UPInt GetHashValue() const 
    {
        return (((UPInt)pFont) >> 6) ^ (UPInt)pFont ^ 
                 (UPInt)GlyphIndex ^
                 (UPInt)HintedVector ^
                 (UPInt)HintedRaster ^
                 (UPInt)Flags ^
                 (UPInt)Outline;
    }

    bool operator == (const VectorGlyphKey& other) const 
    {
        return  pFont       == other.pFont && 
                GlyphIndex  == other.GlyphIndex &&
                HintedVector== other.HintedVector &&
                HintedRaster== other.HintedRaster &&
                Flags       == other.Flags &&
                Outline     == other.Outline;
    }
};

//------------------------------------------------------------------------
struct VectorGlyphShape : public MeshProvider_KeySupport, public ListNode<VectorGlyphShape>
{
    VectorGlyphKey      Key;
    Ptr<GlyphShape>     pShape;
    Ptr<GlyphRaster>    pRaster;
    RectF               Bounds;
    GlyphCache*         pCache;

    VectorGlyphShape() 
    {}
    ~VectorGlyphShape()
    {}

    VectorGlyphShape(GlyphCache* cache) : pCache(cache) {}

    virtual void    OnEvict(MeshBase *mesh);

    virtual bool    GetData(MeshBase *mesh, VertexOutput* out, unsigned meshGenFlags);
    virtual RectF   GetIdentityBounds() const;
    virtual RectF   GetBounds(const Matrix2F& m) const;
    virtual RectF   GetCorrectBounds(const Matrix2F& m, float morphRatio, 
                                     StrokeGenerator* gen, const ToleranceParams* tol) const;

    virtual bool    HitTestShape(const Matrix2F& m, float x, float y, float morphRatio,
                                 StrokeGenerator* gen, const ToleranceParams* tol) const;

    virtual unsigned    GetLayerCount() const;
    
    virtual unsigned    GetFillCount(unsigned layer, unsigned meshGenFlags) const;
    virtual void        GetFillData(FillData* data, unsigned layer,
                                    unsigned fillIndex, unsigned meshGenFlags);

    virtual void        GetFillMatrix(MeshBase *mesh, Matrix2F* matrix, unsigned layer,
                                      unsigned fillIndex, unsigned meshGenFlags);


    UPInt GetHashValue() const { return Key.GetHashValue(); }
    bool operator == (const VectorGlyphShape& other) const { return Key == other.Key; }

    struct PtrHashFunctor
    {    
        UPInt  operator()(const VectorGlyphShape* shape) const
        { return shape->Key.GetHashValue(); }

        UPInt  operator()(const VectorGlyphKey& key) const
        { return key.GetHashValue(); }
    };


    bool generateNullVectorMesh(VertexOutput* verOut);

};

inline bool operator == (const VectorGlyphShape* shape, const VectorGlyphKey& key)
{
    return  shape->Key == key;
}





//------------------------------------------------------------------------
struct GlyphRasterEnt
{
    float            Coord[4];
    const GlyphNode* pGlyph;
};

struct GlyphTextureEnt
{
    float               Coord[4];
    const TextureGlyph* pGlyph;
};

struct GlyphVectorEnt
{
    Font*   pFont;
    UInt16  GlyphIndex;
    UInt16  Flags;
    float   FontSize;
    float   x, y;
};

struct BackgroundEnt
{
    float   Coord[4];
    UInt32  BorderColor;
};

struct SelectionEnt
{
    float   Coord[4];
};

struct UnderlineEnt
{
    unsigned Style;
    float    x,y;
    float    Len;
};

struct TextImageEnt
{
    float   Coord[4];
    Image*  pImage;
};

struct MaskEnt
{
    float   Coord[4];
};

union TextEntryUnion
{
    GlyphRasterEnt  RasterData;
    GlyphTextureEnt PackedData;
    GlyphVectorEnt  VectorData;
    BackgroundEnt   BackgroundData;
    SelectionEnt    SelectionData;
    UnderlineEnt    UnderlineData;
    TextImageEnt    ImageData;
    MaskEnt         MaskData;
};

// A structures used for temporary data created using a linear heap
//------------------------------------------------------------------------
struct TmpTextMeshEntry
{
    UInt16              LayerType;  // Determines the order of drawing. 
    UInt16              TextureId;
    unsigned            EntryIdx;   // Entry index required for stable sorting 
    UInt32              mColor;
    PrimitiveFill*      pFill;      // Primitive Fill pointer for proper image sorting
    TextEntryUnion      EntryData;
};

//------------------------------------------------------------------------
struct TmpTextMeshLayer
{
    TextLayerType       Type;
    unsigned            Start;
    unsigned            Count;
    PrimitiveFill*      pFill;      // Primitive Fill pointer for proper image sorting
};

// Permanent structures stored in regular arrays.
//------------------------------------------------------------------------
struct TextMeshEntry
{
    UInt16              LayerType;  // Determines the order of drawing. 
    UInt16              TextureId;
    UInt32              mColor;
    Ptr<PrimitiveFill>  pFill;      // Primitive Fill pointer for proper image sorting
    TextEntryUnion      EntryData;
};

//------------------------------------------------------------------------
struct TmpTextStorage
{
    Render::LinearHeap                         LHeap;
    Render::ArrayPaged<TmpTextMeshEntry, 6, 4> Entries;
    Render::ArrayPaged<TmpTextMeshLayer, 4, 4> Layers;

    TmpTextStorage(MemoryHeap* heap) : 
        LHeap(heap),
        Entries(&LHeap),
        Layers(&LHeap)
    {}
};

//------------------------------------------------------------------------
struct TextMeshLayer
{
    TextLayerType           Type;
    unsigned                Start;
    unsigned                Count;
    Ptr<Mesh>               pMesh;
    Ptr<MeshKey>            pMeshKey;   // Used for vector glyphs only
    Ptr<VectorGlyphShape>   pShape;
    HMatrix                 M;
    Ptr<PrimitiveFill>      pFill;      // Primitive Fill pointer for proper image sorting
    float                   SizeScale;
};


struct GlyphRunData
{
    TextFieldParam      Param;
    RectF               Bounds;
    Font*               pFont;
    FontCacheHandle*    pFontHandle;
    float               FontSize;
    unsigned            VectorSize;
    unsigned            RasterSize;
    float               NomWidth;
    float               NomHeight;
    float               TexHeight;
    UInt32              mColor;
    float               NewLineX;
    float               NewLineY;
    RectF               GlyphBounds;
    Matrix2F            DirMtx;
    Matrix2F            InvMtx;
    float               HeightRatio;
    unsigned            HintedNomHeight;
    const ShapeDataInterface* pShape;
    const GlyphRaster*        pRaster;
};


class Viewport;

//------------------------------------------------------------------------
// TextMeshProvider is associated with one text field and holds its cached
// glyph batch records and their layers with meshes.
//  - Can NOT be RefCounted due to MeshProvider -> Mesh dependency,
//    for now, we just store it as value.
class TextMeshProvider : public ListNode<TextMeshProvider>, public MeshProvider
{
public:
    enum BatchFlags
    {
        BF_Valid           = 0x0001,
        BF_InUseList       = 0x0002,
        BF_InPinList       = 0x0004,

        BF_Clip            = 0x0008,
        BF_Creating        = 0x0010,
        BF_Created         = 0x0020,

        BF_HasVectorGlyphs = 0x0040,
        BF_HasUnderlines   = 0x0080,
        BF_HasMask         = 0x0100,

        BF_DistField       = 0x0200,
    };

    TextMeshProvider(GlyphCache* cache);
    ~TextMeshProvider();

    // Add entries
    void Clear();
    void ClearEntries();

    // Access elements
    const TextMeshLayer& GetLayer(unsigned idx) const { return Layers[idx]; }
          TextMeshLayer& GetLayer(unsigned idx)       { return Layers[idx]; }
    const TextMeshEntry& GetEntry(const TextMeshLayer& layer, unsigned idx) const
    {
        return Entries[layer.Start + idx];
    }

    // GlyphQueue and MeshCache interaction functions
    //-------------------------------
    void AddNotifier(TextNotifier* notifier);
    void SetInUseList(bool f) { if(f) Flags |= BF_InUseList; else Flags &= ~BF_InUseList; }
    void SetInPinList(bool f) { if(f) Flags |= BF_InPinList; else Flags &= ~BF_InPinList; }
    void ClearInList()        { Flags &= ~(BF_InUseList | BF_InPinList); } 

    bool IsInUseList() const  { return (Flags & (BF_InUseList | BF_InPinList)) == BF_InUseList; }
    bool IsInPinList() const  { return (Flags & (BF_InUseList | BF_InPinList)) == BF_InPinList; }
    bool IsInList()    const  { return (Flags & (BF_InUseList | BF_InPinList)) != 0; }

    bool HasVectorGlyphs() const { return (Flags & BF_HasVectorGlyphs) != 0; }
    bool HasDistanceField() const { return (Flags & BF_DistField) != 0; }

    void PinSlots();
    void UnpinSlots();
    void OnEvictSlots();

    bool IsInUse() const
    {
        return GetMeshUseStatus() >= MUS_InUse;
    }
    void AddToInUseList();
    Fence* GetLatestFence() const;

    bool NeedsUpdate(const HMatrix& m, const Matrix4F& m4, const Viewport& vp, 
                     const TextFieldParam& param) const;

    bool CreateMeshData(const TextLayout* layout, Renderer2DImpl* ren, 
                        const HMatrix& m, const Matrix4F& m4, const Viewport& vp, 
                        unsigned meshGenFlags);

    // MeshProvder is crated by CreateMeshData; reset by Clear.
    bool IsCreated() const { return (Flags & BF_Created) != 0; }

    float GetHeightRatio() const { return HeightRatio; }

    //-------------------------------
    // MeshProvider Interface
    virtual unsigned    GetLayerCount() const { return (unsigned)Layers.GetSize(); }
    virtual bool        GetData(MeshBase *mesh, VertexOutput* out, unsigned meshGenFlags);

    virtual unsigned    GetFillCount(unsigned layer, unsigned meshGenFlags) const
    { SF_UNUSED2(layer, meshGenFlags); return 1; }

    virtual void        GetFillData(FillData* data, unsigned layer,
                                    unsigned fillIndex, unsigned meshGenFlags);

    virtual void        GetFillMatrix(MeshBase *mesh, Matrix2F* matrix, unsigned layer,
                                      unsigned fillIndex, unsigned meshGenFlags)
    {
        matrix->SetIdentity();
        SF_ASSERT(0); SF_UNUSED5(mesh, matrix, layer, fillIndex, meshGenFlags);
    }
    //-------------------------------


    // Pin layer batches; they are pinned while layers are being prepared,
    // they are unpinned once prepare is done (at which point they are held by Mesh status).
    void Pin()      { PinCount++; }
    void Unpin()    { PinCount--; }

    TextPrimitiveBundle* GetBundle() const { return pBundle; }
    void ClearBundle()                     { SetBundle(0, 0);}
    void SetBundle(TextPrimitiveBundle* bundle, BundleEntry* entry)
    {
        pBundle = bundle;
        pBundleEntry = entry;
    }

    MeshUseStatus  GetMeshUseStatus() const;
    bool           HasMask() const { return (Flags & BF_HasMask) != 0; }

    // If batch uses masking, this is the mask clear bounds used
    // for cumulative clear; it should include the area of all shapes
    // and masks that stick out.
    // This must be a cached value, so that it can be searched for during removal.
    HMatrix UpdateMaskClearBounds(HMatrix viewMat);
    HMatrix GetMaskClearBounds() const;

    static unsigned CalcVectorParams(TextMeshLayer& layer, const TextMeshEntry& ent, 
                                     const Matrix2F& scalingMtx, 
                                     float sizeScale, const HMatrix& m, 
                                     Renderer2DImpl* ren, unsigned meshGenFlags, float* keyData);

private:
    struct CmpEntries
    {
        const GlyphCache* pCache;
        CmpEntries(GlyphCache* c) : pCache(c) {}
        bool operator() (const TmpTextMeshEntry& a, const TmpTextMeshEntry& b) const;
    };

    void addLayer(TmpTextStorage& storage, TextLayerType type, unsigned start, unsigned count);
    void addLayer(TmpTextStorage& storage, UPInt start, UPInt end);

    // Vector glyph (shapes)
    void addVectorGlyph(TmpTextStorage& storage, 
                        UInt32 color, FontCacheHandle* font, UInt16 glyphIndex, 
                        UInt16 flags, float fontSize, float x, float y);
    
    // Raster dynamic cache glyph
    void addRasterGlyph(TmpTextStorage& storage, TextLayerType type, GlyphRunData& data, 
                        UInt32 color, GlyphNode* node, float screenSize, bool snap, float stretch);

    void addTextureGlyph(TmpTextStorage& storage, const TextureGlyph* tgl, GlyphRunData& data, UInt32 color);

    // Auxiliary entries
    void addBackground(TmpTextStorage& storage, UInt32 color, UInt32 borderColor, const RectF& rect);
    void addSelection(TmpTextStorage& storage, UInt32 color, const RectF& rect);
    void addCursor(TmpTextStorage& storage, UInt32 color, const RectF& rect);
    void addUnderline(TmpTextStorage& storage, UInt32 color, TextUnderlineStyle style, float x, float y, float len);
    void addImage(TmpTextStorage& storage, const GlyphRunData& data, Image* img, 
                  float scaleX, float scaleY, float baseLine, bool snap);
    void addMask(TmpTextStorage& storage);

    void sortEntries(TmpTextStorage& storage);     // Sort entries and and count layers

    // Create mesh entries functions
    static float calcHeightRatio(const HMatrix& m, const Matrix4F& m4, const Viewport& vp);

    bool addGlyph(TmpTextStorage& storage, GlyphRunData& data, unsigned glyphIndex, 
                  bool fauxBold, bool fauxItalic, bool snap, unsigned meshGenFlags);

    void createVectorGlyph(unsigned layerIdx, Renderer2DImpl* ren, const HMatrix& m, unsigned meshGenFlags);

    struct VertexCountType
    {
        unsigned VStart;
        unsigned IStart;
        VertexCountType() : VStart(0), IStart(0) {}
    };

    // Mesh generation functions
    void setMeshData(TessBase* tess, VertexOutput* verOut, const UInt32* colors, VertexCountType& verCount);
    bool generateNullVectorMesh(VertexOutput* verOut);
    bool generateRectangle(Renderer2DImpl* ren, VertexOutput* verOut, const Matrix2F& mtx, const float* coord, 
                           UInt32 fillColor, UInt32 borderColor, unsigned meshGenFlags);

    bool generateSelection(Renderer2DImpl* ren, VertexOutput* verOut, const TextMeshLayer& layer, 
                           const Matrix2F& mtx, unsigned meshGenFlags);

    bool clipGlyphRect(RectF& chr, RectF& tex) const;
    bool generateRasterMesh(VertexOutput* verOut, const TextMeshLayer& layer);
    bool generatePackedMesh(VertexOutput* verOut, const TextMeshLayer& layer);
    bool generateImageMesh (VertexOutput* verOut, const TextMeshLayer& layer);

    bool generateUnderlines(Renderer2DImpl* ren, VertexOutput* verOut, const TextMeshLayer& layer, 
                            const Matrix2F& mtx, unsigned meshGenFlags);

    bool generateMask(VertexOutput* verOut, const TextMeshLayer& layer);

    float snapX(const GlyphRunData& data) const;
    float snapY(const GlyphRunData& data) const;

    GlyphCache*                 pCache;
    unsigned                    Flags;
    ArrayDH_POD<TextNotifier*>  Notifiers;  // TO DO: Consider more efficient memory management
    ArrayDH    <TextMeshEntry>  Entries;
    ArrayDH    <TextMeshLayer>  Layers;

    // Data for TextPrimitiveBundle
    UPInt                PinCount;    
    // TBD: Pointer back to TextPrimitiveBundle to detect membership?
    TextPrimitiveBundle* pBundle;
    BundleEntry*         pBundleEntry;
    // Flag tracking creation status of MeshProvider object.
    float                HeightRatio;
    RectF                ClipBox;
    RectF                ClearBox;
    RectF                Bounds;
    Renderer2DImpl*      pRenderer;
    HMatrix              ClearBounds;

};



}} // Scaleform::Render

#endif
