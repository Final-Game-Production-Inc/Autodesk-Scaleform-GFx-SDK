/**************************************************************************

PublicHeader:   GFx
Filename    :   GFx_Player.h
Content     :   Public interface to SWF File playback and Complex
                Objects API
Created     :   June 21, 2005
Authors     :   Michael Antonov, Prasad Silva

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GFx_Player_H
#define INC_SF_GFx_Player_H

#include "GFx/GFx_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Render/Render_HAL.h"
#include "Render/Render_Matrix3x4.h"
#include "GFx/GFx_Event.h"
#include "GFx/GFx_Loader.h"

#include "GFx/GFx_PlayerStats.h"
#include "Render/Render_Context.h"

#include <ctype.h>  // for wchar_t
#include <stdarg.h> // for va_list args

namespace Scaleform { 

class File;

namespace Render
{
    class Color;
    class ImageInfoBase;
    class TreeRoot;
}

namespace GFx {

// ***** Declared Classes
class MovieDef;
class Movie;
class Movie;
class MovieImpl;
class SoundRenderer;
class ASUserData;

// ***** External Classes
class LogState;
class InputEventsQueueEntry;
class MouseState;
class ASMovieRootBase;
namespace AMP { class ViewStats; }

// Viewport
class Viewport : public Render::Viewport
{
public:
    float Scale;
    float AspectRatio;

    Viewport()
    {
        Scale = AspectRatio = 1.0f;
    }

    Viewport(int bw, int bh, int left, int top, int w, int h, int scleft, int sctop, 
        int scw, int sch, float scale = 1.0f, float ratio = 1.0f, unsigned flags = 0) 
    :   Render::Viewport(bw, bh, left, top, w, h, scleft, sctop, scw, sch, flags),
        Scale(scale), AspectRatio(ratio)    {}

    Viewport(int bw, int bh, int left, int top, int w, int h, unsigned flags = 0) 
        : Render::Viewport(bw, bh, left, top, w, h, flags) 
    {
        Scale = AspectRatio = 1.0f;
    }

    Viewport(const Viewport& src) : Render::Viewport(src), Scale(src.Scale), AspectRatio(src.AspectRatio) {}
};

// Abstract class that holds memory data for MovieView creation
class MemoryContext : public RefCountBase<MemoryContext, Stat_Default_Mem>
{
public:
    virtual ~MemoryContext() { }

protected:
    // private constructor. Use CreateMemoryContext to create.
    MemoryContext() { }
};

struct MemoryParams
{
    // Heap descriptor for the heap that will be created for the movie. It is
    // possible to specify heap alignment, granularity, limit, flags. If heap '
    // limit is specified then GFx will try to maintain heap footprint on or 
    // below this limit.
    MemoryHeap::HeapDesc Desc;

    // A heap limit multiplier (0..1] that is used to determine how the heap limit
    // grows if it is exceeded. The algorithm is as follows:
    // if (allocs since collect >= heap footprint * HeapLimitMultiplier)
    //     collect
    // else
    //     expand(heap footprint + overlimit + heap footprint * HeapLimitMultiplier)
    float       HeapLimitMultiplier;

    // Number of roots before garbage collection is executed. This is an initial
    // value of max roots cap; it might be increased during the execution.
    // Set this value to 0 turn off number-of-roots-based collections.
    unsigned    MaxCollectionRoots;

    // Number of frames after which collection is forced, even if the max roots
    // cap is not reached. It is useful to perform intermediate collections 
    // in the case if current max roots cap is high, to reduce the cost of that
    // collection when it occurs. It is good to set FramesBetweenCollections to
    // a large value such as 1800, which would force collection every 30 seconds
    // if the file frame rate is 60 FPS. Set this to 0 to turn off frame-based
    // collection.
    unsigned    FramesBetweenCollections;

    // An initial limit of the heap (by default it is equal to INITIAL_DYNAMIC_LIMIT (128K)).
    // This is the limit when OnExceedLimit will be invoked first time, even if Desc.Limit is
    // set to a higher value. Desc.Limit will be used as an 'user limit', meaning the limit when
    // Collect will be enforced. But even if Desc.Limit is not set or not reached yet the OnExceedLimit
    // still may call Collect in the case when size of new allocations exceeds current
    // memory footprint * HeapLimitMutliplier (which is likely for low HeapLimitMutliplier value and/or
    // for complex content at the start). To prevent these Collect calls the value of the 'InitialDynamicLimit'
    // can be set to higher value (for example, equal to Heap.Limit). The downside of the higher value 
    // of 'InitialDynamicLimit' is higher memory consumption at the start; 
    // the benefit - faster execution due to less calls to Collect.
    unsigned    InitialDynamicLimit;

    // Controls 'generations' in GC (AS3 only).
    // RunsToUpgradeGen - is the number of regular calls to GC (excluding ones forced by the user)
    // before generations are upgraded by one step ('newborn' -> 'young', 'young' -> 'old').
    unsigned    RunsToUpgradeGen;
    // RunsToCollectYoung - is the number of regular calls to GC (excluding ones forced by the user)
    // before GC collects 'young' objects.
    unsigned    RunsToCollectYoung;
    // RunsToCollectYoung - is the number of regular calls to GC (excluding ones forced by the user)
    // before GC collects 'old' objects.
    unsigned    RunsToCollectOld;

    MemoryParams(UPInt memoryArena = 0)
    {
        Desc.Arena                 = memoryArena;
        HeapLimitMultiplier        = 0.25; // 25%
        MaxCollectionRoots         = ~0u; // Default value will be used.
        FramesBetweenCollections   = ~0u; // Default value will be used.
        InitialDynamicLimit        = ~0u; // Default value will be used.
        RunsToUpgradeGen           = ~0u; // Default value will be used.
        RunsToCollectYoung         = ~0u; // Default value will be used.
        RunsToCollectOld           = ~0u; // Default value will be used.
    }
};

// ***** Movie Definition and Instance interfaces

// MovieDef represents loaded shared data for an SWF movie file. These
// objects are normally created by Loader::CreateMovie and are shared
// by all movie instances. If the movie definition is registered in the
// library during load time (see Loader::LoadUseLibrary flag), loading
// the same file again will return a pointer to the same cached MovieDef.
//
// Individual movie instances can be created by MovieDef::CreateInstance.
// Once the instance is created, it is normally initialized with a
// renderer and used during playback.

class MovieDef : public Resource, public StateBag
{
public:

    // Query SWF file version, dimensions, and other stats.
    virtual unsigned    GetVersion() const = 0;
    virtual unsigned    GetLoadingFrame() const = 0;
    virtual float       GetWidth() const = 0;
    virtual float       GetHeight() const = 0;
    virtual unsigned    GetFrameCount() const = 0;
    virtual float       GetFrameRate() const = 0;
    // Returns frame rectangle, in pixels
    virtual RectF       GetFrameRect() const = 0;
    // Returns SWF file flags defined by GFxMovieInfo::SWFFlagConstants.
    virtual unsigned    GetSWFFlags() const = 0;
    // Returns the file path/URL from which this SWF was loaded.
    // This will normally be the path passed to Loader::CreateMovie.
    virtual const char* GetFileURL() const = 0;

    // WaitForLoadFinish waits until movie loading is completed.
    // Calling this function is important before Release() if you use
    // ThreadedTaskManager and memory arenas, as it insures that background
    // thread's memory references are released. If 'cancel' flag is passed,
    // loading will be canceled abruptly for the fastest possible shutdown.    
    // This function is used also used internally by GFxFontLib.
    virtual void        WaitForLoadFinish(bool cancel = false) const = 0;
    // Waits until a specified frame is loaded
    virtual void        WaitForFrame(unsigned frame) const = 0;

    // SWF 8 File attributes, returned by GetFileAttributes
    enum FileAttrFlags
    {
        FileAttr_UseNetwork         = 0x0001,
        FileAttr_UseActionScript3   = 0x0008,
        FileAttr_HasMetadata        = 0x0010
    };

    // Returns file attributes for SWF file version 8+. Will
    // return 0 for earlier SWF files.
    virtual unsigned    GetFileAttributes() const = 0;
    // Retrieves meta-data that can be embedded into SWF-8 files. Returns
    // the number of bytes written, up to buffSize. Specify 0 for pbuff
    // to return the buffer size required.
    virtual unsigned    GetMetadata(char *pbuff, unsigned buffSize) const = 0;

    virtual MemoryHeap* GetLoadDataHeap() const = 0;
    virtual MemoryHeap* GetBindDataHeap() const = 0;
    virtual MemoryHeap* GetImageHeap() const = 0;    

    virtual Resource* GetMovieDataResource() const = 0;


    // Exporter/strip tool information - returns data about image extraction
    // arguments passed to 'gfxexport' tool when generating this '.gfx' file.
    // Null will be returned if this movie def came from a '.swf'.
    virtual const ExporterInfo*  GetExporterInfo() const  = 0;


    // Creates a memory context to be passed into CreateInstance, below
    virtual MemoryContext* CreateMemoryContext(const char* heapName, const MemoryParams& memParams, 
                                               bool debugHeap) = 0;

    // Creates a movie view instance that is used for playback. Each movie
    // instance will have its own timeline, animation and Action-Script state.
    //
    // The returned created movie is AddRef-ed, so it can be stored in
    // a smart pointer and released when no longer necessary. Null pointer
    // will be returned in case of failure (low memory, etc.).    
    //   - memParams :      Specifies memory parameters describing heaps and garbage
    //                      collector behavior. memParams.Desc.Arena might be used
    //                      to specify memory arena index.
    //   - initFirstFrame : Set this to 'true' to initialize objects of frame 1,
    //                      populating the display list. If false, the display list
    //                      will be empty until the first Advance call.
    //   - actionControl :  ActionScript verbosity / etc (mostly for debugging)
    //   - queue :          Render thread queue; required for BitmapData commands. Can be provided later,
    //                      except if initFirstFrame is true, and BitmapData commands execute on frame 0.
    //
    // Ex:  Ptr<Movie> pmovie = *pdef->CreateInstance();    
    virtual Movie*   CreateInstance(const MemoryParams& memParams, bool initFirstFrame,
                                    ActionControl* actionControl, Render::ThreadCommandQueue* queue = 0)  = 0;

    //   - memContext :     Memory context that encapsulates the common MovieView heap, as well as 
    //                      other shared objects, such as the garbage collector.
    //   - actionControl :  ActionScript verbosity / etc (mostly for debugging)
    //   - queue :          Render thread queue; required for BitmapData commands. Can be provided later,
    //                      except if initFirstFrame is true, and BitmapData commands execute on frame 0.
    virtual Movie*   CreateInstance(MemoryContext* memContext, bool initFirstFrame,
                                    ActionControl* actionControl, Render::ThreadCommandQueue* queue = 0) = 0;

    //   - initFirstFrame : Set this to 'true' to to initialize objects of frame 1,
    //                      populating the display list. If false, the display list
    //                      will be empty until the first Advance call.
    //   - memoryArena :    Index of memory arena to be used for memory allocations.
    //   - actionControl :  ActionScript verbosity / etc (mostly for debugging)
    //   - queue :          Render thread queue; required for BitmapData commands. Can be provided later,
    //                      except if initFirstFrame is true, and BitmapData commands execute on frame 0.
    SF_INLINE  Movie*   CreateInstance(bool initFirstFrame = false, UPInt memoryArena = 0, 
                                        ActionControl* actionControl = NULL, Render::ThreadCommandQueue* queue = 0)
    {
        MemoryParams memParams;
        memParams.Desc.Arena = memoryArena;
        return CreateInstance(memParams, initFirstFrame, actionControl, queue);
    }



    // ***  Import interface

    // Import support interface. SWF files can import dictionary symbols from
    // other SWF files. This is done automatically if Loader::LoadImports flag
    // is specified when loading a movie (default for Loader::CreateMovie).

    // Visitor interface, used for movie enumeration through VisitImportedMovies.
    struct ImportVisitor
    {
        virtual ~ImportVisitor () { }

        // Users must override Visit.
        //  - pimportedMovieFilename is the partial filename being used for import;
        //    to get the full path use pimportDef->GetFileURL().
        virtual void    Visit(MovieDef* pparentDef, MovieDef* pimportDef,
                              const char* pimportedMovieFilename) = 0;
    };

    // Enumerates a list of names of external SWF files imported into this movie.
    // Calls pvistor->Visit() method for each import movie file name.
    virtual void    VisitImportedMovies(ImportVisitor* pvisitor) = 0;

    // *** Resource enumeration

    // Resource enumeration is used primarily by the GFxExport tool to collect
    // all of the resources within the file.

    // Visitor interface, used for movie enumeration through VisitImportedMovies.
    struct ResourceVisitor : public FileTypeConstants
    {
        virtual ~ResourceVisitor () { }       

        virtual void    Visit(MovieDef* pmovieDef, Resource* presource,
                              ResourceId rid, const char* pexportName) = 0;
    };

    enum VisitResourceMask
    {
        ResVisit_NestedMovies   = 0x8000,

        // Types of resources to visit:
        ResVisit_Fonts          = 0x01,
        ResVisit_Bitmaps        = 0x02,
        ResVisit_GradientImages = 0x04,
        ResVisit_EditTextFields = 0x08,
        ResVisit_Sounds         = 0x10,
        ResVisit_Sprite         = 0x20,
        
        // Combined flags.
        ResVisit_AllLocalImages = (ResVisit_Bitmaps | ResVisit_GradientImages),
        ResVisit_AllImages      = (ResVisit_Bitmaps | ResVisit_GradientImages | ResVisit_NestedMovies)
    };
    
    // Enumerates all resources.
    // Calls pvistor->Visit() method for each resource in the SWF/GFX file.
    virtual void    VisitResources(ResourceVisitor* pvisitor, unsigned visitMask = ResVisit_AllImages) = 0;


    /*
    // Fills in extracted image file info for a given characterId. Character
    // ids are passed to ImageVisitor::Visit method. Returns 0 if such
    // information is not available.
    virtual bool    GetImageFileInfo(GFxImageFileInfo *pinfo, int characterId)      = 0;
    */

    // Retrieves a resource pointer based on an export name. Export name is specified in
    // the flash studio by right-clicking on the bitmap in the library and
    // modifying its Linkage identifier attribute. The symbol must also be
    // exported for runtime or ActionScript sharing.
    virtual Resource*     GetResource(const char *pexportName) const         = 0;

};

// ***** ImportVisitor

// Default import visitor - used to enumerate imports. This visitor is called 
// during loading (import binding stage) after a each import is resolved.
class   ImportVisitor : public State, public MovieDef::ImportVisitor, public FileTypeConstants
{
public:    
    ImportVisitor() : State(State_ImportVisitor) { }       
    virtual ~ImportVisitor () { }       

    // Users must override Visit.
    //  - pimportedMovieFilename is the partial filename being used for import;
    //    to get the full path use pimportDef->GetFileURL().
    virtual void    Visit(MovieDef* pparentDef, MovieDef* pimportDef,
                          const char* pimportedMovieFilename) = 0;
};

// Sets a default visitor that is used when after files are loaded.
SF_INLINE  void StateBag::SetImportVisitor(ImportVisitor *ptr)
{
    SetState(State::State_ImportVisitor, ptr);
}
SF_INLINE  Ptr<ImportVisitor> StateBag::GetImportVisitor() const
{
    return *(ImportVisitor*) GetStateAddRef(State::State_ImportVisitor);
}




// ***** Value - used to pass/receive values from ActionScript.

//
// Value is capable of holding a simple or complex type and the corresponding value.
// These values can be passed to and from the application context to the AS runtime.
// The supported types are:
//  * Undefined     - The AS undefined value
//  * Null          - The AS null value
//  * Boolean       - The AS Boolean type (C++ bool analogue)
//  * Number        - The AS Number type (C++ Double analogue)
//  * String        - The AS String type. Can be a string managed by
//                    the application, or a string from the AS runtime.
//  * StringW       - Similar to String, in that it corresponds to the AS Strinb value,
//                    however, it provides a convenient interface for applications
//                    that work in a wide char environment.
//  * Object        - The AS Object type (and all objects derived from
//                    it). A special API interface is provide to manipulate such 
//                    objects.
//  * Array         - A special case of the Object type. An additional API is provide
//                    to manipulate AS arrays.
//  * DisplayObject - A special case of the Object type. Corresponds to entities on
//                    the stage (MovieClips, Buttons, and TextFields). A custom API
//                    is provide to manipulate display properties of such objects.
//
// ** Strings
// The application is expected to manage their lifetimes when GFx::Values are assigned 
// const char* or const wchar_t* pointers. Strings returned from the AS runtime are 
// managed by the runtime. To create a string that is managed by the runtime, use the
// Movie::CreateString() or Movie::CreateStringW() method.
//
// ** Objects
// GFx::Values of type Object (and Array, DisplayObject) are returned by the AS runtime.
// The application can create instances of the base Object class type, by using 
// the Movie::CreateObject() method. This method also takes a class name that allows
// users to create instances of specific classes. To create an instance of Array, the 
// Movie::CreateArray() method can be used. Currently, there is no interface to
// create instances on the stage (of type DisplayObject).
//
// ** Storage
// GFx::Values are expected to be non-movable, due to the tracking mechanism in debug
// builds. If GFx::Values are stored in a dynamic array, use a C++ compliant dynamic 
// array type (such as the STL vector, or the GArrayCPP container). These guarantee
// that the GFx::Values are destroyed and created appropriately on container resize.
//


// Forward declarations for Value:
namespace AS2 { class MovieRoot; }
namespace AS3 { class MovieRoot; }
struct Value_AS2ObjectData;
class MemberValueSet;

#if defined(SF_BUILD_DEBUG) || defined(GFX_AS_ENABLE_GFXVALUE_CLEANUP)
//
// The list node interface is used to keep track of AS object references. The user
// is warned in debug builds if there are GFx::Values still holding AS object 
// references when the MovieImpl that owns the AS objects is about to be 
// released. This condition can cause a crash because the memory heap used
// by the AS objects is released when the MovieImpl dies.
//
class Value : public ListNode<Value> 
//
#else
//
class Value
//
#endif
{
    friend class MovieImpl;
    friend class AS2::MovieRoot;
    friend class AS3::MovieRoot;
    friend struct Value_AS2ObjectData;
    friend class ASUserData;

public:
    // Structure to modify display properties of an object on the stage (MovieClip,
    // TextField, Button). Used in conjunction with Value::Get/SetDisplayInfo.
    //
    class DisplayInfo
    {
    public:
        enum Flags
        {
            V_x         = 0x01,
            V_y         = 0x02,
            V_rotation  = 0x04,
            V_xscale    = 0x08,
            V_yscale    = 0x10,
            V_alpha     = 0x20,
            V_visible   = 0x40,
            V_z         = 0x80,
            V_xrotation = 0x100,
            V_yrotation = 0x200,
            V_zscale    = 0x400,
            V_FOV       = 0x800,
            V_projMatrix3D = 0x1000,
            V_viewMatrix3D = 0x2000,
            V_edgeaaMode   = 0x4000,
        };

        DisplayInfo() : VarsSet(0)                     {}
        DisplayInfo(double x, double y) : VarsSet(0)   { SetFlags(V_x | V_y); X = x; Y = y; }
        DisplayInfo(double rotation) : VarsSet(0)      { SetFlags(V_rotation); Rotation = rotation; }
        DisplayInfo(bool visible) : VarsSet(0)         { SetFlags(V_visible); Visible = visible; }

        SF_INLINE void    Clear()                         { VarsSet = 0; }

        void    Initialize(UInt16 varsSet, double x, double y, double rotation, double xscale, 
                           double yscale, double alpha, bool visible,
                           double z, double xrotation, double yrotation, double zscale, double fov, 
                           const Matrix3F *pviewm, const Matrix4F *pprojm, Render::EdgeAAMode edgeaaMode)
        {
            VarsSet = varsSet;
            X = x;
            Y = y;
            Rotation = rotation;
            XScale = xscale;
            YScale = yscale;
            Alpha = alpha;
            Visible = visible;            
            Z = z;
            XRotation = xrotation;
            YRotation = yrotation;
            ZScale = zscale;
            FOV = fov;
            if (pviewm)
                ViewMatrix3D = *pviewm;
            if (pprojm)
                ProjectionMatrix3D = *pprojm;
            EdgeAAMode = edgeaaMode;
        }

        SF_INLINE void    SetX(double x)                  { SetFlags(V_x); X = x; }
        SF_INLINE void    SetY(double y)                  { SetFlags(V_y); Y = y; }
        SF_INLINE void    SetRotation(double degrees)     { SetFlags(V_rotation); Rotation = degrees; }
        SF_INLINE void    SetXScale(double xscale)        { SetFlags(V_xscale); XScale = xscale; }    // 100 == 100%
        SF_INLINE void    SetYScale(double yscale)        { SetFlags(V_yscale); YScale = yscale; }    // 100 == 100%
        SF_INLINE void    SetAlpha(double alpha)          { SetFlags(V_alpha); Alpha = alpha; }
        SF_INLINE void    SetVisible(bool visible)        { SetFlags(V_visible); Visible = visible; }
        SF_INLINE void    SetZ(double z)                  { SetFlags(V_z); Z = z; }
        SF_INLINE void    SetXRotation(double degrees)    { SetFlags(V_xrotation); XRotation = degrees; }
        SF_INLINE void    SetYRotation(double degrees)    { SetFlags(V_yrotation); YRotation = degrees; }
        SF_INLINE void    SetZScale(double zscale)        { SetFlags(V_zscale); ZScale = zscale; }    // 100 == 100%
        SF_INLINE void    SetFOV(double fov)              { SetFlags(V_FOV); FOV = fov; }
        SF_INLINE void    SetProjectionMatrix3D(const Matrix4F *pmat)  
        { 
            if (pmat) 
            { 
                SetFlags(V_projMatrix3D); 
                ProjectionMatrix3D = *pmat;
            } 
            else
                ClearFlags(V_projMatrix3D); 
        }
        void    SetViewMatrix3D(const Matrix3F *pmat) 
        { 
            if (pmat) 
            { 
                SetFlags(V_viewMatrix3D); 
                ViewMatrix3D = *pmat;
            } 
            else
                ClearFlags(V_viewMatrix3D); 
        }
        SF_INLINE void  SetEdgeAAMode(Render::EdgeAAMode edgeaaMode)    { SetFlags(V_edgeaaMode); EdgeAAMode = edgeaaMode; }

        SF_INLINE void    SetPosition(double x, double y)         { SetFlags(V_x | V_y); X = x; Y = y; }
        SF_INLINE void    SetScale(double xscale, double yscale)  { SetFlags(V_xscale | V_yscale); XScale = xscale; YScale = yscale; }

        SF_INLINE void    Set(double x, double y, double rotation, double xscale, double yscale, double alpha, bool visible)
        {
            X = x;
            Y = y;
            Rotation = rotation;
            XScale = xscale;
            YScale = yscale;
            Alpha = alpha;
            Visible = visible;
            SetFlags(V_x | V_y | V_rotation | V_xscale | V_yscale | V_alpha | V_visible);
        }
        SF_INLINE void    Set(double x, double y, double rotation, double xscale, double yscale, double alpha, bool visible,
            double z, double xrotation, double yrotation, double zscale)
        {
            X = x;
            Y = y;
            Rotation = rotation;
            XScale = xscale;
            YScale = yscale;
            Alpha = alpha;
            Visible = visible;
            Z = z;
            XRotation = xrotation;
            YRotation = yrotation;
            ZScale = zscale;
            SetFlags(V_x | V_y | V_rotation | V_xscale | V_yscale | V_alpha | V_visible |
                V_z | V_zscale | V_xrotation | V_yrotation);
        }

        SF_INLINE Double  GetX() const            { SF_ASSERT(IsFlagSet(V_x)); return X; }
        SF_INLINE Double  GetY() const            { SF_ASSERT(IsFlagSet(V_y)); return Y; }
        SF_INLINE Double  GetRotation() const     { SF_ASSERT(IsFlagSet(V_rotation)); return Rotation; }
        SF_INLINE Double  GetXScale() const       { SF_ASSERT(IsFlagSet(V_xscale)); return XScale; }
        SF_INLINE Double  GetYScale() const       { SF_ASSERT(IsFlagSet(V_yscale)); return YScale; }
        SF_INLINE Double  GetAlpha() const        { SF_ASSERT(IsFlagSet(V_alpha)); return Alpha; }
        SF_INLINE bool    GetVisible() const      { SF_ASSERT(IsFlagSet(V_visible)); return Visible; }
        SF_INLINE Double  GetZ() const            { return Z; }
        SF_INLINE Double  GetXRotation() const    { return XRotation; }
        SF_INLINE Double  GetYRotation() const    { return YRotation; }
        SF_INLINE Double  GetZScale() const       { return ZScale; }
        SF_INLINE Double  GetFOV() const          { return FOV; }
        SF_INLINE const Matrix4F* GetProjectionMatrix3D() const   { return (IsFlagSet(V_projMatrix3D)) ? &ProjectionMatrix3D : NULL; }
        SF_INLINE const Matrix3F* GetViewMatrix3D() const         { return (IsFlagSet(V_viewMatrix3D)) ? &ViewMatrix3D : NULL; }
        SF_INLINE Render::EdgeAAMode GetEdgeAAMode() const          { SF_ASSERT(IsFlagSet(V_edgeaaMode)); return EdgeAAMode; }

        SF_INLINE bool    IsFlagSet(unsigned flag) const    { return 0 != (VarsSet & flag); }

    private:
        double              X;
        double              Y;
        double              Rotation;
        double              XScale;
        double              YScale;
        double              Alpha;
        double              Z;
        double              XRotation;
        double              YRotation;
        double              ZScale;
        double              FOV;
        Matrix3F            ViewMatrix3D;
        Matrix4F            ProjectionMatrix3D;
        Render::EdgeAAMode  EdgeAAMode;
        UInt16              VarsSet;
        bool                Visible;

        SF_INLINE void    SetFlags(unsigned flags)              { VarsSet |= flags; }
        SF_INLINE void    ClearFlags(unsigned flags)            { VarsSet &= ~flags; }
    };

protected:

    enum ValueTypeControl
    {
        // ** Special Bit Flags
        // Explicit coercion to type requested 
        VTC_ConvertBit      = 0x80,
        // Flag that denotes managed resources
        VTC_ManagedBit      = 0x40,
        // Set if value was referencing managed value and the owner Movie/VM
        // was destroyed.
        VTC_OrphanedBit     = 0x20,

        // ** Type mask
        VTC_TypeMask        = VTC_ConvertBit | 0x0F,
    };

    //
    // Internal interface used by Value to manipulate AS Objects
    //
    class ObjectInterface : public NewOverrideBase<StatMV_Other_Mem>
    {
        friend class MovieImpl;

    public:
        class ObjVisitor
        {
        public:
            virtual ~ObjVisitor() {}

            // AS3 only: By default public members defined in a class are not returned by the visitor.
            //           Returning true will include public members (inherited ones as well) + getters.
            //           This is not applicable to AS2 as AVM1 does not store scope/ns information.
            virtual bool    IncludeAS3PublicMembers() const  { return false; }

            virtual void    Visit(const char* name, const Value& val) = 0;
        };
        class ArrVisitor
        {
        public:
            virtual ~ArrVisitor() {}
            virtual void    Visit(unsigned idx, const Value& val) = 0;
        };

        ObjectInterface(MovieImpl* pmovieRoot) : pMovieRoot(pmovieRoot) {}
        virtual ~ObjectInterface() {}

        GFX_VM_ABSTRACT(void    ObjectAddRef(Value* val, void* pobj));
        GFX_VM_ABSTRACT(void    ObjectRelease(Value* val, void* pobj));

        GFX_VM_ABSTRACT(bool    HasMember(void* pdata, const char* name, bool isdobj) const);
        GFX_VM_ABSTRACT(bool    GetMember(void* pdata, const char* name, Value* pval, bool isdobj) const);
        GFX_VM_ABSTRACT(bool    SetMember(void* pdata, const char* name, const Value& value, bool isdobj));
        GFX_VM_ABSTRACT(bool    Invoke(void* pdata, Value* presult, const char* name,
                                       const Value* pargs, UPInt nargs, bool isdobj));
        GFX_VM_ABSTRACT(bool    InvokeClosure(void* pdata, UPInt dataAux, Value* presult,
                                       const Value* pargs, UPInt nargs));

        GFX_VM_ABSTRACT(bool    DeleteMember(void* pdata, const char* name, bool isdobj));
        GFX_VM_ABSTRACT(void    VisitMembers(void* pdata, ObjVisitor* visitor, bool isdobj) const);

        GFX_VM_ABSTRACT(unsigned GetArraySize(void* pdata) const);
        GFX_VM_ABSTRACT(bool    SetArraySize(void* pdata, unsigned sz));
        GFX_VM_ABSTRACT(bool    GetElement(void* pdata, unsigned idx, Value *pval) const);
        GFX_VM_ABSTRACT(bool    SetElement(void* pdata, unsigned idx, const Value& value));
        GFX_VM_ABSTRACT(void    VisitElements(void* pdata, ArrVisitor* visitor,
                                             unsigned idx, int count) const);
        GFX_VM_ABSTRACT(bool    PushBack(void* pdata, const Value& value));
        GFX_VM_ABSTRACT(bool    PopBack(void* pdata, Value* pval));
        GFX_VM_ABSTRACT(bool    RemoveElements(void* pdata, unsigned idx, int count));

		GFX_VM_ABSTRACT(bool    IsInstanceOf(void* pdata, const char* className) const);

        GFX_VM_ABSTRACT(bool    IsByteArray(void* pdata) const);
        GFX_VM_ABSTRACT(unsigned GetByteArraySize(void* pdata) const);
		GFX_VM_ABSTRACT(void	SetByteArraySize(void* pdata, UPInt setSize));
        GFX_VM_ABSTRACT(bool    ReadFromByteArray(void* pdata, UByte *destBuff, UPInt destBuffSz) const);
        GFX_VM_ABSTRACT(bool    WriteToByteArray(void* pdata, const UByte *srcBuff, UPInt writeSize));
		GFX_VM_ABSTRACT(void*	GetRawDataPtr(void* pdata));

        GFX_VM_ABSTRACT(bool    IsDisplayObjectActive(void* pdata) const);
        GFX_VM_ABSTRACT(bool    GetParent(void* pdata, Value* pparent) const);
        GFX_VM_ABSTRACT(bool    GetDisplayInfo(void* pdata, DisplayInfo* pinfo) const);
        GFX_VM_ABSTRACT(bool    SetDisplayInfo(void* pdata, const DisplayInfo& info));
        GFX_VM_ABSTRACT(bool    GetWorldMatrix(void* pdata, Render::Matrix2F* pmat) const);
        GFX_VM_ABSTRACT(bool    GetDisplayMatrix(void* pdata, Render::Matrix2F* pmat) const);
        GFX_VM_ABSTRACT(bool    SetDisplayMatrix(void* pdata, const Render::Matrix2F& mat));
        GFX_VM_ABSTRACT(bool    GetMatrix3D(void* pdata, Render::Matrix3F* pmat) const);
        GFX_VM_ABSTRACT(bool    SetMatrix3D(void* pdata, const Render::Matrix3F& mat));
        GFX_VM_ABSTRACT(bool    GetCxform(void* pdata, Render::Cxform* pcx) const);
        GFX_VM_ABSTRACT(bool    SetCxform(void* pdata, const Render::Cxform& cx));

        GFX_VM_ABSTRACT(bool    GetText(void* pdata, Value* pval, bool ishtml) const);
        GFX_VM_ABSTRACT(bool    SetText(void* pdata, const char* ptext, bool ishtml));
        GFX_VM_ABSTRACT(bool    SetText(void* pdata, const wchar_t* ptext, bool ishtml));

        GFX_VM_ABSTRACT(bool    CreateEmptyMovieClip(void* pdata, GFx::Value* pmc, 
                                                     const char* instanceName, SInt32 depth));
        GFX_VM_ABSTRACT(bool    AttachMovie(void* pdata, GFx::Value* pmc, const char* symbolName, 
                                            const char* instanceName, SInt32 depth, 
                                            const MemberValueSet* initArgs));

        GFX_VM_ABSTRACT(bool    GotoAndPlay(void* pdata, const char* frame, bool stop));
        GFX_VM_ABSTRACT(bool    GotoAndPlay(void* pdata, unsigned frame, bool stop));

        GFX_VM_ABSTRACT(void    ToString(String* pstr, const GFx::Value& thisVal) const);

#ifdef GFX_AS_ENABLE_USERDATA
        GFX_VM_ABSTRACT(void    SetUserData(void* pdata, ASUserData* puserdata, bool isdobj));
        GFX_VM_ABSTRACT(ASUserData* GetUserData(void* pdata, bool isdobj) const);
        GFX_VM_ABSTRACT(bool    CreateObjectValue(Value* pval, void* pdata, bool isdobj));
#endif  // GFX_AS_ENABLE_USERDATA

        GFX_VM_ABSTRACT(AmpStats* GetAdvanceStats() const);

        SF_INLINE bool    IsSameContext(ObjectInterface* pobjInterface)
        { return (pMovieRoot == pobjInterface->pMovieRoot); }

        MovieImpl*  GetMovieImpl() const { return pMovieRoot; }

    protected:
        MovieImpl*   pMovieRoot;


#if defined(SF_BUILD_DEBUG) || defined(GFX_AS_ENABLE_GFXVALUE_CLEANUP)
        //
        // ** Special Value tracking interface
        //
        // Keeps track of AS object references held by Value. If not empty
        // when MovieImpl is released, warns user to release the GFx::Values
        // holding the AS object references. Not releasing the GFx::Values will
        // lead to a crash due to freed memory heap.
        //
        List<Value>     ExternalObjRefs;
#endif
    };

public:

    // The type of value stored in Value.
    enum ValueType
    {
        // ** Type identifiers

        // Basic types
        VT_Undefined        = 0x00,
        VT_Null             = 0x01,
        VT_Boolean          = 0x02,
        VT_Int              = 0x03,
        VT_UInt             = 0x04,
        VT_Number           = 0x05,
        VT_String           = 0x06,

        // StringW can be passed as an argument type, but it will only be returned 
        // if VT_ConvertStringW was specified, as it is not a native type.
        VT_StringW          = 0x07,  // wchar_t* string
        
        // ActionScript VM objects
        VT_Object           = 0x08,
        VT_Array            = 0x09,
        // Special type for stage instances (MovieClips, Buttons, TextFields)
        VT_DisplayObject    = 0x0a,

#ifdef GFX_AS3_SUPPORT
        // A function closure reference
        VT_Closure          = 0x0b,
#endif

        // Specify this type to request SetVariable/Invoke result to be converted 
        // to the specified type. After return of the function, the specified type
        // will *ALWAYS* be stored in the value.
        VT_ConvertBoolean   = VTC_ConvertBit | VT_Boolean,
        VT_ConvertInt       = VTC_ConvertBit | VT_Int,
        VT_ConvertUInt      = VTC_ConvertBit | VT_UInt,
        VT_ConvertNumber    = VTC_ConvertBit | VT_Number,
        VT_ConvertString    = VTC_ConvertBit | VT_String,
        VT_ConvertStringW   = VTC_ConvertBit | VT_StringW
        // Cannot convert to VM objects
    };

    // Constructors.
    Value()                      : pObjectInterface(NULL), Type(VT_Undefined)  { }
    Value(ValueType type)        : pObjectInterface(NULL), Type(type)          
    { 
        // Cannot create complex types. They are only returned from the AS runtime.
        // To create instances of Object and Array, use the Movie::CreateObject
        // and CreateArray methods.
        SF_ASSERT(type != VT_Object && type != VT_Array && type != VT_DisplayObject);
#ifdef GFX_AS3_SUPPORT
        SF_ASSERT(type != VT_Closure);
#endif
        mValue.pString = 0; 
    }
    Value(SInt32 v)              : pObjectInterface(NULL), Type(VT_Int)        { mValue.IValue = v; }
    Value(UInt32 v)              : pObjectInterface(NULL), Type(VT_UInt)       { mValue.UIValue = v; }
    Value(double v)              : pObjectInterface(NULL), Type(VT_Number)     { mValue.NValue = v; }
    Value(bool v)                : pObjectInterface(NULL), Type(VT_Boolean)    { mValue.BValue = v; }
    Value(const char* ps)        : pObjectInterface(NULL), Type(VT_String)     { mValue.pString = ps; }
    Value(const wchar_t* ps)     : pObjectInterface(NULL), Type(VT_StringW)    { mValue.pStringW = ps; }

    Value(const Value& src)   : pObjectInterface(NULL), Type(src.Type)
    { 
        mValue = src.mValue;
#ifdef GFX_AS3_SUPPORT
        DataAux = src.DataAux;
#endif
        if (src.IsManagedValue()) AcquireManagedValue(src);
    }

    ~Value()     
    { 
        if (IsManagedValue()) ReleaseManagedValue(); 
        Type = VT_Undefined;
    }

    const Value& operator = (const Value& src) 
    {         
        if (this != &src) 
        {
            if (IsManagedValue()) ReleaseManagedValue();
            Type = src.Type; 
            mValue = src.mValue;
#ifdef GFX_AS3_SUPPORT
            DataAux = src.DataAux;
#endif
            if (src.IsManagedValue()) AcquireManagedValue(src);
        }
        return *this; 
    }

    bool operator == (const GFx::Value& other) const
    {
        if (GetType() != other.GetType()) return false;
        switch (GetType())
        {
        case VT_Undefined:   
        case VT_Null:
                            return true;
        case VT_Boolean:	return mValue.BValue == other.mValue.BValue;
        case VT_Int:        return mValue.IValue == other.mValue.IValue;
        case VT_UInt:       return mValue.UIValue == other.mValue.UIValue;
        case VT_Number:		return mValue.NValue == other.mValue.NValue;
        case VT_String:		return !SFstrcmp(GetString(), other.GetString());
        case VT_StringW:	return !SFwcscmp(GetStringW(), other.GetStringW());
#ifdef GFX_AS3_SUPPORT
        case VT_Closure:    return (mValue.pData == other.mValue.pData) && (DataAux == other.DataAux);
#endif
        default:			return mValue.pData == other.mValue.pData;
        }
    }

    String          ToString() const;
    const wchar_t*  ToStringW(wchar_t* pwstr, UPInt buffSz) const;

    // Get type based on which you can interpret the value.
    SF_INLINE ValueType   GetType() const         { return ValueType(Type & VTC_TypeMask); }

    // Check types
    SF_INLINE bool        IsUndefined() const     { return GetType() == VT_Undefined; }
    SF_INLINE bool        IsNull() const          { return GetType() == VT_Null; }
    SF_INLINE bool        IsBool() const          { return GetType() == VT_Boolean; }
    SF_INLINE bool        IsInt() const           { return GetType() == VT_Int; }
    SF_INLINE bool        IsUInt() const          { return GetType() == VT_UInt; }
    SF_INLINE bool        IsNumber() const        { return GetType() == VT_Number; }
    SF_INLINE bool        IsNumeric() const       { return IsInt() || IsUInt() || IsNumber(); }
    SF_INLINE bool        IsString() const        { return GetType() == VT_String; }
    SF_INLINE bool        IsStringW() const       { return GetType() == VT_StringW; }
    SF_INLINE bool        IsObject() const        { return (GetType() == VT_Object) || 
                                                          GetType() == VT_Array || 
#ifdef GFX_AS3_SUPPORT
                                                          GetType() == VT_Closure || 
#endif
                                                          GetType() == VT_DisplayObject; }
    SF_INLINE bool        IsArray() const         { return GetType() == VT_Array; }
    SF_INLINE bool        IsDisplayObject() const { return GetType() == VT_DisplayObject; }

#ifdef GFX_AS3_SUPPORT
    SF_INLINE bool        IsClosure() const       { return GetType() == VT_Closure; }
#endif

    SF_INLINE bool        IsOrphaned() const      { return (Type & VTC_OrphanedBit) != 0; }

    // Get values for each type.
    SF_INLINE bool        GetBool() const         { SF_ASSERT(IsBool());   return mValue.BValue;  }
    //
    // *** NOTE: Int and UInt types are never returned FROM the AS2 VM. Use GetNumber() instead.
    //           Creation of GFx::Values with Int and UInt types and assigning them to the AS2 VM
    //           is legal, however they will be automatically converted to Number type.
    //           The AS3 VM will have a direct mapping between the GFx::Value primitive numeric types
    //           and the VM equivalents, therefore GetInt/UInt is valid and correct for the AS3 VM.
    //
    SF_INLINE SInt32      GetInt() const          { SF_ASSERT(IsInt());    return mValue.IValue;  }
    SF_INLINE UInt32      GetUInt() const         { SF_ASSERT(IsUInt());   return mValue.UIValue; }
    //
    SF_INLINE Double      GetNumber() const       { SF_ASSERT(IsNumber()); return mValue.NValue;  }
    SF_INLINE const char* GetString() const       
    { 
        SF_ASSERT(IsString()); 
        return IsManagedValue() ? *mValue.pStringManaged : mValue.pString; 
    }
    SF_INLINE const wchar_t* GetStringW() const   { SF_ASSERT(IsStringW()); return mValue.pStringW; }

    // Set types and values from user context.
    SF_INLINE void        SetUndefined()                  { ChangeType(VT_Undefined); }
    SF_INLINE void        SetNull()                       { ChangeType(VT_Null); }
    SF_INLINE void        SetBoolean(bool v)              { ChangeType(VT_Boolean);   mValue.BValue = v; }
    SF_INLINE void        SetInt(SInt32 v)                { ChangeType(VT_Int);       mValue.IValue = v; }
    SF_INLINE void        SetUInt(UInt32 v)               { ChangeType(VT_UInt);      mValue.UIValue = v; }
    SF_INLINE void        SetNumber(double v)             { ChangeType(VT_Number);    mValue.NValue = v; }
    SF_INLINE void        SetString(const char* p)        { ChangeType(VT_String);    mValue.pString = p; }
    SF_INLINE void        SetStringW(const wchar_t* p)    { ChangeType(VT_StringW);   mValue.pStringW = p; }

    SF_INLINE void        SetConvertBoolean()      { ChangeType(VT_ConvertBoolean); }
    SF_INLINE void        SetConvertNumber()       { ChangeType(VT_ConvertNumber); }
    SF_INLINE void        SetConvertString()       { ChangeType(VT_ConvertString); }
    SF_INLINE void        SetConvertStringW()      { ChangeType(VT_ConvertStringW); }

    //
    // **** Methods to access and manipulate AS objects
    // 

public:
    // Visitor interfaces
    //
    typedef     ObjectInterface::ObjVisitor     ObjectVisitor;
    typedef     ObjectInterface::ArrVisitor     ArrayVisitor;


    // ----------------------------------------------------------------
    // AS Object support. These methods are only valid for Object type
    // (which includes Array and DisplayObject types)
    //
    SF_INLINE bool        HasMember(const char* name) const 
    {
        SF_ASSERT(IsObject());
        // If this is pointing to a display object and false is returned, then the 
        // display object may not exist on the stage.
        return pObjectInterface->HasMember(mValue.pData, name, IsDisplayObject());
    }
    SF_INLINE bool        GetMember(const char* name, Value* pval) const
    {
        SF_ASSERT(IsObject());
        // If the object does not contain the member, false is returned.If this is 
        // pointing to a display object and false is returned, then the display object 
        // may not exist on the stage. 
        return pObjectInterface->GetMember(mValue.pData, name, pval, IsDisplayObject());
    }
    SF_INLINE bool        SetMember(const char* name, const Value& val)
    {
        SF_ASSERT(IsObject());
        // If this is pointing to a display object and false is returned, then the 
        // display object may not exist on the stage.
        return pObjectInterface->SetMember(mValue.pData, name, val, IsDisplayObject());
    }
#ifdef GFX_AS3_SUPPORT
    //
    // These methods are available for closure references (AS3 ONLY)
    //
    SF_INLINE bool        InvokeSelf(Value* presult, const Value* pargs, UPInt nargs)
    {
        SF_ASSERT(IsClosure());
        return pObjectInterface->InvokeClosure(mValue.pData, DataAux, presult, pargs, nargs);
    }
    SF_INLINE bool        InvokeSelf(Value* presult = NULL)
    {
        SF_ASSERT(IsClosure());
        return InvokeSelf(presult, NULL, 0);
    }
#endif
    SF_INLINE bool        Invoke(const char* name, Value* presult, const Value* pargs, UPInt nargs)
    {
        SF_ASSERT(IsObject());
        // If this is pointing to a display object and false is returned, then the 
        // display object may not exist on the stage.
        return pObjectInterface->Invoke(mValue.pData, presult, name, pargs, nargs, IsDisplayObject());
    }
    SF_INLINE bool        Invoke(const char* name, Value* presult = NULL)   
    { 
        return Invoke(name, presult, NULL, 0); 
    }
    SF_INLINE void        VisitMembers(ObjectVisitor* visitor) const
    {
        SF_ASSERT(IsObject());
        // If this is pointing to a display object and false is returned, then the 
        // display object may not exist on the stage.
        return pObjectInterface->VisitMembers(mValue.pData, visitor, IsDisplayObject());
    }
    SF_INLINE bool        DeleteMember(const char* name)
    {
        SF_ASSERT(IsObject());
        // If this is pointing to a display object and false is returned, then the 
        // display object may not exist on the stage.
        return pObjectInterface->DeleteMember(mValue.pData, name, IsDisplayObject());
    }
    
#ifdef GFX_AS_ENABLE_USERDATA
    //
    // Note: The user data pointer assigned to the AS object is not ref counted and
    //       its lifetime must be managed by the application.
    //
    SF_INLINE void        SetUserData(ASUserData* pdata)
    {
        SF_ASSERT(IsObject());
        pObjectInterface->SetUserData(mValue.pData, pdata, IsDisplayObject());
    }
    SF_INLINE ASUserData* GetUserData() const
    {
        SF_ASSERT(IsObject());
        return pObjectInterface->GetUserData(mValue.pData, IsDisplayObject());
    }

#endif  // GFX_AS_ENABLE_USERDATA

    // ----------------------------------------------------------------
    // AS Array support. These methods are only valid for Array type
    //
    SF_INLINE unsigned    GetArraySize() const
    {
        SF_ASSERT(IsArray());
        return pObjectInterface->GetArraySize(mValue.pData);
    }
    SF_INLINE bool        SetArraySize(unsigned sz)
    {
        SF_ASSERT(IsArray());
        return pObjectInterface->SetArraySize(mValue.pData, sz);
    }
    SF_INLINE bool        GetElement(unsigned idx, Value* pval) const
    {
        SF_ASSERT(IsArray());
        return pObjectInterface->GetElement(mValue.pData, idx, pval);
    }
    SF_INLINE bool        SetElement(unsigned idx, const Value& val)
    {
        SF_ASSERT(IsArray());
        return pObjectInterface->SetElement(mValue.pData, idx, val);
    }
    SF_INLINE void        VisitElements(ArrayVisitor* visitor, unsigned idx, int count = -1) const
    {
        SF_ASSERT(IsArray());
        return pObjectInterface->VisitElements(mValue.pData, visitor, idx, count);
    }
    SF_INLINE void        VisitElements(ArrayVisitor* visitor) const  { VisitElements(visitor, 0); }
    SF_INLINE bool        PushBack(const Value& val)
    {
        SF_ASSERT(IsArray());
        return pObjectInterface->PushBack(mValue.pData, val);
    }
    SF_INLINE bool        PopBack(Value* pval = NULL)
    {
        SF_ASSERT(IsArray());
        return pObjectInterface->PopBack(mValue.pData, pval);
    }
    SF_INLINE bool        RemoveElements(unsigned idx, int count = -1)
    {
        SF_ASSERT(IsArray());
        return pObjectInterface->RemoveElements(mValue.pData, idx, count);
    }
    SF_INLINE bool        RemoveElement(unsigned idx)                     { return RemoveElements(idx, 1); }
    SF_INLINE bool        ClearElements()                             { return RemoveElements(0); }

	SF_INLINE bool		  IsInstanceOf(const char* className) const
	{
		SF_ASSERT(IsObject());
        return pObjectInterface->IsInstanceOf(mValue.pData, className);
	}

    // ----------------------------------------------------------------
    // AS3 ByteArray support. These methods are valid only for the AS3
    // ByteArray type
    //
#ifdef GFX_AS3_SUPPORT
    SF_INLINE bool          IsByteArray() const 
    {
        SF_ASSERT(IsObject());
        return pObjectInterface->IsByteArray(mValue.pData);
    }
    SF_INLINE unsigned      GetByteArraySize() const
    {
        // If not of ByteArray type, then returns 0. Use IsByteArray for type checking
        return pObjectInterface->GetByteArraySize(mValue.pData);
    }
	SF_INLINE void          SetByteArraySize(UPInt setSize)
    {
		pObjectInterface->SetByteArraySize(mValue.pData, setSize);
	}
    SF_INLINE bool          ReadFromByteArray(UByte *destBuff, UPInt destBuffSz)
    {
        // This method will read destBuffSz bytes from the ByteArray object, or if
        // the ByteArray is smaller, then GetByteArraySize() bytes will be read into
        // the destination buffer.
        //
        // If false is returned, then the object is not a ByteArray type
        return pObjectInterface->ReadFromByteArray(mValue.pData, destBuff, destBuffSz);
    }
    SF_INLINE bool          WriteToByteArray(const UByte *srcBuff, UPInt writeSize)
    {
        // If false is returned, then the object is not a ByteArray type
        return pObjectInterface->WriteToByteArray(mValue.pData, srcBuff, writeSize);
    }
	SF_INLINE void*          GetRawDataPtr()
    {
        return pObjectInterface->GetRawDataPtr(mValue.pData);
    }
#endif  // GFX_AS3_SUPPORT

    // ----------------------------------------------------------------
    // AS display object (MovieClips, Buttons, TextFields) support. These
    // methods are only valid for DisplayObject type
    //
    SF_INLINE bool        IsDisplayObjectActive() const
    {
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object does not exist on the stage.
        return pObjectInterface->IsDisplayObjectActive(mValue.pData);
    }
    SF_INLINE bool        GetParent(Value* pparent) const
    {
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object does not exist on the stage.
        return pObjectInterface->GetParent(mValue.pData, pparent);
    }
    SF_INLINE bool        GetDisplayInfo(DisplayInfo* pinfo) const
    {
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage
        return pObjectInterface->GetDisplayInfo(mValue.pData, pinfo);
    }
    SF_INLINE bool        SetDisplayInfo(const DisplayInfo& info)
    {
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage
        return pObjectInterface->SetDisplayInfo(mValue.pData, info);
    }
    SF_INLINE bool        GetWorldMatrix(Render::Matrix2F* pmat) const
    {
        // NOTE: Returned matrix contains translation values in pixels, rotation in 
        // radians and scales in normalized form (100% == 1.0).
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage
        return pObjectInterface->GetWorldMatrix(mValue.pData, pmat);
    }
    SF_INLINE bool        GetDisplayMatrix(Render::Matrix2F* pmat) const
    {
        // NOTE: Returned matrix contains translation values in pixels, rotation in 
        // radians and scales in normalized form (100% == 1.0).
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage
        return pObjectInterface->GetDisplayMatrix(mValue.pData, pmat);
    }
    SF_INLINE bool        SetDisplayMatrix(const Render::Matrix2F& mat)
    {
        // NOTE: Param matrix expected to contain translation values in pixels, rotation in 
        // radians and scales in normalized form (100% == 1.0).
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage or
        // the matrix has invalid (non-finite) values.
        return pObjectInterface->SetDisplayMatrix(mValue.pData, mat);
    }
    SF_INLINE bool        GetMatrix3D(Render::Matrix3F* pmat) const
    {
        // NOTE: Returned matrix contains translation values in pixels, rotation in 
        // radians and scales in normalized form (100% == 1.0).
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage
        return pObjectInterface->GetMatrix3D(mValue.pData, pmat);
    }
    SF_INLINE bool        SetMatrix3D(const Render::Matrix3F& mat)
    {
        // NOTE: Param matrix expected to contain translation values in pixels, rotation in 
        // radians and scales in normalized form (100% == 1.0).
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage or
        // the matrix has invalid (non-finite) values.
        return pObjectInterface->SetMatrix3D(mValue.pData, mat);
    }
    SF_INLINE bool        GetColorTransform(Render::Cxform* pcx) const
    {
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage
        return pObjectInterface->GetCxform(mValue.pData, pcx);
    }
    SF_INLINE bool        SetColorTransform(Render::Cxform& cx)
    {
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage
        return pObjectInterface->SetCxform(mValue.pData, cx);
    }

    // ----------------------------------------------------------------
    // AS TextField support.
    //
    // The following methods provide efficient access to the TextField.text and 
    // TextField.htmlText properties. For all other display object types, they 
    // will be equivalent to the following:
    //
    //  SetText(..)     -> SetMember("text", ..)
    //  SetTextHTML(..) -> SetMember("htmlText"), ..)
    //  GetText(..)     -> GetMember("text", ..)
    //  GetTextHTML(..) -> GetMember("htmlText", ..)
    //
    SF_INLINE bool        SetText(const char* ptext)
    {
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage,
        // or does not contain the 'text' property.
        return pObjectInterface->SetText(mValue.pData, ptext, false);
    }
    SF_INLINE bool        SetText(const wchar_t* ptext)
    {
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage,
        // or does not contain the 'text' property.
        return pObjectInterface->SetText(mValue.pData, ptext, false);
    }
    SF_INLINE bool        SetTextHTML(const char* phtml)
    {
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage,
        // or does not contain the 'htmlText' property.
        return pObjectInterface->SetText(mValue.pData, phtml, true);
    }
    SF_INLINE bool        SetTextHTML(const wchar_t* phtml)
    {
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage,
        // or does not contain the 'htmlText' property.
        return pObjectInterface->SetText(mValue.pData, phtml, true);
    }
    // Returns the raw text in the textField
    // * If textField is displaying HTML, then the displayed text returned
    SF_INLINE bool        GetText(Value* pval) const
    {
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage,
        // or does not contain the 'text' property.
        return pObjectInterface->GetText(mValue.pData, pval, false);
    }
    // Returns the displayed text of a TextField
    // * If textField is displaying HTML, then the HTML code is returned
    SF_INLINE bool        GetTextHTML(Value* pval) const
    {
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage,
        // or does not contain the 'htmlText' property.
        return pObjectInterface->GetText(mValue.pData, pval, true);
    }

    // ----------------------------------------------------------------
    // AS MovieClip support. These methods are only valid for 
    // MovieClips.
    //
    SF_INLINE bool        CreateEmptyMovieClip(GFx::Value* pmc, const char* instanceName, SInt32 depth = -1)
    {
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage,
        // or is not a MovieClip display object.
        return pObjectInterface->CreateEmptyMovieClip(mValue.pData, pmc, instanceName, depth);
    }
    SF_INLINE bool        AttachMovie(GFx::Value* pmc, const char* symbolName, const char* instanceName, 
                            SInt32 depth = -1, const MemberValueSet* initArgs = NULL)
    {
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage,
        // or is not a MovieClip display object.
        return pObjectInterface->AttachMovie(mValue.pData, pmc, symbolName, instanceName, depth, 
            initArgs);
    }
    SF_INLINE bool        GotoAndPlay(const char* frame)
    {
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage,
        // or is not a MovieClip display object.
        return pObjectInterface->GotoAndPlay(mValue.pData, frame, false);
    }
    SF_INLINE bool        GotoAndStop(const char* frame)
    {
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage,
        // or is not a MovieClip display object.
        return pObjectInterface->GotoAndPlay(mValue.pData, frame, true);
    }    
    SF_INLINE bool        GotoAndPlay(unsigned frame)
    {
        // frame is expected to be 1-based, as in Flash
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage,
        // or is not a MovieClip display object.
        return pObjectInterface->GotoAndPlay(mValue.pData, frame, false);
    }
    SF_INLINE bool        GotoAndStop(unsigned frame)
    {
        // frame is expected to be 1-based, as in Flash
        SF_ASSERT(IsDisplayObject());
        // If false is returned, then the display object may not exist on the stage,
        // or is not a MovieClip display object.
        return pObjectInterface->GotoAndPlay(mValue.pData, frame, true);
    }


    // ----------------------------------------------------------------
    // Retrieve the Movie instance that owns this object. This is only valid
    // from Values that contain references to complex types such as Object,
    // Array, DisplayObject, etc. It returns NULL for primitive types, as they
    // do not contain a reference to the ObjectInterface.
    //
    SF_INLINE Movie*    GetMovie() const   
    {
        SF_ASSERT(IsObject());
        return (Movie*)pObjectInterface->GetMovieImpl();
    }

protected:
    ObjectInterface*     pObjectInterface;

    union ValueUnion
    {
        SInt32          IValue;
        UInt32          UIValue;
        double          NValue;
        bool            BValue;
        const char*     pString;
        const char**    pStringManaged;
        const wchar_t*  pStringW;
        void*           pData;
    };

    ValueType   Type;
    ValueUnion  mValue;

#ifdef GFX_AS3_SUPPORT
    UPInt       DataAux;
#endif

    SF_INLINE bool        IsManagedValue() const   
    { 
#ifdef SF_BUILD_DEBUG
        bool ok = ((Type & VTC_ManagedBit) != 0);
        if (ok) SF_ASSERT(pObjectInterface);
        return ok;
#else
        return (Type & VTC_ManagedBit) != 0; 
#endif
    }
    SF_INLINE void        ChangeType(ValueType type) 
    {
        if (IsManagedValue()) ReleaseManagedValue();
        Type = type;
    }
    void                AcquireManagedValue(const Value& src)
    {
        SF_ASSERT(src.mValue.pData && (src.pObjectInterface != NULL));
        pObjectInterface = src.pObjectInterface;
        pObjectInterface->ObjectAddRef(this, mValue.pData);
    }
    void                ReleaseManagedValue()
    {
        SF_ASSERT(mValue.pData && (pObjectInterface != NULL));
        pObjectInterface->ObjectRelease(this, mValue.pData);
        pObjectInterface = NULL;
    }
};

struct MemberValue
{
    String  Key;
    Value   mValue;
    MemberValue(const String& key, const Value& val) :
        Key(key), mValue(val) {}
};
class MemberValueSet : public ArrayCPP<MemberValue> {};


// *****

#ifdef GFX_AS_ENABLE_USERDATA

//
// GFxASUserData allows developers the ability to store custom data with an
// ActionScript VM object and also be notified when it is destroyed. An 
// instance of this handler can be installed with an AS object via the GFxValue 
// interface: GFxValue::SetUserData. 
//
class ASUserData : public RefCountBase<ASUserData, Stat_Default_Mem>
{
    friend class Value;

    // Weak ref descriptor for object that was last set.
    Value::ObjectInterface*     pLastObjectInterface;
    void*                       pLastData;
    bool                        IsLastDispObj;

public:
    ASUserData() : pLastObjectInterface(NULL), pLastData(NULL), IsLastDispObj(false) {}
    virtual ~ASUserData() {}

    // Sets the weak ref descriptor. Only used internally.
    void            SetLastObjectValue(Value::ObjectInterface* pobjIfc, void* pdata, bool isdobj);
    // Fills in a GFxValue for the last object to which this User data was assigned
    // Will fail if OnDestroy was called
    bool            GetLastObjectValue(Value* value) const;

    virtual void    OnDestroy(Movie* pmovie, void* pobject) = 0;
};

#endif   // GFX_AS_ENABLE_USERDATA


// *****

// FunctionHandler allows the creation of C++ callbacks that behave like
// any AS2 VM function. By using Movie::CreateFunction, users can create
// AS2 function objects that can be assigned to any variable in the VM. This
// function object can be invoked from within the VM, and the invokation will
// be reported via the Call method.
//
// The following example sets a custom function object to a member of an AS
// VM object located at '_root.obj':
//
//               Value obj;
//               pmovie->GetVariable(&obj, "_root.obj");
//
//              class MyFunc : public FunctionHandler
//               {
//               public:
//                   virtual void Call(const Params& params)
//                   {
//                       SF_UNUSED(params);
//                       printf("> MY FUNCTION CALLED (Call)\n");
//                   }
//               };
//               Ptr<MyFunc> customFunc = *SF_HEAP_NEW(Memory::GetGlobalHeap()) MyFunc();               
//               Value func, func2;               
//               pmovie->CreateFunction(&func, customFunc);
//               obj.SetMember("func", func);
//
// In ActionScript, the customFunc::Call method can be triggered by simply 
// invoking the AS function object:
//               
//               obj.func(param1, param2, ...);
//
//
// The Params::pThis points to the calling context (undefined if not applicable).
// The calling context is usually the object that owns the function object. Ex:
//          var obj:Object = {};
//          obj.myfunc();   <--- obj is pThis
//
// The Params::pRetVal is expected to be filled in by the user if a return
// value is expected by the function's semantics.
//
// The Params::pArgsWithThisRef is used for chaining other function objects. 
// A chain can be created by using the following syntax (if MyOtherFuncObj
// is a reference to a valid function object):
//
//          MyOtherFuncObj::Invoke("call", NULL, params.pRetVal, 
//                                  params.pArgsWithThisRef, params.ArgCount + 1)
// 
// Chaining is useful if there is a need to inject a callback at the beginning
// or end of an existing AS VM function. 
//
// The Params::pUserData member can be used for custom function object specific
// data, such as an index that can be used as a switch inside a single function
// context object. This data is set by the Movie::CreateFunction.
//
//
// NOTE: If a user defined FunctionHandler instance has a Value member
//       that is holding a reference to an AS VM Object (Object, Array, MovieClip,
//       etc.) then it MUST be cleaned up by the user before the movie root dies.
//       This can be achieved by calling Value.SetUndefined(); on the member.
//



class FunctionHandler : public RefCountBase<FunctionHandler, Stat_Default_Mem>
{
public:
    // Parameters passed in to the callback from the VM
    struct Params 
    {
        Value*          pRetVal;
        Movie*          pMovie;
        Value*          pThis;
        Value*          pArgsWithThisRef;
        Value*          pArgs;
        unsigned        ArgCount;
        void*           pUserData;
    };

    virtual ~FunctionHandler() {}

    virtual void    Call(const Params& params)      = 0;  
};

// *** VirtualKeyboardInterface State

class VirtualKeyboardInterface : public State
{
public:
    VirtualKeyboardInterface() : State(State_VirtualKeyboardInterface) { }

    // Invoked when an input textfield recieves focus.
    // 'textBox' contains bounds of a textfield in world coordinates (stage), in pixels.
    virtual void    OnInputTextfieldFocusIn(bool multiline, const Render::RectF& textBox) =0;

    // Invoked when an input textfield loses focus
    virtual void    OnInputTextfieldFocusOut() =0;
};

// Sets the external interface used.
SF_INLINE void StateBag::SetVirtualKeyboardInterface(VirtualKeyboardInterface* p)
{
    SetState(State::State_VirtualKeyboardInterface, p);
}
SF_INLINE Ptr<VirtualKeyboardInterface> StateBag::GetVirtualKeyboardInterface() const
{
    return *(VirtualKeyboardInterface*) GetStateAddRef(State::State_VirtualKeyboardInterface);
}

// *** ExternalInterface State

class ExternalInterface : public State
{
public:
    ExternalInterface() : State(State_ExternalInterface) { }

    virtual ~ExternalInterface() {}

    // A callback for ExternalInterface.call. 'methodName' may be NULL, if ExternalInterface is
    // called without parameter. 'args' may be NULL, if argCount is 0. The value, returned 
    // by this callback will be returned by 'ExternalInterface.call' method.
    // To return a value, use pmovieView->SetExternalInterfaceRetVal method.
    virtual void Callback(Movie* pmovieView, const char* methodName, const Value* args, unsigned argCount) = 0;
};


// Sets the external interface used.
SF_INLINE void StateBag::SetExternalInterface(ExternalInterface* p)
{
    SetState(State::State_ExternalInterface, p);
}
SF_INLINE Ptr<ExternalInterface> StateBag::GetExternalInterface() const
{
    return *(ExternalInterface*) GetStateAddRef(State::State_ExternalInterface);
}

/////////////////////////////////

// *** ExtensionContextInterface State
// An interface that represents platform specific layer for ExtensionManager.
class ExtensionContextInterface : public State
{
public:
	ExtensionContextInterface() : State(State_ExtensionContextInterface) {}

	virtual bool Call(const char* extensionID, const char* contextID, const char* functionName, unsigned argc, const Scaleform::GFx::Value* const argv, Scaleform::GFx::Value* const result) = 0;
	virtual const char* GetExtensionDirectory(const char* extensionID) = 0;
	virtual void FinalizeExtensionContext(const char* extensionID, const char* contextID) = 0;
	virtual void InitializeExtensionContext(const char* extensionID, const char* contextID) = 0;
	virtual GFx::Value*	GetActionScriptData(const char* extensionID, const char* contextID) = 0;
	virtual void SetActionScriptData(const char* extensionID, const char* contextID, GFx::Value* data) = 0;
	virtual void SetMovie(Movie* pmovie) = 0;
};

// Sets the extension manager used.
SF_INLINE void StateBag::SetExtensionContextInterface(ExtensionContextInterface* p)
{
	SetState(State::State_ExtensionContextInterface, p);
}
SF_INLINE Ptr<ExtensionContextInterface> StateBag::GetExtensionContextInterface() const
{
	return *(ExtensionContextInterface*) GetStateAddRef(State::State_ExtensionContextInterface);
}

// *** MultitouchInterface State

// An interface that represents platform specific layer for Multitouch support.
class MultitouchInterface : public State
{
public:
    enum MultitouchInputMode
    {
        MTI_None        = 0,
        MTI_TouchPoint  = 0x1,
        MTI_Gesture     = 0x2,
        MTI_Mixed       = (MTI_TouchPoint | MTI_Gesture)
    };

    enum GestureMask
    {
        MTG_None        = 0,
        MTG_Pan         = 0x1,
        MTG_Zoom        = 0x2,
        MTG_Rotate      = 0x4,
        MTG_Swipe       = 0x8
    };

    MultitouchInterface() : State(State_MultitouchInterface) {}

    // Should return maximum number of touch points supported by hardware
    virtual unsigned GetMaxTouchPoints() const =0;

    // Should return a bit mask of supported gestures (see MTG_*)
    virtual UInt32   GetSupportedGesturesMask() const =0;

    // Sets multitouch input mode. If the input mode is not supported it should
    // return 'false'; otherwise 'true'.
    virtual bool     SetMultitouchInputMode(MultitouchInputMode) =0;
};

// Sets the external interface used.
SF_INLINE void StateBag::SetMultitouchInterface(MultitouchInterface* p)
{
    SetState(State::State_MultitouchInterface, p);
}
SF_INLINE Ptr<MultitouchInterface> StateBag::GetMultitouchInterface() const
{
    return *(MultitouchInterface*) GetStateAddRef(State::State_MultitouchInterface);
}


// *** AccelerometerInterface State

// An interface that represents platform specific layer for Accelerometer.
class AccelerometerInterface : public State
{
public:
    AccelerometerInterface() : State(State_AccelerometerInterface) {}

	virtual bool	RegisterAccelerometer(int accelerometerId) = 0;
	virtual bool	UnregisterAccelerometer(int accelerometerId) = 0;
	virtual bool	IsAccelerometerMuted() const = 0;
	virtual bool	IsAccelerometerSupported() const = 0;
	virtual void	SetAccelerometerInterval(int accelerometerId, int interval) = 0;
};

// Sets the external interface used.
SF_INLINE void StateBag::SetAccelerometerInterface(AccelerometerInterface* p)
{
    SetState(State::State_AccelerometerInterface, p);
}
SF_INLINE Ptr<AccelerometerInterface> StateBag::GetAccelerometerInterface() const
{
    return *(AccelerometerInterface*) GetStateAddRef(State::State_AccelerometerInterface);
}


// *** GeolocationInterface State

// An interface that represents platform specific layer for Geolocation.
class GeolocationInterface : public State
{
public:
    GeolocationInterface() : State(State_GeolocationInterface) {}

	virtual bool	RegisterGeolocation(int geolocationId) = 0;
	virtual bool	UnregisterGeolocation(int geolocationId) = 0;
	virtual bool	IsGeolocationMuted() const = 0;
	virtual bool	IsGeolocationSupported() const = 0;
	virtual void	SetGeolocationInterval(int geolocationId, int interval) = 0;
};

// Sets the external interface used.
SF_INLINE void StateBag::SetGeolocationInterface(GeolocationInterface* p)
{
    SetState(State::State_GeolocationInterface, p);
}
SF_INLINE Ptr<GeolocationInterface> StateBag::GetGeolocationInterface() const
{
    return *(GeolocationInterface*) GetStateAddRef(State::State_GeolocationInterface);
}

// MovieDisplayHandle identifies Movie's render tree representation,
// intended to be passed to Renderer2D::Display method on the render thread.
// NextCapture will typically need to be called on MovieDisplayHandle
// to obtain the most recent captured snapshot before rendering.

typedef Render::DisplayHandle<Render::TreeRoot> MovieDisplayHandle;


// Movie is created by the MovieDef::CreateInstance function and is a
// primary user interface to a movie instance. After being created,
// Movie is normally configured by setting the renderer, viewport, and
// fscommand callback, if necessary.
//
// After that the movie is ready for playback. Its animation state can be
// advanced by calling Advance() and it can be rendered at by calling Display().
// Both of those methods are inherited from Movie.

class Movie : public RefCountBase<Movie, StatMV_Other_Mem>, public StateBag
{
public:
    Ptr<ASMovieRootBase> pASMovieRoot;

    Movie();
    ~Movie();

    // Obtain the movie definition that created us.
    virtual MovieDef*  GetMovieDef() const = 0;

    // Delegated for convenience.
    SF_INLINE unsigned GetFrameCount() const { return GetMovieDef()->GetFrameCount(); }
    SF_INLINE float    GetFrameRate() const  { return GetMovieDef()->GetFrameRate(); }

    // Retrieve AVM Version (1 = AS2, 2 = AS3)
    int                 GetAVMVersion() const;

    // Frame counts in this API are 0-Based (unlike ActionScript)
    virtual unsigned    GetCurrentFrame() const = 0;
    virtual bool        HasLooped() const = 0;

    // Moves a playhead to a specified frame. Note that calling this method
    // may cause ActionScript tags attached to frames to be bypassed.
    virtual void        GotoFrame(unsigned frameNumber) = 0;
    // Returns true if labeled frame is found.
    virtual bool        GotoLabeledFrame(const char* plabel, int offset = 0) = 0;

    // Changes playback state, allowing animation to be paused
    // and resumed. Setting the state to Stopped will prevent Advance from
    // advancing movie frames.
    virtual void        SetPlayState(PlayState s) = 0;
    virtual PlayState   GetPlayState() const = 0;

    // Validation. Returns whether or not the movie was instantiated by
    // a correctly-licensed Scaleform instance.
    virtual bool        IsValid() = 0;

    // Visibility control.
    // Make the movie visible/invisible.  An invisible movie does not
    // advance and does not render.
    virtual void    SetVisible(bool visible) = 0;
    virtual bool    GetVisible() const = 0;

    // *** Action Script Interface
    // Returns timer, in milliseconds, the same value as ActionScript's getTimer returns.
    virtual UInt64  GetASTimerMs() const = 0;

    // Checks for availability of a field/method/nested clip. This can be used to
    // determine if a certain variable can be accessed, or an Invoke can be called
    // without an unexpected failure warning.
    bool    IsAvailable(const char* ppathToVar) const;

    // String Creation
    // The lifetime of the string created is maintained by the runtime.
    // If the runtime dies, then the Value is invalid. Make sure to release
    // the Value before destroying the movie.
    //
    void    CreateString(Value* pvalue, const char* pstring);
    void    CreateStringW(Value* pvalue, const wchar_t* pstring);

    // Object Creation
    // The lifetime of the object/array created is maintained by the runtime.
    // If the runtime dies, then the Value is invalid. Make sure to release
    // the Value before destroying the movie.
    //
    // The className parameter of CreateObject can accept fully qualified class
    // names, such as flash.geom.Matrix. Without using the fully qualified name
    // classes in packages will not created by this method.
    void    CreateObject(Value* pvalue, const char* className = NULL, 
                                   const Value* pargs = NULL, unsigned nargs = 0);
    void    CreateArray(Value* pvalue);
    // Create a special function object that wraps a C++ function. The function
    // object has the same functionality as any AS2 object, but supports the ability
    // to be invoked in the AS2 VM.
    void    CreateFunction(Value* pvalue, FunctionHandler* pfc, void* puserData = NULL);

    // Variable Access

    // Set an ActionScript variable within this movie. This can be used to
    // to set the value of text fields, variables, and properties of
    // named nested characters within the movie.

    enum SetVarType
    {
        SV_Normal,      // Sets variable only if target clip is found.
        SV_Sticky,      // Sets variable if target clip is found, otherwise
        // queues a set until the clip is created at path.
        // When the target clip, value will be lost.
        SV_Permanent    // Sets variable applied to this and all future
        // clips at given path.
    };

    // Set a variable identified by a path to a new value.
    // Specifying "sticky" flag will cause the value assignment to be pending until its target 
    // object is created. This allows initialization of objects created in later key-frames.
    // SetVariable will fail if path is invalid.
    bool    SetVariable(const char* ppathToVar, const Value& value,
                                  SetVarType setType = SV_Sticky);

    // Obtain a variable. If *pval has a VT_Convert type, the result will always be filled in with that type,
    // even if GetVariable fails. Otherwise, the value is untouched on failure.
    bool    GetVariable(Value *pval, const char* ppathToVar) const;

    // Convenience inline methods for variable access.

    // Set a variable identified by a path to a new value, in UTF8.
    bool    SetVariable(const char* ppathToVar, const char* pvalue,
                                  SetVarType setType = SV_Sticky);
    // Unicode string version of SetVariable for convenience.
    bool    SetVariable(const char* ppathToVar, const wchar_t* pvalue,
                                  SetVarType setType = SV_Sticky);
    // Sets a variable to a Double value.
    bool    SetVariableDouble(const char* ppathToVar, Double value,
                                        SetVarType setType = SV_Sticky);

    // Gets a variable as a Double; return 0.0 if path was not found.
    Double  GetVariableDouble(const char* ppathToVar) const;


    // Variable Array Access

    // Type of array being set.
    enum SetArrayType
    {
        SA_Int,     // Array of 'int '
        SA_Double,  // Array of 'Double' in memory
        SA_Float,   // Array of 'float' in memory
        SA_String,  // Array of 'const char*'.      (Deprecated; use SA_Value)
        SA_StringW, // Array of 'const wchar_t*'.   (Deprecated; use SA_Value)
        SA_Value    // Array of Value.
    };

    // Sets array elements in specified range to data items of specified type.
    // If array does not exist, it is created. If an array already exists but
    // does not contain enough items, it is resized appropriately. Under some
    // conditions index > 0 can cause new elements to be inserted into the
    // array. These elements are initialized to 0 for numeric types, empty
    // string "" for strings, and 'undefined' for values. SetVariableArray 
    // can fail if there is not enough memory or if the path is invalid.
    bool    SetVariableArray(SetArrayType type, const char* ppathToVar,
                                       unsigned index, const void* pdata, unsigned count,
                                       SetVarType setType = SV_Sticky);

    // Sets array's size. If array doesn't exists it will allocate array and resize it. 
    // No data will be assigned to its elements.
    bool    SetVariableArraySize(const char* ppathToVar, unsigned count,
                                           SetVarType setType = SV_Sticky);

    // Returns the size of array buffer necessary for GetVariableArray.
    unsigned    GetVariableArraySize(const char* ppathToVar);

    // Populates a buffer with results from an array. The pdata pointer must
    // contain enough space for count items. If a variable path is not found
    // or is not an array, 0 will be returned an no data written to buffer.
    // The buffers allocated for string content are temporary within the
    // Movie, both when SA_String and SA_Value types are used. These
    // buffers may be overwritten after the next call to GetVariableArray;
    // they will also be lost if Movie dies. This means that users
    // should copy any string data immediately after the call, as access of 
    // it later may cause a crash.
    bool    GetVariableArray(SetArrayType type, const char* ppathToVar,
                                       unsigned index, void* pdata, unsigned count);

    // Convenience methods for Value.
    bool    SetVariableArray(const char* ppathToVar, unsigned index, const Value* pdata, unsigned count,
                                        SetVarType setType = SV_Sticky);
    bool    GetVariableArray(const char* ppathToVar, unsigned index, Value* pdata, unsigned count);    

    // ActionScript Invoke

    // ActionScript method invoke interface. There are two different
    // approaches to passing data into Invoke:
    //  1) Using an array of Value objects (slightly more efficient).
    //  2) Using a format string followed by a variable argument list.
    //
    // Return values are stored in Value object, with potential conversion
    // to UTF8 strings or other types.  If a string is returned, it is stored
    // within a temporary  buffer in Movie and should be copied
    // immediately (if required by user). The value in a temporary buffer may
    // be overwritten by the next call to GetVariable or Invoke.
    //
    // Use case (1):
    //
    //   Value args[3], result;
    //   args[0].SetNumber(i);
    //   args[1].SetString("test");
    //   args[2].SetNumber(3.5);
    //
    //   pMovie->Invoke("path.to.methodName", &result, args, 3);
    //
    // Use case (2):
    //
    //   pmovie->Invoke("path.to.methodName", "%d, %s, %f", i, "test", 3.5);
    //
    // The printf style format string describes arguments that follow in the
    // same  order; it can include commas and whitespace characters that are
    // ignored. Valid format arguments are:
    //
    //  %u      - undefined value, no argument required
    //  %d      - integer argument
    //  %s      - null-terminated char* string argument
    //  %ls     - null-terminated wchar_t* string argument
    //  %f      - double argument
    //  %hf     - float argument, only in InvokeArgs
    //
    // pmethodName is a name of the method to call, with a possible namespace
    // syntax to access methods in the nested objects and movie clips.
    //

    // *** Invokes:

    // presult can be null if no return value is desired.
    bool       Invoke(const char* ppathToMethod, Value *presult, const Value* pargs, unsigned numArgs);
    bool       Invoke(const char* ppathToMethod, Value *presult, const char* pargFmt, ...);
    bool       InvokeArgs(const char* ppathToMethod, Value *presult, const char* pargFmt, va_list args);
    bool       Invoke(const char* ppathToMethod, const char* pargFmt, ...)
    {
        va_list args;
        bool retVal;
        va_start(args, pargFmt);
        retVal = InvokeArgs(ppathToMethod, NULL, pargFmt, args);
        va_end(args);
        return retVal;
    }


    // Renderer configuration is now in GFxStateBag.

    // *** Viewport and Scaling configuration

    // Sets the render-target surface viewport to which the movie is scaled;
    // coordinates are specified in pixels. If not specified, the default
    // viewport is (0,0, MovieWidth, MovieHeight).
    virtual void        SetViewport(const Viewport& viewDesc) = 0;
    virtual void        GetViewport(Viewport *pviewDesc) const = 0;

    // Viewport inline helper; mostly provided for compatibility.
    SF_INLINE void      SetViewport(int bufw, int bufh, int left, int top,
                                    int w, int h, unsigned flags = 0)
    {
        Viewport desc(bufw, bufh, left, top, w, h, flags);
        SetViewport(desc);
    }
    // Scale mode
    enum ScaleModeType
    {
        SM_NoScale,
        SM_ShowAll,
        SM_ExactFit,
        SM_NoBorder
    };
    virtual void          SetViewScaleMode(ScaleModeType) = 0;
    virtual ScaleModeType GetViewScaleMode() const        = 0;
    enum AlignType
    {
        Align_Center,
        Align_TopCenter,
        Align_BottomCenter,
        Align_CenterLeft,
        Align_CenterRight,
        Align_TopLeft,
        Align_TopRight,
        Align_BottomLeft,
        Align_BottomRight
    };
    virtual void        SetViewAlignment(AlignType) = 0;
    virtual AlignType   GetViewAlignment() const    = 0;

    // Returns currently visible frame rectangle, in pixels
    virtual RectF       GetVisibleFrameRect() const = 0;

    // Sets/gets safe rectangle, in pixels. Movie just stores
    // this rectangle, it is user's responsibility to update it. It is 
    // possible to retrieve it through ActionScript - Stage.safeRect 
    // (extensions should be turned on).
    virtual RectF       GetSafeRect() const = 0;
    virtual void        SetSafeRect(const RectF& rect) = 0;


    // Configured EdgeAA mode used for the root of the render tree,
    // which typically controls all of the movie. Pass EdgeAA_Disable
    // here to completely disabled EdgeAA.
    virtual void        SetEdgeAAMode(Render::EdgeAAMode edgeAA) = 0;
    virtual Render::EdgeAAMode  GetEdgeAAMode() const = 0;
 
/*
    // *** Execution State
    virtual void           SetSoundPlayer(SoundRenderer* ps) = 0;
    virtual SoundRenderer*  GetSoundPlayer() const = 0;
*/
    virtual void        Restart(bool advance0 = true) = 0;

    // Advance a movie based on the time that has passed since th last
    // advance call, in seconds. In general, the time interval should
    // be <= 1 / movie FrameRate. If it is not, multiple frames will be
    // advanced only of frameCatchUp == 1.
    // Returns time remaining till next advance must be called, in seconds.
    virtual float       Advance(float deltaT, unsigned frameCatchUpCount = 2,
                                bool capture = true) = 0;

    // Explicitly force a render tree Capture, making it available for
    // the render thread. Explicit call to this function may be necessary
    // if an Invoke or Direct Access API call has modified the movie state,
    // while Advance hasn't yet been called. Without Capture, modification
    // results would not be visible in the render thread.
    virtual void        Capture(bool onChangeOnly = true) = 0;

    // Returns a display handle that should be passed to the render thread
    // and used for movie rendering. Typically the render thread will
    // take the following steps each frame:
    //   - Call NextCapture() on the handle to grab most recent tree snapshot.
    //   - Call Renderer2D::Display to render the movie.
    virtual const MovieDisplayHandle& GetDisplayHandle() const = 0;

    // Sets the expected thread the Capture function will be called from. This
    // is for debugging purposes only; it will trigger asserts if the movie's
    // data is access or modified from any thread other than the one given here.
    // The thread defaults to thread the thread the Movie was created on if this
    // function is not called explicitly.
    virtual void        SetCaptureThread(ThreadId captureThreadId) = 0;

    // ShutdownRendering shuts down rendering of the movie; it can be called
    // before Movie is released to avoid render-thread block in destructor.
    // After ShutdownRendering(false), IsShutdownRenderingComplete call can be 
    // used to determine when shutdown is complete.
    // Advance, Invoke or other state-modifying functions should NOT be called
    // once shutdown has started.
    virtual void    ShutdownRendering(bool wait) = 0;
    // Returns true once rendering has been shut down after ShutdownRendering call; 
    // intended for polling from main thread.
    virtual bool    IsShutdownRenderingComplete() const = 0;


    // pause/unpause the movie
    virtual void    SetPause(bool pause) = 0;
    // returns true if the movie is paused.
    virtual bool    IsPaused() const = 0;


    // *** Background color interface
    
    virtual void    SetBackgroundColor(const Render::Color BgColor) = 0;

    // Sets the background color alpha applied to the movie clip.
    // Set to 0.0f if you do not want the movie to render its background
    // (ex. if you are displaying a hud on top of the game scene).
    // The default value is 1.0f, indication full opacity.
    virtual void    SetBackgroundAlpha(float alpha) = 0;
    virtual float   GetBackgroundAlpha() const = 0;

    // *** Input

    // Event notification interface.
    // Users can call this handler to inform GFxPlayer of mouse and keyboard
    // input, as well as stage and other events.
    enum HE_ReturnValueType
    {
        HE_NotHandled           = 0,
        HE_Handled              = 1,
        HE_NoDefaultAction      = 2,
        HE_Completed            = (HE_Handled | HE_NoDefaultAction)
    };
    virtual unsigned    HandleEvent(const Event &event)          = 0;

    // State interface. This interface can be used to inform about the
    // current mouse/keyboard state. For simple cases, it can be used
    // instead of the event interface; changes in state will automatically
    // generate internal events.

    // Retrieve mouse state, such as coordinates and buttons state.
    // "buttons" is a bit mask, where bit 0 specifies left button, 
    // bit 1 - right, bit 2 - middle.
    virtual void        GetMouseState(unsigned mouseIndex,
                                      float* x, float* y, unsigned* buttons) = 0;

    // Notifies of mouse state; should be called at regular intervals.
    // The mouse coordinates need to be in the coordinate system of the
    // viewport set by SetViewport.
    // Note, this function is deprecated, use HandleEvent instead.
    virtual void        NotifyMouseState(float x, float y,
                                         unsigned buttons, unsigned mouseIndex = 0) = 0;

    enum HitTestType
    {
        HitTest_Bounds = 0,
        HitTest_Shapes = 1,
        HitTest_ButtonEvents = 2,
        HitTest_ShapesNoInvisible = 3
    };
    // Performs a hit test on the movie view and returns whether or not
    // the passed point is inside the valid area
    virtual bool        HitTest(float x, float y, HitTestType testCond = HitTest_Shapes, 
        unsigned controllerIdx = 0) = 0;

    // *** ExternalInterface / User value handling

    // Used to set the return value from ExternalInterface::Callback function.
    void        SetExternalInterfaceRetVal(const Value& v);

    // Set and get user data handle; useful for the FsCommand handler.
    virtual void*       GetUserData() const = 0;
    virtual void        SetUserData(void*) = 0;


    // Returns true, if movie has focus.
    virtual bool        IsMovieFocused() const = 0;

    // Returns and optionally resets the "dirty flag" that indicates 
    // whether anything was changed on the stage (and need to be 
    // re-rendered) or not. Note, 'doReset' parameter will take effect
    // only after Capture is called (either manually or by Advance).
    virtual bool        GetDirtyFlag(bool doReset = true) = 0;

    // Sets/gets number of supported mice. If sets to 0 then mouse support 
    // is disabled. Note, supporting mouse
    // incurs extra processing overhead during Advance due to the execution
    // of extra hit-testing logic when objects move; disabling it will
    // cause mouse events and NotifyMouseState to be ignored.
    // The default state is controlled by the SF_MOUSE_SUPPORT_ENABLED
    // configuration option and is disabled on consoles (all, but Wii) 
    // and set to 1 on PCs.
    virtual void        SetMouseCursorCount(unsigned n) = 0;
    virtual unsigned    GetMouseCursorCount() const = 0;

    virtual void        SetControllerCount(unsigned n) = 0;
    virtual unsigned    GetControllerCount() const = 0;

    // Obtains statistics for the movie view and performs a reset
    // for tracked counters.
    //   - If reset is true, it is done after the stats update.
    //   - If pbag is null, only reset can take place.    
    virtual void        GetStats(StatBag* pbag, bool reset = true) = 0;

    virtual MemoryHeap* GetHeap() const = 0;    

    enum GCFlags
    {
        GCF_Quick  = 0, // quick collect, only relatively new nodes are iterated
        GCF_Medium = 1, // medium collect, older nodes are collected too
        GCF_Full   = 2  // full collect, all nodes are iterated
    };
    // Forces to run garbage collection, if it is enabled. Does nothing otherwise.
    // 'gcFlags' parameter allows to control the speed of GC. GCF_Quick is fastest
    // but less memory is recovered, whereas GCF_Full is slowest but all possible 
    // memory is recovered.
    virtual void        ForceCollectGarbage(unsigned gcFlags = GCF_Full) = 0;

    // Additional GC control functions. 
    // SuspendGC suspends/resumes garbage collection. It is counted operation, meaning
    // if it was suspended N-times then it should be re-enabled N-times to restore normal operation.
    virtual void        SuspendGC(bool suspend) =0;

    // Schedule garbage collection. Unlike ForceCollectGarbage it doesn't execute collection immediately;
    // instead, it will be executed when next Advance is called.
    // 'gcFlags' parameter allows to control the speed of GC. GCF_Quick is fastest
    // but less memory is recovered, whereas GCF_Full is slowest but all possible 
    // memory is recovered.
    virtual void        ScheduleGC(unsigned gcFlags = GCF_Full) =0;

    enum ReportFlags
    {
        // display short filenames (w/o paths)
        Report_ShortFileNames       = 0x0001,

        // avoid displaying circular refs, which involved VMAbcFile.
        Report_NoCircularReferences = 0x0002,

        // suppress overall statistics
        Report_SuppressOverallStats = 0x0004,

        // display addresses only for anonymous objects (the ones that do not have any name)
        Report_AddressesForAnonymObjsOnly = 0x0008,

        // suppress displaying moviedefs held by imports and fonts.
        Report_SuppressMovieDefsStats = 0x0010,

        // don't replace common parts by ellipsis
        Report_NoEllipsis             = 0x0020
    };
    // Prints out a report about objects and links between them.
    // 'flags' - bit masks that controls the output, see ReportFlags enum above.
    // If 'log' is not specified then the dump will be printed by using a default log.
    // 'swfName' - if specified then report will include only objects within the 
    // specified swf file.
    virtual void        PrintObjectsReport(UInt32 flags = 0, 
                                           Log* log = NULL,  
                                           const char* swfName = NULL,
                                           Ptr<AmpMovieObjectDesc>* root = NULL,
                                           MemoryHeap* heap = NULL) = 0;

    static SF_INLINE float SF_STDCALL GetRenderPixelScale() { return 20.f; }

    // Translates the point in Flash coordinates to screen 
    // (window) coordinates. These methods takes into account the world matrix
    // of root, the viewport matrix and the user matrix from the renderer. 
    // Source coordinates should be in _root coordinate space, in pixels.
    virtual Render::PointF TranslateToScreen(const Render::PointF& p,
                                             Matrix2F* puserMatrix = 0) = 0;

    // Translates the rectangle in Flash coordinates to screen 
    // (window) coordinates. These methods takes into account the world matrix
    // of root, the viewport matrix and the user matrix from the renderer. 
    // Source coordinates should be in _root coordinate space, in pixels.
    virtual RectF  TranslateToScreen(const RectF& p, Matrix2F* puserMatrix = 0) = 0;

    // Translates the point in the character's coordinate space to the point on screen (window).
    // pathToCharacter - path to a movieclip, textfield or button, i.e. "_root.hud.mc";
    // pt is in pixels, in coordinate space of the character, specified by the pathToCharacter
    // This method writes new point coordinates into presPt (value is in pixels of screen).
    // Returns false of character is not found; true otherwise.
    virtual bool    TranslateLocalToScreen(const char* pathToCharacter, 
                                           const Render::PointF& pt, Render::PointF* presPt, 
                                           Matrix2F* userMatrix = 0) = 0;
    // associate a focus group with a controller.
    virtual bool    SetControllerFocusGroup(unsigned controllerIdx, unsigned focusGroupIndex) =0;

    // returns focus group associated with a controller
    virtual unsigned GetControllerFocusGroup(unsigned controllerIdx) const =0;

    // Resets input focus the the specified controller. If a textField has a focus it will lose it.
    virtual void    ResetInputFocus(unsigned controllerIdx) =0;

    // Release method for custom behavior
    void            Release();   

    // Checks if exit was requested by ActionScript.
    virtual bool    IsExitRequested() const =0;

    // 3D view matrix for movie
    virtual void    GetViewMatrix3D(Matrix3F *) = 0;
    virtual void    SetViewMatrix3D(const Matrix3F& m) = 0;

    // 3D projection matrix for movie
    virtual void    GetProjectionMatrix3D(Matrix4F *) = 0;
    virtual void    SetProjectionMatrix3D(const Matrix4F& m) = 0;

    // Sets stage auto orientation flag
    virtual void    SetStageAutoOrients(bool v = true) =0;

    // Forces render tree to be updated in order to apply all possible changes
    // related to images and/or textures.
    virtual void    ForceUpdateImages() =0;

    // Scrolls viewport to make 'box' fully visible inside the 'screenRect'.
    // 'screenRect' represents a rectangle in screen pixels;
    // 'box' represents the area that should become visible, in stage pixel coordinates.
    // 'flags' - bit flags (see below)
    // This method changes Viewport matrix; if viewport is reset all changes done by this
    // method are lost. The viewport matrix can be forcedly reset to its original state
    // by calling RestoreViewport.
    enum    
    {
        MAVF_DontScaleDown = 0x1, // Don't scale down if box doesn't fit completely in screenRect
        MAVF_ScaleUp50     = 0x2, // Scale up if box's area is less than 50% of the screenRect
        MAVF_LeftTopToZero = 0x4  // left-top corner of box in (0,0) of screenRect (not center as by default)
    };
    virtual void    MakeAreaVisible(const Render::RectF& screenRect, 
                                    const Render::RectF& box,
                                    UInt32 flags = 0) =0;
    // Restores viewport matrix changed by MakeAreaVisible to its original state. 
    virtual void    RestoreViewport() =0;
};

// *** Inline Implementation - Movie

SF_INLINE bool    Movie::SetVariable(const char* ppathToVar, const char* pvalue, SetVarType setType)
{
    Value v(pvalue);
    return SetVariable(ppathToVar, v, setType);
}
SF_INLINE bool    Movie::SetVariable(const char* ppathToVar, const wchar_t* pvalue, SetVarType setType)
{
    Value v(pvalue);
    return SetVariable(ppathToVar, v, setType);
}
SF_INLINE bool    Movie::SetVariableDouble(const char* ppathToVar, Double value, SetVarType setType)
{
    Value v(value);
    return SetVariable(ppathToVar, v, setType);
}
SF_INLINE Double   Movie::GetVariableDouble(const char* ppathToVar) const
{
    Value v(Value::VT_ConvertNumber);
    GetVariable(&v, ppathToVar);
    if (v.GetType() == Value::VT_Number)
        return v.GetNumber();
    return 0.0;    
}

// Convenience methods for Value.
SF_INLINE  bool    Movie::SetVariableArray(const char* ppathToVar, unsigned index,
                                           const Value* pdata, unsigned count, SetVarType setType)
{
    return SetVariableArray(SA_Value, ppathToVar, index, pdata, count, setType);
}
SF_INLINE  bool    Movie::GetVariableArray(const char* ppathToVar, unsigned index,
                                           Value* pdata, unsigned count)
{
    return GetVariableArray(SA_Value, ppathToVar, index, pdata, count);
}

// *** End Inline Implementation - Movie



// ***** Base class for objects defined in external libraries (such as XML)

class ExternalLibPtr
{
    // No special implementation. This is simply an empty base class.

protected:
    // Pointer to the movie root that holds this pointer
    // Used by the lib ptr object to unregistered itself
    MovieImpl*   pOwner;

public:
    ExternalLibPtr(MovieImpl* pmovieroot) 
        : pOwner(pmovieroot) {}
    virtual ~ExternalLibPtr() {}
};

}} // Scaleform::GFx

#endif // INC_SF_GFx_Player_H
