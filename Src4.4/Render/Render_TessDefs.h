/**************************************************************************

PublicHeader:   Render
Filename    :   Render_TessDefs.h
Content     :   Definitions and base classes for tessellation
Created     :
Authors     :   Maxim Shemanarev

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_TessDefs_H
#define INC_SF_Render_TessDefs_H

#include "Render_Math2D.h"
#include "Render_Matrix2x4.h"
#include "Render_Containers.h"

namespace Scaleform { namespace Render {

//-----------------------------------------------------------------------
enum TessStyleFlags
{
    TessStyleTranslucent = 0x0001, // Don't change the constants
    TessStyleOpaque      = 0x0002,
    TessStyleFactorHalf  = 0x0004,
    TessStyleFactorOne   = 0x0008,
    TessStyleMixColors   = 0x0010,
    TessStyleUseColor2   = 0x0020,
    TessStyleComplex     = 0x8000
};

//-----------------------------------------------------------------------
inline unsigned TessGetAlpha(unsigned flags)         { return (flags     ) & 3; }
inline unsigned TessGetFactor(unsigned flags)        { return (flags >> 2) & 3; }
inline unsigned TessGetUsedStyle(unsigned flags)     { return (flags & TessStyleUseColor2) >> 5; }
inline unsigned TessStyleMixesColors(unsigned flags) { return (flags & TessStyleMixColors); }
inline unsigned TessStyleIsComplex(unsigned flags)   { return (flags & TessStyleComplex); }

//-----------------------------------------------------------------------
struct TessMesh
{
    unsigned MeshIdx;
    unsigned Style1;
    unsigned Style2;
    unsigned Flags1;
    unsigned Flags2;
    unsigned StartVertex;
    unsigned VertexCount;
};

//-----------------------------------------------------------------------
struct TessVertex
{
    CoordType x, y;
    unsigned  Idx;
    UInt16    Styles[2];
    UInt16    Flags;
    UInt16    Mesh;
};


//-----------------------------------------------------------------------
struct VertexBasic
{
    float x,y;
};

//-----------------------------------------------------------------------
struct PathBasic
{
    unsigned Start, Count;
};

//-----------------------------------------------------------------------
class TessBase
{
public:
    virtual ~TessBase() {}

    virtual void        Clear() = 0;
    virtual CoordType   GetLastX() const = 0;
    virtual CoordType   GetLastY() const = 0;
    virtual void        AddVertex(CoordType x, CoordType y) = 0;
    virtual void        ClosePath() = 0;
    virtual void        FinalizePath(unsigned leftStyle, unsigned rightStyle,
                                     bool leftComplex, bool rightComplex) = 0;

    virtual void        Transform(const Matrix2F& m) = 0;
    virtual Matrix2F    StretchTo(float x1, float y1, float x2, float y2) = 0;

    virtual unsigned    GetVertexCount() const = 0;
    virtual unsigned    GetMeshCount() const = 0;
    virtual unsigned    GetMeshVertexCount(unsigned meshIdx)  const = 0;
    virtual unsigned    GetMeshTriangleCount(unsigned meshIdx) const = 0;
    virtual void        GetMesh(unsigned meshIdx, TessMesh* mesh) const = 0;
    virtual unsigned    GetVertices(TessMesh* mesh, TessVertex* vertices, unsigned num) const = 0;
    virtual void        GetTrianglesI16(unsigned meshIdx, UInt16* idx, unsigned start, unsigned num) const = 0;
};

//-----------------------------------------------------------------------
class VertexPath : public TessBase
{
public:
    typedef float CoordType;
    typedef VertexBasic VertexType;

    VertexPath(LinearHeap* heap) : Vertices(heap), Paths(heap) {}

    virtual void        Clear();
    virtual CoordType   GetLastX() const { return Vertices.Back().x; }
    virtual CoordType   GetLastY() const { return Vertices.Back().y; }
    virtual void        AddVertex(CoordType x, CoordType y);
    virtual void        ClosePath();
    virtual void        FinalizePath(unsigned, unsigned, bool, bool);

    virtual void        Transform(const Matrix2F&) {}
    virtual Matrix2F    StretchTo(float, float, float, float) { return Matrix2F(); }

    virtual unsigned    GetVertexCount() const { return 0; }
    virtual unsigned    GetMeshCount() const { return 0; }
    virtual unsigned    GetMeshVertexCount(unsigned) const { return 0; }
    virtual unsigned    GetMeshTriangleCount(unsigned) const { return 0; }
    virtual void        GetMesh(unsigned, TessMesh*) const {}
    virtual unsigned    GetVertices(TessMesh*, TessVertex*, unsigned) const { return 0; }
    virtual void        GetTrianglesI16(unsigned, UInt16*, unsigned, unsigned) const {}


    UPInt             GetNumPaths()           const { return Paths.GetSize(); }
    const PathBasic&  GetPath(unsigned idx)   const { return Paths[idx]; }
    UPInt             GetNumVertices()        const { return Vertices.GetSize(); }
    const VertexType& GetVertex(unsigned idx) const { return Vertices[idx]; }

    void Scale(float sx, float sy)
    {
        for(UPInt i = 0; i < Vertices.GetSize(); ++i)
        {
            Vertices[i].x *= sx;
            Vertices[i].y *= sy;
        }
    }

private:
    ArrayPaged<VertexBasic, 4, 16>  Vertices;
    ArrayPaged<PathBasic, 2, 4>     Paths;
    unsigned                        LastVertex;
};


//------------------------------------------------------------------------
class TransformerBase
{
public:
    virtual ~TransformerBase() {}
    virtual void Transform(float* x, float* y) const = 0;
    virtual float GetScale() const = 0;
    virtual float GetXScale() const = 0;
    virtual float GetYScale() const = 0;
};


}} // Scaleform::Render

#endif
