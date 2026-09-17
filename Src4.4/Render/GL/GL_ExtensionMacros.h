/**********************************************************************

Filename    :   GL_ExtensionMacros.h
Content     :   GL  extension macros header.
Created     :   Automatically generated.
Authors     :   Extensions.pl

Copyright   :   (c) 2001-2014 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_GL_ExtensionMacros_H
#define INC_SF_Render_GL_ExtensionMacros_H

#define glActiveTexture GetHAL()->GetGraphicsDevice()->glActiveTexture
#define glAttachShader GetHAL()->GetGraphicsDevice()->glAttachShader
#define glBindAttribLocation GetHAL()->GetGraphicsDevice()->glBindAttribLocation
#define glBindBuffer GetHAL()->GetGraphicsDevice()->glBindBuffer
#define glBindFragDataLocation GetHAL()->GetGraphicsDevice()->glBindFragDataLocation
#define glBindFramebuffer GetHAL()->GetGraphicsDevice()->glBindFramebuffer
#define glBindProgramPipeline GetHAL()->GetGraphicsDevice()->glBindProgramPipeline
#define glBindRenderbuffer GetHAL()->GetGraphicsDevice()->glBindRenderbuffer
#define glBindTexture GetHAL()->GetGraphicsDevice()->glBindTexture
#define glBindVertexArray GetHAL()->GetGraphicsDevice()->glBindVertexArray
#define glBlendEquation GetHAL()->GetGraphicsDevice()->glBlendEquation
#define glBlendEquationSeparate GetHAL()->GetGraphicsDevice()->glBlendEquationSeparate
#define glBlendFunc GetHAL()->GetGraphicsDevice()->glBlendFunc
#define glBlendFuncSeparate GetHAL()->GetGraphicsDevice()->glBlendFuncSeparate
#define glBufferData GetHAL()->GetGraphicsDevice()->glBufferData
#define glBufferSubData GetHAL()->GetGraphicsDevice()->glBufferSubData
#define glCheckFramebufferStatus GetHAL()->GetGraphicsDevice()->glCheckFramebufferStatus
#define glClear GetHAL()->GetGraphicsDevice()->glClear
#define glClearColor GetHAL()->GetGraphicsDevice()->glClearColor
#define glClientWaitSync GetHAL()->GetGraphicsDevice()->glClientWaitSync
#define glColorMask GetHAL()->GetGraphicsDevice()->glColorMask
#define glCompileShader GetHAL()->GetGraphicsDevice()->glCompileShader
#define glCompressedTexImage2D GetHAL()->GetGraphicsDevice()->glCompressedTexImage2D
#define glCreateProgram GetHAL()->GetGraphicsDevice()->glCreateProgram
#define glCreateShader GetHAL()->GetGraphicsDevice()->glCreateShader
#define glDebugMessageCallback GetHAL()->GetGraphicsDevice()->glDebugMessageCallback
#define glDebugMessageControl GetHAL()->GetGraphicsDevice()->glDebugMessageControl
#define glDeleteBuffers GetHAL()->GetGraphicsDevice()->glDeleteBuffers
#define glDeleteFramebuffers GetHAL()->GetGraphicsDevice()->glDeleteFramebuffers
#define glDeleteProgram GetHAL()->GetGraphicsDevice()->glDeleteProgram
#define glDeleteProgramPipelines GetHAL()->GetGraphicsDevice()->glDeleteProgramPipelines
#define glDeleteRenderbuffers GetHAL()->GetGraphicsDevice()->glDeleteRenderbuffers
#define glDeleteShader GetHAL()->GetGraphicsDevice()->glDeleteShader
#define glDeleteSync GetHAL()->GetGraphicsDevice()->glDeleteSync
#define glDeleteTextures GetHAL()->GetGraphicsDevice()->glDeleteTextures
#define glDeleteVertexArrays GetHAL()->GetGraphicsDevice()->glDeleteVertexArrays
#define glDepthFunc GetHAL()->GetGraphicsDevice()->glDepthFunc
#define glDepthMask GetHAL()->GetGraphicsDevice()->glDepthMask
#define glDisable GetHAL()->GetGraphicsDevice()->glDisable
#define glDisableVertexAttribArray GetHAL()->GetGraphicsDevice()->glDisableVertexAttribArray
#define glDrawArrays GetHAL()->GetGraphicsDevice()->glDrawArrays
#define glDrawElements GetHAL()->GetGraphicsDevice()->glDrawElements
#define glDrawElementsInstanced GetHAL()->GetGraphicsDevice()->glDrawElementsInstanced
#define glEnable GetHAL()->GetGraphicsDevice()->glEnable
#define glEnableVertexAttribArray GetHAL()->GetGraphicsDevice()->glEnableVertexAttribArray
#define glFenceSync GetHAL()->GetGraphicsDevice()->glFenceSync
#define glFlush GetHAL()->GetGraphicsDevice()->glFlush
#define glFlushMappedBufferRange GetHAL()->GetGraphicsDevice()->glFlushMappedBufferRange
#define glFramebufferRenderbuffer GetHAL()->GetGraphicsDevice()->glFramebufferRenderbuffer
#define glFramebufferTexture2D GetHAL()->GetGraphicsDevice()->glFramebufferTexture2D
#define glGenBuffers GetHAL()->GetGraphicsDevice()->glGenBuffers
#define glGenFramebuffers GetHAL()->GetGraphicsDevice()->glGenFramebuffers
#define glGenProgramPipelines GetHAL()->GetGraphicsDevice()->glGenProgramPipelines
#define glGenRenderbuffers GetHAL()->GetGraphicsDevice()->glGenRenderbuffers
#define glGenTextures GetHAL()->GetGraphicsDevice()->glGenTextures
#define glGenVertexArrays GetHAL()->GetGraphicsDevice()->glGenVertexArrays
#define glGenerateMipmap GetHAL()->GetGraphicsDevice()->glGenerateMipmap
#define glGetActiveUniform GetHAL()->GetGraphicsDevice()->glGetActiveUniform
#define glGetError GetHAL()->GetGraphicsDevice()->glGetError
#define glGetFloatv GetHAL()->GetGraphicsDevice()->glGetFloatv
#define glGetFragDataLocation GetHAL()->GetGraphicsDevice()->glGetFragDataLocation
#define glGetFramebufferAttachmentParameteriv GetHAL()->GetGraphicsDevice()->glGetFramebufferAttachmentParameteriv
#define glGetIntegerv GetHAL()->GetGraphicsDevice()->glGetIntegerv
#define glGetProgramBinary GetHAL()->GetGraphicsDevice()->glGetProgramBinary
#define glGetProgramInfoLog GetHAL()->GetGraphicsDevice()->glGetProgramInfoLog
#define glGetProgramiv GetHAL()->GetGraphicsDevice()->glGetProgramiv
#define glGetRenderbufferParameteriv GetHAL()->GetGraphicsDevice()->glGetRenderbufferParameteriv
#define glGetShaderInfoLog GetHAL()->GetGraphicsDevice()->glGetShaderInfoLog
#define glGetShaderiv GetHAL()->GetGraphicsDevice()->glGetShaderiv
#define glGetString GetHAL()->GetGraphicsDevice()->glGetString
#define glGetStringi GetHAL()->GetGraphicsDevice()->glGetStringi
#define glGetSynciv GetHAL()->GetGraphicsDevice()->glGetSynciv
#define glGetTexImage GetHAL()->GetGraphicsDevice()->glGetTexImage
#define glGetTexLevelParameteriv GetHAL()->GetGraphicsDevice()->glGetTexLevelParameteriv
#define glGetTexParameteriv GetHAL()->GetGraphicsDevice()->glGetTexParameteriv
#define glGetUniformLocation GetHAL()->GetGraphicsDevice()->glGetUniformLocation
#define glIsFramebuffer GetHAL()->GetGraphicsDevice()->glIsFramebuffer
#define glIsProgram GetHAL()->GetGraphicsDevice()->glIsProgram
#define glIsRenderbuffer GetHAL()->GetGraphicsDevice()->glIsRenderbuffer
#define glLinkProgram GetHAL()->GetGraphicsDevice()->glLinkProgram
#define glMapBuffer GetHAL()->GetGraphicsDevice()->glMapBuffer
#define glMapBufferRange GetHAL()->GetGraphicsDevice()->glMapBufferRange
#define glPixelStorei GetHAL()->GetGraphicsDevice()->glPixelStorei
#define glPolygonMode GetHAL()->GetGraphicsDevice()->glPolygonMode
#define glPopGroupMarker GetHAL()->GetGraphicsDevice()->glPopGroupMarker
#define glProgramBinary GetHAL()->GetGraphicsDevice()->glProgramBinary
#define glProgramParameteri GetHAL()->GetGraphicsDevice()->glProgramParameteri
#define glProgramUniform1fv GetHAL()->GetGraphicsDevice()->glProgramUniform1fv
#define glProgramUniform1iv GetHAL()->GetGraphicsDevice()->glProgramUniform1iv
#define glProgramUniform2fv GetHAL()->GetGraphicsDevice()->glProgramUniform2fv
#define glProgramUniform3fv GetHAL()->GetGraphicsDevice()->glProgramUniform3fv
#define glProgramUniform4fv GetHAL()->GetGraphicsDevice()->glProgramUniform4fv
#define glProgramUniformMatrix4fv GetHAL()->GetGraphicsDevice()->glProgramUniformMatrix4fv
#define glPushGroupMarker GetHAL()->GetGraphicsDevice()->glPushGroupMarker
#define glReadPixels GetHAL()->GetGraphicsDevice()->glReadPixels
#define glReleaseShaderCompiler GetHAL()->GetGraphicsDevice()->glReleaseShaderCompiler
#define glRenderbufferStorage GetHAL()->GetGraphicsDevice()->glRenderbufferStorage
#define glScissor GetHAL()->GetGraphicsDevice()->glScissor
#define glShaderSource GetHAL()->GetGraphicsDevice()->glShaderSource
#define glStencilFunc GetHAL()->GetGraphicsDevice()->glStencilFunc
#define glStencilMask GetHAL()->GetGraphicsDevice()->glStencilMask
#define glStencilOp GetHAL()->GetGraphicsDevice()->glStencilOp
#define glStringMarker GetHAL()->GetGraphicsDevice()->glStringMarker
#define glTexImage2D GetHAL()->GetGraphicsDevice()->glTexImage2D
#define glTexParameteri GetHAL()->GetGraphicsDevice()->glTexParameteri
#define glTexSubImage2D GetHAL()->GetGraphicsDevice()->glTexSubImage2D
#define glUniform1f GetHAL()->GetGraphicsDevice()->glUniform1f
#define glUniform1fv GetHAL()->GetGraphicsDevice()->glUniform1fv
#define glUniform1i GetHAL()->GetGraphicsDevice()->glUniform1i
#define glUniform1iv GetHAL()->GetGraphicsDevice()->glUniform1iv
#define glUniform2f GetHAL()->GetGraphicsDevice()->glUniform2f
#define glUniform2fv GetHAL()->GetGraphicsDevice()->glUniform2fv
#define glUniform3fv GetHAL()->GetGraphicsDevice()->glUniform3fv
#define glUniform4fv GetHAL()->GetGraphicsDevice()->glUniform4fv
#define glUniformMatrix4fv GetHAL()->GetGraphicsDevice()->glUniformMatrix4fv
#define glUnmapBuffer GetHAL()->GetGraphicsDevice()->glUnmapBuffer
#define glUseProgram GetHAL()->GetGraphicsDevice()->glUseProgram
#define glUseProgramStages GetHAL()->GetGraphicsDevice()->glUseProgramStages
#define glVertexAttribPointer GetHAL()->GetGraphicsDevice()->glVertexAttribPointer
#define glViewport GetHAL()->GetGraphicsDevice()->glViewport
#endif
