/**************************************************************************

Filename    :   CE4100MiniApp.h
Content     :
Created     :
Authors     :

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/
#ifndef INC_LINUXMINIAPP_H
#define INC_LINUXMINIAPP_H

#include "FxMiniApp.h"

// GFx Includes
#include "GFx_Kernel.h"
#include "GFx.h"
#include "GFx_Renderer_GL.h"

#include <termios.h>
#include <unistd.h>

#include <EGL/egl.h>

#if defined(LINUX_CE)
#include "libgdl.h"
#endif

namespace SF = Scaleform;
using namespace Scaleform;
using namespace Render;
using namespace GFx;

class LinuxMiniApp : public FxMiniApp
{
public:
    Ptr<GL::HAL>            pRenderHAL;
    char                    ContentDir[256];
    String                  FileName;
    Array<Array<char> >     Filenames;
    int                     CurFile;
    bool                    QuitFlag;

	EGLDisplay m_EGLDisplay;
	EGLConfig  m_EGLConfig;
	EGLSurface m_EGLSurface;
	EGLContext m_EGLContext;

	// Key handling
	int devfd;
	struct termios termio;
    struct termios termio_orig;

#if defined(LINUX_CE)
	gdl_plane_id_t m_Plane; // A gdl plane
#endif

public:
    LinuxMiniApp ();
    virtual ~LinuxMiniApp() { }

    bool SetupWindow();
    void KillWindow();
    int  Loop();

    virtual const char* GetContentDirectory();
    virtual const char* GetDeviceType() { return "Linux"; }
    virtual bool HasKeyboard() { return 1; }
    virtual void RequestExit();
    virtual void SetWindowTitle(const char *ptitle) { }
    virtual void SetFrameTime (int t);
    virtual void SetOrientationMode(bool change);
    virtual bool GetNextFilename(char *pfilename, bool prev);

    virtual void Present();
    virtual void Clear();

    virtual GL::HAL* GetRenderer() { return pRenderHAL; }

    static LinuxMiniApp* CreateImpl();

#ifdef FXPLAYERMINI_MISD
    virtual bool NeedsOrientationMatrix() { return true; }
#endif
};

#define FXMINIAPP_MAIN(AppClass)                      					\
int main(int argc, char **argv)											\
{																		\
	static unsigned char SystemStorage[sizeof(GFx::System)];			\
	new (&SystemStorage) GFx::System();									\
																		\
	AppClass* pApp = new AppClass();									\
    SF_DEBUG_MESSAGE(1, "FxPlayerMini startup");						\
																		\
    if (!pApp->SetupWindow())                                    		\
        SF_DEBUG_MESSAGE(1, "SetupWindow failed");                 		\
																		\
	if (pApp->pRenderHAL)                                            	\
        pApp->OnDropFile(pApp->FileName);		  						\
        																\
    do																	\
    {																	\
		pApp->AdvanceAndDisplay();										\
		pApp->Present();												\
	}																	\
	while(!pApp->QuitFlag);												\
																		\
	pApp->KillWindow();													\
	delete pApp;														\
																		\
	return 0;															\
}																		\

#endif //INC_LINUXMINIAPP_H
