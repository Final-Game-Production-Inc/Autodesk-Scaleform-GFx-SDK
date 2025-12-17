/**************************************************************************

PublicHeader:   Render
Filename    :   Render_Viewport.h
Content     :   Viewport class definition
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_Viewport_H
#define INC_SF_Render_Viewport_H

#include "Render_Types2D.h"

namespace Scaleform { namespace Render {



// HAL Viewport.
//
// NOTE: Scale and AspectRatio variables have been removed from HW viewport 
//       they should be applied as a part of Flash scaling by ViewMatrix,
//       which can be a part of RootNode and converts Flash coordinate system
//       into the pixel-based one used by Render2D.

class Viewport
{
public:
    int      BufferWidth, BufferHeight;
    int      Left, Top;
    int      Width, Height;
    // Scissor left & top are in same coordinate system as Left & Top (in buffer's ones)
    int      ScissorLeft,  ScissorTop; 
    int      ScissorWidth, ScissorHeight;    
    unsigned Flags;

    enum
    {
        View_IsRenderTexture    = 1,

        // Support blending into the destination through a render target texture.
        // Only blend/alpha and add modes are supported. Render the texture with
        // source blend factor 1, dest blend factor 1-Sa.
        View_AlphaComposite     = 2,
        View_UseScissorRect     = 4,

        // Don't set viewport/scissor state on device. Currently only supported by some
        // Renderer implementations.
        View_NoSetState         = 8,

        // Defines orientation of the viewport coordinate system within the
        // buffer. This means that BufferWidth/Height remain the same for both orientations,
        // but Top, Left, Width, Height (ans scissor coordinates) change.
        // So, to rotate 480x960 view to landscape, would specify:
        // { BufferWidth,Height = 480,960; Left,Top = 0,0; Width,Height = 960,480, Flags = View_Orientation_R90 }
        View_Orientation_Normal = 0x00,
        View_Orientation_R90    = 0x10,
        View_Orientation_180    = 0x20,
        View_Orientation_L90    = 0x30,
        View_Orientation_Mask   = 0x30,

		// For stereo in display hardware only; uses the same size buffer but half for each eye.
		View_Stereo_SplitV		= 0x40,
		View_Stereo_SplitH		= 0x80,
		View_Stereo_AnySplit    = 0xc0,

        View_RenderTextureAlpha = View_IsRenderTexture|View_AlphaComposite,

        // The viewport should offset by a half-pixel to be on the pixel centers.
        View_HalfPixelOffset    = 0x100,
        
        // The viewport should not invoke a clear operation. Useful if a customer knows that his or her
        // content will have a fully opaque backdrop of some sort and wants to avoid an extra 100% overdraw.
        View_NoClear            = 0x200
    };

    Viewport()
    { 
        BufferWidth = 0; BufferHeight = 0;
        Left = Top = 0; Width = Height = 1;
        ScissorLeft = ScissorTop = ScissorWidth = ScissorHeight = 0; Flags = 0;
    }
    Viewport(int bw, int bh, int left, int top, int w, int h, int scleft, int sctop, int scw, int sch, unsigned flags = 0)
    { 
        BufferWidth = bw; BufferHeight = bh;
        Left = left; Top = top; Width = w; Height = h; Flags = flags | View_UseScissorRect;
        SetScissorRect(scleft, sctop, scw, sch);
    }    
    Viewport(int bw, int bh, int left, int top, int w, int h, unsigned flags = 0)
    { 
        BufferWidth = bw; BufferHeight = bh; Left = left; Top = top; Width = w; Height = h; Flags = flags; 
        ScissorLeft = ScissorTop = ScissorWidth = ScissorHeight = 0;
    }
    Viewport(Size<int> bufferSize, Rect<int> view, unsigned flags = 0)
    {
        BufferWidth = bufferSize.Width; 
        BufferHeight = bufferSize.Height; 
        Left = view.x1; 
        Top = view.y1; 
        Width = view.x2-view.x1; 
        Height = view.y2-view.y1; 
        Flags = flags; 
        ScissorLeft = ScissorTop = ScissorWidth = ScissorHeight = 0;
    }
    Viewport(const Viewport& src)
    { 
        BufferWidth = src.BufferWidth; BufferHeight = src.BufferHeight;
        Left = src.Left; Top = src.Top; Width = src.Width; Height = src.Height; Flags = src.Flags; 
        ScissorLeft = src.ScissorLeft; ScissorTop = src.ScissorTop; ScissorWidth = src.ScissorWidth; ScissorHeight = src.ScissorHeight;         
    }

    void SetScissorRect(int scleft, int sctop, int scw, int sch)
    {
        ScissorLeft = scleft; ScissorTop = sctop; ScissorWidth = scw; ScissorHeight = sch;
        Flags |= View_UseScissorRect;
    }

    bool    IsValid() const { return (BufferWidth != 0) && (BufferHeight != 0); }

    // Calculates clipped viewport rectangle, together with dx and dy adjustments that
    // are made for the clipping to be done. Returns false if the resulting clipping
    // rectangle is empty (so no rendering can take place).
    // Clipping includes:
    //  1. Ensuring that {Left, Top, Width, Height} fall within Buffer size.
    //  2. Clipping to UseScissorRect if needed.
    template <typename IntType>
    bool GetClippedRect(Rect<IntType> *prect, bool orient = false) const
    {
        IntType w, h;
        if ( orient )
        {
            switch(Flags & View_Orientation_Mask)
            {
            default:
                w = Width;
                h = Height;
                break;
            case View_Orientation_R90:
            case View_Orientation_L90:
                w = Height;
                h = Width;
                break;
            }
        }
        else
        {
            w = Width;
            h = Height;
        }

        if (!Rect<IntType>(Left, Top, Left+w, Top+h)
            .IntersectRect(prect, Rect<IntType>(BufferWidth, BufferHeight)))
        {
            prect->Clear();
            return false;
        }

        if (Flags & View_UseScissorRect)
        {
            Rect<IntType> scissor(ScissorLeft, ScissorTop,
                ScissorLeft + ScissorWidth, ScissorTop + ScissorHeight);
            if (!prect->IntersectRect(prect, scissor))
            {
                prect->Clear();
                return false;
            }
        }
        return true;
    }

    template <typename IntType>
    bool GetClippedRect(Rect<IntType> *prect, IntType* pdx, IntType* pdy) const
    {
        if (!GetClippedRect(prect))
            return false;
        *pdx = prect->x1 - Left;
        *pdy = prect->y1 - Top;
        return true;
    }

    // Obtain rectangle that will be used for calling, in viewport-relative
    // coordinates (floating-point).
    bool GetCullRectF(RectF* prect, bool orient = true) const
    {
        Rect<int> clipRect;
        if (!GetClippedRect(&clipRect, orient))
            return false;
        prect->SetRect((float)(clipRect.x1 - Left), (float)(clipRect.y1 - Top),
                       (float)(clipRect.x2 - Left), (float)(clipRect.y2 - Top));
        return true;
    }    

    // Utility function - scales a bounds rectangle (assumed to be in clip-space) into viewport space.
    template< typename T >
    static RectF ScaleToViewport( T left, T top, T width, T height, RectF & bounds )
    {
        RectF viewportBounds;
        viewportBounds.x1 = scaleX((int)left, (int)width,  bounds.x1);
        viewportBounds.y1 = scaleY((int)top,  (int)height, bounds.y2);
        viewportBounds.x2 = scaleX((int)left, (int)width,  bounds.x2);
        viewportBounds.y2 = scaleY((int)top,  (int)height, bounds.y1);     // negate top and bottom to invert Y
        return viewportBounds;
    }

    static float scaleX(int left, int width, float x)
    {
        SF_UNUSED(left);
        return width * (x+1) / 2.f;
    }

    static float scaleY(int top, int height, float y)
    {
        SF_UNUSED(top);
        return height * (-y+1) / 2.f;
    }

    bool operator == (const Viewport& other) const
    {
        return (BufferWidth == other.BufferWidth) && (BufferHeight == other.BufferHeight) &&
               (Left == other.Left) && (Top == other.Top) &&
               (Width == other.Width) && (Height == other.Height) &&
               (ScissorLeft == other.ScissorLeft) && (ScissorTop == other.ScissorTop) &&
               (ScissorWidth == other.ScissorWidth) && (ScissorHeight == other.ScissorHeight) &&
               (Flags == other.Flags);
    }

    bool operator != (const Viewport& other) const
    {
        return !operator == (other);
    }

	void SetStereoViewport(unsigned display);
};

}} // Scaleform::Render

#endif
