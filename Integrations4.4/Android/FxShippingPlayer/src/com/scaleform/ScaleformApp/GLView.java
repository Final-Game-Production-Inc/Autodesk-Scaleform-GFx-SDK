package com.scaleform.${APP_NAME};

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.text.InputType;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.KeyEvent;
import android.util.Log;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL10;

class GLView extends GLSurfaceView
{
    private static native void NativeReset();
    private static native void NativeResize(int width, int height);
    private static native void NativeDraw();
    
    private static boolean NativeDrawComplete = false;
    
    private MainActivity    mActivity;
    private boolean MultilineTextfieldMode;

    public GLView(Context context, MainActivity activity)
    {
        super(context);
        
        mActivity = activity;
        
        setEGLContextFactory(new ContextFactory());
        setEGLConfigChooser(new ConfigChooser());

        setRenderer(new Renderer());
        
        // PPS: Needed for soft input
        setFocusable(true);
        setFocusableInTouchMode(true);
        MultilineTextfieldMode = false;
    }
    
    @Override
     public InputConnection onCreateInputConnection(EditorInfo outAttrs) 
     {
        // PPS: This can be forced via InputMethodManager.restartInput()
        if (Debug.ENABLED){ Log.d("GFxPlayer", "onCreateInputConnection"); }
         outAttrs.initialCapsMode = 0;
        outAttrs.initialSelEnd = outAttrs.initialSelStart = -1;
        outAttrs.inputType = (InputType.TYPE_CLASS_TEXT |
                InputType.TYPE_TEXT_VARIATION_NORMAL |
                InputType.TYPE_TEXT_FLAG_AUTO_COMPLETE);
        outAttrs.imeOptions = ((!MultilineTextfieldMode ? EditorInfo.IME_ACTION_DONE : 0) |
                EditorInfo.IME_FLAG_NO_EXTRACT_UI);

         return new GLViewInputConnection(this, mActivity);
     }

     @Override
     public boolean onCheckIsTextEditor() 
     {
         return true;
     }
    
    public void setMultilineTextfieldMode(boolean multiline)
    {
        MultilineTextfieldMode = multiline;
    }
    public boolean getMultilineTextfieldMode()
    {
        return MultilineTextfieldMode;
    }
    
    private static class GLViewInputConnection extends BaseInputConnection 
    {
        MainActivity     mActivity;
        GLView            mView;
        public GLViewInputConnection(GLView view, MainActivity activity) 
        {
            super(view, false);
            mView = view;
            mActivity = activity;
        }
    
        @Override
        public boolean sendKeyEvent(KeyEvent event) 
        {
            int c = event.getKeyCode();
            if (Debug.ENABLED){ Log.d("GFxPlayer", "sendKeyEvent - code = " + event.getKeyCode()); }
            
            switch (event.getAction()) 
            {
                case KeyEvent.ACTION_DOWN:
                    if (c == KeyEvent.KEYCODE_ENTER) 
                    {
                        if ((event.getFlags() & KeyEvent.FLAG_EDITOR_ACTION) != 0 && !mView.getMultilineTextfieldMode())
                        {
                            mActivity.CloseVirtualKeyboard();
                            return true;
                        }
                        c = 13;    // Convert to GFx Key::Code::Return
                    }
                    else if (c == KeyEvent.KEYCODE_DEL)
                    {
                        c = 8; // Convert to GFx Key::Code::Backspace
                    }
                    else 
                    {
                        c = event.getUnicodeChar(event.getMetaState());
                        if (Debug.ENABLED){ Log.d("GFxPlayer", "sendKeyEvent - ACTION_DOWN (char) - " + c); }
                        mActivity.onChar(c);
                        return true;
                    }
                    mActivity.onKeyDown(c, event);
                    return true;
                    
                case KeyEvent.ACTION_UP:
                    if (c == KeyEvent.KEYCODE_ENTER) 
                    {
                        c = 13;    // Convert to GFx Key::Code::Return
                    }
                    else if (c == KeyEvent.KEYCODE_DEL)
                    {
                        c = 8; // Convert to GFx Key::Code::Backspace
                    }
                    else 
                    {
                        // Ignore other codes
                        return true;
                    }
                    mActivity.onKeyUp(c, event);
                    return true;
                        
                case KeyEvent.ACTION_MULTIPLE:
                    if (event.getKeyCode() == KeyEvent.KEYCODE_UNKNOWN) 
                    {
                        String chars = event.getCharacters();
                        if (Debug.ENABLED){ Log.d("GFxPlayer", "sendKeyEvent - ACTION_MULTIPLE - " + chars); }
                        if (chars != null) 
                        {
                            for (int i = 0; i < chars.length(); i++) 
                            {
                                c = chars.charAt(i);
                                mActivity.onChar(c);
                            }
                        }
                    }
                    return true;
                default:
                    return false;
            }
        }
    }

    
    private static class ContextFactory implements GLSurfaceView.EGLContextFactory
    {
        private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
        public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig config)
        {
            int[] attribs = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE };
            EGLContext context = egl.eglCreateContext(display, config, EGL10.EGL_NO_CONTEXT, attribs);
            return context;
        }

        public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context)
        {
            egl.eglDestroyContext(display, context);
        }
    }
   
    private static class ConfigChooser implements GLSurfaceView.EGLConfigChooser
    {
        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display)
        {
            int[] searchattr = new int[]
                {
                    EGL10.EGL_RED_SIZE, 4,
                    EGL10.EGL_GREEN_SIZE, 4,
                    EGL10.EGL_BLUE_SIZE, 4,
                    EGL10.EGL_RENDERABLE_TYPE, 4,
                    EGL10.EGL_NONE
                };
            int[] nconfigs = new int[1];
            int[] attr = new int[1];
            egl.eglChooseConfig(display, searchattr, null, 0, nconfigs);
            EGLConfig[] configs = new EGLConfig[nconfigs[0]];
            egl.eglChooseConfig(display, searchattr, configs, nconfigs[0], nconfigs);

            EGLConfig best = null;
            for(EGLConfig config : configs)
            {
                if (!egl.eglGetConfigAttrib(display, config, EGL10.EGL_STENCIL_SIZE, attr) || attr[0] < 4) 
                    continue;

                int r = 0, g = 0, b = 0, a = 0;
                if (egl.eglGetConfigAttrib(display, config, EGL10.EGL_RED_SIZE, attr)) r = attr[0];
                if (egl.eglGetConfigAttrib(display, config, EGL10.EGL_GREEN_SIZE, attr)) g = attr[0];
                if (egl.eglGetConfigAttrib(display, config, EGL10.EGL_BLUE_SIZE, attr)) b = attr[0];
                if (egl.eglGetConfigAttrib(display, config, EGL10.EGL_ALPHA_SIZE, attr)) a = attr[0];

                if (r == 5 && g == 6 && b == 5 && a >= 0)
                    return config;
                else if (best == null)
                    best = config;
            }
            return best;
        }
    }

	@Override
	public void onPause()
	{
		super.onPause();
        
        // When the surface is paused, indicate that rendering is not happening.
		NativeDrawComplete = false;
	}

	public boolean RendererInitComplete()
	{
		return NativeDrawComplete;
	}

    private static class Renderer implements GLSurfaceView.Renderer
    {
        public void onDrawFrame(GL10 gl)
        {
            NativeDraw();
            
            NativeDrawComplete = true;
        }

        public void onSurfaceChanged(GL10 gl, int width, int height)
        {
            NativeResize(width, height);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config)
        {
            NativeReset();
        }
    }
}
