/**************************************************************************

PublicHeader:   Render
Filename    :   Render_DrawableImage.h
Content     :   DrawableImage is a software-accessible render-target image
                virtualization used to implement BitmapData
Created     :   September 2011
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_DrawableImage_H
#define INC_SF_Render_DrawableImage_H

#include "Render/Render_Image.h"
#include "Render/Render_TreeNode.h"
#include "Render/Render_ThreadCommandQueue.h"
#include "Render/Render_Buffer.h"
#include "Render/Render_Context.h"
#include "Render/Render_Sync.h"

namespace Scaleform { namespace Render {

class DICommandQueue;

// One DrawableImageContext may be shared between multiple DrawableImages.

class DrawableImageContext : public RefCountBase<DrawableImageContext, Stat_Default_Mem>,
                             public ContextCaptureNotify
{
    friend class DrawableImage;
    friend class DICommandQueue;

    DrawableImageContext* getThis() { return this; }

public:

    DrawableImageContext(Context* controlContext,
						 ThreadCommandQueue* commandQueue,
                         const Interfaces& i = Interfaces());
    ~DrawableImageContext();

    void                SetQueue(ThreadCommandQueue* queue) { pRTCommandQueue = queue; }
    ThreadCommandQueue* GetQueue() const { return pRTCommandQueue; }
    Context*            GetControlContext() const { return pControlContext; }

    // Fills in render interfaces on the render thread.
    // Current logic will use query the render thread for any non-default values.
    void GetRenderInterfacesRT(Interfaces* p);

    // ContextCaptureNotify Implementation
    virtual void OnCapture();
    virtual void OnNextCapture(ContextImpl::RenderNotify* notify);
    virtual void OnShutdown(bool waitFlag);
    virtual void ExecuteNextCapture(ContextImpl::RenderNotify* notify);

    void AddTreeRootToKillList( TreeRoot* proot );
	bool IsShutdownComplete() const { return !RContext || RContext->IsShutdownComplete(); }

protected:

    void processTreeRootKillList();

    // Cloned tree nodes are allocated from here on Draw()
    Context*     RContext;
    char         RContextBacking[sizeof(Context)];

    // Associated context that controls the render thread frames
    // We should try to execute frame rendering when this context
    // calls NextCapture() on a captured frame of modifications
    //  -- in other words, events in this context affect our render queue processing.
    Context*    pControlContext;

    ThreadCommandQueue* pRTCommandQueue;

    // Manages a list of tree roots that need to be released (by the Advance thread)
    Lock*                   getLock() { return &TreeRootKillListLock; }
    Lock                    TreeRootKillListLock;
    ArrayLH<TreeRoot*>      TreeRootKillList;
    Ptr<DICommandQueue>     Queue;

private:
    // Default interface to be used if none are provided by render thread.
    // TBD: We can provide an extra interface on top that for customizable lookup.
    Interfaces  IDefaults;

};

class DIPixelProvider;
struct DICommand;


//--------------------------------------------------------------------
// ***** DrawableImage

// DrawableImage is an image virtualization object that combines software
// pixel data access with hardware rendering. DrawableImage object is used
// primarily to implement Flash BitmapData interface, but can also be useful
// when Advance thread access to rendered content is needed.
//
// One of key features of DrawableImage is that is allows rendering of a
// render tree on a separate thread, while feeding this data back to advance
// thread for processing. Since such communication can involve significant 
// stalling, several important use cases are optimized:
//  a) Draw() to DrawableImage one or few files, using it as a render
//     source only for other tree rendering. This should queue up render
//     thread processing with no stalls and read-back communication necessary.
//  b) Modify image with either draw() or software to initialize content,
//     then do multiple HitTest / pixel access operations that are fast.
// Both of these cases are likely to be heavily used in Flash games.
// In general, DrawableImage tries to minimize render thread stalls through
// command queuing and minimizing render thread transitions.
//
// Due to high overhead of thread communication, DrawableImage commands 
// that don't return a value may be queued up for delayed group processing.
//


class DrawableImage : public Image, public ListNode<DrawableImage>
{    
    friend class  Render::Texture;
    friend class  DICommandSet;
    friend class  DICommandQueue;
    friend struct DICommand;
    friend struct DICommand_CreateTexture;
    friend struct DICommand_Map;
    friend struct DICommand_Unmap;
public:

    enum DrawableImageStateFlags
    {
        DIState_Mapped      = 0x00000001,   // Set if the image's texture data is mapped from GPU data (in RT).
        DIState_MappedRead  = 0x00000002,   // Set if the image's texture data is mapped and readable only (ie. not writable).
        DIState_CPUDirty    = 0x00000008,   // Set if a CPU-command has modified the mapped data (implies in pCPUModified list).
        DIState_GPUDirty    = 0x00000010,   // Set if a GPU-command has modified the mapped data (implies in pGPUModified list).
    };

    // Init with regular image
    //   - Passed image provides original data
    DrawableImage(bool transparent,
                  ImageBase* originalData, DrawableImageContext* drawableContext = 0);

    // Init 
    DrawableImage(ImageFormat format, ImageSize size, bool transparent,
                  Color fillColor, DrawableImageContext* drawableContext = 0);

    virtual void            Release();
    ~DrawableImage();

    // ***** Data Accessors
    bool                    IsTransparent() const  { return Transparent; }
    DrawableImageContext*   GetContext() const { return pContext; }
    virtual RenderTarget*   GetRenderTarget() const { return pRT; }
    static bool             MapImageSource(ImageData* data, ImageBase* i);

    // ***** Rendering APIs


    // Generates a filtered image from a source. Source can be a DrawableImage, furthermore
    // they may refer to the same image, although thats more expensive due to a temp copy.
    void    ApplyFilter(DrawableImage* source,
                        const Rect<SInt32>& sourceRect, const Point<SInt32>& destPoint,
                        Filter* filter);

    // Determine what rectangle ApplyFilter would affect (twips).
    static void    CalcFilterRect(RectF* result,
                                  const Rect<SInt32>& sourceRect, Filter* filter);

    void    ColorTransform(const Rect<SInt32>& rect, const Cxform& cxform);

    void    Compare(DrawableImage* image0, DrawableImage* image1);

    enum ChannelBits
    {
        // These match flash.display.BitmapDataChannel
        Channel_Red     = 1,
        Channel_Green   = 2,
        Channel_Blue    = 4,
        Channel_Alpha   = 8,
        Channel_RGB     = Channel_Red | Channel_Green | Channel_Blue
    };

    void        CopyChannel(DrawableImage* source,
                            const Rect<SInt32>& sourceRect, const Point<SInt32>& destPoint,
                            ChannelBits sourceChannel, ChannelBits destChannel);


    void        CopyPixels(DrawableImage* source,
                           const Rect<SInt32>& sourceRect, const Point<SInt32>& destPoint,
                           DrawableImage* alphaSource = 0, const Point<SInt32>* alphaPoint = 0,
                           bool mergeAlpha = false);

    // Draw the render tree
    // Does deep tree copy inside for queuing purposes
    void        Draw(TreeNode* subtree,
                     const Matrix2F& matrix = Matrix2F(), const Cxform& cform = Cxform(),
                     BlendMode blendMode = Blend_None, const Rect<SInt32>* clipRect = 0);

    void        Draw(Image* source,
                     const Matrix2F& matrix = Matrix2F(), const Cxform& cform = Cxform(),
                     BlendMode blendMode = Blend_None, const Rect<SInt32>* clipRect = 0,
                     bool smoothing = false);

    void        FillRect(const Rect<SInt32>& rect, Color color);

    void        FloodFill(const Point<SInt32>& pt, Color color);


    // Calculate rectangular bounds that fully encloses all pixels of the specified color
    // if (findColor = true), all pixels that doesn't include the specified color (fildColor = false).
    // Determination holds for each pixel if (value & mask) == color.
    Rect<SInt32> GetColorBoundsRect(UInt32 mask, UInt32 color, bool findColor);

    // Get pixel value without alpha. If out of bounds, returns 0.
    Color       GetPixel(SInt32 x, SInt32 y);
    // Get pixel value with alpha.
    Color       GetPixel32(SInt32 x, SInt32 y);

    // Returns false in case of bounds error.
    bool        GetPixels(DIPixelProvider& provider, const Rect<SInt32>& sourceRect);

    // Computes populations counts of each channels color values within the given rectangle.
    // If the rectangle is NULL, the historgram for the entire image is calculated.
    // Receives four pointers of arrays of 256 values (RGBA, in order).
    void        Histogram(Rect<SInt32>* rect, unsigned colors[4][256]);
  
    // Hittest performs hit testing between two images or rectangles. A hit is successful if any
    // 'opaque' part of the image intersects. If rectangles are used, they are considered to be complete
    // opqaue.
    bool        HitTest(const Point<SInt32>& firstPoint, Rect<SInt32>& secondImage,
                        unsigned alphaThreshold);
    bool        HitTest(ImageBase* secondImage,
                        const Point<SInt32>& firstPoint, const Point<SInt32>& secondPoint,
                        unsigned firstThreshold, unsigned secondThreshold);

    // Merge source image into destination image with per-channel blending.
    void        Merge(DrawableImage* source,
                      const Rect<SInt32>& sourceRect, const Point<SInt32>& destPoint,
                      unsigned redMult, unsigned greenMult, unsigned blueMult, unsigned alphaMult);


    // Noise
    void        Noise(unsigned randomSeed, unsigned low = 0, unsigned high = 255,
                      unsigned channelMask = Channel_RGB, bool grayscale = false);

    // PaletteMap
    void        PaletteMap(DrawableImage* source,
                           const Rect<SInt32>& sourceRect, const Point<SInt32>& destPoint,
                           UInt32** channels);

    // PerlinNoise
    void        PerlinNoise(float frequencyX, float frequencyY, unsigned numOctaves, unsigned randomSeed, bool stitch, bool fractal,
                            unsigned channelMask = Channel_RGB, bool grayScale = false, float* offsets = 0, unsigned offsetCount = 0);

    // PixelDisolve
    unsigned    PixelDissolve(DrawableImage* source,
                              const Rect<SInt32>& sourceRect, const Point<SInt32>& destPoint,
                              unsigned randomSeed, unsigned numPixels, Color fill);

    void        Scroll(int x, int y);

    void        SetPixel(SInt32 x, SInt32 y, Color c);
    void        SetPixel32(SInt32 x, SInt32 y, Color c);

    // Returns false in case of a bounds error; or the buffer does not contain enough pixels to 
    // fill the rectangle. It does fill as many pixels as possible in the second case.
    bool        SetPixels(const Rect<SInt32>& destRect, DIPixelProvider& provider);

    // Threshold
    enum OperationType
    {
        Operator_LT,
        Operator_LE,
        Operator_GT,
        Operator_GE,
        Operator_EQ,
        Operator_NE
    };
    void        Threshold(DrawableImage* source,
                          const Rect<SInt32>& sourceRect, const Point<SInt32>& destPoint,
                          OperationType op, 
                          UInt32 threshold, UInt32 color, UInt32 mask, bool copySource);

    // ***** Render::Image interface implementation

    // TBD
    virtual ImageType       GetImageType() const  { return Type_DrawableImage; }
    virtual ImageFormat     GetFormat() const     { return Format; }
    virtual unsigned        GetUse() const        { return 0; }
    virtual ImageSize       GetSize() const       { return ISize; }    
    virtual unsigned        GetMipmapCount() const{ return 1; }


    virtual Texture*        GetTexture(TextureManager* pmanager);

    virtual bool    Decode(ImageData* pdest, CopyScanlineFunc copyScanline = CopyScanlineDefault,
                           void* arg = 0) const;    

    virtual Image* GetAsImage() { return this; }
	SF_AMP_CODE(
	    //virtual bool    Decode(ImageData* pdest, CopyScanlineFunc copyScanline = CopyScanlineDefault, void* arg = 0) const;
//	    virtual UPInt   GetBytes() const { return ImageData::GetFormatBitsPerPixel(GetFormat()) * ISize.Area() / 8; }
	    virtual UPInt   IsExternal() const { return false; }
        virtual UInt32  GetImageId() const { return ImageId; }
        virtual UInt32  GetBaseImageId() const { return 0; }
	)

    virtual void  TextureLost(TextureLossReason reason);

protected:

    bool createTextureFromManager(HAL* phal, TextureManager* tmanager);

    bool mapTextureRT(bool readOnly);

    bool ensureGPURenderable();

    void unmapTextureRT();
    void updateRenderTargetRT();
    void updateStagingTargetRT();

    template<class C>
    void addCommand(const C& cmd);

    ImageData& getMappedData();
    bool isMapped() const;

    void addToCPUModifiedList();
    void addToGPUModifiedListRT();

    bool isRenderableRT() const { return pRT != 0; }
    bool ensureRenderableRT();
    void initialize(ImageFormat format, const ImageSize &size, DrawableImageContext* dicontext);

    void setViewProj3DHelper( TreeNode* subtree, TreeRoot* root );

private:

    // TODOBM: Update rectangle, could be useful for partial updates.
    
    ImageFormat                 Format;             // Format of the DrawableImage data.
    ImageSize                   ISize;              // Size of the DrawableImage.
    volatile unsigned           DrawableImageState; // The state of the DrawableImage. See DrawableImageStateFlags.
    bool                        Transparent;        // Whether the BitmapData object was created with the 'transparent' flag.
    Ptr<DICommandQueue>         pQueue;             // The queue that holds commands requested by the DrawableImage.
    ImageData                   MappedData;         // Image data mapped, either internally or from an explicit user-mapping.
    
    Ptr<DrawableImage>          pCPUModifiedNext;   // Linked list of images that have been modified by CPU-side functions (implies DIState_CPUDirty).
    Ptr<DrawableImage>          pGPUModifiedNext;   // Linked list of images that have been modified by GPU-side functions (implies DIState_GPUDirty).

    SF_AMP_CODE(UInt32 ImageId;)

    // Original delegate image
    Ptr<ImageBase>              pDelegateImage;     // If the BitmapData was created with an image resource, this is it.
    Ptr<DrawableImageContext>   pContext;           // The DrawableImageContext (Movie-level object) for this DrawableImage.
    Ptr<RenderTarget>           pRT;                // The data associated with this DrawableImage (CPU & GPU both).
    Ptr<Fence>                  pFence;             // Fence preventing CPU access to the DrawableImage, while it being executed on the GPU.
};

// Interface to providing pixels for Get/SetPixels. AS3 ByteArray may require endian swapping, and
// this logic should not be contained in the renderer.
class DIPixelProvider
{
public:
    virtual ~DIPixelProvider() { }
    virtual UPInt  GetLength() const = 0;           // Total number of pixels available
    virtual UInt32 ReadNextPixel() = 0;             // Reads the next pixel from the provider as an ARGB UInt32.
    virtual void   WriteNextPixel(UInt32 v) = 0;    // Writes a pixel to the provider.
};


}}; // namespace Scaleform::Render

#endif // INC_SF_Render_DrawableImage_H
