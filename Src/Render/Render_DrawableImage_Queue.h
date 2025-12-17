/**************************************************************************

PublicHeader:   Render
Filename    :   Render_DrawableImage_Queue.cpp
Content     :   Commands and command queue implementation for DrawableImage
Created     :   January 2012
Authors     :   Michael Antonov

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_Render_DrawableImage_Queue_H
#define INC_SF_Render_DrawableImage_Queue_H


#include "Render_DrawableImage.h"
#include "Render_Context.h"
#include "Render_Filters.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {

// Helper functions.
bool MapImageSource(ImageData* data, ImageBase* i);

// 'RTTI' enum for DICommand types. This is used when querying the HAL about which
// DICommands it supports; it may be that a HAL partially supports a command, and thus
// it will need to inspect the DICommand object (and thus cast it to the appropriate type).
enum DICommandType
{
    // These commands *must* be supported by the GPU.
    DICommandType_Map,
    DICommandType_Unmap,
    DICommandType_CreateTexture,

    DICommandType_Clear,
    DICommandType_ApplyFilter,      // No CPU implementation possible.
    DICommandType_Draw,             // No CPU implementation possible.
    DICommandType_CopyChannel,
    DICommandType_CopyPixels,
    DICommandType_ColorTransform,
    DICommandType_Compare,
    DICommandType_FillRect,
    DICommandType_FloodFill,
    DICommandType_GetColorBoundsRect,
    DICommandType_GetPixel32,
    DICommandType_GetPixels,
    DICommandType_Histogram,
    DICommandType_HitTest,
    DICommandType_Merge,
    DICommandType_Noise,
    DICommandType_PaletteMap,
    DICommandType_PerlinNoise,
    DICommandType_PixelDissolve,
    DICommandType_SetPixel32,
    DICommandType_SetPixels,
    DICommandType_Scroll,
    DICommandType_Threshold,
    DICommandType_Count
};


//--------------------------------------------------------------------
// DrawableImage Command and CommandQueue implementations

struct DICommandContext
{ 
    Renderer2D*     pR2D;
    Render::HAL*    pHAL;

    // Constructor for SWF rendering (no HAL)
    DICommandContext() : pR2D(0), pHAL(0)
    { }
    // Constructor for HW rendering
    DICommandContext(ThreadCommandQueue* rtQueue)
    {
        Interfaces i;
        if ( !rtQueue )
            return;
        rtQueue->GetRenderInterfaces(&i);
        pR2D = i.pRenderer2D;
        pHAL = i.pHAL;
    }
};



struct DISourceImages
{
    static const unsigned MaximumSources = 2;
    Image* pImages[MaximumSources];

    DISourceImages()        
    { 
        memset(pImages, 0, sizeof(pImages));
    }

    Image* operator [] (unsigned i) const 
    { 
        SF_DEBUG_ASSERT(i < MaximumSources, "OOB image access.");
        return pImages[i]; 
    }
};


// DICommand is a base command for DrawableImage commands that are
// queued up for the render thread.
struct DICommand
{
    friend class DICommandSet;

    enum RenderCaps
    {
        RC_CPU           = 0x01,  // Command is executable on the CPU.
        RC_GPU           = 0x02,  // Command is executable on the GPU.
        RC_GPUPreference = 0x04,  // GPU execution is preferred but not necessary (prevents immediate CPU execution even when image is mapped).
        RC_GPU_NoRT      = 0x08,  // GPU command does not require the Image's RenderTarget to be set (eg. for Create/Map/Unmap).
        RC_CPU_Return    = 0x10,  // Command requires a return value (and therefore must be waited for to execute on the CPU).
        RC_CPU_NoModify  = 0x20,  // CPU command does not modify the image.
    };

    DICommand(DrawableImage* image = 0) : pImage(image) { }

    virtual ~DICommand() { }
    
    virtual DICommandType GetType() const = 0;
    virtual unsigned GetCPUCaps() const = 0;
    virtual unsigned GetRenderCaps() const;
    virtual unsigned GetSize() const = 0;

    // Fills in SourceImages data structure; returns the number of images used.
    virtual unsigned GetSourceImages(DISourceImages*) const { return 0; }
    
    // RenderThread execution function; does proper map/unmap and dispatches
    // to either ExecuteHW or ExecuteSW. 
    void             ExecuteRT(DICommandContext& context) const;

    // Attempts to execute the command immediately on the Advance thread, if it is
    // required to return a value to the VM. Returns true if it is executed, false if
    // it is deferred to the render thread.
    bool             ExecuteSWOnAddCommand(DrawableImage* i) const;

    virtual void     ExecuteHW(DICommandContext&) const { }
    virtual void     ExecuteSW(DICommandContext& context,
                               ImageData& dest, ImageData** psrc = 0) const
    { SF_UNUSED3(context, dest, psrc); }

    // ExecuteDiscard happens when the command is not executed, but discarded. This can
    // happen if the context is being shutdown, with no waiting for completion request.
    virtual void     ExecuteDiscard() { }

protected:

    bool     executeSWHelper(DICommandContext& context,
                             DrawableImage* i,
                             DISourceImages& images,
                             unsigned imageCount) const;

    bool     executeHWHelper(DICommandContext& context,
                             DrawableImage* i) const;

    Ptr<DrawableImage> pImage;
};
template <class D, class B = DICommand>
struct DICommandImpl : public B
{
    DICommandImpl() {}
    DICommandImpl(DrawableImage* image) : B(image) { }
    
    virtual unsigned GetSize() const { return sizeof(D); }
};


//--------------------------------------------------------------------

// DIQueuePage represents a block of memory that can hold multiple DICommands,
// allocated and executed consecutively.
struct DIQueuePage : public ListNode<DIQueuePage>, public NewOverrideBase<Stat_Default_Mem>
{
    enum { BufferSize = 512 - 16 };
    UByte        Buffer[BufferSize];
    // Allocation offset in page.
    unsigned     Offset;
    unsigned     CaptureFrameId;

    DIQueuePage() : Offset(0), CaptureFrameId(0) { }

    DICommand* getFirst()
    {
        return Offset ? ((DICommand*)Buffer) : 0;
    }
    DICommand* getNext(DICommand* prevCommand)
    {
        SF_ASSERT((prevCommand >= (DICommand*)Buffer) &&
                  (prevCommand < (DICommand*)(Buffer + Offset)));
        UPInt   commandSize = prevCommand->GetSize();
        UByte*  next = ((UByte*)prevCommand) + commandSize;
        if ((UByte*)next >= (Buffer + Offset))
            return 0;
        return (DICommand*)next;
    }

    DICommand* destroyAndGetNext(DICommand* prevCommand)
    {
        DICommand* next = getNext(prevCommand);
        Destruct(prevCommand);
        return next;
    }


    void* AllocSize(unsigned size)
    {
        if (size > (BufferSize-Offset))
            return 0;
        UByte* data = Buffer + Offset;
        Offset += size;
        return data;
    }

    bool HasFreeSize(unsigned size)
    {
        return (size <= (BufferSize-Offset));
    }
};

enum DICommandSetType
{
    DICommand_All,
    DICommand_Displaying
};


// DICommandSet represents a set of commands obtained from DICommandQueue.
class DICommandSet
{
    friend class DICommandQueue;
public:

    DICommandSet(DICommandQueue* queue)
        : pQueue(queue)
    { }
    ~DICommandSet()
    {
        // All obtained commands should've been executed.
        SF_ASSERT(QueueList.IsEmpty());
    }

    void ExecuteCommandsRT(DICommandContext& context);

    bool IsEmpty() const { return QueueList.IsEmpty(); }

private:
    DICommandQueue*     pQueue;
    List<DIQueuePage>   QueueList;
};



// DICommandQueue is ref-counted because it can be shared among drawable images.
// Such sharing is done by merging queues when one DrawableImage is copied into
// another.


class DICommandQueue  : public RefCountBase<DICommandQueue, Stat_Default_Mem>,
                        public ListNode<DICommandQueue>
{
    friend class DICommandSet;
    friend struct DICommand;

    enum {
        FreePageLimit  = 3,
        AllocPageLimit = 16
    };

public:
    DICommandQueue(DrawableImageContext* dicontext);
    ~DICommandQueue();

    // We keep a separate queue for each snapshot state
    enum DIQueueType 
    {
        DIQueue_Active,
        DIQueue_Captured,
        DIQueue_Displaying,
        DIQueue_Free, // Small free list
        DIQueue_Item_Count
    };


    template<class C>
    bool AddCommand_NTS(const C& src)
    {
        SF_COMPILER_ASSERT(sizeof(C) < DIQueuePage::BufferSize);
        void* buffer = allocCommandFromPage(sizeof(C), &QueueLock);
        if (buffer)
        {
            Construct(buffer, src);
            return true;
        }
        return false;
    }

    template<class C>
    bool AddCommand(const C& src)
    {
        return AddCommand_NTS(src);
    }

    // Mimics interface of ContextCaptureNotify; DrawableImageContext is the real notify, these
    // are called in turn by the context, to ensure proper ordering.
    void OnCapture();
    void OnNextCapture(ContextImpl::RenderNotify* notify);
    void OnShutdown();


    void ExecuteNextCapture(ContextImpl::RenderNotify* notify);
    void ExecuteCommandsAndWait();   
    void DiscardCommands();

    class ExecuteCommand : public ThreadCommand
    {
        DICommandQueue* pQueue;
        Event           ExecuteDone;
    public:
        ExecuteCommand(DICommandQueue* queue) : pQueue(queue)
        {
        }

        virtual void Execute();

        void   WaitDoneAndReset()
        {
            ExecuteDone.Wait();
            ExecuteDone.ResetEvent();
        }
    };

protected:
    friend class ExecuteCommand;
    friend class DrawableImage;

    void*           allocCommandFromPage(unsigned size, Lock* locked);
    DIQueuePage*    allocPage();
    void            freePage(DIQueuePage* page);
    void            popCommandSet(DICommandSet* cmdSet, DICommandSetType type);
    DICommandQueue* getThis() { return this; }

    void            updateCPUModifiedImagesRT();
    void            updateGPUModifiedImagesRT();

    void            lockCommandSetAndWait(DICommandSet** pcommands);
    void            unlockCommandSet();
    void            notifyCommandSetFinished(DICommandSet** pcommands);

    Mutex               CommandSetMutex;
    WaitCondition       CommandSetWC;
    DICommandSet*       pRTCommands;        // Render thread command in progress.

    // Drawable image update lists. One tracks images that were modified by the CPU, and thus
    // will require the GPU data to be updated, the other is the reverse, GPU modified data that
    // will need to be propagated to the CPU.
    Ptr<DrawableImage>      pCPUModifiedImageList;
    Ptr<DrawableImage>      pGPUModifiedImageList;

    ThreadCommandQueue* pRTCommandQueue;
    Lock                QueueLock;
    
    Ptr<ExecuteCommand> ExecuteCmd;

    List<DIQueuePage> Queues[DIQueue_Item_Count];
    unsigned          CaptureFrameId;
    unsigned          FreePageCount;
    unsigned          AllocPageCount;
};



//--------------------------------------------------------------------
// ***** Commands
//--------------------------------------------------------------------

// CreateTexture command is emitted to the thread queue to initialize texture
// in constructor.

struct DICommand_CreateTexture : public DICommandImpl<DICommand_CreateTexture>
{    
    DICommand_CreateTexture(DrawableImage* image)
        : DICommandImpl<DICommand_CreateTexture>(image)
    { }

    virtual DICommandType GetType() const { return DICommandType_CreateTexture; }
    virtual unsigned GetCPUCaps() const { return 0; }

    virtual void ExecuteHW(DICommandContext& context) const;
};




struct DICommand_Clear : public DICommandImpl<DICommand_Clear>
{    
    Color FillColor;

    DICommand_Clear(DrawableImage* image, Color color)
        : DICommandImpl<DICommand_Clear>(image), FillColor(color)
    { }

    virtual DICommandType GetType() const { return DICommandType_Clear; }
    virtual unsigned GetCPUCaps() const { return RC_CPU; }

    virtual void ExecuteSW(DICommandContext& context,
        ImageData& dest, ImageData** src = 0) const;

    virtual void ExecuteHW(DICommandContext&) const;
};



struct DICommand_SourceRect : public DICommand
{
    Ptr<DrawableImage>  pSource;
    Rect<SInt32>        SourceRect;
    Point<SInt32>       DestPoint;    
  
    DICommand_SourceRect(DrawableImage* image, DrawableImage* source,
                         const Rect<SInt32>& sr, const Point<SInt32>& dp)
        : DICommand(image), pSource(source), SourceRect(sr), DestPoint(dp)
    {
    }

    virtual unsigned GetSourceImages(DISourceImages* ps) const
    {
        ps->pImages[0] = pSource;
        return 1;
    }

    // Calculates the destination clipped rectangle. Used to determine the area on the destination 
    // which should be overwritten, based on the SourceRect size, DestPoint and dimensions of both images.
    bool CalculateDestClippedRect( const ImageData &src, const ImageData &dest, const Rect<SInt32>& srcRect,
        Rect<SInt32> &dstClippedRect, Point<SInt32> &delta ) const
    {
        return CalculateDestClippedRect(src.GetSize(), dest.GetSize(), srcRect, dstClippedRect, delta);
    }
    bool CalculateDestClippedRect( const ImageSize &src, const ImageSize &dest, const Rect<SInt32>& srcRect,
        Rect<SInt32> &dstClippedRect, Point<SInt32> &delta ) const;
};

template<class D>
struct DICommand_SourceRectImpl : public DICommand_SourceRect
{
    DICommand_SourceRectImpl(DrawableImage* image, DrawableImage* source,
                             const Rect<SInt32>& sr, const Point<SInt32>& dp)
        : DICommand_SourceRect(image, source, sr, dp)
    { }

    virtual unsigned GetSize() const { return sizeof(D); }

    virtual void ExecuteHW(DICommandContext& context) const;

    // Helper function to fill out an array of images, and their offsets for texturing (in pixels). If
    // the destination image is required, it should be placed in slot 0.
    virtual void ExecuteHWGetImages( DrawableImage** images, Size<float>* readOffsets) const
    {
        unsigned index = 0;
        if ( GetRequireSourceRead() )
        {
            images[index] = pImage;
            readOffsets[index] = Size<float>((float)DestPoint.x, (float)DestPoint.y);
            index++;
        }
        images[index] = pSource;
        readOffsets[index] = Size<float>((float)SourceRect.TopLeft().x, (float)SourceRect.TopLeft().y);
    }

    static const unsigned MaximumDrawableSources = 3;

    // Peforms the actual operation, based on the filter type.
    virtual void ExecuteHWCopyAction( DICommandContext& context, Render::Texture** tex, const Matrix2F* texgen ) const 
    { SF_UNUSED3(context, tex, texgen); };

    // Returns whether this command will require reading from the destination's source data (in HW commands). 
    // This happens always for certain commands (eg. CopyChannel, as it will need to always read the other
    // channels of the destination image), never for certain commands (CopyPixels, as it just blends
    // on top of the destination).
    virtual bool GetRequireSourceRead() const { return false; };
};

struct DICommand_ApplyFilter : public DICommand_SourceRectImpl<DICommand_ApplyFilter>
{
    Ptr<Filter> pFilter;
    
    DICommand_ApplyFilter(DrawableImage* image, DrawableImage* source,
                          const Rect<SInt32>& sr, const Point<SInt32>& dp,
                          Filter* filter)
        : DICommand_SourceRectImpl<DICommand_ApplyFilter>(image, source, sr, dp), pFilter(filter)
    { }

    virtual DICommandType GetType() const { return DICommandType_ApplyFilter; }
    virtual unsigned GetCPUCaps() const { return 0; }

    virtual void ExecuteHWGetImages( DrawableImage** images, Size<float>* readOffsets) const;
    virtual void ExecuteHWCopyAction( DICommandContext& context, Render::Texture** tex, const Matrix2F* texgen) const;

    // Filters should always generate their own RenderTargets, so reporting no-read is valid.
    virtual bool GetRequireSourceRead() const { return false; };
};

struct DICommand_Draw : public DICommandImpl<DICommand_Draw>
{
    TreeRoot*     pRoot;
    Rect<SInt32>  ClipRect;
    bool          HasClipRect;

    DICommand_Draw(DrawableImage* image, TreeRoot* root, const Rect<SInt32>* clipRect)
        : DICommandImpl<DICommand_Draw>(image),
          pRoot(root), HasClipRect(clipRect ? true : false)
    {
        if (clipRect)
            ClipRect = *clipRect;
    }

    virtual DICommandType GetType() const { return DICommandType_Draw; }
    virtual unsigned GetCPUCaps() const { return 0; }

    virtual void ExecuteHW(DICommandContext&) const;

    // DICommand_Draw needs to override ExecuteDiscard, so it can properly handle the destruction of pRoot.
    virtual void ExecuteDiscard();
};





struct DICommand_CopyChannel : public DICommand_SourceRectImpl<DICommand_CopyChannel>
{
    typedef DrawableImage::ChannelBits ChannelBits;
    ChannelBits SourceChannel;
    ChannelBits DestChannel;

    DICommand_CopyChannel(DrawableImage* image, DrawableImage* source,
                          const Rect<SInt32>& sr, const Point<SInt32>& dp,
                          ChannelBits sourceChannel, ChannelBits destChannel)
    : DICommand_SourceRectImpl<DICommand_CopyChannel>(image, source, sr, dp),
      SourceChannel(sourceChannel), DestChannel(destChannel)
    {

    }

    virtual DICommandType GetType() const { return DICommandType_CopyChannel; }
    virtual unsigned GetCPUCaps() const { return RC_CPU; }

    virtual void ExecuteHWCopyAction( DICommandContext& context, Render::Texture** tex, const Matrix2F* texgen ) const;
    virtual bool GetRequireSourceRead() const { return true; };

    virtual void ExecuteSW(DICommandContext& context,
                           ImageData& dest, ImageData** src = 0) const;
};


struct DICommand_CopyPixels : public DICommand_SourceRectImpl<DICommand_CopyPixels>
{
    Ptr<DrawableImage>  pAlphaSource;
    Point<SInt32>       AlphaPoint;
    bool                MergeAlpha;

    DICommand_CopyPixels(DrawableImage* image, DrawableImage* source,
                         const Rect<SInt32>& sourceRect, const Point<SInt32>& destPoint,
                         DrawableImage* alphaSource, const Point<SInt32>* alphaPoint,
                         bool mergeAlpha)
    : DICommand_SourceRectImpl<DICommand_CopyPixels>(image, source, sourceRect, destPoint),
      pAlphaSource(alphaSource),
      AlphaPoint(alphaPoint ? *alphaPoint : Point<SInt32>(0)),
      MergeAlpha(mergeAlpha)
    { }

    virtual DICommandType GetType() const { return DICommandType_CopyPixels; }
    virtual unsigned GetCPUCaps() const { return RC_CPU; }

    virtual void Execute(DICommandContext&) const { }

    // CopyPixels requires an overriden ExecuteHWGetImages, because it has an additional alphasource image.
    virtual void ExecuteHWGetImages( DrawableImage** images, Size<float>* readOffsets) const;
    virtual void ExecuteHWCopyAction( DICommandContext& context, Render::Texture** tex, const Matrix2F* texgen ) const;
    virtual bool GetRequireSourceRead() const { return true; };

    virtual void ExecuteSW(DICommandContext& context,
                           ImageData& dest, ImageData** src = 0) const;

    virtual unsigned GetSourceImages(DISourceImages* ps) const
    {
        unsigned count = 1;
        ps->pImages[0] = pSource;
        if (pAlphaSource)
        {
            ps->pImages[1] = pAlphaSource;
            count++;
        }
        return count;
    }

};

struct DICommand_ColorTransform : public DICommand_SourceRectImpl<DICommand_ColorTransform>
{
    Cxform       Cx;

    DICommand_ColorTransform(DrawableImage* image, const Rect<SInt32>& rect, const Cxform& cxform)
        : DICommand_SourceRectImpl<DICommand_ColorTransform>(image, image, rect, rect.TopLeft()),
          Cx(cxform)
    { }

    virtual DICommandType GetType() const { return DICommandType_ColorTransform; }
    virtual unsigned GetCPUCaps() const { return RC_CPU; }
    virtual void ExecuteHWCopyAction( DICommandContext& context, Render::Texture** tex, const Matrix2F* texgen ) const;
    virtual bool GetRequireSourceRead() const { return true; };
    virtual void ExecuteSW(DICommandContext& context, ImageData& dest, ImageData** src = 0) const;
};

struct DICommand_Compare : public DICommand_SourceRectImpl<DICommand_Compare>
{
    Ptr<DrawableImage> pImageCompare1;

    DICommand_Compare(DrawableImage* image, DrawableImage* cmp0, DrawableImage* cmp1) :
        DICommand_SourceRectImpl<DICommand_Compare>(image, cmp0, Rect<SInt32>(image->GetSize()), Point<SInt32>(0,0) ),
        pImageCompare1(cmp1)
    { }

    virtual unsigned GetSourceImages(DISourceImages* ps) const
    {
        ps->pImages[0] = pSource;
        ps->pImages[1] = pImageCompare1;
        return 2;
    }
    virtual DICommandType GetType() const { return DICommandType_Compare; }
    virtual unsigned GetCPUCaps() const { return RC_CPU; }
    virtual void ExecuteHWCopyAction( DICommandContext& context, Render::Texture** tex, const Matrix2F* texgen ) const;
    virtual bool GetRequireSourceRead() const { return pSource == pImage || pImageCompare1 == pImage; };
    virtual void ExecuteSW(DICommandContext& context, ImageData& dest, ImageData** src = 0) const;
};


struct DICommand_FillRect : public DICommandImpl<DICommand_FillRect>
{
    Rect<SInt32> ApplyRect;
    Color        FillColor;

    DICommand_FillRect(DrawableImage* image, const Rect<SInt32>& rect, Color color)
        : DICommandImpl<DICommand_FillRect>(image), ApplyRect(rect), FillColor(color)
    { }

    virtual DICommandType GetType() const { return DICommandType_FillRect; }
    virtual unsigned GetCPUCaps() const { return RC_CPU; }

    virtual void ExecuteSW(DICommandContext& context,
                           ImageData& dest, ImageData** src = 0) const;

    virtual void ExecuteHW(DICommandContext& context) const;
};


struct DICommand_FloodFill : public DICommandImpl<DICommand_FloodFill>
{
    Point<SInt32> Pt;
    Color         FillColor;

    DICommand_FloodFill(DrawableImage* image, const Point<SInt32>& pt, Color color)
        : DICommandImpl<DICommand_FloodFill>(image), Pt(pt), FillColor(color)
    { }

    virtual DICommandType GetType() const { return DICommandType_FloodFill; }
    virtual unsigned GetCPUCaps() const { return RC_CPU; }

    virtual void ExecuteSW(DICommandContext& context,
                           ImageData& dest, ImageData** src = 0) const;
};

struct DICommand_GetColorBoundsRect : public DICommandImpl<DICommand_GetColorBoundsRect>
{
    UInt32                  Mask;           // The mask to apply to the image data when doing the comparison.
    UInt32                  SearchColor;    // The color that is being searched for.
    bool                    FindColor;      // Whether the search is for the rectangle containing color, or the absence of the color.
    mutable Rect<SInt32>*   Result;         // The resultant rectangle (return value).

    DICommand_GetColorBoundsRect(DrawableImage* image, UInt32 mask, UInt32 color, bool findColor, Rect<SInt32>* result )
        : DICommandImpl<DICommand_GetColorBoundsRect>(image), Mask(mask), SearchColor(color), FindColor(findColor),
          Result(result)
    {
        // If the image is not transparent, never compare the alpha channel.
        if (!image->IsTransparent())
            Mask &= 0x00FFFFFF;
    }

    virtual DICommandType GetType() const { return DICommandType_GetColorBoundsRect; }
    virtual unsigned GetCPUCaps() const { return RC_CPU | RC_CPU_Return | RC_CPU_NoModify; }

    virtual void ExecuteSW(DICommandContext& context,
                           ImageData& dest, ImageData** src = 0) const;

};

struct DICommand_GetPixel32 : public DICommandImpl<DICommand_GetPixel32>
{
    UInt32          X;          // X coordinate of the pixel to retrieve.
    UInt32          Y;          // Y coordinate of the pixel to retrieve.
    mutable Color*  Result;     // The resultant pixel color (return value)

    DICommand_GetPixel32(DrawableImage* image, SInt32 x, SInt32 y, Color* result )
        : DICommandImpl<DICommand_GetPixel32>(image), X(x), Y(y), Result(result)
    {
    }

    virtual DICommandType GetType() const { return DICommandType_GetPixel32; }
    virtual unsigned GetCPUCaps() const { return RC_CPU | RC_CPU_Return | RC_CPU_NoModify; }

    virtual void ExecuteSW(DICommandContext& context,
        ImageData& dest, ImageData** src = 0) const;

};

struct DICommand_GetPixels : public DICommandImpl<DICommand_GetPixels>
{
    Rect<SInt32>        SourceRect;     // The rectangle, containing the pixels to retrieve.
    DIPixelProvider&    Provider;       // The object which will receive pixel values.
    mutable bool*       Result;         // Is set to true if a bounds error occurred during execution.

    DICommand_GetPixels(DrawableImage* image, Rect<SInt32> srcRect, DIPixelProvider& provider, bool* result )
        : DICommandImpl<DICommand_GetPixels>(image), SourceRect(srcRect), Provider(provider), Result(result)
    {
    }

    DICommand_GetPixels& operator=(const DICommand_GetPixels& other)
    {
        SourceRect = other.SourceRect;
        Provider = other.Provider;
        Result = other.Result;
        return *this;
    }

    virtual DICommandType GetType() const { return DICommandType_GetPixels; }
    virtual unsigned GetCPUCaps() const { return RC_CPU | RC_CPU_Return | RC_CPU_NoModify; }

    virtual void ExecuteSW(DICommandContext& context,
        ImageData& dest, ImageData** src = 0) const;

};

struct DICommand_Histogram : public DICommandImpl<DICommand_Histogram>
{
    Rect<SInt32>        SourceRect;     // The rectangle, containing the pixels to compute the histogram.
    mutable unsigned*   Result;         // Array (4x256 values) which will receive the histogram results.

    DICommand_Histogram(DrawableImage* image, Rect<SInt32> srcRect, unsigned* result)
        : DICommandImpl<DICommand_Histogram>(image), SourceRect(srcRect), Result(result)
    {
    }

    virtual DICommandType GetType() const { return DICommandType_Histogram; }
    virtual unsigned GetCPUCaps() const { return RC_CPU | RC_CPU_Return | RC_CPU_NoModify; }

    virtual void ExecuteSW(DICommandContext& context,
        ImageData& dest, ImageData** src = 0) const;

};

struct DICommand_HitTest : public DICommandImpl<DICommand_HitTest>
{
    Ptr<Image>       SecondImage;        // The image to do the hittest operation with (NULL, if using a hittest rectangle/point).
    Rect<SInt32>     SecondArea;         // The rectangle to do the hittest operation with (if SecondImage is NULL).
    Point<SInt32>    FirstPoint;         // The relative point on the first image to use as the origin.
    Point<SInt32>    SecondPoint;        // The relative point on the second image to use as the origin (only used if SecondImage is NULL).
    unsigned         FirstThreshold;     // The threshold alpha value to consider opaque on the first image.
    unsigned         SecondThreshold;    // The threshold alpha value to consider opaque on the second image (if non-NULL).
    mutable bool*    Result;             // Result (whether a 'hit' occurred).

    DICommand_HitTest(DrawableImage* image, const Point<SInt32>& firstPoint, Rect<SInt32>& secondImage, unsigned alphaThreshold, bool* result)
        : DICommandImpl<DICommand_HitTest>(image), SecondImage(0), SecondArea(secondImage), FirstPoint(firstPoint), FirstThreshold(alphaThreshold),
          SecondThreshold(0), Result(result)
    {
    }
    DICommand_HitTest(DrawableImage* image, ImageBase* secondImage, const Point<SInt32>& firstPoint, const Point<SInt32>& secondPoint,
        unsigned firstThreshold, unsigned secondThreshold, bool* result)
        : DICommandImpl<DICommand_HitTest>(image), SecondImage((Image*)secondImage), FirstPoint(firstPoint), SecondPoint(secondPoint), 
          FirstThreshold(firstThreshold), SecondThreshold(secondThreshold), Result(result)
    {
    }

    virtual unsigned GetSourceImages(DISourceImages* ps) const
    {
        if (SecondImage)
        {
            ps->pImages[0] = SecondImage;
            return 1;
        }
        return 0;
    }

    virtual DICommandType GetType() const { return DICommandType_HitTest; }
    virtual unsigned GetCPUCaps() const { return RC_CPU | RC_CPU_Return | RC_CPU_NoModify; }

    virtual void ExecuteSW(DICommandContext& context,
        ImageData& dest, ImageData** src = 0) const;

};

struct DICommand_Merge : public DICommand_SourceRectImpl<DICommand_Merge>
{
    unsigned RedMultiplier;
    unsigned GreenMultiplier;
    unsigned BlueMultiplier;
    unsigned AlphaMultiplier;

    DICommand_Merge(DrawableImage* image, DrawableImage* source,
        const Rect<SInt32>& sr, const Point<SInt32>& dp, 
        unsigned rmul, unsigned gmul, unsigned bmul, unsigned amul) :
        DICommand_SourceRectImpl<DICommand_Merge>(image, source, sr, dp),
        RedMultiplier(rmul), GreenMultiplier(gmul), BlueMultiplier(bmul), AlphaMultiplier(amul)
    { }

    virtual DICommandType GetType() const { return DICommandType_Merge; }
    virtual unsigned GetCPUCaps() const { return RC_CPU; }
    virtual void ExecuteHWCopyAction( DICommandContext& context, Render::Texture** tex, const Matrix2F* texgen ) const;
    virtual bool GetRequireSourceRead() const { return true; };
    virtual void ExecuteSW(DICommandContext& context, ImageData& dest, ImageData** src = 0) const;
};

struct DICommand_Noise : public DICommandImpl<DICommand_Noise>
{
    unsigned RandomSeed;
    unsigned Low;
    unsigned High;
    unsigned ChannelMask;
    bool     GrayScale;

    DICommand_Noise(DrawableImage* image, unsigned randomSeed, 
        unsigned low, unsigned high,unsigned channelMask, bool grayscale)
        : DICommandImpl<DICommand_Noise>(image), RandomSeed(randomSeed), Low(low), 
        High(high), ChannelMask(channelMask), GrayScale(grayscale)
    { }

    virtual DICommandType GetType() const { return DICommandType_Noise; }
    virtual unsigned GetCPUCaps() const { return RC_CPU; }

    virtual void ExecuteSW(DICommandContext& context, ImageData& dest, 
        ImageData** psrc = 0) const;
};

struct DICommand_PaletteMap : public DICommand_SourceRectImpl<DICommand_PaletteMap>
{
    static const unsigned ChannelSize = 256 * sizeof(UInt32);
    static const unsigned PaletteSize = 4 * ChannelSize;
    UInt32* Channels;

    // Holds which channel remappings were actually provided.
    unsigned ChannelMask;

    DICommand_PaletteMap(DrawableImage* image, DrawableImage* source,
        const Rect<SInt32>& sr, const Point<SInt32>& dp, 
        UInt32** channels) :
        DICommand_SourceRectImpl<DICommand_PaletteMap>(image, source, sr, dp),
        ChannelMask(0)
    {
        Channels = (UInt32*)SF_ALLOC(PaletteSize, Stat_Default_Mem);
        memset(Channels, 0, PaletteSize);
        for ( int i = 0; i < 4; ++i )
        {
            if ( channels[i] )
            {
                ChannelMask |= 1<<i;
                memcpy(((char*)Channels)+(i*ChannelSize), channels[i], ChannelSize);
            }
        }
    }

    DICommand_PaletteMap(const DICommand_PaletteMap& other) :
        DICommand_SourceRectImpl<DICommand_PaletteMap>(other),
            ChannelMask(other.ChannelMask)
        {
            Channels = (UInt32*)SF_ALLOC(PaletteSize, Stat_Default_Mem);
            memcpy(Channels, other.Channels, PaletteSize );
        }

    ~DICommand_PaletteMap()
    {
        if (Channels)
            SF_FREE(Channels);
        Channels = 0;
    }

    virtual DICommandType GetType() const { return DICommandType_PaletteMap; }
    virtual unsigned GetCPUCaps() const { return RC_CPU; }
    virtual void ExecuteHWCopyAction( DICommandContext& context, Render::Texture** tex, const Matrix2F* texgen ) const;
    virtual bool GetRequireSourceRead() const { return pImage == pSource; };
    virtual void ExecuteSW(DICommandContext& context, ImageData& dest, ImageData** src = 0) const;
};

struct DICommand_PerlinNoise : public DICommandImpl<DICommand_PerlinNoise>
{
    float       FrequencyX, FrequencyY;
    unsigned    NumOctaves;
    unsigned    RandomSeed;
    bool        Stitch;
    bool        FractalNoise;
    unsigned    ChannelMask;
    bool        GrayScale;

    // The maximum number of octaves that offsets that can be applied to. 
    static const unsigned MaximumOffsetOctaves = 16;
    float       Offsets[MaximumOffsetOctaves*2];
    unsigned    OffsetCount;

    DICommand_PerlinNoise(DrawableImage* image, 
        float frequencyX, float frequencyY, unsigned octaves, unsigned seed,
        bool stitch, bool fractal, unsigned channels, bool grayScale, float* offsets =0,
        unsigned offsetCount = 0) :
        DICommandImpl<DICommand_PerlinNoise>(image),
        FrequencyX(frequencyX), FrequencyY(frequencyY), NumOctaves(octaves), RandomSeed(seed),
        Stitch(stitch), FractalNoise(fractal), ChannelMask(channels), GrayScale(grayScale), 
        OffsetCount(Alg::Min(offsetCount, DICommand_PerlinNoise::MaximumOffsetOctaves))
    {
        if ( offsetCount > 0 )
            memcpy(Offsets, offsets, sizeof(unsigned)*offsetCount );
    }

    DICommand_PerlinNoise(const DICommand_PerlinNoise& other) : 
        DICommandImpl<DICommand_PerlinNoise>(other), 
        FrequencyX(other.FrequencyX), FrequencyY(other.FrequencyY), NumOctaves(other.NumOctaves), 
        RandomSeed(other.RandomSeed), Stitch(other.Stitch), FractalNoise(other.FractalNoise), 
        ChannelMask(other.ChannelMask), GrayScale(other.GrayScale), OffsetCount(other.OffsetCount)        
    {
        if ( other.OffsetCount > 0 )
            memcpy(Offsets, other.Offsets, Alg::Min(sizeof(Offsets), sizeof(unsigned)*other.OffsetCount) );
    }

    virtual DICommandType GetType() const { return DICommandType_PerlinNoise; }
    virtual unsigned GetCPUCaps() const { return RC_CPU; }

    virtual void ExecuteSW(DICommandContext& context,
        ImageData& dest, ImageData** src = 0) const;
};

struct DICommand_PixelDissolve: public DICommandImpl<DICommand_PixelDissolve>
{
    Ptr<DrawableImage>      pSource;        // The source for the pixel dissolve operation (may be the same as the destination).
    Rect<SInt32>            SourceRect;     // The source rectangle for the pixel dissolve operation.
    Point<SInt32>           DestPoint;      // The destination point for the pixel dissolve operation.
    unsigned                RandomSeed;     // The random seed value to compute the dissolve with.
    unsigned                NumPixels;      // The number of pixels to be dissolved.
    Color                   Fill;           // The color to dissolve to.
    mutable unsigned*       Result;         // The value to pass into the next iteration of PixelDissolve.

    DICommand_PixelDissolve(DrawableImage* image, DrawableImage* source,
        const Rect<SInt32>& sourceRect, const Point<SInt32>& destPoint,
        unsigned randomSeed, unsigned numPixels, Color fill, unsigned* result)
        : DICommandImpl<DICommand_PixelDissolve>(image), pSource(source), 
          SourceRect(sourceRect), DestPoint(destPoint), RandomSeed(randomSeed), NumPixels(numPixels), 
          Fill(fill), Result(result)
    {
    }

    virtual unsigned GetSourceImages(DISourceImages* ps) const
    {
        ps->pImages[0] = pSource;
        return 1;
    }

    virtual DICommandType GetType() const { return DICommandType_PixelDissolve; }
    virtual unsigned GetCPUCaps() const { return RC_CPU | RC_CPU_Return; }

    virtual void ExecuteSW(DICommandContext& context,
        ImageData& dest, ImageData** src = 0) const;

};

struct DICommand_SetPixel32 : public DICommandImpl<DICommand_SetPixel32>
{
    UInt32                  X;              // X coordinate of the pixel to set.
    UInt32                  Y;              // Y coordinate of the pixel to set.
    Color                   Fill;           // The color to write to the given pixel.
    bool                    OverwriteAlpha; // True if this command will overwrite the alpha value.
    mutable bool            Result;

    DICommand_SetPixel32(DrawableImage* image, UInt32 x, UInt32 y, Color fill, bool alpha)
        : DICommandImpl<DICommand_SetPixel32>(image), X(x), Y(y), Fill(fill), OverwriteAlpha(alpha)
    {
    }

    virtual DICommandType GetType() const { return DICommandType_SetPixel32; }
    virtual unsigned GetCPUCaps() const { return RC_CPU; }

    virtual void ExecuteSW(DICommandContext& context,
        ImageData& dest, ImageData** src = 0) const;

};

struct DICommand_SetPixels : public DICommandImpl<DICommand_SetPixels>
{
    Rect<SInt32>        DestRect;       // The destination rectangle for the pixels
    DIPixelProvider&    Provider;       // The object which will provide pixel values.
    mutable bool*       Result;         // Set to false if the provider did not contain enough pixels to fill the rectangle.

    DICommand_SetPixels(DrawableImage* image, Rect<SInt32> destRect, DIPixelProvider& provider, bool* result)
        : DICommandImpl<DICommand_SetPixels>(image), DestRect(destRect), Provider(provider), Result(result)
    {
    }
    DICommand_SetPixels& operator=(const DICommand_SetPixels& other)
    {
        DestRect = other.DestRect;
        Provider = other.Provider;
        Result = other.Result;
        return *this;
    }

    virtual DICommandType GetType() const { return DICommandType_SetPixels; }
    virtual unsigned GetCPUCaps() const { return RC_CPU | RC_CPU_Return; }

    virtual void ExecuteSW(DICommandContext& context,
        ImageData& dest, ImageData** src = 0) const;

};

struct DICommand_Scroll : public DICommand_SourceRectImpl<DICommand_Scroll>
{
    signed X;
    signed Y;

    DICommand_Scroll(DrawableImage* image, int x, int y) :
    DICommand_SourceRectImpl<DICommand_Scroll>(image, image, 
        Rect<SInt32>(0, 0, image->GetSize().Width, image->GetSize().Height), Point<SInt32>(x, y)),
        X(x), Y(y)
    { }

    virtual DICommandType GetType() const { return DICommandType_Scroll; }
    virtual unsigned GetCPUCaps() const { return RC_CPU; }
    // Note: does nothing, the copyback will handle the actual implementation
    virtual void ExecuteHWCopyAction( DICommandContext& context, Render::Texture** tex, const Matrix2F* texgen ) const;
    virtual bool GetRequireSourceRead() const { return true; };
    virtual void ExecuteSW(DICommandContext& context, ImageData& dest, ImageData** src = 0) const;
};

struct DICommand_Threshold : public DICommand_SourceRectImpl<DICommand_Threshold>
{
    DrawableImage::OperationType Operation;
    unsigned      Threshold;
    unsigned      ThresholdColor;
    unsigned      Mask;
    bool          CopySource;

    DICommand_Threshold(DrawableImage* image, DrawableImage* source,
        const Rect<SInt32>& sr, const Point<SInt32>& dp, 
        DrawableImage::OperationType op, unsigned th, unsigned col, unsigned mask, bool copySource) :
        DICommand_SourceRectImpl<DICommand_Threshold>(image, source, sr, dp),
        Operation(op), Threshold(th), ThresholdColor(col), Mask(mask), CopySource(copySource)
    { }

    virtual DICommandType GetType() const { return DICommandType_Threshold; }
    virtual unsigned GetCPUCaps() const { return RC_CPU; }
    virtual void ExecuteHWCopyAction( DICommandContext& context, Render::Texture** tex, const Matrix2F* texgen ) const;
    virtual bool GetRequireSourceRead() const { return CopySource; };
    virtual void ExecuteSW(DICommandContext& context, ImageData& dest, ImageData** src = 0) const;
};

}}; // namespace Scaleform::Render

#endif // INC_SF_Render_DrawableImage_Queue_H
