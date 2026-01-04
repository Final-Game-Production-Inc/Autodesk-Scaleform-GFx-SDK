/**************************************************************************

Filename    :   Platform_Android_GL.cpp
Content     :   Platform::App implementation for Android GL.
Created     :   Jan 10, 2008
Authors     :

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

// GFx Includes
#include "Platform.h"
#include "Platform_Android.h"
#include "GFx/GFx_Loader.h"
#include "GFx/GFx_Gesture.h"
#include "Kernel/SF_System.h"
#include "Kernel/SF_WString.h"
#include "Kernel/SF_HeapNew.h"

#include "Render/GL/GL_HAL.h"

#include "Platform/Platform_SystemRenderHALThread.h"

//------------------------------------------------------------------------

namespace Scaleform {

extern File *FileFILEOpen(FILE *file, const char* path, int closeFd);

namespace Platform {

//------------------------------------------------------------------------
// ***** AppImpl

AppImpl::AppImpl(AppBase* app) :
AppImplBase(app),
pMovie(0),
RTInitialized(false),
SetupFilesComplete(false),
NeedPaint(true),
BlockOnSize(false)
{
    Created = false;
    CursorEnabled = true;
    Modifiers = 0;
#ifdef SF_USE_ANE
    ExtensionManagerInstance = NULL;
#endif
    pAppThread = new AppThread(app);
}

bool AppImpl::setupWindow(const String& title, const ViewConfig& config)
{
    Title  = title;
    Config = config;
    pRenderThread = pApp->GetAppRenderThread();

    // Make sure to set the VConfig and pWindow on the render thread, before allowing it to continue.
    pRenderThread->SetConfigAndWindow(config, this);

    // This is called after the render thread object is created in OnInitHelper, so let the RT proceed now.
    RTInitialized = 1;
    RTInitializedEvent.PulseEvent();

    glLineWidth(1.0f);

    return 1;
}

void AppImpl::SetupFiles()
{
    if (!SetupFilesComplete)
    {
        // Only proceed if we have a valid JNI environment
        // (NativeAppInit has completed)
        JNIEnv* jenv;
        if (!pJVM || pJVM->GetEnv((void**)&jenv, JNI_VERSION_1_6) < 0)
        {
            SF_DEBUG_ERROR(1, "SetupFiles: Cannot get jni env");
            return;
        }

        jclass ActivityClass = jenv->GetObjectClass(MainActivity);
        jmethodID getAssets = jenv->GetMethodID(ActivityClass, "getAssets", "()Landroid/content/res/AssetManager;");

        jobject AssetManager = jenv->CallObjectMethod(MainActivity, getAssets);
        jclass AssetManagerClass = jenv->GetObjectClass(AssetManager);

        // Stash in GlobalRefs for external access (req. for compat. with API Level >= 14 ICS)
        globalAssetManagerClass = (jclass)(jenv->NewGlobalRef(AssetManagerClass));
        globalAssetManagerObject = jenv->NewGlobalRef(AssetManager);

        jmethodID getFilesDir = jenv->GetMethodID(ActivityClass, "getFilesDir", "()Ljava/io/File;");
        jobject filesDirH = jenv->CallObjectMethod(MainActivity, getFilesDir);
        jclass FileClass = jenv->GetObjectClass(filesDirH);
        jmethodID getAbsPath = jenv->GetMethodID(FileClass, "getAbsolutePath", "()Ljava/lang/String;");
        jstring filesDir = (jstring) jenv->CallObjectMethod(filesDirH, getAbsPath);

        jboolean copy;
        const char* fs = jenv->GetStringUTFChars(filesDir, &copy);

        // Does not exist by default.
        mkdir(fs, 0700);

        FilesDir = fs;
        FilesDir += "/";

        SF_DEBUG_MESSAGE1(1, "Files dir: %s", fs);

        jenv->ReleaseStringUTFChars(filesDir, fs);

        jmethodID getCacheDir = jenv->GetMethodID(ActivityClass, "getCacheDir", "()Ljava/io/File;");
        jobject cacheDirH = jenv->CallObjectMethod(MainActivity, getCacheDir);
        jstring cacheDir = (jstring) jenv->CallObjectMethod(cacheDirH, getAbsPath);

        const char* fcs = jenv->GetStringUTFChars(cacheDir, &copy);

        // Does not exist by default.
        mkdir(fcs, 0755);

        CacheDir = fcs;
        CacheDir += "/";

        SF_DEBUG_MESSAGE1(1, "Cache dir: %s", fcs);
        jenv->ReleaseStringUTFChars(cacheDir, fcs);

        SetupFilesComplete = 1;
        SetupFilesCompleteEvent.PulseEvent();
    }
}

void AppImpl::OnFocus(bool hasFocus)
{
    if (hasFocus)
    {
        GFx::SetFocusEvent event;
        pAppThread->OnEvent(event);
    }
    else
    {
        GFx::Event event(GFx::Event::KillFocus);
        pAppThread->OnEvent(event);
    }
}

File* AppImpl::OpenAssetFile(const char* name, int flags)
{
    JNIEnv* jenv;
    static bool detachFdAvailable = true;
    bool didAttach = false;
    if (pJVM->GetEnv((void**)&jenv, JNI_VERSION_1_6) < 0)
    {
        if (pJVM->AttachCurrentThread(&jenv, 0) < 0)
        {
            SF_DEBUG_ERROR(1, "OpenAssetFile: Cannot get jni env");
            return 0;
        }

        didAttach = true;
    }

    jmethodID openFd = jenv->GetMethodID(globalAssetManagerClass, "openFd", "(Ljava/lang/String;)Landroid/content/res/AssetFileDescriptor;");
    jobject jfd = jenv->CallObjectMethod(globalAssetManagerObject, openFd, jenv->NewStringUTF(name));
    jthrowable exceptionObj = jenv->ExceptionOccurred();
    jenv->ExceptionClear();

    if (!jfd || exceptionObj)
    {
        SF_DEBUG_WARNING1(1, "OpenAssetFile: %s not found\n", name);

        if (didAttach)
            pJVM->DetachCurrentThread();
        return 0;
    }

    jclass AssetFdClass = jenv->GetObjectClass(jfd);
    jmethodID getParcelFd = jenv->GetMethodID(AssetFdClass, "getParcelFileDescriptor", "()Landroid/os/ParcelFileDescriptor;");
    jmethodID getStartOffset = jenv->GetMethodID(AssetFdClass, "getStartOffset", "()J");
    jmethodID getLength = jenv->GetMethodID(AssetFdClass, "getLength", "()J");
    jlong start = jenv->CallLongMethod(jfd, getStartOffset);
    jlong length = jenv->CallLongMethod(jfd, getLength);

    SF_DEBUG_MESSAGE3(1, "Loading asset %s: start %lld size %lld\n", name, start, length);
    jfd = jenv->CallObjectMethod(jfd, getParcelFd);

    jclass ParcelFdClass = jenv->GetObjectClass(jfd);

    int fd = 0;
    jmethodID detachFd = 0;

    // JDM: detachFd only available starting with API Level 12 but
    // using the older method on API >= 12 leads to stale file descriptors on occasion
    if (detachFdAvailable)
    {
        detachFd = jenv->GetMethodID(ParcelFdClass, "detachFd", "()I");
        jthrowable exceptionObj = jenv->ExceptionOccurred();
        jenv->ExceptionClear();

        if (exceptionObj)
        {
            detachFdAvailable = false;
        }
    }

    if (!detachFdAvailable)
    {
        jmethodID getFd = jenv->GetMethodID(ParcelFdClass, "getFileDescriptor", "()Ljava/io/FileDescriptor;");
        jfd = jenv->CallObjectMethod(jfd, getFd);
        jclass FdClass = jenv->GetObjectClass(jfd);

        jfieldID descriptor = jenv->GetFieldID(FdClass, "descriptor", "I");
        int localFd = jenv->GetIntField(jfd, descriptor);
        fd = dup(localFd);
    }
    else
    {
        fd = jenv->CallIntMethod(jfd, detachFd);
    }

    SF_DEBUG_WARNING2(fd < 0, "Asset %s: bad fd: %d", name, fd);

    File* returnValue = 0;
    if (fd >= 0)
    {
        FILE* pfile = fdopen(fd, "rb");
        SF_DEBUG_WARNING1(pfile == 0, "Asset %s: stale file descriptor", name);
        if (pfile)
        {
            fseek(pfile, start, SEEK_SET);
            returnValue = SF_NEW FileRange(FileFILEOpen(pfile, name, 1), start, length);
        }
    }

    if (didAttach)
        pJVM->DetachCurrentThread();

    return returnValue;
}

void AppImpl::failSetupWindow(const char* cause)
{
}

void AppImpl::killWindow()
{
    if (!Created)
        return;

    JNIEnv* jenv;
    bool didAttach = false;
    if (pJVM->GetEnv((void**)&jenv, JNI_VERSION_1_6) < 0)
    {
        if (pJVM->AttachCurrentThread(&jenv, 0) < 0)
        {
            SF_DEBUG_ERROR(1, "killWindow: Cannot get jni env");
        }
        else
        {
            didAttach = true;
            SF_DEBUG_MESSAGE(1, "killWindow: Cleaning up globalAssetManagerClass\n");

            if (globalAssetManagerClass)
            {
                jenv->DeleteGlobalRef(globalAssetManagerClass);
                globalAssetManagerClass = NULL;
            }
            if (globalAssetManagerObject)
            {
                jenv->DeleteGlobalRef(globalAssetManagerObject);
                globalAssetManagerObject = NULL;
            }
        }
    }

    if (didAttach)
        pJVM->DetachCurrentThread();

    Created = 0;
}

void AppImpl::ConfigureWindow(ViewConfig& config)
{
}

void AppImpl::OnSize(const Size<unsigned>& newSize)
{
    Config.ViewSize = newSize;
}

void AppImpl::WaitForInit()
{
    pAppThread->WaitForInit();
}

ThreadingType AppImpl::GetArgsThreadingType() const
{
    // Android must be multithreaded.
    return TT_MultiThreaded;
}

void AppImpl::ProcessUrl(const String& url)
{
    SF_DEBUG_MESSAGE(1, "ProcessUrl: Setting up JVM callbacks\n");
    bool attached;
    JNIEnv *jenv;
    int status = pJVM->GetEnv((void **)&jenv, JNI_VERSION_1_6);
    if(status < 0)
    {
        SF_DEBUG_MESSAGE(1, "ProcessUrl: Failed to get JNI jenvironment, assuming native thread\n");
        status = pJVM->AttachCurrentThread(&jenv, NULL);
        if(status < 0)
        {
            SF_DEBUG_MESSAGE(1, "ProcessUrl: Failed to attach current thread\n");
            return;
        }
        attached = true;
    }

    SF_DEBUG_MESSAGE(1, "ProcessUrl: Getting class and methodID\n");
    jclass cls = jenv->GetObjectClass(JObj);
    if (NULL == cls)
    {
        SF_DEBUG_MESSAGE(1, "ProcessUrl: Failed to get class reference\n");
        return;
    }

    jmethodID openURL = jenv->GetMethodID(cls, "OpenURL", "(Ljava/lang/String;)V");

    SF_DEBUG_MESSAGE4(1, "ProcessUrl: Calling Java method (%p, %p, %p) - %s\n", jenv, JObj, openURL, url.ToCStr());
    jenv->CallVoidMethod(JObj, openURL, jenv->NewStringUTF(url.ToCStr()));

    SF_DEBUG_MESSAGE(1, "ProcessUrl: Done!\n");
    if (attached) pJVM->DetachCurrentThread();
}


#ifdef SF_USE_ANE
////////////////////////////////////////////////////////////////////////////////
/////////////////// Native Extensions Implementation ///////////////////////////
////////////////////////////////////////////////////////////////////////////////

//////////////////// FREObjectHelper ////////////////////

JavaVM*    FREHelper::pJVM = NULL;
jclass FREHelper::ClsFREObject = NULL;
jclass FREHelper::ClsByteBuffer = NULL;
jmethodID FREHelper::MdFREObjectConstructor = NULL;
jmethodID FREHelper::JmNewObjectInt = NULL;
jmethodID FREHelper::JmNewObjectDouble = NULL;
jmethodID FREHelper::JmNewObjectBoolean = NULL;
jmethodID FREHelper::JmNewObjectString = NULL;
jfieldID FREHelper::jfpObject = NULL;
jclass FREHelper::ClsIllegalArgumentException = NULL;
jclass FREHelper::ClsFRETypeMismatchException = NULL;
jclass FREHelper::ClsFREInvalidObjectException = NULL;
jclass FREHelper::ClsFREASErrorException = NULL;
jclass FREHelper::ClsFRENoSuchNameException = NULL;
jclass FREHelper::ClsFREWrongThreadException = NULL;


bool FREHelper::GetStringFromJString(JNIEnv *jenv, String& strResult, jstring jstrSource)
{
    if (jenv == NULL || GetEnv(&jenv) == false)
        return false;

    int len = jenv->GetStringLength(jstrSource);
    const jchar *jchars = jenv->GetStringChars(jstrSource, NULL);
    strResult.Clear();
    for (int i = 0; i < len; i++)
        strResult.AppendChar(jchars[i]);
    jenv->ReleaseStringChars(jstrSource, jchars);
    return true;
}

jobject FREHelper::NewObject(int value, JNIEnv *jenv)
{
    if (jenv == NULL || GetEnv(&jenv) == false)
        return NULL;

    SF_DEBUG_MESSAGE1(1, "FREHelper::NewObject %d\n", value);

    jmethodID jmNewObject = GetNewObjectIntMethod(jenv);
    if (jmNewObject == NULL)
        return NULL;
    jobject jObj = jenv->CallStaticObjectMethod(ClsFREObject, jmNewObject, (jint)value);

    SF_DEBUG_MESSAGE1(1, "FREHelper::NewObject done %p\n", jObj);
    return jObj;
}

jobject FREHelper::NewObject(double value, JNIEnv *jenv)
{
    if (jenv == NULL || GetEnv(&jenv) == false)
        return NULL;

    jmethodID jmNewObject = GetNewObjectDoubleMethod(jenv);
    if (jmNewObject == NULL)
        return NULL;
    jobject jObj = jenv->CallStaticObjectMethod(ClsFREObject, jmNewObject, (jdouble)value);

    return jObj;
}

jobject FREHelper::NewObject(bool value, JNIEnv *jenv)
{
    if (jenv == NULL || GetEnv(&jenv) == false)
        return NULL;

    jmethodID jmNewObject = GetNewObjectBooleanMethod(jenv);
    if (jmNewObject == NULL)
        return NULL;
    jobject jObj = jenv->CallStaticObjectMethod(ClsFREObject, jmNewObject, (jboolean)value);

    return jObj;
}

jobject FREHelper::NewObject(const String& value, JNIEnv *jenv)
{
    if (jenv == NULL || GetEnv(&jenv) == false)
        return NULL;

    jmethodID jmNewObject = GetNewObjectStringMethod(jenv);
    if (jmNewObject == NULL)
        return NULL;

    jstring jstrValue = jenv->NewStringUTF(value.ToCStr());
    jobject jObj = jenv->CallStaticObjectMethod(ClsFREObject, jmNewObject, jstrValue);
    jenv->DeleteLocalRef(jstrValue);

    return jObj;
}

jobject FREHelper::NewObject(const char* value, JNIEnv *jenv)
{
    if (jenv == NULL || GetEnv(&jenv) == false)
        return NULL;

    jmethodID jmNewObject = GetNewObjectStringMethod(jenv);
    if (jmNewObject == NULL)
        return NULL;

    jstring jstrValue = jenv->NewStringUTF(value);
    jobject jObj = jenv->CallStaticObjectMethod(ClsFREObject, jmNewObject, jstrValue);
    jenv->DeleteLocalRef(jstrValue);

    return jObj;
}

jmethodID FREHelper::GetNewObjectIntMethod(JNIEnv *jenv)
{
    if (JmNewObjectInt == NULL)
    {
        jclass clsFREObject = GetFREObjectClass(jenv);
        if (clsFREObject == NULL)
            return NULL;
        JmNewObjectInt = jenv->GetStaticMethodID(clsFREObject, "newObject", "(I)Lcom/adobe/fre/FREObject;");
    }

    return JmNewObjectInt;
}

jmethodID FREHelper::GetNewObjectDoubleMethod(JNIEnv *jenv)
{
    if (JmNewObjectDouble == NULL)
    {
        jclass clsFREObject = GetFREObjectClass(jenv);
        if (clsFREObject == NULL)
            return NULL;
        JmNewObjectDouble = jenv->GetStaticMethodID(clsFREObject, "newObject", "(D)Lcom/adobe/fre/FREObject;");
    }

    return JmNewObjectDouble;
}

jmethodID FREHelper::GetNewObjectBooleanMethod(JNIEnv *jenv)
{
    if (JmNewObjectBoolean == NULL)
    {
        jclass clsFREObject = GetFREObjectClass(jenv);
        if (clsFREObject == NULL)
            return NULL;
        JmNewObjectBoolean = jenv->GetStaticMethodID(clsFREObject, "newObject", "(Z)Lcom/adobe/fre/FREObject;");
    }

    return JmNewObjectBoolean;
}

jmethodID FREHelper::GetNewObjectStringMethod(JNIEnv *jenv)
{
    if (JmNewObjectString == NULL)
    {
        jclass clsFREObject = GetFREObjectClass(jenv);
        if (clsFREObject == NULL)
            return NULL;
        JmNewObjectString = jenv->GetStaticMethodID(clsFREObject, "newObject", "(Ljava/lang/String;)Lcom/adobe/fre/FREObject;");
    }

    return JmNewObjectString;
}

jclass FREHelper::GetFREObjectClass(JNIEnv* jenv)
{
    if (jenv == NULL)
        return NULL;

    if (ClsFREObject == NULL)
    {
        jclass clsFREObject = jenv->FindClass("com/adobe/fre/FREObject");
        //jthrowable exc = jenv->ExceptionOccurred();
        //if (exc) { //     //jclass newExcCls; jenv->ExceptionDescribe(); jenv->ExceptionClear(); //}
        ClsFREObject = (jclass)jenv->NewGlobalRef(clsFREObject);
    }

    return ClsFREObject;
}

jmethodID FREHelper::GetFREObjectConstructor(JNIEnv* jenv)
{
    if (jenv == NULL)
        return NULL;

    if (MdFREObjectConstructor == NULL)
        MdFREObjectConstructor = jenv->GetMethodID(GetFREObjectClass(jenv), "<init>", "()V");

    return MdFREObjectConstructor;
}


jclass FREHelper::GetByteBufferClass(JNIEnv* jenv)
{
    if (jenv == NULL)
        return NULL;

    if (ClsByteBuffer == NULL)
    {
        jclass clsByteBuffer = jenv->FindClass("java/nio/ByteBuffer");
        ClsByteBuffer = (jclass)jenv->NewGlobalRef(clsByteBuffer);
    }

    return ClsByteBuffer;
}

void FREHelper::GenerateFREExceptionClasses(JNIEnv* jenv)
{
    //SF_DEBUG_MESSAGE1(1, "GenerateFREExceptionClasses %p\n", jenv);
    if (jenv == NULL)
        return;

    if (ClsFRETypeMismatchException == NULL)
    {
        jclass clsFRETypeMismatchException = jenv->FindClass("com/adobe/fre/FRETypeMismatchException");
        ClsFRETypeMismatchException = (jclass)jenv->NewGlobalRef(clsFRETypeMismatchException);

        jclass clsFREInvalidObjectException = jenv->FindClass("com/adobe/fre/FREInvalidObjectException");
        ClsFREInvalidObjectException = (jclass)jenv->NewGlobalRef(clsFREInvalidObjectException);

        jclass clsFREASErrorException = jenv->FindClass("com/adobe/fre/FREASErrorException");
        ClsFREASErrorException = (jclass)jenv->NewGlobalRef(clsFREASErrorException);

        jclass clsFRENoSuchNameException = jenv->FindClass("com/adobe/fre/FRENoSuchNameException");
        ClsFRENoSuchNameException = (jclass)jenv->NewGlobalRef(clsFRENoSuchNameException);

        jclass clsFREWrongThreadException = jenv->FindClass("com/adobe/fre/FREWrongThreadException");
        ClsFREWrongThreadException = (jclass)jenv->NewGlobalRef(clsFREWrongThreadException);

        jclass clsIllegalArgumentException = jenv->FindClass("java/lang/IllegalArgumentException");
        ClsIllegalArgumentException = (jclass)jenv->NewGlobalRef(clsIllegalArgumentException);
    }
}

bool FREHelper::SetObjectPointer(JNIEnv* jenv, jobject jobj, long ptr)
{
    if (jenv == NULL || GetEnv(&jenv) == false)
        return false;

    //SF_DEBUG_MESSAGE2(1, "FREHelper::SetObjectPointer %p %d\n", jobj, ptr);

    if (jfpObject == NULL)
    {
        jclass clsFREObject = GetFREObjectClass(jenv);
        if (clsFREObject == NULL)
            return false;

        jfpObject = jenv->GetFieldID(clsFREObject, "pObject", "J");
        if (jfpObject == 0)
            return false;
    }

    jenv->SetLongField(jobj, jfpObject, ptr);
    return true;
}

long FREHelper::GetObjectPointer(JNIEnv* jenv, jobject jobj)
{
    if (jenv == NULL || GetEnv(&jenv) == false)
        return 0;

    //SF_DEBUG_MESSAGE1(1, "FREHelper::GetObjectPointer %p \n", jobj);
    if (jfpObject == NULL)
    {
        jclass clsFREObject = GetFREObjectClass(jenv);
        if (clsFREObject == NULL)
        {
            jenv->ThrowNew(FREHelper::ClsFREInvalidObjectException, "Cannot find FREObject class definition.");
            return 0;
        }

        jfpObject = jenv->GetFieldID(clsFREObject, "pObject", "J");
        if (jfpObject == 0)
        {
            jenv->ThrowNew(FREHelper::ClsFREInvalidObjectException, "Cannot find pObject member in FREObject class.");
            return 0;
        }
    }

    long ptr = jenv->GetLongField(jobj, jfpObject);
    if (ptr == 0)
        jenv->ThrowNew(FREHelper::ClsFREInvalidObjectException, "Object is invalid.");

    return ptr;
}

bool FREHelper::GetEnv(JNIEnv **penv)
{
    if (pJVM == NULL)
    {
        SF_DEBUG_MESSAGE(1, "JNI failed to get JNIEnv\n");
        *penv = NULL;
        return false;
    }

    int status = pJVM->GetEnv((void **)penv, JNI_VERSION_1_6);
    if(status < 0)
    {
        status = pJVM->AttachCurrentThread(penv, NULL);
        if(status < 0)
        {
            SF_DEBUG_MESSAGE(1, "JNI failed to get JNIEnv\n");
            *penv = NULL;
            return false;
        }
    }
    return true;
}

/* Must be done at startup as stated in https://developer.android.com/training/articles/perf-jni.html#faq_FindClass if you start your own thread */
bool FREHelper::Init(JNIEnv* jenv)
{
    if (jenv == NULL)
        return false;

    GetFREObjectClass(jenv);
    GenerateFREExceptionClasses(jenv);
    GetByteBufferClass(jenv);
    GetNewObjectIntMethod(jenv);
    GetNewObjectDoubleMethod(jenv);
    GetNewObjectBooleanMethod(jenv);
    GetNewObjectStringMethod(jenv);
    return true;
}


bool FREHelper::Finalize(JNIEnv* jenv)
{
    if (jenv == NULL)
        return false;

    if (ClsFREObject)
    {
        jenv->DeleteGlobalRef(ClsFREObject);
        ClsFREObject = NULL;
    }
    if (ClsByteBuffer)
    {
        jenv->DeleteGlobalRef(ClsByteBuffer);
        ClsByteBuffer = NULL;
    }
    if (ClsIllegalArgumentException)
    {
        jenv->DeleteGlobalRef(ClsIllegalArgumentException);
        ClsIllegalArgumentException = NULL;
    }
    if (ClsFRETypeMismatchException)
    {
        jenv->DeleteGlobalRef(ClsFRETypeMismatchException);
        ClsFRETypeMismatchException = NULL;
    }
    if (ClsFREInvalidObjectException)
    {
        jenv->DeleteGlobalRef(ClsFREInvalidObjectException);
        ClsFREInvalidObjectException = NULL;
    }
    if (ClsFREASErrorException)
    {
        jenv->DeleteGlobalRef(ClsFREASErrorException);
        ClsFREASErrorException = NULL;
    }
    if (ClsFRENoSuchNameException)
    {
        jenv->DeleteGlobalRef(ClsFRENoSuchNameException);
        ClsFRENoSuchNameException = NULL;
    }
    if (ClsFREWrongThreadException)
    {
        jenv->DeleteGlobalRef(ClsFREWrongThreadException);
        ClsFREWrongThreadException = NULL;
    }
    if (MdFREObjectConstructor)
        MdFREObjectConstructor = NULL;
    if (JmNewObjectInt)
        JmNewObjectInt = NULL;
    if (JmNewObjectDouble)
        JmNewObjectDouble = NULL;
    if (JmNewObjectBoolean)
        JmNewObjectBoolean = NULL;
    if (JmNewObjectString)
        JmNewObjectString = NULL;

    return true;
}

//////////////////// AndroidFREFunction ////////////////////

AndroidFREFunction::AndroidFREFunction(const char* name, AndroidExtensionContext& context, jobject jobjFunction, JNIEnv* jenv)
    : FREFunctionBase(name), RefContext(context)
{
    //SF_DEBUG_MESSAGE1(1, "AndroidFREFunction::AndroidFREFunction %s", name);
    if (jenv == NULL || FREHelper::GetEnv(&jenv) == false)
        ObjFunction = NULL;
    else
        ObjFunction = jenv->NewGlobalRef(jobjFunction);
}

AndroidFREFunction::~AndroidFREFunction()
{
    JNIEnv* jenv;
    if (FREHelper::GetEnv(&jenv) && ObjFunction != NULL)
    {
        jenv->DeleteGlobalRef(ObjFunction);
        ObjFunction = NULL;
    }
}

bool AndroidFREFunction::Call(unsigned argc, const Scaleform::GFx::Value* const argv, Scaleform::GFx::Value* const result)
{
    /* Convert params. */
    JNIEnv* jenv;
    if (!FREHelper::GetEnv(&jenv) || ObjFunction == NULL)
        return false;

    jclass clsFREObject = FREHelper::GetFREObjectClass(jenv);

    if (clsFREObject == NULL)
        return false;
    jobjectArray jArgv = jenv->NewObjectArray(argc, clsFREObject, NULL);

    SF_DEBUG_MESSAGE1(1, "AndroidFREFunction::Call :%p", jArgv);

    for(unsigned i = 0; i < argc; i++)
    {
        jobject jObj = NULL;
        switch(argv[i].GetType())
        {
            case Scaleform::GFx::Value::VT_String:    jObj = FREHelper::NewObject(argv[i].GetString(), jenv); break;
            case Scaleform::GFx::Value::VT_UInt:    jObj = FREHelper::NewObject((double)argv[i].GetUInt(), jenv); break;
            case Scaleform::GFx::Value::VT_Number:     jObj = FREHelper::NewObject((double)argv[i].GetNumber(), jenv); break;
            case Scaleform::GFx::Value::VT_Int:        jObj = FREHelper::NewObject(argv[i].GetInt(), jenv); break;
            case Scaleform::GFx::Value::VT_Boolean: jObj = FREHelper::NewObject(argv[i].GetBool(), jenv); break;
            default:
                break;
        }

        //SF_DEBUG_MESSAGE1(1, "2 AndroidFREFunction::Call :%p", jObj);
        jenv->SetObjectArrayElement(jArgv, i, jObj);
    }

    /* Call 'call' on FREObject. */
    jclass clsFunction = jenv->GetObjectClass(ObjFunction);
    jmethodID jmCall = jenv->GetMethodID(clsFunction, "call", "(Lcom/adobe/fre/FREContext;[Lcom/adobe/fre/FREObject;)Lcom/adobe/fre/FREObject;");

    jobject jobjResult = jenv->CallObjectMethod(ObjFunction, jmCall, RefContext.GetJObject(), jArgv);

    // convert jobjResult to GfxValue...
    if (jobjResult != NULL)
    {
        long ptr = FREHelper::GetObjectPointer(jenv, jobjResult);
        if (ptr != 0)
            *result = *(GFx::Value*)ptr;
    }

    return true;
}

//////////////////// AndroidExtensionContext ////////////////////

AndroidExtensionContext::AndroidExtensionContext(const char* name, jobject objContext, JNIEnv* jenv, AndroidExtension& extension)
    : ExtensionContext(name), RefExtension(extension)
{
    if (jenv == NULL || FREHelper::GetEnv(&jenv) == false)
        ObjContext = NULL;
    else
        ObjContext = jenv->NewGlobalRef(objContext);
}

AndroidExtensionContext::~AndroidExtensionContext()
{
    this->Dispose();
}


AndroidExtension& AndroidExtensionContext::GetExtension() const
{
    return RefExtension;
}

void AndroidExtensionContext::Dispose()
{
    JNIEnv* jenv;
    if (ObjContext == NULL || !FREHelper::GetEnv(&jenv))
        return;

    jclass clsContext = jenv->GetObjectClass(ObjContext);
    jmethodID jmDispose = jenv->GetMethodID(clsContext, "dispose", "()V");
     if (jmDispose != 0)
        jenv->CallVoidMethod(ObjContext, jmDispose);

    jenv->DeleteGlobalRef(ObjContext);
    ObjContext = NULL;
}

bool AndroidExtensionContext::GenerateFREFunctions()
{
    if (ObjContext == NULL)
        return false;

    if (!FREFunctionArray)
        FREFunctionArray = SF_NEW ArrayLH<AndroidFREFunction*>();

    JNIEnv* jenv;
    if (!FREHelper::GetEnv(&jenv))
        return false;

    // Call overrided
    jclass clsContext = jenv->GetObjectClass(ObjContext);
    jmethodID jmGetFunctions = jenv->GetMethodID(clsContext, "getFunctions", "()Ljava/util/Map;");

    if (jmGetFunctions == 0)
        return false;

    jobject objMap = jenv->CallObjectMethod(ObjContext, jmGetFunctions);
    jclass clsMap = jenv->GetObjectClass(objMap);

    jmethodID jmKeySet = jenv->GetMethodID(clsMap, "keySet", "()Ljava/util/Set;");
    jobject objSet = jenv->CallObjectMethod(objMap, jmKeySet);
    jclass clsSet = jenv->GetObjectClass(objSet);

    jmethodID jmToArray = jenv->GetMethodID(clsSet, "toArray", "()[Ljava/lang/Object;");
    jobject tab = jenv->CallObjectMethod(objSet, jmToArray);

    jmethodID jmSetSize = jenv->GetMethodID(clsSet, "size", "()I");
    jint size = jenv->CallIntMethod(objSet, jmSetSize);
    //SF_DEBUG_MESSAGE5(1, "AndroidExtensionContext::GenerateFREFunctions %p %p %p %p %p", objMap, clsMap , jmKeySet , objSet , clsSet);
    //SF_DEBUG_MESSAGE4(1, "AndroidExtensionContext::GenerateFREFunctions %p %p %p %d", jmToArray , tab , jmSetSize, size);

    jobjectArray * arrKey = reinterpret_cast<jobjectArray*>(&tab);
    for (int i = 0; i < size; i++)
    {
        jstring jstrFunction = (jstring) jenv->GetObjectArrayElement(*arrKey, (jsize)i);
        jmethodID jmMapGet = jenv->GetMethodID(clsMap, "get", "(Ljava/lang/Object;)Ljava/lang/Object;"); //(Ljava/lang/Object;)Lcom/adobe/fre/FREFunction;
        jobject jobjFunction = jenv->CallObjectMethod(objMap, jmMapGet, jstrFunction);

        jboolean copy;
        const char *strFunc = jenv->GetStringUTFChars(jstrFunction, &copy);
        AndroidFREFunction* freFunction = SF_NEW AndroidFREFunction(strFunc, *this, jobjFunction, jenv);
        FREFunctionArray->PushBack(freFunction);
        jenv->ReleaseStringUTFChars(jstrFunction, strFunc);
    }

    return true;
}

FREFunctionBase* AndroidExtensionContext::GetFREFunction(const char* functionName) const
{
    if (FREFunctionArray)
    {
        for (UPInt i = 0; i < FREFunctionArray->GetSize(); ++i)
        {
            if (strcmp((*FREFunctionArray)[i]->GetName(), functionName) == 0)
                return (*FREFunctionArray)[i];
        }
    }

    // Not found
    return NULL;
};

jobject AndroidExtensionContext::GetJObject() const
{
    return ObjContext;
}


//////////////////// AndroidExtension ////////////////////

AndroidExtension::AndroidExtension(const char* name, const char* initializer)
    : Extension(name, "")
{
    SF_DEBUG_MESSAGE1(1, "AndroidExtension::AndroidExtension name:%s\n", name);

    JNIEnv* jenv;
    if (!FREHelper::GetEnv(&jenv))
        return;

    /* convert from com.package.class to com/package/class */
    StringBuffer classString(initializer);
    for (UPInt i = 0; i < classString.GetLength(); i++)
    {
        if (classString[i] == '.')
            classString[i] = '/';
    }

    jclass clsExtension = jenv->FindClass(classString.ToCStr());
    if (clsExtension == NULL)
        return;
    jmethodID jmConstructor = jenv->GetMethodID(clsExtension, "<init>", "()V");
     if (jmConstructor == 0)
        return;
    jobject objExtension = jenv->NewObject(clsExtension, jmConstructor);
    if (objExtension == 0)
        return;
    ObjExtension = jenv->NewGlobalRef(objExtension);

    // call initialize
    this->Initializer();
}

AndroidExtension::~AndroidExtension()
{
    /* Call Extension native finalizer. */
    this->Finalizer();

    JNIEnv* jenv;
    if (FREHelper::GetEnv(&jenv) && ObjExtension != NULL)
    {
        jenv->DeleteGlobalRef(ObjExtension);
        ObjExtension = NULL;
    }
}

void AndroidExtension::Initializer()
{
    if (ObjExtension == NULL)
        return;

    JNIEnv* jenv;
    if (!FREHelper::GetEnv(&jenv))
        return;

    jclass clsExtension = jenv->GetObjectClass(ObjExtension);
    jmethodID jmInitialize = jenv->GetMethodID(clsExtension, "initialize", "()V");
     if (jmInitialize == 0)
        return;
    jenv->CallVoidMethod(ObjExtension, jmInitialize);
}

void AndroidExtension::Finalizer()
{
    if (ObjExtension == NULL)
        return;

    JNIEnv* jenv;
    if (!FREHelper::GetEnv(&jenv))
        return;

    jclass clsExtension = jenv->GetObjectClass(ObjExtension);
    jmethodID jmDispose = jenv->GetMethodID(clsExtension, "dispose", "()V");
     if (jmDispose == 0)
        return;
    jenv->CallVoidMethod(ObjExtension, jmDispose);
}

bool AndroidExtension::RemoveContext(const char* contextType)
{
    if (FREContextArray)
    {
        for (UPInt i = 0; i < FREContextArray->GetSize(); ++i)
        {
            if ((*FREContextArray)[i] == NULL)
                continue;

            if (strcmp((*FREContextArray)[i]->GetName().ToCStr(), contextType) == 0)
            {
                (*FREContextArray)[i]->Dispose();
                (*FREContextArray)[i] = NULL;
                return true;
            }
        }
    }

    return false;
}

ExtensionContext* AndroidExtension::GetContext(const char* contextType) const
{
    if (FREContextArray)
    {
        for (SPInt i = FREContextArray->GetSize() - 1; i >= 0;  --i)
        {
            AndroidExtensionContext* pExtCtx = (*FREContextArray)[i];
            if (pExtCtx == NULL)
            {
                FREContextArray->RemoveAt(i);
                continue;
            }

            if (strcmp(pExtCtx->GetName().ToCStr(), contextType) == 0)
                return pExtCtx;
        }
    }

    return NULL;
}

AndroidExtensionContext* AndroidExtension::GetContextFromJObject(jobject jcontext) const
{
    JNIEnv* jenv;
    if (!FREHelper::GetEnv(&jenv))
        return NULL;

    jclass clsContext = jenv->GetObjectClass(jcontext);
    jmethodID jmEquals = jenv->GetMethodID(clsContext, "equals", "(Ljava/lang/Object;)Z");


    if (FREContextArray)
    {
        for (UPInt i = 0; i < FREContextArray->GetSize(); ++i)
        {
            if ((*FREContextArray)[i] == NULL)
                continue;

            jboolean bSame = jenv->CallBooleanMethod(jcontext, jmEquals, (*FREContextArray)[i]->GetJObject());

            //SF_DEBUG_MESSAGE3(1, "GetContextFromJObject %p: %p %i\n", (*FREContextArray)[i]->GetJObject(), jcontext, bSame);
            if (bSame == JNI_TRUE)
                return (*FREContextArray)[i];
        }
    }

    return NULL;
}

bool AndroidExtension::CreateContext(const char* contextType)
{
    //SF_DEBUG_MESSAGE1(1, "AndroidExtension::CreateContext contextType:<%s>\n", contextType);
    if (ObjExtension == NULL)
        return false;

    JNIEnv* jenv;
    if (!FREHelper::GetEnv(&jenv))
        return false;

    // call create context
    jclass clsExtension = jenv->GetObjectClass(ObjExtension);
    jmethodID jmCreateContext = jenv->GetMethodID(clsExtension, "createContext", "(Ljava/lang/String;)Lcom/adobe/fre/FREContext;");

    if (jmCreateContext == 0)
        return false;

    jstring jstrContextType = jenv->NewStringUTF(contextType);
    jobject objContext = jenv->CallObjectMethod(ObjExtension, jmCreateContext, jstrContextType);
    jenv->DeleteLocalRef(jstrContextType);

    // Create c++ object
    if (!FREContextArray)
        FREContextArray = SF_NEW ArrayLH<AndroidExtensionContext*>();

    AndroidExtensionContext* pContext =  SF_NEW AndroidExtensionContext(contextType, objContext, jenv, *this);
    pContext->GenerateFREFunctions();
    FREContextArray->PushBack(pContext);

    return true;
}

//////////////////// AndroidExtensionManager ////////////////////

AndroidExtensionManager::AndroidExtensionManager()
{
    /* Nothing to do. */
}

AndroidExtensionManager::~AndroidExtensionManager()
{
    /* Nothing to do. */
}

void AndroidExtensionManager::InitNativeExtension(const String& extensionName, const String& initializer)
{
    SF_DEBUG_MESSAGE1(1, "AndroidExtensionManager::InitNativeExtension %s", extensionName.ToCStr());

    if (!FREExtensionArray)
        FREExtensionArray = SF_NEW ArrayLH<AndroidExtension*>();

    if (GetExtension(extensionName.ToCStr()) == NULL)
    {
            AndroidExtension* pExtension =  SF_NEW AndroidExtension(extensionName.ToCStr(), initializer.ToCStr());
            FREExtensionArray->PushBack(pExtension);
    }
}

Extension* AndroidExtensionManager::GetExtension(const char* ExtensionID) const
{
    if (FREExtensionArray)
    {
        for (UPInt i = 0; i < FREExtensionArray->GetSize(); ++i)
        {
            if (strcmp((*FREExtensionArray)[i]->GetName(), ExtensionID) == 0)
                return (*FREExtensionArray)[i];
        }
    }

    return NULL;
}

AndroidExtensionContext* AndroidExtensionManager::GetExtensionContextFromJavaContext(jobject jcontext) const
{
    AndroidExtensionContext* context = NULL;

    if (FREExtensionArray)
    {
        for (UPInt i = 0; i < FREExtensionArray->GetSize(); ++i)
        {
            context = (*FREExtensionArray)[i]->GetContextFromJObject(jcontext);
            if (context != NULL)
                break;
        }
    }

    return context;
}

////////////////////////////
//  ActionScript Methods //
///////////////////////////

bool AndroidExtensionManager::Call(const char* extensionID, const char* contextID, const char* functionName, unsigned argc, const Scaleform::GFx::Value* const argv, Scaleform::GFx::Value* const result)
{
    SF_DEBUG_MESSAGE3(1, "AndroidExtensionManager::Call(%s, %s, %s)", extensionID, contextID, functionName);

    Extension* pExtension = this->GetExtension(extensionID);
    if (pExtension == NULL)
        return false;

    //SF_DEBUG_MESSAGE(1, "AndroidExtensionManager::Call 2");

    ExtensionContext* pContext = pExtension->GetContext(contextID == NULL ? "" : contextID);
    if (pContext == NULL)
        return false;

    FREFunctionBase* pFunction = pContext->GetFREFunction(functionName);
    if (pFunction == NULL)
        return false;

    //SF_DEBUG_MESSAGE(1, "AndroidExtensionManager::Call 3");

    return pFunction->Call(argc, argv, result);
}

const char* AndroidExtensionManager::GetExtensionDirectory(const char* extensionID)
{
    return "libs";
}

void AndroidExtensionManager::InitializeExtensionContext(const char* extensionID, const char* contextID)
{
    SF_DEBUG_MESSAGE1(1, "AndroidExtensionManager::InitializeExtensionContext %s", extensionID);

    Extension* pExtension = this->GetExtension(extensionID);
    if (pExtension)
    {
        pExtension->CreateContext(contextID == NULL ? "" : contextID);
    }
}

void AndroidExtensionManager::FinalizeExtensionContext(const char* extensionID, const char* contextID)
{
    SF_DEBUG_MESSAGE1(1, "AndroidExtensionManager::FinalizeExtensionContext %s", extensionID);

    Extension* pExtension = this->GetExtension(extensionID);
    if (pExtension)
    {
        pExtension->RemoveContext(contextID == NULL ? "" : contextID);
    }
}

GFx::Value*    AndroidExtensionManager::GetActionScriptData(const char* extensionID, const char* contextID)
{
    SF_DEBUG_MESSAGE(1, "AS ExtensionContext::GetActionScriptData\n");

    Extension* pExtension = this->GetExtension(extensionID);
    if (pExtension == NULL)
        return NULL;

    ExtensionContext* pContext = pExtension->GetContext(contextID == NULL ? "" : contextID);
    if (pContext == NULL)
        return NULL;

    return pContext->GetActionScriptData();
}

void AndroidExtensionManager::SetActionScriptData(const char* extensionID, const char* contextID, GFx::Value* data)
{
    SF_DEBUG_MESSAGE1(1, "AS ExtensionContext::SetActionScriptData %p \n", data);

    Extension* pExtension = this->GetExtension(extensionID);
    if (pExtension == NULL)
        return;

    ExtensionContext* pContext = pExtension->GetContext(contextID == NULL ? "" : contextID);
    if (pContext == NULL)
        return;

    return pContext->SetActionScriptData(data);
}

void AndroidExtensionManager::SetActionScriptData(JNIEnv* jenv, jobject obj, jobject jobjASData)
{
    AndroidExtensionContext* context = GetExtensionContextFromJavaContext(obj);
    if (context == NULL)
    {
        jenv->ThrowNew(FREHelper::ClsFREASErrorException, "Context not found.");
        return;
    }

    long ptr = FREHelper::GetObjectPointer(jenv, jobjASData);
    if (ptr == 0)
        return;
    GFx::Value* pValASData = (GFx::Value*)ptr;
    context->SetActionScriptData(pValASData);
}

jobject AndroidExtensionManager::GetActionScriptData(JNIEnv* jenv, jobject obj)
{
    AndroidExtensionContext* context = GetExtensionContextFromJavaContext(obj);
    if (context == NULL)
    {
        jenv->ThrowNew(FREHelper::ClsFREASErrorException, "Context not found.");
        return NULL;
    }

    GFx::Value* asData = context->GetActionScriptData();
    if (asData == NULL)
        return NULL;
    jobject freObject = jenv->NewObject(FREHelper::GetFREObjectClass(jenv), FREHelper::GetFREObjectConstructor(jenv));
    FREHelper::SetObjectPointer(jenv, freObject, (long)asData);
    return freObject;
}

////////////////////////////
//  AppImpl ANE Methods //
///////////////////////////


ExtensionManager* AppImpl::GetExtensionManager()
{
    if (ExtensionManagerInstance == 0)
        ExtensionManagerInstance = new AndroidExtensionManager();

    return ExtensionManagerInstance.GetPtr();
}

const char* AppImpl::GetExtensionDirectory(const char* extensionID)
{
    return this->GetExtensionManager()->GetExtensionDirectory(extensionID);
}

void AppImpl::InitializeExtensionContext(const char* extensionID, const char* contextID)
{
    this->GetExtensionManager()->InitializeExtensionContext(extensionID, contextID);
}

void AppImpl::FinalizeExtensionContext(const char* extensionID, const char* contextID)
{
    this->GetExtensionManager()->FinalizeExtensionContext(extensionID, contextID);
}

bool AppImpl::Call(const char* extensionID, const char* contextID, const char* functionName, unsigned argc, const Scaleform::GFx::Value* const argv, Scaleform::GFx::Value* const result)
{
    return this->GetExtensionManager()->Call(extensionID, contextID, functionName, argc, argv, result);
}

GFx::Value*    AppImpl::GetActionScriptData(const char* extensionID, const char* contextID)
{
    return this->GetExtensionManager()->GetActionScriptData(extensionID, contextID);
}

void AppImpl::SetActionScriptData(const char* extensionID, const char* contextID, GFx::Value* data)
{
    this->GetExtensionManager()->SetActionScriptData(extensionID, contextID, data);
}

void AppImpl::SetActionScriptData1(JNIEnv* jenv, jobject obj, jobject jobjASData)
{
    static_cast<AndroidExtensionManager*>(this->GetExtensionManager())->SetActionScriptData(jenv, obj, jobjASData);
}

jobject AppImpl::GetActionScriptData1(JNIEnv* jenv, jobject obj)
{
    return static_cast<AndroidExtensionManager*>(this->GetExtensionManager())->GetActionScriptData(jenv, obj);
}

void AppImpl::OnNativeLibraryInit1(const String& extensionName, const String& initializer)
{
    GetExtensionManager()->InitNativeExtension(extensionName, initializer);
}

void AppImpl::DispatchStatusEventAsync1(jobject obj, const String& eventCode, const String& eventLevel)
{
    SF_DEBUG_MESSAGE3(1, "DispatchStatusEventAsync %p: %s %s\n", obj, eventCode.ToCStr(), eventLevel.ToCStr());

    AndroidExtensionContext* context = static_cast<AndroidExtensionManager*>(GetExtensionManager())->GetExtensionContextFromJavaContext(obj);
    if (context != NULL)
    {
        GFx::StatusEvent event (GFx::Event::Status, new String(eventCode), new String(eventLevel), new String(context->GetExtension().GetName()), new String(context->GetName()));
        pAppThread->OnEvent(event);
    }
}

void AppImpl::FREObjectFromInt1(JNIEnv* jenv, jobject jobj, jint value)
{
    SF_DEBUG_MESSAGE2(1, "FREObjectFromInt1 %p: %d\n", jenv, value);

    GFx::Value* intValue = new GFx::Value();
    intValue->SetInt(value);
    FREHelper::SetObjectPointer(jenv, jobj, (long)intValue);
}

void AppImpl::FREObjectFromDouble1(JNIEnv* jenv, jobject jobj, jdouble value)
{
    GFx::Value* doubleValue = new GFx::Value();
    doubleValue->SetNumber(value);
    FREHelper::SetObjectPointer(jenv, jobj, (long)doubleValue);
}

void AppImpl::FREObjectFromBoolean1(JNIEnv* jenv, jobject jobj, jboolean value)
{
    GFx::Value* boolValue = new GFx::Value();
    boolValue->SetBoolean(value);
    FREHelper::SetObjectPointer(jenv, jobj, (long)boolValue);
}

void AppImpl::FREObjectFromString1(JNIEnv* jenv, jobject jobj, jstring value)
{
    if (pMovie != NULL)
    {
        String strTmp;
        FREHelper::GetStringFromJString(jenv, strTmp, value);

        /* Create AS object, set the pointer to the java object. */
        GFx::Value* strValue = new GFx::Value();
        pMovie->CreateString(strValue, strTmp.ToCStr());
        FREHelper::SetObjectPointer(jenv, jobj, (long)strValue);
    }
}

void AppImpl::FREObjectFromClass1(JNIEnv* jenv, jobject jobj, jstring value, jobjectArray arrValue)
{
    if (pMovie != NULL)
    {
         if (jenv == NULL || FREHelper::GetEnv(&jenv) == false)
         {
             jenv->ThrowNew(FREHelper::ClsFREWrongThreadException, "Unable to get JNI Env.");
             return;
         }

        /* handle params */
        jsize size = arrValue ? jenv->GetArrayLength(arrValue) : 0;
        GFx::Value gfxParams[size];

        for (jsize i = 0; i < size; i++)
        {
            jobject freObject = (jobject) jenv->GetObjectArrayElement(arrValue, i);

            long ptr = FREHelper::GetObjectPointer(jenv, freObject);
            if (ptr == 0)
                return;

            gfxParams[i] = *((GFx::Value*)ptr);
        }

        String className;
        FREHelper::GetStringFromJString(jenv, className, value);

        GFx::Value* objectValue = new GFx::Value();
        pMovie->CreateObject(objectValue, className.ToCStr(), size == 0 ? NULL : gfxParams, size);
        FREHelper::SetObjectPointer(jenv, jobj, (long)objectValue);
        SF_DEBUG_MESSAGE2(1, "FREObjectFromClass %s %p\n", className.ToCStr(), objectValue);
    }
}

jint AppImpl::GetAsInt1(JNIEnv* jenv, jobject jobj)
{
    //SF_DEBUG_MESSAGE1(1, "FREHelper::GetAsInt1 %lu \n", ptr);
    long ptr = FREHelper::GetObjectPointer(jenv, jobj);
    if (ptr == 0)
        return 0;

    GFx::Value* val = (GFx::Value*)ptr;
    if (!val->IsInt())
    {
        SF_DEBUG_MESSAGE1(1, "FREHelper::GetAsInt1 2 THROW %p \n", FREHelper::ClsFRETypeMismatchException);
        jenv->ThrowNew(FREHelper::ClsFRETypeMismatchException, "Object is not a valid int." );
        return 0;
    }

    return val->GetInt();
}

jdouble AppImpl::GetAsDouble1(JNIEnv* jenv, jobject jobj)
{
    long ptr = FREHelper::GetObjectPointer(jenv, jobj);
    if (ptr == 0)
        return 0;

    GFx::Value* val = (GFx::Value*)ptr;
    if (!val->IsNumber())
    {
        jenv->ThrowNew(FREHelper::ClsFRETypeMismatchException, "Object is not a valid double.");
        return 0;
    }

    return val->GetNumber();
}

jboolean AppImpl::GetAsBool1(JNIEnv* jenv, jobject jobj)
{
    long ptr = FREHelper::GetObjectPointer(jenv, jobj);
    if (ptr == 0)
        return false;

    GFx::Value* val = (GFx::Value*)ptr;
    if (!val->IsBool())
    {
        jenv->ThrowNew(FREHelper::ClsFRETypeMismatchException, "Object is not a valid boolean.");
        return false;
    }

    return val->GetBool();
}

jstring AppImpl::GetAsString1(JNIEnv* jenv, jobject jobj)
{
    SF_DEBUG_MESSAGE1(1, "FREHelper::GetAsString1 %p \n", jobj);

    long ptr = FREHelper::GetObjectPointer(jenv, jobj);
    if (ptr == 0)
        return NULL;

    GFx::Value* val = (GFx::Value*)ptr;
    if (!val->IsString())
    {
        jenv->ThrowNew(FREHelper::ClsFRETypeMismatchException, "Object is not a valid string.");
        return NULL;
    }

    return jenv->NewStringUTF(val->GetString());
}


jobject AppImpl::NewObject1(JNIEnv* jenv, jclass clsFREObj, jstring value, jobjectArray arrValue)
{
    //SF_DEBUG_MESSAGE2(1, "NewObject1 %p: %p\n", jenv, clsFREObj);
    jmethodID jmFREObjectConstructor = jenv->GetMethodID(clsFREObj, "<init>", "(Ljava/lang/String;[Lcom/adobe/fre/FREObject;)V");
    jobject freObject = jenv->NewObject(clsFREObj, jmFREObjectConstructor, value, arrValue);
    if (freObject == 0)
    {
        jenv->ThrowNew(FREHelper::ClsFREASErrorException, "Error at Object Creation.");
        return NULL;
    }

    return freObject;
}

jobject AppImpl::GetProperty1(JNIEnv* jenv, jobject jobj, jstring jpropertyName)
{
    long ptr = FREHelper::GetObjectPointer(jenv, jobj);
    if (ptr == 0)
        return NULL;

    GFx::Value* pVal = (GFx::Value*)ptr;
    if (!pVal->IsObject())
    {
        jenv->ThrowNew(FREHelper::ClsFRETypeMismatchException, "[GetProperty1] Primitive types are not valid objects.");
        return NULL;
    }

    String strProperty;
    FREHelper::GetStringFromJString(jenv, strProperty, jpropertyName);

    if (!pVal->HasMember(strProperty.ToCStr()))
    {
        jenv->ThrowNew(FREHelper::ClsFRETypeMismatchException, "Property does not exist.");
        return NULL;
    }

    GFx::Value* pPropertyVal = new GFx::Value();
    if (!pVal->GetMember(strProperty.ToCStr(), pPropertyVal))
    {
        jenv->ThrowNew(FREHelper::ClsFRETypeMismatchException, "Basic types are not valid object.");
        return NULL;
    }

    jobject freObject = jenv->NewObject(FREHelper::GetFREObjectClass(jenv), FREHelper::GetFREObjectConstructor(jenv));
    FREHelper::SetObjectPointer(jenv, freObject, (long)pPropertyVal);

    SF_DEBUG_MESSAGE1(1, "GetProperty1 %s done.\n", strProperty.ToCStr());
    return freObject;
}

void AppImpl::SetProperty1(JNIEnv* jenv, jobject jobj, jstring jpropertyName, jobject jpropertyObj)
{
    long ptr = FREHelper::GetObjectPointer(jenv, jobj);
    if (ptr == 0)
        return;

    GFx::Value* pVal = (GFx::Value*)ptr;
    if (!pVal->IsObject())
    {
        jenv->ThrowNew(FREHelper::ClsFRETypeMismatchException, "[SetProperty1] Primitive types are not valid objects.");
        return;
    }

    long ptrProperty = FREHelper::GetObjectPointer(jenv, jpropertyObj);
    if (ptrProperty == 0)
    {
        jenv->ThrowNew(FREHelper::ClsFREInvalidObjectException, "Property is invalid.");
        return;
    }
    GFx::Value* pPropertyVal = (GFx::Value*)ptrProperty;

    String strProperty;
    FREHelper::GetStringFromJString(jenv, strProperty, jpropertyName);


    /* Special case, set length to ByteArray */
    if (pVal->IsByteArray() && (strProperty == "length"))
    {
        if (!pPropertyVal->IsInt())
        {
            jenv->ThrowNew(FREHelper::ClsIllegalArgumentException, "Param is not a valid integer.");
            return;
        }

        //SF_DEBUG_MESSAGE1(1, "SetByteArraySize: %d\n", pPropertyVal->GetInt());
        pVal->SetByteArraySize((unsigned)pPropertyVal->GetInt());
    }
    else
    {
        /* Common case. */
        pVal->SetMember(strProperty.ToCStr(), *pPropertyVal);
    }

    SF_DEBUG_MESSAGE2(1, "SetProperty1 %s done. params:%p\n", strProperty.ToCStr(), jpropertyObj);
}

jobject AppImpl::CallMethod1(JNIEnv* jenv, jobject jobj, jstring jmethodName, jobjectArray arrValue)
{
    long ptr = FREHelper::GetObjectPointer(jenv, jobj);
    if (ptr == 0)
        return NULL;

    GFx::Value* pVal = (GFx::Value*)ptr;
    if (!pVal->IsObject())
    {
        jenv->ThrowNew(FREHelper::ClsFRETypeMismatchException, "[CallMethod1] types are not valid objects.");
        return NULL;
    }

    jsize size = arrValue ? jenv->GetArrayLength(arrValue) : 0;
    GFx::Value gfxParams[size];

    for (jsize i = 0; i < size; i++)
    {
        jobject freObject = (jobject) jenv->GetObjectArrayElement(arrValue, i);

        long ptr = FREHelper::GetObjectPointer(jenv, freObject);
        if (ptr == 0)
            return NULL;

        gfxParams[i] = *((GFx::Value*)ptr);
    }

    String methodName;
    FREHelper::GetStringFromJString(jenv, methodName, jmethodName);

    SF_DEBUG_MESSAGE2(1, "CallMethod1 %s params:%d\n", methodName.ToCStr(), size);

    GFx::Value *presult = new GFx::Value();
    if (!pVal->Invoke(methodName.ToCStr(), presult, gfxParams, size))
    {
        jenv->ThrowNew(FREHelper::ClsFREASErrorException, "Error while calling AS method.");
        delete presult;
        return NULL;
    }

    jobject freObject = jenv->NewObject(FREHelper::GetFREObjectClass(jenv), FREHelper::GetFREObjectConstructor(jenv));
    FREHelper::SetObjectPointer(jenv, freObject, (long)presult);
    return freObject;
}


void AppImpl::ReleaseNative1(JNIEnv* jenv, jobject jobj)
{
    long ptr = FREHelper::GetObjectPointer(jenv, jobj);
    if (ptr == 0)
        return;

    GFx::Value* pVal = (GFx::Value*)ptr;
    delete pVal;
    SF_DEBUG_MESSAGE(1, "ReleaseNative1 done!\n");
}

//////////////////////////

jlong AppImpl::FREArrayGetLength(JNIEnv* jenv, jobject jobj)
{
    long ptr = FREHelper::GetObjectPointer(jenv, jobj);
    if (ptr == 0)
        return 0;
    GFx::Value* pVal = (GFx::Value*)ptr;
    if (pVal->IsArray())
        return pVal->GetArraySize();

    if (pVal->IsObject() && pVal->IsInstanceOf("Vector"))
    {
        if (pVal->HasMember("length"))
        {
            GFx::Value propertyVal;
            if (pVal->GetMember("length", &propertyVal) && propertyVal.IsUInt())
            {
                UInt32 uLength = propertyVal.GetUInt();
                return uLength;
            }
        }
    }

    jenv->ThrowNew(FREHelper::ClsFRETypeMismatchException, "This object is not a valid Array.");
    return 0;
}

void AppImpl::FREArraySetLength(JNIEnv* jenv, jobject jobj, jlong lLen)
{
    long ptr = FREHelper::GetObjectPointer(jenv, jobj);
    if (ptr == 0)
        return;
    GFx::Value* pVal = (GFx::Value*)ptr;
    if (pVal->IsArray())
    {
        pVal->SetArraySize(lLen);
        return;
    }

    if (pVal->IsObject() && pVal->IsInstanceOf("Vector"))
    {
        if (pVal->HasMember("length"))
        {
            GFx::Value propertyVal((UInt32)lLen);
            if (pVal->SetMember("length", propertyVal))
                return;
        }
    }

    jenv->ThrowNew(FREHelper::ClsFRETypeMismatchException, "This object is not a valid Array.");
    return;
}

jobject AppImpl::FREArrayGetObjectAt(JNIEnv* jenv, jobject jobj, jlong lPos)
{
    long ptr = FREHelper::GetObjectPointer(jenv, jobj);
    if (ptr == 0)
        return NULL;
    GFx::Value* pVal = (GFx::Value*)ptr;
    if (pVal->IsArray())
    {
        GFx::Value *presult = new GFx::Value();
        if (!pVal->GetElement(lPos, presult))
        {
            jenv->ThrowNew(FREHelper::ClsIllegalArgumentException, "Array index out of bounds.");
            return NULL;
        }

        jobject freObject = jenv->NewObject(FREHelper::GetFREObjectClass(jenv), FREHelper::GetFREObjectConstructor(jenv));
        FREHelper::SetObjectPointer(jenv, freObject, (long)presult);
        return freObject;
    }

    // Must be a vector
    if (pVal->IsObject() && pVal->IsInstanceOf("Vector"))
    {
        return NULL;
    }

    jenv->ThrowNew(FREHelper::ClsFRETypeMismatchException, "This object is not a valid Array.");
    return 0;
}

void AppImpl::FREArraySetObjectAt(JNIEnv* jenv, jobject jobj, jlong lPos, jobject jobjParam)
{
    //SF_DEBUG_MESSAGE2(1, "FREArraySetObjectAt %p: %d\n", jenv, jobj);
    long ptr = FREHelper::GetObjectPointer(jenv, jobj);
    if (ptr == 0)
        return;
    GFx::Value* pVal = (GFx::Value*)ptr;
    if (pVal->IsArray())
    {
        // Get element to set in the array.
        long ptrElem = FREHelper::GetObjectPointer(jenv, jobjParam);
        if (ptrElem == 0)
            return;
        pVal->SetElement(lPos, *(GFx::Value*)ptrElem);
        return;
    }

    // Must be a vector
    if (pVal->IsObject() && pVal->IsInstanceOf("Vector"))
        return;

    jenv->ThrowNew(FREHelper::ClsFRETypeMismatchException, "This object is not a valid Array.");
    return;
}

jlong AppImpl::FREByteArrayGetLength(JNIEnv* jenv, jobject jobj)
{
    long ptr = FREHelper::GetObjectPointer(jenv, jobj);
    if (ptr == 0)
        return 0;
    GFx::Value* pVal = (GFx::Value*)ptr;
    if (pVal->IsByteArray())
        return pVal->GetByteArraySize();

    jenv->ThrowNew(FREHelper::ClsFRETypeMismatchException, "This object is not a valid ByteArray.");
    return 0;
}

jobject AppImpl::FREByteArrayGetBytes(JNIEnv* jenv, jobject jobj)
{
    long ptr = FREHelper::GetObjectPointer(jenv, jobj);
    if (ptr == 0)
        return NULL;
    GFx::Value* pVal = (GFx::Value*)ptr;

    if (pVal->IsByteArray())
    {
        SF_DEBUG_MESSAGE1(1, "FREByteArrayGetBytes1 %p\n", jobj);

        //jclass clsByteBuffer = FREHelper::GetByteBufferClass(jenv);
        unsigned sizeBuffer = pVal->GetByteArraySize();
        void* bufferAddr = pVal->GetRawDataPtr();

        if ((sizeBuffer > 0) && bufferAddr)
            return jenv->NewDirectByteBuffer(bufferAddr, sizeBuffer);
        else
        {
            jenv->ThrowNew(FREHelper::ClsFREASErrorException, "ByteArray length has not been set.");
            return NULL;
        }
    }

    jenv->ThrowNew(FREHelper::ClsFRETypeMismatchException, "This object is not a valid ByteArray.");
    return NULL;
}

void AppImpl::FREByteArrayAcquire(JNIEnv* jenv, jobject jobj)
{
}

void AppImpl::FREByteArrayRelease(JNIEnv* jenv, jobject jobj)
{
}

jint AppImpl::FREBitmapDataGetWidth(JNIEnv* jenv, jobject jobj)
{
    long ptr = FREHelper::GetObjectPointer(jenv, jobj);
    if (ptr == 0)
        return 0;
    GFx::Value* pVal = (GFx::Value*)ptr;
    if (pVal->IsObject() && pVal->IsInstanceOf("flash.display.BitmapData"))
    {
        if (pVal->HasMember("width"))
        {
            GFx::Value propertyVal;
            if (pVal->GetMember("width", &propertyVal) && propertyVal.IsInt())
                return propertyVal.GetInt();
        }
    }

    jenv->ThrowNew(FREHelper::ClsFRETypeMismatchException, "This object is not a valid BitmapData.");
    return 0;
}

jint AppImpl::FREBitmapDataGetHeight(JNIEnv* jenv, jobject jobj)
{
    long ptr = FREHelper::GetObjectPointer(jenv, jobj);
    if (ptr == 0)
        return 0;
    GFx::Value* pVal = (GFx::Value*)ptr;
    if (pVal->IsObject() && pVal->IsInstanceOf("flash.display.BitmapData"))
    {
        if (pVal->HasMember("height"))
        {
            GFx::Value propertyVal;
            if (pVal->GetMember("height", &propertyVal) && propertyVal.IsInt())
                return propertyVal.GetInt();
        }
    }

    jenv->ThrowNew(FREHelper::ClsFRETypeMismatchException, "This object is not a valid BitmapData.");
    return 0;
}

jboolean AppImpl::FREBitmapDataHasAlpha(JNIEnv* jenv, jobject jobj)
{
    long ptr = FREHelper::GetObjectPointer(jenv, jobj);
    if (ptr == 0)
        return 0;
    GFx::Value* pVal = (GFx::Value*)ptr;
    if (pVal->IsObject() && pVal->IsInstanceOf("flash.display.BitmapData"))
    {
        if (pVal->HasMember("transparent"))
        {
            GFx::Value propertyVal;
            if (pVal->GetMember("transparent", &propertyVal) && propertyVal.IsBool())
                return propertyVal.GetBool();
        }
    }

    jenv->ThrowNew(FREHelper::ClsFRETypeMismatchException, "This object is not a valid BitmapData.");
    return 0;
}

jboolean AppImpl::FREBitmapDataIsPremultiplied(JNIEnv* jenv, jobject jobj)
{
    return false;
}

jint AppImpl::FREBitmapDataGetLineStride32(JNIEnv* jenv, jobject jobj)
{
    return 0;
}

jboolean AppImpl::FREBitmapDataIsInvertedY(JNIEnv* jenv, jobject jobj)
{
    return false;
}

jobject AppImpl::FREBitmapDataGetBits(JNIEnv* jenv, jobject jobj)
{
    return NULL;
}

void AppImpl::FREBitmapDataAcquire(JNIEnv* jenv, jobject jobj)
{
}

void AppImpl::FREBitmapDataInvalidateRect(JNIEnv* jenv, jobject jobj, jint lX, jint lY, jint lWidth, jint lHeight)
{
}

void AppImpl::FREBitmapDataRelease(JNIEnv* jenv, jobject jobj)
{
}

#endif //#ifdef SF_USE_ANE

class DeviceImpl : public NewOverrideBase<Stat_Default_Mem>
{
public:
    DeviceImpl(Render::ThreadCommandQueue *commandQueue);
    ~DeviceImpl();

    bool adjustViewConfig(ViewConfig* config);

    bool initGraphics(const ViewConfig& config, Device::Window* window, ThreadId renderThreadId = 0);
    bool reconfigureGraphics(const ViewConfig& config);
    void shutdownGraphics();

    void updateStatus();

    AppImpl*                pWindow;
    Ptr<Render::GL::HAL>    pHal;
    DeviceStatus            Status;
};


DeviceImpl::DeviceImpl(Render::ThreadCommandQueue *commandQueue)
: pWindow(0), pHal(0), Status(Device_NeedInit)
{
    pHal = *SF_NEW Render::GL::ProfilerHAL(commandQueue);
}

DeviceImpl::~DeviceImpl()
{
    pHal.Clear();
}


bool DeviceImpl::adjustViewConfig(ViewConfig* config)
{
    if (config->HasFlag(View_Size))
    {
        if (config->ViewSize.Width < 1)
            config->ViewSize.Width = 1;
        if (config->ViewSize.Height< 1)
            config->ViewSize.Height= 1;
    }
    config->SetFlag(View_Stereo, false);
    return true;
}


bool DeviceImpl::initGraphics(const ViewConfig& config, Device::Window* window,
                              ThreadId renderThreadId)
{
    if (!pHal || !window)
        return false;

    pWindow = (AppImpl*) window;

    SF_DEBUG_MESSAGE(1, "Device initGraphics ");

    // Initialize the HAL. Note, we are setting the dynamic shader compilation here, because on Android, shader compilation
    // can take several seconds and thus cause an ANR warning on startup (or resume). We also set HALConfig_MultipleShaderCacheFiles,
    // so the binary shaders will be saved as they are compiled (and reloaded onResume or startup).
    unsigned configFlags = config.GetInitHALConfigFlags() | Render::GL::HALConfig_DynamicShaderCompile | Render::GL::HALConfig_MultipleShaderCacheFiles;
    if (!pHal->InitHAL(Render::GL::HALInitParams(configFlags, renderThreadId, pWindow->CacheDir)))
    {
        shutdownGraphics();
        pWindow = 0;
        return false;
    }

    SF_DEBUG_MESSAGE(1, "Device initGraphics finished");

    Status = Device_Ready;
    return true;
}

bool DeviceImpl::reconfigureGraphics(const ViewConfig& config)
{
    return true;
}


void DeviceImpl::shutdownGraphics()
{
    if (pHal)
        pHal->ShutdownHAL();

    pWindow = 0;
    Status = Device_NeedInit;
}

void DeviceImpl::updateStatus()
{
}


//------------------------------------------------------------------------
Device::Device(Render::ThreadCommandQueue *commandQueue)
{
    pImpl = SF_NEW DeviceImpl(commandQueue);
}
Device::~Device()
{
    delete pImpl;
}

Render::HAL* Device::GetHAL() const
{
    return pImpl->pHal;
}
DeviceStatus Device::GetStatus() const
{
    pImpl->updateStatus();
    return pImpl->Status;
}

//------------------------------------------------------------------------
bool Device::GraphicsConfigOnMainThread() const
{
    return false;
}

void Device::ResizeFrame(void*) { }

bool Device::AdjustViewConfig(ViewConfig* config)
{
    return pImpl->adjustViewConfig(config);
}

bool Device::InitGraphics(const ViewConfig& config, Device::Window* window,
                          ThreadId renderThreadId)
{
    return pImpl->initGraphics(config, window, renderThreadId);
}

bool Device::ReconfigureGraphics(const ViewConfig& config)
{

    return true;
}
void Device::ShutdownGraphics()
{
    pImpl->shutdownGraphics();
}

Size<unsigned> Device::GetDefaultViewSize()
{
    return Size<unsigned>(800, 480);
}


//------------------------------------------------------------------------
void Device::Clear(UInt32 color)
{
    float   rgba[4];
    GLboolean scissorEnabled = glIsEnabled(GL_SCISSOR_TEST);
    if (scissorEnabled)
        glDisable(GL_SCISSOR_TEST);
    Render::Color c(color);
    c.GetRGBAFloat(rgba);
    glClearColor(rgba[0], rgba[1], rgba[2], rgba[3]);
    glClear(GL_COLOR_BUFFER_BIT);
    if (scissorEnabled)
        glEnable(GL_SCISSOR_TEST);
}

int AppBase::GetDisplayCount()
{
    return 1;
}

Device::Window* Device::CreateGraphicsWindow(int WindowId, const ViewConfig&)
{
    return 0;
}

void Device::SetWindow(Window*)
{
}

void Device::PresentFrame(unsigned)
{
    SF_AMP_SCOPE_RENDER_TIMER_ID("Device::PresentFrame", Amp_Native_Function_Id_Present);
}

void Device::BeginFrame()
{
}

bool Device::TakeScreenShot(const String&)
{
    return false;
}

UInt32 Device::GetCaps() const
{
    return 0;
}


//------------------------------------------------------------------------
// ***** AppBase

AppBase::AppBase()
: Cursor(false), ShuttingDown(false)
{
    pImplBase = pImpl = SF_NEW AppImpl(this);
    pDevice = 0;
}
AppBase::~AppBase()
{
    delete pImpl;
}

void AppImpl::ApplyViewConfigArgs(ViewConfig* config, const Args& args)
{
    // No options modify view config arguments/flags.
}

//------------------------------------------------------------------------

// This call cleans up the application and kills the window.
void AppBase::Shutdown()
{
    pImpl->killWindow();
    ShuttingDown = true;
}

bool AppBase::IsPadConnected(unsigned)
{
    return false;
}

void AppBase::BringWindowToFront()
{
}

Device::Window* AppBase::GetDeviceWindow() const
{
    return pImpl;
}

bool AppBase::IsConsole()
{
    return false;
}

// Amount (multiple of size) to reduce viewport by to fit in visible area on TVs.
// This is a floating point value such as 0.1 for 10% safe frame border.
Size<float> AppBase::GetSafeArea()
{
    return Size<float>(0.0f);
}

const char* AppBase::GetDefaultFilePath()
{
    return SF_ANDROID_CONTENT_DIR;
}

void AppImpl::OnAccelerometerUpdate1(int idAcc, double timestamp, double accelerationX, double accelerationY, double accelerationZ)
{
    GFx::AccelerometerEvent event (GFx::Event::Accelerometer, idAcc, timestamp, accelerationX, accelerationY, accelerationZ);
    pAppThread->OnEvent(event);
}

void AppImpl::OnGeolocationUpdate1(int idGeo, double latitude, double longitude, double altitude, double hAccuracy, double vAccuracy, double speed, double heading, double timestamp)
{
    GFx::GeolocationEvent event (GFx::Event::Geolocation, idGeo, latitude, longitude, altitude, hAccuracy, vAccuracy, speed, heading, timestamp);
    pAppThread->OnEvent(event);
}

void AppImpl::OnStatus1(const String& code, const String& level)
{
    GFx::StatusEvent event (GFx::Event::Status, new String(code), new String(level), NULL, NULL);
    pAppThread->OnEvent(event);
}

static const GFx::Event::EventType MouseEvents[] = {GFx::Event::MouseDown, GFx::Event::MouseUp, GFx::Event::MouseMove, GFx::Event::Unknown, GFx::Event::Unknown,
GFx::Event::MouseDown, GFx::Event::MouseUp, GFx::Event::MouseMove};

void AppImpl::OnTouchMouse1(int action, float x, float y)
{
    if (MouseEvents[action & 0xff] != GFx::Event::Unknown)
    {
        GFx::MouseEvent event (MouseEvents[action & 0xff], 0, x, y);
        pAppThread->OnEvent(event);
    }
}

static const GFx::Event::EventType TouchEvents[] = {GFx::Event::TouchBegin, GFx::Event::TouchEnd, GFx::Event::TouchMove, GFx::Event::Unknown, GFx::Event::Unknown,
GFx::Event::TouchBegin, GFx::Event::TouchEnd, GFx::Event::TouchMove};

void AppImpl::OnTouch1(int pointerId, int action, float x, float y)
{
    if (TouchEvents[action] != GFx::Event::Unknown)
    {
        GFx::TouchEvent event(TouchEvents[action], pointerId, x, y, 0, 0, pointerId == 0);
        pAppThread->OnEvent(event);
    }
}

static const GFx::Event::EventType GestureEvents[] = {GFx::Event::GestureEnd, GFx::Event::GestureBegin, GFx::Event::Gesture, GFx::Event::GestureSimple};
static const unsigned GestureTypes[] = {0, GFx::GestureEvent::GestureBit_Pan, GFx::GestureEvent::GestureBit_Zoom, GFx::GestureEvent::GestureBit_Swipe};

void AppImpl::OnGesture1(int action, int mode, float x, float y, float panx, float pany, float scale)
{
    GFx::GestureEvent event(GestureEvents[action], GestureTypes[mode], x, y, panx, pany, scale, scale, 0);
    pAppThread->OnEvent(event);
}

void AppImpl::OnOrientation1(int orient)
{
    switch (orient)
    {
        default:
        case 0: // ROTATION_0
            pAppThread->OnOrientation(Render::Viewport::View_Orientation_Normal, true);
            break;
        case 1: // ROTATION_90
            pAppThread->OnOrientation(Render::Viewport::View_Orientation_R90, true);
            break;
        case 2: // ROTATION_180
            pAppThread->OnOrientation(Render::Viewport::View_Orientation_180, true);
            break;
        case 3: // ROTATION_270
            pAppThread->OnOrientation(Render::Viewport::View_Orientation_L90, true);
            break;
    }
}

void AppImpl::OnKeyboard1(int kbId, int key, int down, int mods)
{
    GFx::KeyEvent event(down ? GFx::Event::KeyDown : GFx::Event::KeyUp, (Key::Code)key, 0, 0, mods, kbId);
    pAppThread->OnEvent(event);
}

void AppImpl::OnChar1(int kbId, UInt32 code, int mods)
{
    GFx::KeyEvent event(GFx::Event::KeyDown, Key::None, 0, code, mods, kbId);
    pAppThread->OnEvent(event);
}

void AppImpl::HandleVirtualKeyboardOpen(bool multiline, const Render::RectF& textBox)
{
    SF_DEBUG_MESSAGE(1, "HandleVirtualKeyboardOpen: Setting up JVM callbacks\n");
    bool attached;
    JNIEnv *jenv;
    int status = pJVM->GetEnv((void **)&jenv, JNI_VERSION_1_6);
    if(status < 0)
    {
        SF_DEBUG_MESSAGE(1, "HandleVirtualKeyboardOpen: Failed to get JNI jenvironment, assuming native thread\n");
        status = pJVM->AttachCurrentThread(&jenv, NULL);
        if(status < 0)
        {
            SF_DEBUG_MESSAGE(1, "HandleVirtualKeyboardOpen: Failed to attach current thread\n");
            return;
        }
        attached = true;
    }

    SF_DEBUG_MESSAGE(1, "HandleVirtualKeyboardOpen: Getting class and methodID\n");
    jclass cls = jenv->GetObjectClass(JObj);
    if (NULL == cls)
    {
        SF_DEBUG_MESSAGE(1, "HandleVirtualKeyboardOpen: Failed to get class reference\n");
        return;
    }
    jmethodID cb = jenv->GetMethodID(cls, "OpenVirtualKeyboard", "(Z)V");

    SF_DEBUG_MESSAGE4(1, "HandleVirtualKeyboardOpen: Calling Java method (%p, %p, %p) - %d\n", jenv, JObj, cb, multiline);
    jenv->CallVoidMethod(JObj, cb, multiline);

    SF_DEBUG_MESSAGE(1, "HandleVirtualKeyboardOpen: Done!\n");
    if (attached) pJVM->DetachCurrentThread();
}

void AppImpl::HandleVirtualKeyboardClose()
{
    SF_DEBUG_MESSAGE(1, "HandleVirtualKeyboardClose: Setting up JVM callbacks\n");
    bool attached;
    JNIEnv *jenv;
    int status = pJVM->GetEnv((void **)&jenv, JNI_VERSION_1_6);
    if(status < 0)
    {
        SF_DEBUG_MESSAGE(1, "HandleVirtualKeyboardClose: Failed to get JNI jenvironment, assuming native thread\n");
        status = pJVM->AttachCurrentThread(&jenv, NULL);
        if(status < 0)
        {
            SF_DEBUG_MESSAGE(1, "HandleVirtualKeyboardClose: Failed to attach current thread\n");
            return;
        }
        attached = true;
    }

    SF_DEBUG_MESSAGE(1, "HandleVirtualKeyboardClose: Getting class and methodID\n");
    jclass cls = jenv->GetObjectClass(JObj);
    if (NULL == cls)
    {
        SF_DEBUG_MESSAGE(1, "HandleVirtualKeyboardClose: Failed to get class reference\n");
        return;
    }
    jmethodID cb = jenv->GetMethodID(cls, "CloseVirtualKeyboard", "()V");

    SF_DEBUG_MESSAGE3(1, "HandleVirtualKeyboardOpen: Calling Java method (%p, %p, %p)\n", jenv, JObj, cb);
    jenv->CallVoidMethod(JObj, cb);

    SF_DEBUG_MESSAGE(1, "HandleVirtualKeyboardClose: Done!\n");
    if (attached) pJVM->DetachCurrentThread();
}


//////////////// Accelerometer //////////////////////

bool AppImpl::GetEnv(JNIEnv **penv) const
{
    int status = pJVM->GetEnv((void **)penv, JNI_VERSION_1_6);
    if(status < 0)
    {
        status = pJVM->AttachCurrentThread(penv, NULL);
        if(status < 0)
        {
            SF_DEBUG_MESSAGE(1, "JNI call Method: Failed to attach current thread\n");
            *penv = NULL;
        }
        return true;
    }
    return false;
}

bool AppImpl::IsAccelerometerMuted() const
{
    JNIEnv *env;
    bool attached = this->GetEnv(&env);
    if (env == NULL)
        return false;

    jclass cls = env->GetObjectClass(JObj);
    if (NULL == cls)
        return false;
    jmethodID cb = env->GetMethodID(cls, "isAccelerometerMuted", "()Z");
    //SF_DEBUG_MESSAGE3(1, "isAccelerometerMuted: Calling Java method (%p, %p, %p)\n", env, JObj, cb);
    jboolean isMuted = env->CallBooleanMethod(JObj, cb);

    if (attached)
        pJVM->DetachCurrentThread();

    return isMuted != JNI_FALSE;
}


bool AppImpl::IsAccelerometerSupported() const
{
    JNIEnv *env;
    bool attached = this->GetEnv(&env);
    if (env == NULL)
        return false;

    jclass cls = env->GetObjectClass(JObj);
    if (NULL == cls)
        return false;
    jmethodID cb = env->GetMethodID(cls, "isAccelerometerSupported", "()Z");
    //SF_DEBUG_MESSAGE3(1, "isAccelerometerSupported: Calling Java method (%p, %p, %p)\n", env, JObj, cb);
    jboolean isSupported = env->CallBooleanMethod(JObj, cb);

    if (attached)
        pJVM->DetachCurrentThread();

    return isSupported != JNI_FALSE;
}


void AppImpl::SetAccelerometerInterval(int accelerometerId, int interval)
{
    JNIEnv *env;
    bool attached = this->GetEnv(&env);
    if (env == NULL)
        return;

    jclass cls = env->GetObjectClass(JObj);
    if (NULL == cls)
        return;
    jmethodID cb = env->GetMethodID(cls, "setAccelerometerInterval", "(II)V");
    //SF_DEBUG_MESSAGE3(1, "setAccelerometerInterval: Calling Java method (%p, %p, %p)\n", env, JObj, cb);
    env->CallVoidMethod(JObj, cb, accelerometerId, interval);

    if (attached)
        pJVM->DetachCurrentThread();
}


bool AppImpl::RegisterAccelerometer(int accelerometerId)
{
    JNIEnv *env;
    bool attached = this->GetEnv(&env);
    if (env == NULL)
        return false;

    jclass cls = env->GetObjectClass(JObj);
    if (NULL == cls)
        return false;
    jmethodID cb = env->GetMethodID(cls, "registerAccelerometer", "(I)Z");
    //SF_DEBUG_MESSAGE3(1, "registerAccelerometer: Calling Java method (%p, %p, %p)\n", env, JObj, cb);
    jboolean isRegistered = env->CallBooleanMethod(JObj, cb, accelerometerId);

    if (attached)
        pJVM->DetachCurrentThread();
    SF_DEBUG_MESSAGE(1, "registerAccelerometer: Done!\n");

    return isRegistered != JNI_FALSE;
}


bool AppImpl::UnregisterAccelerometer(int accelerometerId)
{
    JNIEnv *env;
    bool attached = this->GetEnv(&env);
    if (env == NULL)
        return false;

    jclass cls = env->GetObjectClass(JObj);
    if (NULL == cls)
        return false;
    jmethodID cb = env->GetMethodID(cls, "unregisterAccelerometer", "(I)Z");
    //SF_DEBUG_MESSAGE3(1, "unregisterAccelerometer: Calling Java method (%p, %p, %p)\n", env, JObj, cb);
    jboolean isUnregistered = env->CallBooleanMethod(JObj, cb, accelerometerId);

    if (attached)
        pJVM->DetachCurrentThread();
    SF_DEBUG_MESSAGE(1, "unregisterAccelerometer: Done!\n");

    return isUnregistered != JNI_FALSE;
}


//////////////////// Geolocation ////////////////////

bool AppImpl::IsGeolocationMuted() const
{
    JNIEnv *env;
    bool attached = this->GetEnv(&env);
    if (env == NULL)
        return false;

    jclass cls = env->GetObjectClass(JObj);
    if (NULL == cls)
        return false;
    jmethodID cb = env->GetMethodID(cls, "isGeolocationMuted", "()Z");
    jboolean isMuted = env->CallBooleanMethod(JObj, cb);

    if (attached)
        pJVM->DetachCurrentThread();

    return isMuted != JNI_FALSE;
}


bool AppImpl::IsGeolocationSupported() const
{
    JNIEnv *env;
    bool attached = this->GetEnv(&env);
    if (env == NULL)
        return false;

    jclass cls = env->GetObjectClass(JObj);
    if (NULL == cls)
        return false;
    jmethodID cb = env->GetMethodID(cls, "isGeolocationSupported", "()Z");
    //SF_DEBUG_MESSAGE3(1, "isGeolocationSupported: Calling Java method (%p, %p, %p)\n", env, JObj, cb);
    jboolean isSupported = env->CallBooleanMethod(JObj, cb);

    if (attached)
        pJVM->DetachCurrentThread();

    return isSupported != JNI_FALSE;
}


void AppImpl::SetGeolocationInterval(int geolocationId, int interval)
{
    JNIEnv *env;
    bool attached = this->GetEnv(&env);
    if (env == NULL)
        return;

    jclass cls = env->GetObjectClass(JObj);
    if (NULL == cls)
        return;
    jmethodID cb = env->GetMethodID(cls, "setGeolocationInterval", "(II)V");
    env->CallVoidMethod(JObj, cb, geolocationId, interval);

    if (attached)
        pJVM->DetachCurrentThread();
}


bool AppImpl::RegisterGeolocation(int geolocationId)
{
    JNIEnv *env;
    bool attached = this->GetEnv(&env);
    if (env == NULL)
        return false;

    jclass cls = env->GetObjectClass(JObj);
    if (NULL == cls)
        return false;
    jmethodID cb = env->GetMethodID(cls, "registerGeolocation", "(I)Z");
    jboolean isRegistered = env->CallBooleanMethod(JObj, cb, geolocationId);

    if (attached)
        pJVM->DetachCurrentThread();
    SF_DEBUG_MESSAGE(1, "registerGeolocation: Done!\n");

    return isRegistered != JNI_FALSE;
}


bool AppImpl::UnregisterGeolocation(int geolocationId)
{
    JNIEnv *env;
    bool attached = this->GetEnv(&env);
    if (env == NULL)
        return false;

    jclass cls = env->GetObjectClass(JObj);
    if (NULL == cls)
        return false;
    jmethodID cb = env->GetMethodID(cls, "unregisterGeolocation", "(I)Z");
    jboolean isUnregistered = env->CallBooleanMethod(JObj, cb, geolocationId);

    if (attached)
        pJVM->DetachCurrentThread();
    SF_DEBUG_MESSAGE(1, "unregisterGeolocation: Done!\n");

    return isUnregistered != JNI_FALSE;
}

void AppImpl::SetMovie(Ptr<Scaleform::GFx::Movie> pmovie)
{
    SF_DEBUG_MESSAGE1(1, "SetMovie: ptr[%p]\n", pmovie.GetPtr());
    pMovie = pmovie;
}

// void AppImpl::SetupJVMCallbacks()
// {
//     SF_DEBUG_MESSAGE(1, "SetupJVMCallbacks: Setting up JVM callbacks\n");
//     bool attached;
//     JNIEnv *jenv;
//     int status = pJVM->GetEnv((void **)&jenv, JNI_VERSION_1_6);
//     if(status < 0)
//     {
//         SF_DEBUG_MESSAGE(1, "SetupJVMCallbacks: Failed to get JNI jenvironment, assuming native thread\n");
//         status = pJVM->AttachCurrentThread(&jenv, NULL);
//         if(status < 0)
//         {
//             SF_DEBUG_MESSAGE(1, "SetupJVMCallbacks: Failed to attach current thread\n");
//             exit(1);
//         }
//         attached = true;
//     }
//
//
//     /* PPS: @TODO The class is in fact dynamically created at runtime; perhaps we need to pass this via -D */
//     /* Pass string to ctor and use string subst */
//     const char* clsName = "com/scaleform/FxPlayerMobile_Debug_NoRTTI/MainActivity";
//     SF_DEBUG_MESSAGE1(1, "SetupJVMCallbacks: Trying to find class '%s'\n", clsName);
//     JMainActivityClass = jenv->FindClass(clsName);
//     if (NULL == JMainActivityClass)
//     {
//         SF_DEBUG_MESSAGE(1, "SetupJVMCallbacks: Can't find app class\n");
//         if (attached) pJVM->DetachCurrentThread();
//         exit(1);
//     }
//
//     JVKOpenCallback = jenv->GetMethodID(JMainActivityClass, "OpenVirtualKeyboard", "()V");
//     if (NULL == JVKOpenCallback)
//     {
//         SF_DEBUG_MESSAGE(1, "SetupJVMCallbacks: Can't find 'OpenVirtualKeyboard' method\n");
//         if (attached) pJVM->DetachCurrentThread();
//         //exit(1);
//     }
//
//     JVKCloseCallback = jenv->GetMethodID(JMainActivityClass, "CloseVirtualKeyboard", "()V");
//     if (NULL == JVKCloseCallback)
//     {
//         SF_DEBUG_MESSAGE(1, "SetupJVMCallbacks: Can't find 'CloseVirtualKeyboard' method\n");
//         if (attached) pJVM->DetachCurrentThread();
//         //exit(1);
//     }
//
//     SF_DEBUG_MESSAGE(1, "SetupJVMCallbacks: Done!n");
//     if (attached) pJVM->DetachCurrentThread();
//
// }



// Application Main function
int AppBase::AppMain(int argc, char* argv[])
{
    // Scaleform::System initSFSystem;

    SF_DEBUG_MESSAGE(1, "About to parse and handle cmd line args..\n");

    // Parse and handle command line arguments.
    InitArgDescriptions(&pImplBase->Arguments);
    if (!OnArgs(pImplBase->Arguments, pImplBase->Arguments.ParseCommandLine(argc, argv)))
        return 0;

    SF_DEBUG_MESSAGE(1, "About to init application and window..\n");

#ifdef GFX_GESTURE_RECOGNIZE
    Scaleform::GFx::GestureRecognizer *gestureRecognizer = new Scaleform::GFx::GestureRecognizer(*this);
    gestureRecognizer->setup();
    pGestureManager = gestureRecognizer;
    SF_DEBUG_MESSAGE(1, "Gesture recognizer setup\n");
#endif

    // Initialize application and its window.
    ViewConfig config(Device::GetDefaultViewSize());
    ApplyViewConfigArgs(&config, GetArgs());
    if (!OnInit(config))
    {
        return 0;
    }

    //     SF_DEBUG_MESSAGE(1, "About to setup JVM callbacks..\n");
    //     AppImpl *aimpl = (AppImpl*)GetAppImpl();
    //     aimpl->SetupJVMCallbacks();

    return 0;
}

String AppBase::GetContentDirectory()
{
    return SF_ANDROID_CONTENT_DIR;
}

int AppBase::GetKeyboardCount()
{
    return 1;
}

int AppBase::GetMouseCount()
{
    return 1;
}

void AppBase::SetOrientationMode(bool)
{
}

UInt32 AppBase::GetCaps() const
{
    return 0;
}

//------------------------------------------------------------------------
// ***** SystemCursorManager

SystemCursorManager::SystemCursorManager(bool systemCursorAvailable) :
SystemCursorState(systemCursorAvailable, false),
pImpl(0)
{
}
SystemCursorManager::~SystemCursorManager()
{
}

void SystemCursorManager::update(bool forceSet)
{
    // System cursors update. Since there are none, do nothing.
}


String AppBase::GetSaveDataDirectory()
{
    // Must ensure SetupFiles has completed successfully before returning
    if (!pImpl->SetupFilesComplete)
    {
        SF_DEBUG_MESSAGE(1,"JNI not initialized. Waiting..");
        pImpl->SetupFilesCompleteEvent.Wait();
    }

    return pImpl->FilesDir;
}


}} // Scaleform::Platform
