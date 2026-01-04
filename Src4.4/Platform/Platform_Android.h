/**************************************************************************

Filename    :   Platform_Android.h
Content     :
Created     :
Authors     :

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_Platform_Android_H
#define INC_Platform_Android_H

#include "GFx_Kernel.h"
#include "GFx_Renderer_GL.h"
#include "Platform.h"

#include <jni.h>

void    GFx_SetEvalKey(const char* key);

namespace Scaleform { namespace Platform {

class AppThread;

#if 0
class AndroidAssetFile : public File
{
    String Path;
    int    Errno;

    static jmethodID AssetManager_open, InputStream_read, InputStream_seek

 public:
    AndroidAssetFile(jobject
    virtual const char* GetFilePath()  { return Path.ToCStr(); }


    virtual bool        IsValid() { return 1; }
    virtual bool        IsWritable() { return 0; }

    // Return position
    virtual int         Tell ();
    virtual SInt64      LTell () { return Tell(); }

    // File size
    virtual int         GetLength ();
    virtual SInt64      LGetLength () { return GetLength(); }

    virtual int         GetErrorCode() { return Errno; }

    virtual int         Write(const UByte *pbuffer, int numBytes) { return -1; }
    virtual int         Read(UByte *pbuffer, int numBytes);
    virtual int         SkipBytes(int numBytes) { return Skip(numBytes); }
    virtual int         BytesAvailable() { return GetLength()-Tell(); }

    virtual bool        Flush() { return 1; }

    virtual int         Seek(int offset, int origin=Seek_Set);
    virtual SInt64      LSeek(SInt64 offset, int origin=Seek_Set) { return Seek(offset, origin); }

    virtual bool        ChangeSize(int newSize) { return 0; }
    virtual int         CopyFromStream(File *pstream, int byteSize) { return -1; }

    virtual bool        Close();
};
#endif

#ifdef SF_USE_ANE

/* Forward declaration */
class AndroidExtension;
class AndroidExtensionContext;

/* Helper for Native Extensions. */
class FREHelper
{
public:
    static jobject NewObject(int value, JNIEnv *jenv);
    static jobject NewObject(double value, JNIEnv *jenv);
    static jobject NewObject(bool value, JNIEnv *jenv);
    static jobject NewObject(const String& value, JNIEnv *jenv);
    static jobject NewObject(const char* value, JNIEnv *jenv);

    static jmethodID GetNewObjectIntMethod(JNIEnv *jenv);
    static jmethodID GetNewObjectDoubleMethod(JNIEnv *jenv);
    static jmethodID GetNewObjectBooleanMethod(JNIEnv *jenv);
    static jmethodID GetNewObjectStringMethod(JNIEnv *jenv);

    static bool SetObjectPointer(JNIEnv* jenv, jobject jobj, long ptr);
    static long GetObjectPointer(JNIEnv* jenv, jobject jobj);

    static bool Init(JNIEnv* jenv);
    static bool Finalize(JNIEnv* jenv);
    static bool GetStringFromJString(JNIEnv *jenv, String& strResult, jstring jstrSource);
    static jclass GetFREObjectClass(JNIEnv* jenv);
    static jclass GetByteBufferClass(JNIEnv* jenv);
    static jmethodID GetFREObjectConstructor(JNIEnv* jenv);
    static void GenerateFREExceptionClasses(JNIEnv* jenv);
    static bool GetEnv(JNIEnv **penv);

    static JavaVM* pJVM;
    static jclass ClsFREObject, ClsByteBuffer;
    static jmethodID JmNewObjectInt, JmNewObjectDouble, JmNewObjectBoolean, JmNewObjectString;
    static jmethodID MdFREObjectConstructor;
    static jfieldID jfpObject;
    static jclass ClsFRETypeMismatchException, ClsFREInvalidObjectException, ClsFREASErrorException, ClsFRENoSuchNameException, ClsFREWrongThreadException, ClsIllegalArgumentException;

};

/* Maps FREFunctionBase */
class AndroidFREFunction : public RefCountBase<AndroidFREFunction, Stat_Default_Mem>, public FREFunctionBase
{
public:
    AndroidFREFunction(const char* name, AndroidExtensionContext& context, jobject jobjFunction, JNIEnv* jenv);
    ~AndroidFREFunction();

    bool Call(unsigned argc, const Scaleform::GFx::Value* const argv, Scaleform::GFx::Value* const result);

protected:
    AndroidExtensionContext& RefContext;
    jobject ObjFunction;
};

/* Maps FREExtensionContext*/
class AndroidExtensionContext : public RefCountBase<AndroidExtensionContext, Stat_Default_Mem>, public ExtensionContext
{
public:
    AndroidExtensionContext(const char* name, jobject objContext, JNIEnv* jenv, AndroidExtension& extension);
    ~AndroidExtensionContext();

    void Dispose();

    /* Map of functions */
    bool GenerateFREFunctions();
    FREFunctionBase* GetFREFunction(const char* functionName) const;

    jobject GetJObject() const;
    AndroidExtension& GetExtension() const;

protected:
    AutoPtr<ArrayLH<AndroidFREFunction*> > FREFunctionArray;
    jobject ObjContext; // context java object
    AndroidExtension& RefExtension;
};

/* Maps FREExtension*/
class AndroidExtension : public RefCountBase<AndroidExtension, Stat_Default_Mem>, public Extension
{
public:
    AndroidExtension(const char* name, const char* initializer);
    ~AndroidExtension();

    void Initializer();
    void Finalizer();
    bool CreateContext(const char* contextType);
    ExtensionContext* GetContext(const char* contextType) const;
    bool RemoveContext(const char* contextType);
    AndroidExtensionContext* GetContextFromJObject(jobject jcontext) const;

protected:
    AutoPtr<ArrayLH<AndroidExtensionContext*> > FREContextArray;
    jobject    ObjExtension; // Extension java object
};

// ***** Class Extension Manager
class AndroidExtensionManager : public ExtensionManager
{
public:
    AndroidExtensionManager();
    ~AndroidExtensionManager();

    bool Call(const char* extensionID, const char* contextID, const char* functionName, unsigned argc, const Scaleform::GFx::Value* const argv, Scaleform::GFx::Value* const result);

    const char* GetExtensionDirectory(const char* extensionID);
    Extension* GetExtension(const char* ExtensionID) const;
    AndroidExtensionContext* GetExtensionContextFromJavaContext(jobject jcontext) const;

    void InitializeExtensionContext(const char* extensionID, const char* contextID);
    void FinalizeExtensionContext(const char* extensionID, const char* contextID);
    void InitNativeExtension(const String& extensionName, const String& initializer);

    GFx::Value*    GetActionScriptData(const char* extensionID, const char* contextID);
    void SetActionScriptData(const char* extensionID, const char* contextID, GFx::Value* data);
    jobject GetActionScriptData(JNIEnv* jenv, jobject obj);
    void SetActionScriptData(JNIEnv* jenv, jobject obj, jobject jobjASData);

protected:
    AutoPtr<ArrayLH<AndroidExtension*> > FREExtensionArray;
};

#endif //#ifdef SF_USE_ANE

class AppImpl : public Device::Window, public AppImplBase
{
public:
    AppImpl(AppBase* app);

    // Window Interface
    virtual void* GetHandle() { return this; }
    virtual void  GetViewConfig(ViewConfig *config) const { *config = Config; }
    virtual void  ConfigureWindow(ViewConfig& newConfig);

    virtual void  OnSize(const Size<unsigned>& newSize);

    bool          setupWindow(const String& title, const ViewConfig& config);
    void          failSetupWindow(const char* cause);

    void          WaitForInit();

    File* OpenAssetFile(const char* name, int flags);

    void SetupFiles();
    void OnFocus(bool hasFocus);
    void OnOrientation1(int orient);
    void OnTouchMouse1(int action, float x, float y);
    void OnTouch1(int pointerId, int action, float x, float y);
    void OnGesture1(int action, int mode, float x, float y, float panx, float pany, float scale);
    void OnKeyboard1(int kbId, int key, int down, int mods);
    void OnChar1(int kbId, UInt32 code, int mods);
    void OnAccelerometerUpdate1(int idAcc, double timestamp, double accelerationX, double accelerationY, double accelerationZ);
    void OnGeolocationUpdate1(int idGeo, double latitude, double longitude, double altitude, double hAccuracy, double vAccuracy, double speed, double heading, double timestamp);
    void OnStatus1(const String& code, const String& level);
    void OnNativeLibraryInit1(const String& extensionName, const String& initializer);
    void DispatchStatusEventAsync1(jobject obj, const String& eventCode, const String& eventLevel);

    //bool          AdvanceAndDisplay();

    // AppImplBase
    virtual ThreadingType   GetArgsThreadingType() const;
    virtual void            ApplyViewConfigArgs(ViewConfig* config, const Args& args);
    virtual void            killWindow();

#ifdef SF_USE_ANE
    //////////////////////////// Extension context ////////////////////////////
    virtual ExtensionManager* GetExtensionManager();
    virtual bool Call(const char* extensionID, const char* contextID, const char* functionName, unsigned argc, const Scaleform::GFx::Value* const argv, Scaleform::GFx::Value* const result);
    virtual const char* GetExtensionDirectory(const char* extensionID);
    virtual void FinalizeExtensionContext(const char* extensionID, const char* contextID);
    virtual void InitializeExtensionContext(const char* extensionID, const char* contextID);
    virtual GFx::Value*    GetActionScriptData(const char* extensionID, const char* contextID);
    virtual void SetActionScriptData(const char* extensionID, const char* contextID, GFx::Value* data);
    AutoPtr<ExtensionManager> ExtensionManagerInstance;

    jint GetAsInt1(JNIEnv* jenv, jobject jobj);
    jdouble GetAsDouble1(JNIEnv* jenv, jobject jobj);
    jboolean GetAsBool1(JNIEnv* jenv, jobject jobj);
    jstring GetAsString1(JNIEnv* jenv, jobject jobj);
    void SetActionScriptData1(JNIEnv* jenv, jobject obj, jobject jobjASData);
    jobject GetActionScriptData1(JNIEnv* jenv, jobject obj);
    void FREObjectFromInt1(JNIEnv* jenv, jobject jobj, jint value);
    void FREObjectFromDouble1(JNIEnv* jenv, jobject jobj, jdouble value);
    void FREObjectFromBoolean1(JNIEnv* jenv, jobject jobj, jboolean value);
    void FREObjectFromString1(JNIEnv* jenv, jobject jobj, jstring value);
    void FREObjectFromClass1(JNIEnv* jenv, jobject jobj, jstring value, jobjectArray arrValue);
    jobject NewObject1(JNIEnv* jenv, jclass clsFREObj, jstring value, jobjectArray arrValue);
    jobject GetProperty1(JNIEnv* jenv, jobject jobj, jstring jpropertyName);
    void SetProperty1(JNIEnv* jenv, jobject jobj, jstring jpropertyName, jobject jpropertyObj);
    jobject CallMethod1(JNIEnv* jenv, jobject jobj, jstring jmethodName, jobjectArray jobjArr);
    void ReleaseNative1(JNIEnv* jenv, jobject jobj);

    jlong FREArrayGetLength(JNIEnv* jenv, jobject jobj);
    void FREArraySetLength(JNIEnv* jenv, jobject jobj, jlong lLen);
    jobject FREArrayGetObjectAt(JNIEnv* jenv, jobject jobj, jlong lPos);
    void FREArraySetObjectAt(JNIEnv* jenv, jobject jobj, jlong lPos, jobject jobjParam);

    jlong FREByteArrayGetLength(JNIEnv* jenv, jobject jobj);
    jobject FREByteArrayGetBytes(JNIEnv* jenv, jobject jobj);
    void FREByteArrayAcquire(JNIEnv* jenv, jobject jobj);
    void FREByteArrayRelease(JNIEnv* jenv, jobject jobj);

    jint FREBitmapDataGetWidth(JNIEnv* jenv, jobject jobj);
    jint FREBitmapDataGetHeight(JNIEnv* jenv, jobject jobj);
    jboolean FREBitmapDataHasAlpha(JNIEnv* jenv, jobject jobj);
    jboolean FREBitmapDataIsPremultiplied(JNIEnv* jenv, jobject jobj);
    jint FREBitmapDataGetLineStride32(JNIEnv* jenv, jobject jobj);
    jboolean FREBitmapDataIsInvertedY(JNIEnv* jenv, jobject jobj);
    jobject FREBitmapDataGetBits(JNIEnv* jenv, jobject jobj);
    void FREBitmapDataAcquire(JNIEnv* jenv, jobject jobj);
    void FREBitmapDataInvalidateRect(JNIEnv* jenv, jobject jobj, jint lX, jint lY, jint lWidth, jint lHeight);
    void FREBitmapDataRelease(JNIEnv* jenv, jobject jobj);
#endif //#ifdef SF_USE_ANE

    virtual void    HandleVirtualKeyboardOpen(bool multiline, const Render::RectF& textBox);
    virtual void    HandleVirtualKeyboardClose();
    virtual void    ProcessUrl(const String& url);

    virtual bool    RegisterAccelerometer(int accelerometerId);
    virtual bool    UnregisterAccelerometer(int accelerometerId);
    virtual bool    IsAccelerometerMuted() const;
    virtual bool    IsAccelerometerSupported() const;
    virtual void    SetAccelerometerInterval(int accelerometerId, int interval);

    virtual bool    RegisterGeolocation(int geolocationId);
    virtual bool    UnregisterGeolocation(int geolocationId);
    virtual bool    IsGeolocationMuted() const;
    virtual bool    IsGeolocationSupported() const;
    virtual void    SetGeolocationInterval(int geolocationId, int interval);

    virtual void SetMovie(Ptr<Scaleform::GFx::Movie> pmovie);

    bool            GetEnv(JNIEnv **penv) const; //internal

    virtual bool    dynamicShaderInitializationDefault() const { return true; }

    AppThread*      pAppThread;
    RenderHALThread* pRenderThread;
    Ptr<Scaleform::GFx::Movie>    pMovie;
    bool            RTInitialized;
    Event           RTInitializedEvent;
    bool            SetupFilesComplete;
    Event           SetupFilesCompleteEvent;
    bool            Created;
    unsigned int    Modifiers;
    // Window location saved when going full-screen.
    ViewConfig      OldConfig;

    bool            NeedPaint, BlockOnSize, CursorEnabled;

    JavaVM*         pJVM;
    jobject         MainActivity;
    jclass          globalAssetManagerClass;
    jobject         globalAssetManagerObject;
    String          FilesDir, CacheDir;
    jobject         JObj; // PPS: We use this to retrieve the java class (See JNIExample.pdf)
};


class FileRange : public DelegatedFile
{
    SInt64 Start, Size;
public:
    // Constructors
    FileRange(File *pfile, SInt64 start, SInt64 size) : DelegatedFile(pfile), Start(start), Size(size) {}

    virtual int    Tell()        { return (int)(pFile->Tell()-Start); }
    virtual SInt64 LTell()       { return pFile->LTell()-Start; }

    virtual int    GetLength()   { return Size; }
    virtual SInt64 LGetLength()  { return Size; }

    //virtual int  Write(const UByte *pbuffer, int numBytes) { return pFile->Write(pbuffer,numBytes); }
    virtual int    Read(UByte *pbuffer, int numBytes)
    {
        SInt64 pos = LTell();
        if (pos + numBytes >= Size)
            numBytes = Size - pos;
        return pFile->Read(pbuffer,numBytes);
    }

    //virtual int  SkipBytes(int numBytes) { return pFile->SkipBytes(numBytes); }
    virtual int    BytesAvailable()
    {
        return Size-Tell();
    }

    virtual int    Seek(int offset, int origin=Seek_Set) { return (int)LSeek(offset,origin); }
    virtual SInt64 LSeek(SInt64 offset, int origin=Seek_Set)
    {
        if (origin == Seek_Set)
            offset += Start;
        else if (origin == Seek_End)
        {
            offset = Start + Size - offset;
            origin = Seek_Set;
        }
        return pFile->LSeek(offset,origin) - Start;
    }
};

}}

#if defined (SF_USE_ANE)
#define ANE_INIT_JVM() FREHelper::pJVM = pJVM; FREHelper::Init(env);
#define ANE_INIT_JOBJ() FREHelper::Init(env);
#define ANE_FINALIZE_JOBJ() FREHelper::Finalize(env);

#else
#define ANE_INIT_JVM()
#define ANE_INIT_JOBJ()
#define ANE_FINALIZE_JOBJ()
#endif

#if defined (SF_USE_ANE)
#define ANE_FUNC(SystemClass,AppClass,Package,Activity,args) }              \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_##Activity##_NativeOnLibraryInit(JNIEnv * env, jobject obj, jstring objExtensionName, jstring objInitializer) \
{                                                                           \
    int len = env->GetStringLength(objExtensionName);                       \
    const jchar *jchars = env->GetStringChars(objExtensionName, NULL);      \
    String extensionName;                                                   \
    for (int i = 0; i < len; i++)                                           \
        extensionName.AppendChar(jchars[i]);                                \
    env->ReleaseStringChars(objExtensionName,jchars);                       \
                                                                            \
    len = env->GetStringLength(objInitializer);                             \
    const jchar *jchars1 = env->GetStringChars(objInitializer, NULL);       \
    String initializer;                                                     \
    for (int i = 0; i < len; i++)                                           \
        initializer.AppendChar(jchars1[i]);                                 \
    env->ReleaseStringChars(objInitializer,jchars1);                        \
    pImpl->OnNativeLibraryInit1(extensionName, initializer);                \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT jobject JNICALL Java_com_adobe_fre_FREContext_getActivity(JNIEnv *, jobject) \
{                                                                           \
    return pImpl->MainActivity;                                             \
}                                                                           \
extern "C" JNIEXPORT void JNICALL Java_com_adobe_fre_FREContext_dispatchStatusEventAsync(JNIEnv * env, jobject obj, jstring jcode, jstring jlevel) \
{                                                                           \
    int len = env->GetStringLength(jcode);                                  \
    const jchar *jchars = env->GetStringChars(jcode, NULL);                 \
    String eventName;                                                       \
    for (int i = 0; i < len; i++)                                           \
        eventName.AppendChar(jchars[i]);                                    \
    env->ReleaseStringChars(jcode,jchars);                                  \
                                                                            \
    len = env->GetStringLength(jlevel);                                     \
    const jchar *jchars1 = env->GetStringChars(jlevel, NULL);               \
    String eventParam;                                                      \
    for (int i = 0; i < len; i++)                                           \
        eventParam.AppendChar(jchars1[i]);                                  \
    env->ReleaseStringChars(jlevel,jchars1);                                \
    pImpl->DispatchStatusEventAsync1(obj, eventName, eventParam);           \
}                                                                           \
extern "C" JNIEXPORT jobject JNICALL Java_com_adobe_fre_FREContext_getActionScriptData(JNIEnv * jenv, jobject obj) \
{                                                                           \
    return pImpl->GetActionScriptData1(jenv, obj);                          \
}                                                                           \
extern "C" JNIEXPORT void JNICALL Java_com_adobe_fre_FREContext_setActionScriptData(JNIEnv * jenv, jobject obj, jobject jobjASData) \
{                                                                           \
    pImpl->SetActionScriptData1(jenv, obj, jobjASData);                     \
}                                                                           \
extern "C" JNIEXPORT jint JNICALL Java_com_adobe_fre_FREObject_getAsInt(JNIEnv * jenv, jobject jobj) \
{                                                                           \
    return pImpl->GetAsInt1(jenv, jobj);                                    \
}                                                                           \
extern "C" JNIEXPORT jdouble JNICALL Java_com_adobe_fre_FREObject_getAsDouble(JNIEnv * jenv, jobject jobj) \
{                                                                           \
    return pImpl->GetAsDouble1(jenv, jobj);                                 \
}                                                                           \
extern "C" JNIEXPORT jboolean JNICALL Java_com_adobe_fre_FREObject_getAsBool(JNIEnv * jenv, jobject jobj) \
{                                                                           \
    return pImpl->GetAsBool1(jenv, jobj);                                   \
}                                                                           \
extern "C" JNIEXPORT jstring JNICALL Java_com_adobe_fre_FREObject_getAsString(JNIEnv * jenv, jobject jobj)    \
{                                                                           \
    return pImpl->GetAsString1(jenv, jobj);                                 \
}                                                                           \
extern "C" JNIEXPORT void JNICALL Java_com_adobe_fre_FREObject_FREObjectFromInt(JNIEnv * jenv, jobject jobj, jint value) \
{                                                                           \
    pImpl->FREObjectFromInt1(jenv, jobj, value);                            \
}                                                                           \
extern "C" JNIEXPORT void JNICALL Java_com_adobe_fre_FREObject_FREObjectFromDouble(JNIEnv * jenv, jobject jobj, jdouble value) \
{                                                                           \
    pImpl->FREObjectFromDouble1(jenv, jobj, value);                         \
}                                                                           \
extern "C" JNIEXPORT void JNICALL Java_com_adobe_fre_FREObject_FREObjectFromBoolean(JNIEnv * jenv, jobject jobj, jboolean value) \
{                                                                           \
    pImpl->FREObjectFromBoolean1(jenv, jobj, value);                        \
}                                                                           \
extern "C" JNIEXPORT void JNICALL Java_com_adobe_fre_FREObject_FREObjectFromString(JNIEnv * jenv, jobject jobj, jstring value) \
{                                                                           \
    pImpl->FREObjectFromString1(jenv, jobj, value);                         \
}                                                                           \
extern "C" JNIEXPORT void JNICALL Java_com_adobe_fre_FREObject_FREObjectFromClass(JNIEnv * jenv, jobject jobj, jstring value, jobjectArray arrValue) \
{                                                                           \
    pImpl->FREObjectFromClass1(jenv, jobj, value, arrValue);                \
}                                                                           \
extern "C" JNIEXPORT jobject JNICALL Java_com_adobe_fre_FREObject_newObject(JNIEnv * jenv, jclass clsFREObj , jstring value, jobjectArray arrValue) \
{                                                                           \
    return pImpl->NewObject1(jenv, clsFREObj, value, arrValue);             \
}                                                                           \
extern "C" JNIEXPORT jobject JNICALL Java_com_adobe_fre_FREObject_getProperty(JNIEnv * jenv, jobject jobj, jstring jpropertyName) \
{                                                                           \
    return pImpl->GetProperty1(jenv, jobj, jpropertyName);                  \
}                                                                           \
extern "C" JNIEXPORT void JNICALL Java_com_adobe_fre_FREObject_setProperty(JNIEnv * jenv, jobject jobj, jstring jpropertyName, jobject jpropertyObj) \
{                                                                           \
    pImpl->SetProperty1(jenv, jobj, jpropertyName, jpropertyObj);           \
}                                                                           \
extern "C" JNIEXPORT jobject JNICALL Java_com_adobe_fre_FREObject_callMethod(JNIEnv * jenv, jobject jobj, jstring jmethodName, jobjectArray jobjArr) \
{                                                                           \
    return pImpl->CallMethod1(jenv, jobj, jmethodName, jobjArr);            \
}                                                                           \
extern "C" JNIEXPORT void JNICALL Java_com_adobe_fre_FREObject_releaseNative(JNIEnv * jenv, jobject jobj) \
{                                                                           \
    return pImpl->ReleaseNative1(jenv, jobj);                               \
}                                                                           \
extern "C" JNIEXPORT jlong JNICALL Java_com_adobe_fre_FREArray_getLength(JNIEnv* jenv, jobject jobj) \
{                                                                           \
    return pImpl->FREArrayGetLength(jenv, jobj);                            \
}                                                                           \
extern "C" JNIEXPORT void JNICALL Java_com_adobe_fre_FREArray_setLength(JNIEnv* jenv, jobject jobj, jlong lLen) \
{                                                                           \
    pImpl->FREArraySetLength(jenv, jobj, lLen);                             \
}                                                                           \
extern "C" JNIEXPORT jobject JNICALL Java_com_adobe_fre_FREArray_getObjectAt(JNIEnv* jenv, jobject jobj, jlong lPos) \
{                                                                           \
    return pImpl->FREArrayGetObjectAt(jenv, jobj, lPos);                    \
}                                                                           \
extern "C" JNIEXPORT void JNICALL Java_com_adobe_fre_FREArray_setObjectAt(JNIEnv* jenv, jobject jobj, jlong lPos, jobject jobjParam) \
{                                                                           \
    pImpl->FREArraySetObjectAt(jenv, jobj, lPos, jobjParam);                \
}                                                                           \
extern "C" JNIEXPORT jlong JNICALL Java_com_adobe_fre_FREByteArray_getLength(JNIEnv* jenv, jobject jobj) \
{                                                                           \
    return pImpl->FREByteArrayGetLength(jenv, jobj);                        \
}                                                                           \
extern "C" JNIEXPORT jobject JNICALL Java_com_adobe_fre_FREByteArray_getBytes(JNIEnv* jenv, jobject jobj) \
{                                                                           \
    return pImpl->FREByteArrayGetBytes(jenv, jobj);                         \
}                                                                           \
extern "C" JNIEXPORT void JNICALL Java_com_adobe_fre_FREByteArray_acquire(JNIEnv* jenv, jobject jobj) \
{                                                                           \
    pImpl->FREByteArrayAcquire(jenv, jobj);                                 \
}                                                                           \
extern "C" JNIEXPORT void JNICALL Java_com_adobe_fre_FREByteArray_release(JNIEnv* jenv, jobject jobj) \
{                                                                           \
    pImpl->FREByteArrayRelease(jenv, jobj);                                 \
}                                                                           \
extern "C" JNIEXPORT jint JNICALL Java_com_adobe_fre_FREBitmapData_getWidth(JNIEnv* jenv, jobject jobj) \
{                                                                           \
    return pImpl->FREBitmapDataGetWidth(jenv, jobj);                        \
}                                                                           \
extern "C" JNIEXPORT jint JNICALL Java_com_adobe_fre_FREBitmapData_getHeight(JNIEnv* jenv, jobject jobj) \
{                                                                           \
    return pImpl->FREBitmapDataGetHeight(jenv, jobj);                       \
}                                                                           \
extern "C" JNIEXPORT jboolean JNICALL Java_com_adobe_fre_FREBitmapData_hasAlpha(JNIEnv* jenv, jobject jobj) \
{                                                                           \
    return pImpl->FREBitmapDataHasAlpha(jenv, jobj);                        \
}                                                                           \
extern "C" JNIEXPORT jboolean JNICALL Java_com_adobe_fre_FREBitmapData_isPremultiplied(JNIEnv* jenv, jobject jobj) \
{                                                                           \
    return pImpl->FREBitmapDataIsPremultiplied(jenv, jobj);                 \
}                                                                           \
extern "C" JNIEXPORT jint JNICALL Java_com_adobe_fre_FREBitmapData_getLineStride32(JNIEnv* jenv, jobject jobj) \
{                                                                           \
    return pImpl->FREBitmapDataGetLineStride32(jenv, jobj);                 \
}                                                                           \
extern "C" JNIEXPORT jboolean JNICALL Java_com_adobe_fre_FREBitmapData_isInvertedY(JNIEnv* jenv, jobject jobj) \
{                                                                           \
    return pImpl->FREBitmapDataIsInvertedY(jenv, jobj);                     \
}                                                                           \
extern "C" JNIEXPORT jobject JNICALL Java_com_adobe_fre_FREBitmapData_getBits(JNIEnv* jenv, jobject jobj) \
{                                                                           \
    return pImpl->FREBitmapDataGetBits(jenv, jobj);                         \
}                                                                           \
extern "C" JNIEXPORT void JNICALL Java_com_adobe_fre_FREBitmapData_acquire(JNIEnv* jenv, jobject jobj) \
{                                                                           \
    pImpl->FREBitmapDataAcquire(jenv, jobj);                                \
}                                                                           \
extern "C" JNIEXPORT void JNICALL Java_com_adobe_fre_FREBitmapData_invalidateRect(JNIEnv* jenv, jobject jobj, jint lX, jint lY, jint lWidth, jint lHeight) \
{                                                                           \
    pImpl->FREBitmapDataInvalidateRect(jenv, jobj, lX, lY, lWidth, lHeight);\
}                                                                           \
extern "C" JNIEXPORT void JNICALL Java_com_adobe_fre_FREBitmapData_release(JNIEnv* jenv, jobject jobj) \
{                                                                           \
    pImpl->FREBitmapDataRelease(jenv, jobj);                                \
}
#else
#define ANE_FUNC(SystemClass,AppClass,Package,Activity,args) }
#endif

#define SF_ANDROID_JNI(SystemClass,AppClass,Package,Activity,args)          \
using namespace Scaleform;                                                  \
using namespace Render;                                                     \
using namespace Platform;                                                   \
static unsigned char SystemStorage[sizeof(SystemClass)];                    \
static AppClass*     pApp;                                                  \
static AppImpl*      pImpl;                                                 \
static JavaVM   *pJVM = 0;                                                  \
                                                                            \
extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)   \
{                                                                           \
    pJVM = jvm;                                                             \
    JNIEnv* env = NULL;                                                     \
    if (jvm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK)               \
    {                                                                       \
        SF_DEBUG_MESSAGE(1, "JNI_OnLoad: Could not get environment..\n");   \
        return -1;                                                          \
    }                                                                       \
    SF_DEBUG_MESSAGE(1, "JNI_OnLoad: Done\n");                              \
    return JNI_VERSION_1_6;                                                 \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_##Activity##_NativeAppInit(JNIEnv * env, jobject obj) \
{                                                                           \
    SF_DEBUG_MESSAGE(1, #Activity " NativeAppInit START");                  \
    /*GFx_SetEvalKey("PLACE KEY HERE");*/                                   \
    new (SystemStorage) SystemClass();                                      \
    pApp = new AppClass args;                                               \
    pImpl = (AppImpl*)pApp->GetAppImpl();                                   \
    pImpl->pJVM = pJVM;                                                     \
ANE_INIT_JVM()                                                              \
    pImpl->pAppThread->Start(Thread::Running);                              \
    SF_DEBUG_MESSAGE(1, #Activity " NativeAppInit END");                    \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_##Activity##_NativeCacheObject(JNIEnv * env, jobject obj) \
{                                                                           \
    SF_DEBUG_MESSAGE(1, #Activity " NativeCacheObject START");              \
    jobject globRefObj = env->NewGlobalRef(obj);                            \
    pImpl->JObj = globRefObj;                                               \
ANE_INIT_JOBJ()                                                             \
    SF_DEBUG_MESSAGE(1, #Activity " NativeCacheObject END");                \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_##Activity##_NativeClearObject(JNIEnv * env, jobject obj) \
{                                                                           \
    SF_DEBUG_MESSAGE(1, #Activity " NativeClearObject START");              \
    if((pImpl->JObj))                                                       \
        env->DeleteGlobalRef(pImpl->JObj);                                  \
    pImpl->JObj = NULL;                                                     \
ANE_FINALIZE_JOBJ()                                                         \
    SF_DEBUG_MESSAGE(1, #Activity " NativeClearObject END");                \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_##Activity##_NativeOnCreate(JNIEnv * env, jobject obj) \
{                                                                           \
    SF_DEBUG_MESSAGE(1, #Activity " NativeOnCreate START");                 \
    pImpl->MainActivity = obj;                                              \
    pImpl->SetupFiles();                                                    \
    SF_DEBUG_MESSAGE1(1, #Activity " JNI init environment %d", pImpl->SetupFilesComplete);\
    SF_DEBUG_MESSAGE(1, #Activity " NativeOnCreate END");                   \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_##Activity##_NativeOnResume(JNIEnv * env, jobject obj) \
{                                                                           \
    SF_DEBUG_MESSAGE(1, #Activity " NativeOnResume START");                 \
    pImpl->NextUpdateTicks = Timer::GetTicks()/1000;                        \
    pImpl->pAppThread->OnLifecycleEvent(GFx::AppLifecycleEvent::OnResume);  \
    SF_DEBUG_MESSAGE(1, #Activity " NativeOnResume END");                   \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_##Activity##_NativeOnPause(JNIEnv * env, jobject obj) \
{                                                                           \
    SF_DEBUG_MESSAGE(1, #Activity " NativeOnPause START");                  \
    pImpl->pAppThread->OnLifecycleEvent(GFx::AppLifecycleEvent::OnPause);   \
    SF_DEBUG_MESSAGE(1, #Activity " NativeOnPause END");                    \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_##Activity##_NativeOnWindowFocusChanged(JNIEnv * env, jobject obj, jboolean hasFocus) \
{                                                                           \
    SF_DEBUG_MESSAGE(1, #Activity " NativeOnWindowFocusChanged START");     \
    pImpl->OnFocus(hasFocus);                                               \
    SF_DEBUG_MESSAGE(1, #Activity " NativeOnWindowFocusChanged END");       \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_##Activity##_NativeOnOpenFile(JNIEnv * env, jobject obj, jstring path) \
{                                                                           \
    SF_DEBUG_MESSAGE(1, #Activity " NativeOnOpenFile START");               \
    int len = env->GetStringLength(path);                                   \
    const jchar *jchars = env->GetStringChars(path, NULL);                  \
    String fileName;                                                        \
    for (int i = 0; i < len; i++)                                           \
        fileName.AppendChar(jchars[i]);                                     \
    env->ReleaseStringChars(path,jchars);                                   \
                                                                            \
    if (pImpl->pRenderThread)                                               \
        pImpl->pAppThread->OnDropFiles(fileName);                           \
    SF_DEBUG_MESSAGE(1, #Activity " NativeOnOpenFile END");                 \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_GLView_NativeReset(JNIEnv * env, jobject obj) \
{                                                                           \
    SF_DEBUG_MESSAGE(1, #Activity " GLView_NativeReset START");             \
    if (!pImpl->RTInitialized)                                              \
    {                                                                       \
        SF_DEBUG_MESSAGE(1,"Application not initialized. Waiting.");        \
        pImpl->RTInitializedEvent.Wait();                                   \
    }                                                                       \
    if (pImpl->Created)                                                     \
    {                                                                       \
        SF_DEBUG_MESSAGE(1, "Resetting gl context");                        \
        ((Render::GL::HAL*)pApp->GetDevice()->GetHAL())->PrepareForReset(); \
        ((Render::GL::HAL*)pApp->GetDevice()->GetHAL())->RestoreAfterReset(); \
        SF_DEBUG_MESSAGE(1, #Activity " GLView_NativeReset END");           \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        SF_DEBUG_MESSAGE1(1, #Activity " RT init graphics %d", pImpl->RTInitialized);\
        if (!pImpl->pRenderThread->InitGraphics_RenderThread())             \
            SF_DEBUG_MESSAGE(1, "InitGraphics failed");                     \
        pImpl->Created = 1;                                                 \
    }                                                                       \
    SF_DEBUG_MESSAGE(1, #Activity " GLView_NativeReset END");               \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_GLView_NativeResize(JNIEnv * env, jobject obj,  jint width, jint height) \
{                                                                           \
    SF_DEBUG_MESSAGE(1, #Activity " GLView_NativeResize START");            \
    pImpl->pRenderThread->AbortFrame();                                     \
    pImpl->OnSize(Size<unsigned>(width,height));                            \
    pImpl->pAppThread->OnSize(width,height);                                \
    pImpl->pRenderThread->OnSize_RenderThread(width, height);               \
    SF_DEBUG_MESSAGE(1, #Activity " GLView_NativeResize END");              \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_GLView_NativeDraw(JNIEnv * env, jobject obj) \
{                                                                           \
    pImpl->pRenderThread->DrawFrame_RenderThread();                         \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_##Activity##_NativeOnOrientation(JNIEnv * env, jobject obj,  jint orient) \
{                                                                           \
    pImpl->OnOrientation1(orient);                                          \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_##Activity##_NativeOnKey(JNIEnv * env, jobject obj, jboolean down, jint key) \
{                                                                           \
    pImpl->OnKeyboard1(0, key, down, 0);                                    \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_##Activity##_NativeOnChar(JNIEnv * env, jobject obj, jint code) \
{                                                                           \
    pImpl->OnChar1(0, code, 0);                                             \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_##Activity##_NativeOnTouch(JNIEnv * env, jobject obj, jint pointerId, jint event, jfloat x, jfloat y) \
{                                                                           \
    pImpl->OnTouch1(pointerId, event, x, y);                                \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_##Activity##_NativeOnGesture(JNIEnv * env, jobject obj, jint event, jint mode, jfloat x, jfloat y, jfloat panx, jfloat pany, jfloat scale) \
{                                                                           \
    pImpl->OnGesture1(event, mode, x, y, panx, pany, scale);                \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_##Activity##_NativeOnTouchMouse(JNIEnv * env, jobject obj, jint event, jfloat x, jfloat y) \
{                                                                           \
    pImpl->OnTouchMouse1(event, x, y);                                      \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_##Activity##_NativeOnAccelerometerUpdate(JNIEnv * env, jobject obj, jint idAcc, jdouble timestamp, jdouble accelerationX, jdouble accelerationY, jdouble accelerationZ) \
{                                                                           \
    pImpl->OnAccelerometerUpdate1(idAcc, timestamp, accelerationX, accelerationY, accelerationZ); \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_##Activity##_NativeOnGeolocationUpdate(JNIEnv * env, jobject obj, jint idGeo, jdouble latitude, jdouble longitude, jdouble altitude, jdouble hAccuracy, jdouble vAccuracy, jdouble speed, jdouble heading, jdouble timestamp) \
{                                                                           \
    pImpl->OnGeolocationUpdate1(idGeo, latitude, longitude, altitude, hAccuracy, vAccuracy, speed, heading, timestamp); \
}                                                                           \
                                                                            \
extern "C" JNIEXPORT void JNICALL Java_##Package##_##Activity##_NativeOnStatus(JNIEnv * env, jobject obj, jstring jcode, jstring jlevel) \
{                                                                           \
    int len = env->GetStringLength(jcode);                                  \
    const jchar *jchars = env->GetStringChars(jcode, NULL);                 \
    String code;                                                            \
    for (int i = 0; i < len; i++)                                           \
        code.AppendChar(jchars[i]);                                         \
    env->ReleaseStringChars(jcode,jchars);                                  \
                                                                            \
    len = env->GetStringLength(jlevel);                                     \
    const jchar *jchars1 = env->GetStringChars(jlevel, NULL);               \
    String level;                                                           \
    for (int i = 0; i < len; i++)                                           \
        level.AppendChar(jchars1[i]);                                       \
    env->ReleaseStringChars(jlevel,jchars1);                                \
    pImpl->OnStatus1(code, level);                                          \
ANE_FUNC(SystemClass,AppClass,Package,Activity,args)

#include "Platform_AppThread.h"

#endif //INC_Platform_Android_H
