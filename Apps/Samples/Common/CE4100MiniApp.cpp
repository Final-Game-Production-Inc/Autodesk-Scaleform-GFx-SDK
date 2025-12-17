/**************************************************************************

Filename    :   CE4100MiniApp.cpp
Content     :
Created     :
Authors     :

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "CE4100MiniApp.h"

// GFx Includes
#include "GFx_Kernel.h"
#include "GFx.h"
#include "GFx_Renderer_GL.h"
#include "GFx_Render.h"

// System includes
#include <GLES2/gl2.h>
#include <unistd.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace Scaleform;
using namespace Render;

#if !defined(CONNAME)
#define CONNAME "/dev/tty"
#endif

#if defined(LINUX_CE)

#ifdef I_WANT_LR

#include "LR_PICInterface.h"

char clientData[128];
unsigned short g_usRemoteLastKey;

// Callback function for the remote control
INT32 pic24_client_callback(UINT8 type, UINT8 length, void *data, void *clientData)
{
	unsigned char *buffer = (unsigned char *)data;

	if (length == 3 && buffer[0] == 0x01)
	{
		g_usRemoteLastKey = (unsigned short)buffer[1] << 8 | buffer[2];

		if(g_usRemoteLastKey == 0x4512)
		{
			SF_DEBUG_MESSAGE(1, "poweroff\n");
			system("poweroff &");
		}
		
		printf( "Button pressed.\n" );
	}

	return 0;
}

LR_PICInterface pic_if(pic24_client_callback, clientData);

#if !defined(REMOTE)
#define REMOTE "/dev/ttyS1"
#endif

#endif 

#endif

LinuxMiniApp::LinuxMiniApp() :  m_EGLDisplay(0),
								m_EGLConfig(0),
								m_EGLSurface(0),
								m_EGLContext(0)
{
    QuitFlag = 0;
	
	// Defaults.
	Width = 1280;
	Height = 720;
	
    SFstrcpy(ContentDir, sizeof(ContentDir), "./");
}

bool LinuxMiniApp::SetupWindow()
{
#if defined(LINUX_CE)
	gdl_init(0);
	
	gdl_display_info_t info;

	if( gdl_get_display_info( GDL_DISPLAY_ID_0, &info ) == GDL_SUCCESS )
	{
		Width = info.tvmode.width;
		Height = info.tvmode.height;
	}


	m_Plane = GDL_PLANE_ID_UPP_C;

 	gdl_pixel_format_t pixelFormat 	= GDL_PF_ARGB_32;
    gdl_color_space_t colorSpace 	= GDL_COLOR_SPACE_RGB;
    gdl_rectangle_t srcRect, dstRect;

    gdl_ret_t rc = GDL_SUCCESS;

    dstRect.origin.x = 0;
    dstRect.origin.y = 0;

    srcRect.origin.x = 0;
    srcRect.origin.y = 0;

    srcRect.width  = dstRect.width  = Width;
    srcRect.height = dstRect.height = Height;

    rc = gdl_plane_reset(m_Plane);

    rc = gdl_plane_config_begin(m_Plane);

    if(rc != GDL_SUCCESS)
    {
        SF_DEBUG_MESSAGE1(1, "Failed to begin config of GDL plane. (Error code 0x%x)\n", rc);
        return false;
    }

    rc = gdl_plane_set_attr(GDL_PLANE_SRC_COLOR_SPACE, &colorSpace);

    if(rc != GDL_SUCCESS)
    {
        SF_DEBUG_MESSAGE1(1, "Failed to set color space of GDL plane. (Error code 0x%x)\n", rc);
        return false;
	}

    rc = gdl_plane_set_attr(GDL_PLANE_PIXEL_FORMAT, &pixelFormat);

    if(rc != GDL_SUCCESS)
    {
        SF_DEBUG_MESSAGE1(1, "Failed to set pixel format of GDL plane. (Error code 0x%x)\n", rc);
        return false;
    }

    rc = gdl_plane_set_attr(GDL_PLANE_DST_RECT, &dstRect);

    if(rc != GDL_SUCCESS)
    {
        SF_DEBUG_MESSAGE1(1, "Failed to set dst rect of GDL plane. (Error code 0x%x)\n", rc);
        return false;
    }

    rc = gdl_plane_set_attr(GDL_PLANE_SRC_RECT, &srcRect);

    if(rc != GDL_SUCCESS)
    {
        SF_DEBUG_MESSAGE1(1, "Failed to set src rect of GDL plane. (Error code 0x%x)\n", rc);
        return false;
    }

/*
    if(m_Plane == GDL_PLANE_ID_IAP_B || m_Plane == GDL_PLANE_ID_IAP_A)
    {
	gdl_uint32 alpha_global = 255;
        gdl_uint32 alpha_out = GDL_PLANE_ID_UNDEFINED;
	gdl_pixel_format_t pix_fmt = GDL_PF_ARGB_8;

	rc = gdl_plane_set_attr(GDL_PLANE_ALPHA_GLOBAL, &alpha_global);
        if(rc != GDL_SUCCESS)
	{
		SF_DEBUG_MESSAGE1(1, "Failed to set alpha_global of GDL plane. Error code 0x%x\n", rc);
		return false;
	}

        rc = gdl_plane_set_attr(GDL_PLANE_ALPHA_OUT, &alpha_out);
	if(rc != GDL_SUCCESS)
	{
		SF_DEBUG_MESSAGE1(1, "Failed to set alpha_out of GDL plane. Error code 0x%x\n", rc);
		return false;
	}

	rc = gdl_plane_set_attr(GDL_PLANE_PIX_FORMAT, &pix_fmt);
	if(rc != GDL_SUCCESS)
	{
		SF_DEBUG_MESSAGE1(1, "Failed to set the pixel format of GDL plane. Error code : 0x%x\n", rc);
		return false;
	}

	
    }
*/


    rc = gdl_plane_config_end(GDL_FALSE);

    if(rc != GDL_SUCCESS)
    {
        gdl_plane_config_end(GDL_TRUE);
        SF_DEBUG_MESSAGE1(1, "Failed to end config of GDL plane. (Error code 0x%x)\n", rc);
        return false;
	}

#endif

	// Initialise EGL
	EGLint ai32ContextAttribs[] = 
{
        EGL_CONTEXT_CLIENT_VERSION, 2, // Specifies to use OpenGL ES 2.0
        EGL_NONE
    };


	m_EGLDisplay = eglGetDisplay((EGLNativeDisplayType)0);//(NativeDisplayType)EGL_DEFAULT_DISPLAY);//(EGLNativeDisplayType)0);

	EGLint iMajorVersion, iMinorVersion;

	if(m_EGLDisplay == EGL_NO_DISPLAY) 
	{
		SF_DEBUG_MESSAGE(1, "Error : no display found.\n");
		return false;
	}

	if(!eglInitialize(m_EGLDisplay, &iMajorVersion, &iMinorVersion))
	{
		SF_DEBUG_MESSAGE(1, "Error: eglInitialize() failed.\n");
		return false;
	}

	eglBindAPI(EGL_OPENGL_ES_API);


/*	EGLint pi32ConfigAttribs[32];
	int i32Index = 0;
	pi32ConfigAttribs[i32Index++] = EGL_SURFACE_TYPE;
	pi32ConfigAttribs[i32Index++] = EGL_WINDOW_BIT;
	pi32ConfigAttribs[i32Index++] = EGL_RENDERABLE_TYPE;
	pi32ConfigAttribs[i32Index++] = EGL_OPENGL_ES2_BIT;
#if defined(LINUX_CE)
	pi32ConfigAttribs[i32Index++] = EGL_BUFFER_SIZE;
	pi32ConfigAttribs[i32Index++] = 32;
#endif
	pi32ConfigAttribs[i32Index++] = EGL_NONE;*/

	EGLint pi32ConfigAttribs[] =
 {
        EGL_BUFFER_SIZE,       EGL_DONT_CARE,
        EGL_SAMPLE_BUFFERS,    1,   // multisample is set 
        EGL_SAMPLES,           4,   // number of samples per pixel 
        EGL_DEPTH_SIZE,        16,
	EGL_ALPHA_SIZE,	       8,
        EGL_RED_SIZE,          8,
        EGL_GREEN_SIZE,        8,
        EGL_BLUE_SIZE,         8,
        EGL_RENDERABLE_TYPE,   EGL_OPENGL_ES2_BIT,
//	EGL_TRANSPARENT_TYPE,  EGL_TRANSPARENT_RGB
        EGL_NONE
    };

	int iConfigs;

	if(!eglChooseConfig(m_EGLDisplay, pi32ConfigAttribs, &m_EGLConfig, 1, &iConfigs) || (iConfigs != 1))
	{
		SF_DEBUG_MESSAGE(1, "Error: eglChooseConfig() failed.\n");
		return false;
	}

	m_EGLSurface = eglCreateWindowSurface(m_EGLDisplay, m_EGLConfig, (EGLNativeWindowType) NULL, NULL);

	if(m_EGLSurface == EGL_NO_SURFACE)
	{
		SF_DEBUG_MESSAGE(1, "eglCreateWindowSurface didn't return a valid surface.\n");
		return false;
	}

	m_EGLContext = eglCreateContext(m_EGLDisplay, m_EGLConfig, NULL, ai32ContextAttribs);

	if(m_EGLContext == EGL_NO_CONTEXT)
	{
		SF_DEBUG_MESSAGE(1, "eglCreateContext failed to create a context.\n");
		return false;
	}

	eglMakeCurrent(m_EGLDisplay, m_EGLSurface, m_EGLSurface, m_EGLContext);

    // Create renderer.
    if (!(pRenderHAL = *new Render::GL::HAL))
    {
        KillWindow();
        return 0;
    }

	// Get our surface's width and height
    EGLint w,h;
    eglQuerySurface(m_EGLDisplay, m_EGLSurface, EGL_WIDTH, &w);
	eglQuerySurface(m_EGLDisplay, m_EGLSurface, EGL_HEIGHT, &h);

    OnSize(w, h);

	// Setup keyboard
#if defined(LINUX_CE)

#ifdef I_WANT_LR
	// Initialise the remote control
	g_usRemoteLastKey = 0x0;
	pic_if.Init(REMOTE);
	pic_if.setIrRepeatMode(IR_REPEAT_MODE_BUFFER);
#endif 

#else
	// Keyboard handling
	if((devfd=open(CONNAME, O_RDWR|O_NDELAY)) <= 0)
	{
		SF_DEBUG_MESSAGE(1, "Can't open tty (" CONNAME ")\n");
	}
	else
	{
		tcgetattr(devfd,&termio_orig);
		tcgetattr(devfd,&termio);
		cfmakeraw(&termio);
		termio.c_oflag |= OPOST | ONLCR; // Turn back on cr-lf expansion on output
		termio.c_cc[VMIN]=1;
		termio.c_cc[VTIME]=0;

		if(tcsetattr(devfd,TCSANOW,&termio) == -1)
		{
			SF_DEBUG_MESSAGE(1, "Can't set tty attributes for " CONNAME "\n");
		}
	}
#endif

    //glEnable(GL_CULL_FACE); 
    if (!InitPreGraphics())
        return 0;
    pRenderHAL->SetDependentVideoMode();
    return InitPostGraphics();
}

void LinuxMiniApp::KillWindow()
{
    Shutdown();
    pRenderHAL = 0;

    // Tidy up EGL
    eglMakeCurrent(m_EGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(m_EGLDisplay, m_EGLContext);
	eglDestroySurface(m_EGLDisplay, m_EGLSurface);
   	eglTerminate(m_EGLDisplay);

#if defined(LINUX_CE)
	gdl_close();
#endif
}

void LinuxMiniApp::Present()
{
	// Disabled for plane transparency !
/*
	glClearColor( 1.f, 0.f, 1.f, 0.5f );
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE );
	glClear( GL_COLOR_BUFFER_BIT );
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
*/

	eglSwapBuffers(m_EGLDisplay, m_EGLSurface);

	// Check keyboard: Should this go somewhere else?
#if defined(LINUX_CE)

#ifdef I_WANT_LR

	switch(g_usRemoteLastKey)
	{
		case 0x4522: // Exit
			QuitFlag = 1;
			break;
		case 0x4521: // Ok
			OnKeyboard(0, SF::Key::Pause, 1, 0);
			OnKeyboard(0, SF::Key::Pause, 0, 0);
			break;
		case 0x4523: // Chapter back
			OnKeyboard(0, SF::Key::Left, 1, 0);
			OnKeyboard(0, SF::Key::Left, 0, 0);
			printf( "Left\n" );
			break;
		case 0x454d: // Chapter forward
			OnKeyboard(0, SF::Key::Right, 1, 0);
			OnKeyboard(0, SF::Key::Right, 0, 0);
			printf( "Right\n" );
			break;
	}

	g_usRemoteLastKey = 0;

#endif 

#else
	int		ckb = 0;
	size_t  bytes_read;

	// Keyboard.
	if(devfd > 0)
	{
		while ((bytes_read = read(devfd, &ckb, 1)) == 1);

		switch(ckb)
		{
			case 'q':
			case 'Q':
				QuitFlag = 1;
			break;
			case 27:
				OnKeyboard(0, SF::Key::Pause, 1, 0);
				OnKeyboard(0, SF::Key::Pause, 0, 0);
			break;
			case 68: // Left Arrow
				OnKeyboard(0, SF::Key::Left, 1, 0);
				OnKeyboard(0, SF::Key::Left, 0, 0);
			break;
			case 67: // Right Arrow
				OnKeyboard(0, SF::Key::Right, 1, 0);
				OnKeyboard(0, SF::Key::Right, 0, 0);
			break;
		}
	}
#endif
}

void LinuxMiniApp::Clear()
{
    glClearColor(0.5f, 0, 0.5f, 0);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void LinuxMiniApp::RequestExit()
{
    exit(0);
}

void LinuxMiniApp::SetFrameTime (int t)
{
}

void LinuxMiniApp::SetOrientationMode(bool change)
{
}

const char* LinuxMiniApp::GetContentDirectory()
{
    return ContentDir;
}

bool LinuxMiniApp::GetNextFilename(char* filename, bool prev)
{
    const char* path = ContentDir;

loop:
    if (Filenames.GetSize() == 0)
    {
        DIR* dirp = opendir(path);
        if (dirp != NULL)
        {
            struct dirent *dp;
            while ((dp = readdir(dirp)) != NULL)
            {
                Array<char> fn;
                int ext = 0;
                for (int i = 0; dp->d_name[i]; i++)
                {
                    if (dp->d_name[i] == '.')
                        ext = i;
                    fn.PushBack(dp->d_name[i]);
                }
                fn.PushBack(0);
                if ((!strcmp(&fn[ext], ".swf") || !strcmp(&fn[ext], ".gfx")))
                    Filenames.PushBack(fn);
            }
            closedir(dirp);
            CurFile = 0;
        }
    }

    if (CurFile >= 0 && CurFile < int(Filenames.GetSize()))
    {
        CurFile += (prev ? -1 : 1);
        if (CurFile < 0)
            CurFile = int(Filenames.GetSize()-1);
        else if (CurFile >= int(Filenames.GetSize()))
            CurFile = 0;
        SFstrcpy(filename, 256, ContentDir);
        SFstrcat(filename, 256-strlen(ContentDir), &Filenames[CurFile].Front());
		
		printf( "Loading : %s\n", filename );
		
        return 1;
    }
    else
    {
        Filenames.Clear();
        goto loop;
    }

    return 0;
}
