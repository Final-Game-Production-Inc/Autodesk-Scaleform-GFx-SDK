/**************************************************************************

Filename    :   GL_Shader.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#include "Render/GL/GL_Common.h"

#include "Render/GL/GL_Shader.h"
#include "Render/GL/GL_HAL.h"
#include "Kernel/SF_Debug.h"

#if defined(SF_RENDER_GLES)
#include "Render/GL/GLES_ShaderDescs.h"
#else
#include "Render/GL/GL_ShaderDescs.h"
#endif

#include "Kernel/SF_MsgFormat.h"

#include "Kernel/SF_SysFile.h"

#include "Render/GL/GL_ExtensionMacros.h"

#if defined(GL_ARB_get_program_binary) || defined(GL_OES_get_program_binary)
    #define SF_GL_BINARY_SHADER
    #define SF_GL_BINARY_SHADER_DEBUG 0 // 0 == no optional binary shader output, 1 == most optional output, 2 == all optional output.
#endif

#if defined(SF_GL_BINARY_SHADER_DEBUG) && SF_GL_BINARY_SHADER_DEBUG
    #define SF_BINARYSHADER_DEBUG_MESSAGE(x, str)                  SF_DEBUG_MESSAGE(x, str)
    #define SF_BINARYSHADER_DEBUG_MESSAGE1(x, str, p1)             SF_DEBUG_MESSAGE1(x, str, p1)
    #define SF_BINARYSHADER_DEBUG_MESSAGE2(x, str, p1, p2)         SF_DEBUG_MESSAGE2(x, str, p1, p2)
    #define SF_BINARYSHADER_DEBUG_MESSAGE3(x, str, p1, p2, p3)     SF_DEBUG_MESSAGE3(x, str, p1, p2, p3)
    #define SF_BINARYSHADER_DEBUG_MESSAGE4(x, str, p1, p2, p3, p4) SF_DEBUG_MESSAGE4(x, str, p1, p2, p3, p4)
#else
    #define SF_BINARYSHADER_DEBUG_MESSAGE(...)
    #define SF_BINARYSHADER_DEBUG_MESSAGE1(...)
    #define SF_BINARYSHADER_DEBUG_MESSAGE2(...)
    #define SF_BINARYSHADER_DEBUG_MESSAGE3(...)
    #define SF_BINARYSHADER_DEBUG_MESSAGE4(...)
#endif

namespace Scaleform { namespace Render { namespace GL {

extern const char* ShaderUniformNames[Uniform::SU_Count];
unsigned ShaderInterface::MaxRowsPerInstance = 0;
static const unsigned MaxShaderCodeSize = 4096; // Assume all shaders have a buffer smaller than this.

// Replaces the array size of a shader variable with the new count. This method assumes that 
// psrcPtr is a buffer with at least MaxShaderCodeSize bytes, and arrayString is the name of
// an shader variable which is an array.
void overwriteArrayCount(char* psrcPtr, const char* arrayString, unsigned newCount)
{
    char tempBuffer[MaxShaderCodeSize];
    if ( !psrcPtr )
        return;

    char * matFind = strstr(psrcPtr, arrayString);
    if (!matFind)
        return;
    
    SF_DEBUG_ASSERT(newCount > 0, "Can't have an array of size zero.");
    char tempNumber[16];
    SFsprintf(tempNumber, 16, "[%d]", newCount);
    UPInt size = (matFind - psrcPtr) + SFstrlen(arrayString);
    SFstrncpy(tempBuffer, MaxShaderCodeSize, psrcPtr, size);
    tempBuffer[size] = 0;
    SFstrcat(tempBuffer, MaxShaderCodeSize, tempNumber);

    char* endPtr = SFstrchr(matFind, ']');
    SF_DEBUG_ASSERT1(endPtr != 0, "Expected shader variable to be an array %s, but closing bracket not found.", arrayString );
    if (!endPtr)
        return;
    SFstrcat(tempBuffer, MaxShaderCodeSize, endPtr+1);

    // Overwrite the original buffer with the modified code.
    SFstrcpy(psrcPtr, MaxShaderCodeSize, tempBuffer);
}

// *** ShaderObject

ShaderObject::ShaderObject() :
    pHal(0),
    pVDesc(0),
    pFDesc(0),
    ShaderVer(ShaderDesc::ShaderVersion_Default),
    ComboIndex(-1),
    Separated(false),
    Pipeline(0),
    IsLinked(false),
    IsValidated(false)
{
}

bool ShaderObject::Init(HAL* phal, ShaderDesc::ShaderVersion ver, unsigned comboIndex, bool separable, 
                        HashLH<unsigned, ShaderHashEntry>& shaderHash, bool testCompilation, bool linkProgram)
{
    pHal = phal;
    ShaderVer   = ver;
    ComboIndex  = comboIndex;
    Separated   = separable;
    IsLinked    = false;
    IsValidated = false;

    ShaderDesc::ShaderType shader = ShaderDesc::GetShaderTypeForComboIndex(ComboIndex, ver);
    pVDesc      = VertexShaderDesc::GetDesc(shader, ver);
    pFDesc      = FragShaderDesc::GetDesc(shader, ver);
    
    releasePrograms();

    if ( !pVDesc || !pFDesc )
    {
        SF_DEBUG_WARNING1(1, "Failed to find shader descriptor for shader type %d", shader);
        return false;
    }

    // Attempt to locate the shaders.
    char shaderModificationBuffer[MaxShaderCodeSize];
    ShaderHashEntry shaders[ShaderStage_Count];
    memset(shaders, 0, sizeof(shaders));

    unsigned maxUniforms = (phal->GetGraphicsDevice()->GetCaps() & Cap_MaxUniforms) >> Cap_MaxUniforms_Shift;
    for ( unsigned stage = ShaderStage_Vertex; stage < ShaderStage_Count; ++stage)
    {
        // Attempt to find the program for this stage (separated or not).
        unsigned hashCode = getShaderPipelineHashCode(true, (ShaderStages)stage);
        if (shaderHash.Get(hashCode, &shaders[stage]))
        {
            StagePrograms[stage] = shaders[stage].Program;
            continue;
        }

        if (!Separated)
        {
            // If we are not separated, we can also search for the shader (not the program), which may also be hashed.
            hashCode = getShaderPipelineHashCode(false, (ShaderStages)stage);
            if (shaderHash.Get(hashCode, &shaders[stage]))
                continue;
        }

        // We cannot find the program or shader, we must compile from source. Get the source code, so we
        // can compile. If there is no source code, it means that this stage is not supported by this shader.
        const char* shaderCode = getShaderPipelineCode((ShaderStages)stage, maxUniforms, shaderModificationBuffer);
        if (!shaderCode)
            continue;

        if (Separated)
            shaders[stage].Program = *createProgram((ShaderStages)stage, shaderCode);
        else
            shaders[stage].Shader  = *createShader((ShaderStages)stage, shaderCode);
    
        // If there was code, but either a program or shader was not created, something went wrong.
        if (!shaders[stage].Program && !shaders[stage].Shader)
            return false;

        shaderHash.Set(hashCode, shaders[stage]);
    }

    // Do linking if requested.
    return linkProgram ? link(shaderHash, testCompilation) : true;
}

bool ShaderObject::Init(HAL* phal, HALGLProgram* program, const VertexShaderDesc* vdesc, const FragShaderDesc* fdesc )
{
    pHal = phal;
    pVDesc = vdesc;
    pFDesc = fdesc;
    ShaderVer = ShaderDesc::ShaderVersion_Default;
    ComboIndex = -1;
    Separated = false;
    Pipeline = 0;;                           // A shader pipeline object (only used if Separated = true).
    for (unsigned stage = 0; stage < ShaderStage_Count; ++stage)
        StagePrograms[stage] = program;

    IsLinked = true;
    IsValidated = true;
    initUniforms();
    return true;
}

bool ShaderObject::initUniforms()
{
    for (unsigned i = 0; i < Uniform::SU_Count; i++)
    {
        if (pVDesc->Uniforms[i].Location >= 0)
            Uniforms[i].Program = StagePrograms[ShaderStage_Vertex];
        else if (pFDesc->Uniforms[i].Location >= 0)
            Uniforms[i].Program = StagePrograms[ShaderStage_Frag];
        else
        {
            Uniforms[i].Program = 0;
            continue;
        }

        glGetUniformLocation(Uniforms[i].Program, ShaderUniformNames[i], &Uniforms[i].Location);

        // NOTE: The check for array variables has been moved inside GL::GraphicsDevice::glGetUniformLocation. 
        // If using a recorded device, there is no way to determine immediately whether looking up a uniform fails.
    }
    return true;
}

bool ShaderObject::link(HashLH<unsigned, ShaderHashEntry>& shaderHash, bool testCompilation)
{
	ShaderHashEntry shaders[ShaderStage_Count];
	memset(shaders, 0, sizeof(shaders));

	for ( unsigned stage = ShaderStage_Vertex; stage < ShaderStage_Count; ++stage)
	{
		// Attempt to find the program for this stage (separated or not).
		unsigned hashCode = getShaderPipelineHashCode(true, (ShaderStages)stage);
		if (shaderHash.Get(hashCode, &shaders[stage]))
			continue;

		if (!Separated)
		{
			// If we are not separated, we can also search for the shader (not the program), which may also be hashed.
			hashCode = getShaderPipelineHashCode(false, (ShaderStages)stage);
			if (shaderHash.Get(hashCode, &shaders[stage]))
				continue;
		}
	}

	if (!createProgramOrPipeline(shaders, Separated, testCompilation) || 
        !initUniforms())
	{
		releasePrograms();
		return false;
	}

    IsLinked = true;
	return true;
}

ShaderObject::ValidationStatus ShaderObject::validate(HashLH<unsigned, ShaderHashEntry>& shaderHash, bool testCompilation)
{
	ShaderHashEntry shaders[ShaderStage_Count];
    bool hasStage[ShaderStage_Count];
    memset(shaders, 0, sizeof(shaders));
	memset(hasStage, 0, sizeof(hasStage));

    // Gather the shaders. If they do not yet have program and/or shader names, then, 
    // assume we are running in a deferred context and return false immediately.
	for ( unsigned stage = ShaderStage_Vertex; stage < ShaderStage_Count; ++stage)
	{
        unsigned hashCode = getShaderPipelineHashCode(Separated, (ShaderStages)stage);
        if (shaderHash.Get(hashCode, &shaders[stage]))
        {
            hasStage[stage] = true;
            if ((shaders[stage].Program && !*shaders[stage].Program) ||
                (shaders[stage].Shader  && !*shaders[stage].Shader))
            {
                return Validation_NotInitialized;
            }
        }
    }

    // Now validate the shaders/program.
    for ( unsigned stage = ShaderStage_Vertex; stage < ShaderStage_Count; ++stage)
    {
        if (!hasStage[stage])
            continue;
        if (!validateShaderOrProgram(shaders, (ShaderStages)stage, Separated, testCompilation))
			return Validation_Failed;
	}
	
    // Now validate the program/pipeline.
	if (!validateProgramOrPipeline(shaders, Separated, shaderHash))
	{
		releasePrograms();
		return Validation_Failed;
	}
	return Validation_Succeeded;
}

void ShaderObject::Shutdown( )
{
    releasePrograms();

    pVDesc      = 0;
    pFDesc      = 0;
    pHal        = 0;
    IsLinked    = false;
    IsValidated = false;
}

bool ShaderObject::IsInitialized() const
{
    return IsLinked;
}

void ShaderObject::ApplyShader() const
{
    if (Separated)
    {
        glBindProgramPipeline(Pipeline);
    }
    else
    {
        glUseProgram(StagePrograms[ShaderStage_Vertex]);
    }
}

const UniformVar* ShaderObject::GetUniformVariable(unsigned var) const
{
    if (pVDesc->Uniforms[var].Location >= 0)
        return &pVDesc->Uniforms[var];
    else if (pFDesc->Uniforms[var].Location >= 0)
        return &pFDesc->Uniforms[var];
    else
        return 0;
}

HALGLProgram* ShaderObject::GetUniformVariableProgram(unsigned var) const
{
    return Uniforms[var].Program;
}

ShaderObject::~ShaderObject()
{
    Shutdown();
}

HALGLShader* ShaderObject::createShader(ShaderStages stage, const char* shaderCode)
{
    GLenum type = getShaderTypeForStage(stage);
    HALGLShader* shader = SF_NEW HALGLShader();
    glCreateShader(type, shader);
    glShaderSource(shader, 1, const_cast<const char**>(&shaderCode), 0);
    glCompileShader(shader);
    return shader;
}

HALGLProgram* ShaderObject::createProgram(ShaderStages stage, const char* shaderCode)
{
    // Note: although it would be convenient, we cannot use glCreateShaderProgramv, because of the issues #15 and #16
    // http://www.opengl.org/registry/specs/ARB/separate_shader_objects.txt. We require shader attributes to be bound
    // to particular locations.
    Ptr<HALGLShader> shader = *SF_NEW HALGLShader();
    GLenum type = getShaderTypeForStage(stage);
    glCreateShader(type, shader);
    glShaderSource(shader, 1, &shaderCode, 0);
    glCompileShader(shader);
    HALGLProgram* program = SF_NEW HALGLProgram();
    glCreateProgram(program);

    // Bind the vertex attribute locations.
    if (stage == ShaderStage_Vertex)
    {
        for (int i = 0; i < pVDesc->NumAttribs; i++)
            glBindAttribLocation(program, i, pVDesc->Attributes[i].Name);
    }

    glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE);
#if defined(SF_RENDER_OPENGL) && defined(SF_GL_BINARY_SHADER)
    // In OpenGL, we must set the retrievable hint, otherwise, it won't generate a binary format we can save.
    if (pHal->GetGraphicsDevice()->GetCaps() & Cap_BinaryShaders)
        glProgramParameteri(program, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
#endif

    glAttachShader(program, shader);
    glLinkProgram(program);
    glDeleteShader(shader);
    return program;
}

bool ShaderObject::validateShaderOrProgram(ShaderHashEntry* shaders, ShaderStages stage, bool separable, bool testCompilation)
{
    SF_UNUSED(testCompilation);
    if (!separable)
    {
        GLint result = GL_FALSE;
        GLchar msg[512];
        Ptr<HALGLShader> shader = shaders[stage].Shader;

        glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
        if (!result)
        {
            glGetShaderInfoLog(shader, sizeof(msg), 0, msg);
            SF_DEBUG_ERROR1(!testCompilation, "%s:\n", msg);
            glDeleteShader(shader);
            return false;
        }
    }

    IsValidated = true;
    return true;
}


bool ShaderObject::createProgramOrPipeline( ShaderHashEntry* shaders, bool separable, bool testCompilation )
{
    if (!separable)
    {
        // If non-separated, and we already have a program, then skip creating the program, because it
        // means that it was already created (loaded from binary).
        if (StagePrograms[ShaderStage_Vertex] != 0)
            return true;

        StagePrograms[ShaderStage_Vertex] = *SF_NEW HALGLProgram();
        glCreateProgram(StagePrograms[ShaderStage_Vertex]);
        for (unsigned stage = ShaderStage_Vertex; stage < ShaderStage_Count; ++stage)
        {
            if (shaders[stage].Shader)
            {
                glAttachShader(StagePrograms[ShaderStage_Vertex], shaders[stage].Shader);

                // If the stage exists, copy the uber-program to that stage program.
                StagePrograms[stage] = StagePrograms[ShaderStage_Vertex];
            }
        }
    }
    else
    {
        if (!Pipeline)
            Pipeline = *SF_NEW HALGLProgramPipeline();
        glGenProgramPipelines(1, &Pipeline.GetRawRef());
        glBindProgramPipeline(Pipeline);

        for (unsigned stage = ShaderStage_Vertex; stage < ShaderStage_Count; ++stage)
        {
            if (shaders[stage].Program)
                glUseProgramStages(Pipeline, getShaderBitForStage((ShaderStages)stage), shaders[stage].Program);

            StagePrograms[stage] = shaders[stage].Program;
        }
    }

    if (!StagePrograms[ShaderStage_Vertex] || !StagePrograms[ShaderStage_Frag])
    {
        SF_DEBUG_MESSAGE1(!StagePrograms[ShaderStage_Vertex],   "Vertex stage required in shader (type = %d).", pVDesc->Type);
        SF_DEBUG_MESSAGE1(!StagePrograms[ShaderStage_Frag],     "Fragment stage required in shader (type = %d).", pFDesc->Type);
        return false;
    }

#if defined(SF_RENDER_OPENGL)
    // In GLSL 1.5, we need to explicitly bind the output variable to a color output.
    if (ShaderVer == ShaderDesc::ShaderVersion_GLSL150)
    {
        SF_DEBUG_ASSERT(pHal->CheckGLVersion(3,0) || pHal->CheckExtension(SF_GL_EXT_gpu_shader4), "Must have glBindFragDataLocation if using GLSL 1.5.");
        glBindFragDataLocation(StagePrograms[ShaderStage_Frag], 0, "fcolor");
    }
    // NOTE: in GLES3, this is not required, the shader should contain layouts. However, since they default to 0 by definition, we don't
    // need to specify them here, or within the shader itself.
#endif

    if (!separable)
    {
        for (int i = 0; i < pVDesc->NumAttribs; i++)
            glBindAttribLocation(StagePrograms[ShaderStage_Vertex], i, pVDesc->Attributes[i].Name);

#if !defined(SF_RENDER_GLES) && defined(SF_GL_BINARY_SHADER)
        // In OpenGL, we must set the retrievable hint, otherwise, it won't generate a binary format we can save.
        if (pHal->GetGraphicsDevice()->GetCaps() & Cap_BinaryShaders)
            glProgramParameteri(StagePrograms[ShaderStage_Vertex], GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
#endif

        glLinkProgram(StagePrograms[ShaderStage_Vertex]);

        // Note: it is valid for linking to fail in only one situation - Tegra GPUs do not support dynamic loops in GLSL ES 1.0,
        // and will fail when attempting to compile filter shaders. This should happen on startup, and using the immediate
        // device always. In this failure case, detect the linking error, and return so that further GL errors (asserts) do
        // not occur.
        if (testCompilation)
        {
            SF_DEBUG_ASSERT(!GetHAL()->GetGraphicsDevice()->IsDeferred(), "Cannot perform test compilation (ShaderManager::Initialize) in a deferred context.");
            GLint linkStatus;
            glGetProgramiv(StagePrograms[ShaderStage_Vertex], GL_LINK_STATUS, &linkStatus);
            if (linkStatus == GL_FALSE)
                return false;
        }
    }
    IsLinked = true;

    return true;
}

bool ShaderObject::validateProgramOrPipeline(ShaderHashEntry* shaders, bool separable, HashLH<unsigned, ShaderHashEntry>& shaderHash)
{
    if (!separable)
    {
        // The shaders will not actually be deleted until the program is destroyed.
        // We check the status of deletion, because some platforms (iOS) generate 
        // errors for deleting shaders multiple times.
        GLint status;
        for (unsigned stage = ShaderStage_Vertex; stage < ShaderStage_Count; ++stage)
        {
            if (!shaders[stage].Shader || glIsProgram(shaders[stage].Program))
                continue;
            glGetShaderiv(shaders[stage].Shader, GL_DELETE_STATUS, &status);
            if (status == GL_FALSE)
                glDeleteShader(shaders[stage].Shader);
        }

        // Check to see that the program linking succeeded.
        GLint result = GL_FALSE;
        glGetProgramiv(StagePrograms[ShaderStage_Vertex], GL_LINK_STATUS, &result);
        if (!result)
        {
            GLchar msg[512];
            glGetProgramInfoLog(StagePrograms[ShaderStage_Vertex], sizeof(msg), 0, msg);
            SF_DEBUG_ERROR1(1, "link: %s\n", msg);
            return false;
        }

        // Put the shader program in the hash, if it was not already there (as a result of binary shader loading).
        unsigned hashCode = getShaderPipelineHashCode(true, ShaderStage_Vertex);
        if (!shaderHash.Get(hashCode))
        {
            ShaderHashEntry entry;
            entry.Program = StagePrograms[ShaderStage_Vertex];

            // NOTE: store binary size as zero, so we will know to save this shader.
            shaderHash.Set(hashCode, entry);
        }
    }

#if defined(SF_RENDER_OPENGL)
    // In GLSL 1.5, we need to explicitly bind the output variable to a color output.
    // TODO: generate fragout name from ShaderMaker.
    if (ShaderVer == ShaderDesc::ShaderVersion_GLSL150)
    {
        SF_DEBUG_ASSERT(pHal->CheckGLVersion(3,0) || pHal->CheckExtension(SF_GL_EXT_gpu_shader4), "Must have glGetFragDataLocation if using GLSL 1.5.");
        SF_DEBUG_ASSERT(glGetFragDataLocation(StagePrograms[ShaderStage_Frag], "fcolor") != -1, "fcolor not bound to an output stage.");            
    }
#endif

    return true;
}

const char* ShaderObject::getShaderPipelineCode(ShaderStages stage, unsigned maxUniforms, char* modifiedShaderSource)
{
    SF_DEBUG_ASSERT(modifiedShaderSource, "Cannot pass in NULL for modifierShaderSource (needed, in case shader is modified)");
    switch(stage)
    {
        case ShaderStage_Vertex:
        {
            // By default, the batch shaders are compiled with a batch count of 30. However, depending
            // on the maximum number of uniforms supported, this may not be possible, and the shader source
            // will not compile. Thus, we need to modify the incoming source, so it can compile.
            const char * vdescpSource = (const char*)pVDesc->pSource;

            // If we are using separated shaders, they need to declare gl_Position semantic. We cannot put this
            // directly into ShaderMaker, because some Android platforms fail to compile if it is there.
#if !defined(SF_RENDER_GLES)
            if (Separated && ShaderVer == ShaderDesc::ShaderVersion_GLSL150)
            {
                UPInt originalLength = SFstrlen(vdescpSource);
				SF_UNUSED(originalLength);
                SF_DEBUG_ASSERT(originalLength < MaxShaderCodeSize, "Shader is too large.");

                // Put it after the last directive line.
                const char* startOfDirective = SFstrrchr(vdescpSource, '#'), *endOfDirective = 0;
                const char* insertLocation = 0;
                if (startOfDirective != 0)
                    endOfDirective = insertLocation = SFstrchr(startOfDirective, '\n') +1;
                else
                    endOfDirective = insertLocation = vdescpSource;

                SFstrncpy(modifiedShaderSource, MaxShaderCodeSize, vdescpSource, insertLocation-vdescpSource);
                modifiedShaderSource[insertLocation-vdescpSource] = 0;
                SFstrcat(modifiedShaderSource, MaxShaderCodeSize, "out gl_PerVertex\n{\n\tvec4 gl_Position;\n};\n");
                SFstrcat(modifiedShaderSource, MaxShaderCodeSize, endOfDirective);
                vdescpSource = modifiedShaderSource;
            }
#endif

            if ( pVDesc->Flags & Shader_Batch )
            {
                unsigned maxInstances = Alg::Min<unsigned>(SF_RENDER_MAX_BATCHES, 
                    maxUniforms / ShaderInterface::GetMaximumRowsPerInstance());

                if ( maxInstances < SF_RENDER_MAX_BATCHES)
                {
                    // Distribute the uniforms that we have available to the two batching arrays.
                    unsigned vecUniforms  = ShaderInterface::GetCountPerInstance(pVDesc, Uniform::SU_vfuniforms);
                    unsigned numInstances = maxUniforms / (vecUniforms);

                    // We still may have enough uniforms to do SF_RENDER_MAX_BATCHES, using dynamic batch sizing.
                    if (numInstances < SF_RENDER_MAX_BATCHES)
                    {
                        SF_DEBUG_WARNONCE3(1, "The default batch count is %d, up to %d uniforms are required to achieve this."
                            "System only supports %d uniforms, batch count will be reduced.\n",
                            SF_RENDER_MAX_BATCHES, SF_RENDER_MAX_BATCHES * ShaderInterface::GetMaximumRowsPerInstance(), maxUniforms);

                        SF_DEBUG_ASSERT(SFstrlen(vdescpSource) < MaxShaderCodeSize, "Shader is too large.");
                        SFstrcpy(modifiedShaderSource, MaxShaderCodeSize, vdescpSource);
                        overwriteArrayCount(modifiedShaderSource, "vfuniforms", vecUniforms * numInstances);
                        vdescpSource = modifiedShaderSource;
                    }
                }
            }
            return vdescpSource;
        }

        case ShaderStage_Frag:
            return pFDesc->pSource;

        default:
            return 0;
    }
}

unsigned ShaderObject::getShaderPipelineHashCode(bool program, ShaderStages stage)
{
    return getShaderPipelineHashCode(ComboIndex, ShaderVer, Separated, program, stage);
}

unsigned ShaderObject::getShaderPipelineHashCode(unsigned comboIndex, ShaderDesc::ShaderVersion ver, bool separated, 
                                                 bool program, ShaderStages stage)
{
    unsigned shaderIndex = 0;
    switch(stage)
    {
    case ShaderStage_Vertex:
    {
        if (!program || separated)
            shaderIndex = (unsigned)VertexShaderDesc::GetShaderIndexForComboIndex(comboIndex, ver);
        break;
    }
    case ShaderStage_Frag:
    {
        if (!program || separated)
            shaderIndex = (unsigned)FragShaderDesc::GetShaderIndexForComboIndex(comboIndex, ver);
        break;
    }
    default:
        return 0;
    }

    // If we are not using separted pipelines, store/retrieve all programs as vertex programs. This will
    // ensure that programs will not have duplicate entries in the shader hash.
    if (program && !separated)
    {
        shaderIndex = comboIndex;
        stage = ShaderStage_Vertex;
    }

    return (program ? 0x80000000 : 0x00000000) | ((stage& 0x7FFF) << 16) | (shaderIndex & 0xFFFF);
}

// Returns the shader type, given the shader stage.
GLenum ShaderObject::getShaderTypeForStage(ShaderStages stage)
{
    switch(stage)
    {
    case ShaderStage_Vertex:    return GL_VERTEX_SHADER;
    case ShaderStage_Frag:      return GL_FRAGMENT_SHADER;

    // These stages do not exist on GLES.
#if defined(GL_GEOMETRY_SHADER)
    case ShaderStage_Geometry:  return GL_GEOMETRY_SHADER;
#endif // GL_GEOMETRY_SHADER
            
#if defined(GL_TESS_CONTROL_SHADER) && defined(GL_TESS_EVALUATION_SHADER)
    case ShaderStage_Hull:      return GL_TESS_CONTROL_SHADER;      // TODOBM: verify Hull == TESS_CONTROL
    case ShaderStage_Domain:    return GL_TESS_EVALUATION_SHADER;
#endif // GL_TESS_CONTROL_SHADER && GL_TESS_EVALUATION_SHADER
            
#if defined(GL_COMPUTE_SHADER)
    case ShaderStage_Compute:   return GL_COMPUTE_SHADER; // Need to update glext.h
#endif // GL_COMPUTE_SHADER
            
    default:
        SF_DEBUG_ASSERT1(0, "Shader stage %d is unavailable.", stage);
        return 0;
    }
}

GLenum ShaderObject::getShaderBitForStage(ShaderStages stage)
{
    switch(stage)
    {
    case ShaderStage_Vertex:    return GL_VERTEX_SHADER_BIT;
    case ShaderStage_Frag:      return GL_FRAGMENT_SHADER_BIT;

        // These stages do not exist on GLES.
#if defined(GL_GEOMETRY_SHADER_BIT)
    case ShaderStage_Geometry:  return GL_GEOMETRY_SHADER_BIT;
#endif // GL_GEOMETRY_SHADER_BIT
            
#if defined(GL_TESS_CONTROL_SHADER_BIT) && defined(GL_TESS_EVALUATION_SHADER_BIT)
    case ShaderStage_Hull:      return GL_TESS_CONTROL_SHADER_BIT;      // TODOBM: verify Hull == TESS_CONTROL
    case ShaderStage_Domain:    return GL_TESS_EVALUATION_SHADER_BIT;
#endif // GL_TESS_CONTROL_SHADER_BIT && GL_TESS_EVALUATION_SHADER_BIT

#if defined(GL_COMPUTE_SHADER_BIT)
    case ShaderStage_Compute:   return GL_COMPUTE_SHADER_BIT; // Need to update glext.h
#endif // GL_COMPUTE_SHADER_BIT

    default:
        SF_DEBUG_ASSERT1(0, "Shader stage %d is unavailable.", stage);
        return 0;
    }
}

void ShaderObject::releasePrograms()
{
    // Pipelines are not contained in the ShaderManager's hash, and should only exist in
    // a single ShaderObject, so they should be deleted.
    if (Separated && Pipeline != 0)
    {
        glDeleteProgramPipelines(1, &Pipeline.GetRawRef());
        Pipeline = 0;
    }
    for (unsigned stage = 0; stage < ShaderStage_Count; ++stage)
        StagePrograms[stage].Clear();

}

void ShaderObject::dumpUniforms(unsigned shader)
{
#if defined(SF_GL_BINARY_SHADER_DEBUG) && SF_GL_BINARY_SHADER_DEBUG >= 2

    if (shader != 0)
    {
        GLint uniformCount;
            glGetProgramiv(shader, GL_ACTIVE_UNIFORMS, &uniformCount);
            SF_DEBUG_MESSAGE2(1, "Shader program %d has %d uniforms:", shader, uniformCount);
        for ( int uniform = 0; uniform < uniformCount; ++uniform)
        {
            char uniformName[128];
            GLsizei length;
            GLint size;
            GLenum type;
                glGetActiveUniform(shader, uniform, 128, &length, &size, &type, uniformName);
            SF_DEBUG_MESSAGE3(1,"\t%16s (size=%d, type=%d)", uniformName, size, type);
        }
    }
    else
    {
        if (Separated)
        {
            for (unsigned prog = 0; prog < ShaderStage_Count; ++prog)
            {
                if (StagePrograms[prog] == 0)
                    continue;
                dumpUniforms(StagePrograms[prog]);
            }
        }
        else
        {
            dumpUniforms(StagePrograms[ShaderStage_Vertex]);
        }
    }

#else
    SF_UNUSED(shader);
#endif
}

// *** ShaderInterface

ShaderInterface::ShaderInterface( Render::HAL* phal )
{
    pHal = reinterpret_cast<HAL*>(phal); 
    memset(TextureUniforms, -1, sizeof(TextureUniforms));
}

bool ShaderInterface::SetFilterShader(const Matrix2F& viewMatrix, const Filter* filter, ShaderDesc::ShaderType shaderType, unsigned pass, unsigned passCount, const VertexFormat* pvf)
{
    // If this is a dynamic loop shader, and we don't support dynamic loops, the shader
    // needs to be located and/or recompiled into the dynamic-turned-literal loop hash.
    if ( !pHal->SManager.GetDynamicLoopSupport() &&
        ((FragShaderDesc::GetDesc(shaderType, pHal->SManager.GLSLVersion)->Flags & Shader_DynamicLoop) ||
        (VertexShaderDesc::GetDesc(shaderType, pHal->SManager.GLSLVersion)->Flags & Shader_DynamicLoop)))

    {
        float fsize[4], texscale[4];
        memset(fsize, 0, sizeof(fsize));
        memset(texscale, 0, sizeof(fsize));
        const BlurFilterImpl* blurFilter = reinterpret_cast<const BlurFilterImpl*>(filter);
        pHal->SManager.GenerateBlurFilterParameters(blurFilter->GetParams(), shaderType, pass, fsize, viewMatrix, texscale, passCount);

        ShaderManager::DynamicLoopKey key = { shaderType, fsize[0], fsize[1] };
        CurShader.pShaderObj = pHal->SManager.getDynamicLoopShaderWithLiteralReplaced(key);
        SF_DEBUG_WARNING(!CurShader.pShaderObj, "Error generating loop filter shader with literals.\n");
        if (!CurShader->pShaderObj)
            return false;
        CurShader.pVDesc = CurShader.pShaderObj->pVDesc;
        CurShader.pFDesc = CurShader.pShaderObj->pFDesc;
        CurShader.pShaderObj->ApplyShader();
        return true;
    }
    else
    {
        return SetStaticShader(shaderType, pvf);
    }
}

bool ShaderInterface::SetStaticShader(ShaderDesc::ShaderType shader, const VertexFormat*)
{
    ShaderObject* pnewShader = pHal->GetStaticShader(shader);

    // Redundancy checking (don't set the same shader twice in a row).
    if (CurShader.pShaderObj == pnewShader)
        return true;

    CurShader.pShaderObj = pnewShader;
    if ( !CurShader.pShaderObj || !CurShader.pShaderObj->IsInitialized() )
    {
        CurShader.pVDesc = 0;
        CurShader.pFDesc = 0;
        SF_DEBUG_ASSERT1(0, "Shader does not exist, or was not initialized (type=%d)", shader);
        return false;
    }
    CurShader.pVDesc = CurShader.pShaderObj->pVDesc;
    CurShader.pFDesc = CurShader.pShaderObj->pFDesc;
    CurShader.pShaderObj->ApplyShader();
    
    return true;
}

void ShaderInterface::SetTexture(const Shader& sd, unsigned var, Render::Texture* ptex, ImageFillMode fm, unsigned index)
{
    GL::Texture* ptexture = (GL::Texture*)ptex;

    int *textureStages = 0;
    int *stageCount = 0;
    int baseLocation = sd->pFDesc->Uniforms[var].Location;

    // Find our texture uniform index.
    int tu;
    for ( tu = 0; tu < FragShaderDesc::MaxTextureSamplers; ++tu)
    {
        if ( TextureUniforms[tu].UniformVar < 0 || TextureUniforms[tu].UniformVar == (int)var )
        {
            TextureUniforms[tu].UniformVar = var;
            textureStages = TextureUniforms[tu].SamplerStages;
            stageCount = &TextureUniforms[tu].StagesUsed;
            break;
        }
    }
    SF_DEBUG_ASSERT(tu < FragShaderDesc::MaxTextureSamplers, "Unexpected number of texture uniforms used.");

    for (unsigned plane = 0; plane < ptexture->GetTextureStageCount() ; plane++)
    {
        int stageIndex = baseLocation + index + plane;
        textureStages[plane + index] = stageIndex;
        *stageCount = Alg::Max<int>(*stageCount, index+plane+1);
    }

    // Texture::ApplyTexture applies each stage internally.
    ptexture->ApplyTexture(baseLocation + index, fm);
}

void ShaderInterface::Finish(unsigned batchCount)
{
    ShaderInterfaceBase<Uniform,ShaderPair>::Finish(batchCount);

    SF_DEBUG_ASSERT(CurShader.pShaderObj->IsInitialized(), "Shader trying to update uniforms, but is uninitialized.");

    ShaderObject* pCurShader = CurShader.pShaderObj;
    for (int var = 0; var < Uniform::SU_Count; var++)
    {
        if (UniformSet[var])
        {
            const UniformVar* uniformPtr = pCurShader->GetUniformVariable(var);
            if (!uniformPtr)
                continue;
            const UniformVar& uniformDef = *uniformPtr;

            unsigned size;
            if (uniformDef.BatchSize > 0)
                size = batchCount * uniformDef.BatchSize;
            else if (uniformDef.ElementSize)
                size = uniformDef.Size / uniformDef.ElementSize;
            else
                continue;

            if (!pCurShader->Separated)
            {
                switch (uniformDef.ElementSize)
                {
                case 16:
                        glUniformMatrix4fv(&pCurShader->Uniforms[var].Location, size, false /* transpose */,
                            UniformData + uniformDef.ShadowOffset);
                    break;
                case 4:
                        glUniform4fv(&pCurShader->Uniforms[var].Location, size,
                            UniformData + uniformDef.ShadowOffset);
                    break;
                case 3:
                        glUniform3fv(&pCurShader->Uniforms[var].Location, size,
                            UniformData + uniformDef.ShadowOffset);
                    break;
                case 2:
                        glUniform2fv(&pCurShader->Uniforms[var].Location, size,
                            UniformData + uniformDef.ShadowOffset);
                    break;
                case 1:
                        glUniform1fv(&pCurShader->Uniforms[var].Location, size,
                            UniformData + uniformDef.ShadowOffset);
                    break;

                default:
                        SF_DEBUG_ASSERT2(0, "Uniform %d has unhandled element size %d.", var, uniformDef.ElementSize);
                }

                // Set sampler stage uniforms.
                for (int tu = 0; tu < FragShaderDesc::MaxTextureSamplers; ++tu)
                {
                    if ( TextureUniforms[tu].UniformVar < 0 )
                        break;

                    glUniform1iv( &pCurShader->Uniforms[TextureUniforms[tu].UniformVar].Location, 
                        TextureUniforms[tu].StagesUsed, TextureUniforms[tu].SamplerStages );
                }
            }
            else
            {
                Ptr<HALGLProgram> program = pCurShader->GetUniformVariableProgram(var);
                SF_UNUSED(program); // if GL_EXT_separate_shaderObjects is not available.

                switch (uniformDef.ElementSize)
                {
                case 16:
                    glProgramUniformMatrix4fv(program, &pCurShader->Uniforms[var].Location, size, false /* transpose */,
                        UniformData + uniformDef.ShadowOffset);
                    break;
                case 4:
                    glProgramUniform4fv(program, &pCurShader->Uniforms[var].Location, size,
                        UniformData + uniformDef.ShadowOffset);
                    break;
                case 3:
                    glProgramUniform3fv(program, &pCurShader->Uniforms[var].Location, size,
                        UniformData + uniformDef.ShadowOffset);
                    break;
                case 2:
                    glProgramUniform2fv(program, &pCurShader->Uniforms[var].Location, size,
                        UniformData + uniformDef.ShadowOffset);
                    break;
                case 1:
                    glProgramUniform1fv(program, &pCurShader->Uniforms[var].Location, size,
                        UniformData + uniformDef.ShadowOffset);
                    break;

                default:
                    SF_DEBUG_ASSERT2(0, "Uniform %d has unhandled element size %d.", var, uniformDef.ElementSize);
                }

                // Set sampler stage uniforms.
                for (int tu = 0; tu < FragShaderDesc::MaxTextureSamplers; ++tu)
                {
                    if ( TextureUniforms[tu].UniformVar < 0 )
                        break;

                    Ptr<HALGLProgram> program = pCurShader->GetUniformVariableProgram(TextureUniforms[tu].UniformVar);
                    SF_UNUSED(program); // if GL_EXT_separate_shaderObjects is not available.
                    glProgramUniform1iv( program, &pCurShader->Uniforms[TextureUniforms[tu].UniformVar].Location, 
                        TextureUniforms[tu].StagesUsed, TextureUniforms[tu].SamplerStages );
                }
            }
        }
    }

    memset(UniformSet, 0, Uniform::SU_Count);
    memset(TextureUniforms, -1, sizeof(TextureUniforms));
}

unsigned ShaderInterface::GetMaximumRowsPerInstance()
{
    // Check for cached value. This should not change between runs. TBD: precalculate.
    if ( MaxRowsPerInstance == 0 )
    {
        // Note: this assumes that batch variables are stored in shader descs.
        MaxRowsPerInstance = 0;
        for ( unsigned desc = 0; desc < VertexShaderDesc::VSI_Count; ++desc )
        {
            const VertexShaderDesc* pvdesc = VertexShaderDesc::Descs[desc];
            MaxRowsPerInstance = Alg::Max(MaxRowsPerInstance, GetRowsPerInstance(pvdesc));
        }
    }
    return MaxRowsPerInstance;
}

unsigned ShaderInterface::GetRowsPerInstance( const VertexShaderDesc* pvdesc )
{
    // Desc doesn't exist, or isn't batched, don't consider it.
    if ( !pvdesc || (pvdesc->Flags & Shader_Batch) == 0 )
        return 0;

    unsigned currentUniforms = 0;
    for ( unsigned uniform = 0; uniform < Uniform::SU_Count; ++uniform )
    {
        if ( pvdesc->BatchUniforms[uniform].Size > 0 )
            currentUniforms += pvdesc->BatchUniforms[uniform].Size;
    }
    return currentUniforms;
}

// Returns the number of entries per instance of the given uniform type.
unsigned ShaderInterface::GetCountPerInstance(const VertexShaderDesc* pvdesc, Uniform::UniformType arrayType)
{
    // Desc doesn't exist, or isn't batched, don't consider it.
    if ( !pvdesc || (pvdesc->Flags & Shader_Batch) == 0 )
        return 0;

    unsigned currentUniforms = 0;
    for ( unsigned uniform = 0; uniform < Uniform::SU_Count; ++uniform )
    {
        if ( pvdesc->BatchUniforms[uniform].Size > 0 )
        {
            if ( pvdesc->BatchUniforms[uniform].Array == arrayType )
                currentUniforms += pvdesc->BatchUniforms[uniform].Size;
        }
    }
    return currentUniforms;
}

ShaderManager::ShaderManager(ProfileViews* prof) :
    StaticShaderManagerType(prof), 
    pHal(0),
    GLSLVersion(ShaderDesc::ShaderVersion_Default),
    DynamicLoops(-1),
    ShouldUseBinaryShaders(false),
    SingleBinaryShaderFile(true),
    SeparablePipelines(false),
    SeparablePipelineExtension(false),
    NextValidationEntry(0)
{
    memset(ValidationQueue, -1, sizeof(ValidationQueue));
}

// If an incompatibility between the binary shader file format is introduced, change the
// header string, so that it will not match older versions. All files are checked with shader
// generation timestamps, so modifying the shaders themsevles will be automatically detected.
// 'GFxShaders'   = initial version supporting binary shaders.
// 'GFxShadersV2' = supports loading separated binary shaders (previous version did not).
#if defined(SF_GL_BINARY_SHADER)
static const char* ShaderHeaderString = "GFxShadersV2";
static const unsigned ShaderHeaderSize = 12;
#endif
    
bool ShaderManager::Initialize(HAL* phal, unsigned vmcFlags)
{
    pHal = phal;

    unsigned & caps = phal->GetGraphicsDevice()->Caps;

    // On GL (Mac/PC), if the driver is GL 3.2+, it does not support GLSL 1.1/1.2.
    // However, if the driver is GL3.1-, it might not support GLSL 1.5, so we need to
    // support both.
#if defined(SF_RENDER_OPENGL)
    const GLubyte* glVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
   	if (glVersion == 0 || glGetError() != 0)
    {
        if (!ShaderDesc::IsShaderVersionSupported(GLSLVersion))
        {
            SF_DEBUG_MESSAGE(1, "GL_VERSION return NULL, or produced error, but GLSL v1.10 support is not present. Failing.");
            return false;
        }
        else
        {
            SF_DEBUG_WARNING(1, "GL_VERSION returned NULL, or produced error. Assuming GLSL v1.10.\n");
            return true;
        }
    }
    
    // Parse the version string.
    unsigned majorVersion, minorVersion;
    SFsscanf(reinterpret_cast<const char*>(glVersion), "%d.%d", &majorVersion, &minorVersion);
    
    // Per the spec, minorVersion should always be two digits.
    if (majorVersion > 1 || minorVersion >= 50)
    {
        GLSLVersion = ShaderDesc::ShaderVersion_GLSL150;
        if (!ShaderDesc::IsShaderVersionSupported(GLSLVersion))
        {
            SF_DEBUG_WARNING1(1, "GLSL version reported %s, however GLSL v1.50 support is unavailable. Failing.", glVersion);
            return false;
        }
    }
    else if (majorVersion > 1 || minorVersion >= 20)
    {
        caps &= ~Cap_Instancing; // can't support instancing in GLSL 1.20
        GLSLVersion = ShaderDesc::ShaderVersion_GLSL120;
        if (!ShaderDesc::IsShaderVersionSupported(GLSLVersion))
        {
            SF_DEBUG_WARNING1(1, "GLSL version reported %s, however GLSL v1.20 support is unavailable. Failing.", glVersion);
            return false;
        }
    }
    else
    {
        caps &= ~Cap_Instancing; // can't support instancing in GLSL 1.10
        GLSLVersion = ShaderDesc::ShaderVersion_GLSL110;
        if (!ShaderDesc::IsShaderVersionSupported(GLSLVersion))
        {
            SF_DEBUG_WARNING1(1, "GLSL version reported %s, however GLSL v1.10 support is unavailable. Failing.", glVersion);
            return false;
        }
    }
    
#elif defined(SF_RENDER_GLES)
    if (pHal->CheckGLVersion(3,0) && ShaderDesc::IsShaderVersionSupported(ShaderDesc::ShaderVersion_GLES30))
    {
        GLSLVersion = ShaderDesc::ShaderVersion_GLES30;
    }
    else
    {
        // Assume version is GLES. GetDynamicLoopSupport will use this internally to check if a dynamic
        // loop shader will compile. If that fails, it will fallback to GLES100_NDL.
        GLSLVersion = ShaderDesc::ShaderVersion_GLES;
        if (!pHal->CheckGLVersion(3,0) && GetDynamicLoopSupport() && ShaderDesc::IsShaderVersionSupported(ShaderDesc::ShaderVersion_GLES))
        {
            GLSLVersion = ShaderDesc::ShaderVersion_GLES;
        }
        else if (!pHal->CheckGLVersion(3,0) && ShaderDesc::IsShaderVersionSupported(ShaderDesc::ShaderVersion_GLES100_NDL))
        {
            GLSLVersion = ShaderDesc::ShaderVersion_GLES100_NDL;
        }
        else
        {
            SF_DEBUG_WARNING(1, "Neither GLSL 3.00 or GLSL 1.00 were compatible, either the device does not support them, or support was not compiled in.");
            return false;
        }
    }

    // Disable features in GLES 1.00, if the extensions are not available. This may be required if using GLES 3.00, but GLSL 1.00.
    if (GLSLVersion == ShaderDesc::ShaderVersion_GLES)
    {
        if (!pHal->CheckExtension(SF_GL_EXT_draw_instanced))
            caps &= ~Cap_Instancing;
        if (!pHal->CheckExtension(SF_GL_OES_standard_derivatives))
            caps |= Cap_NoDerivatives;
    }

#endif // SF_RENDER_GLES

    // NOTE: on android, some chipsets report that they have the separate_shader_objects extension, however, we link the
    // GL runtime statically, and the library does not contain the function, and gl2ext.h does not have the define. In this
    // case, do not allow SeparablePipelines.
#if defined(SF_OS_ANDROID) && !defined(SF_GL_RUNTIME_LINK)
    SeparablePipelines          = false;
    SeparablePipelineExtension  = false;
#else
    SeparablePipelineExtension  = pHal->CheckExtension(SF_GL_ARB_separate_shader_objects);
    SeparablePipelines          = ((vmcFlags & HALConfig_DisableShaderPipelines) == 0) && SeparablePipelineExtension;
#endif

    ShouldUseBinaryShaders = (vmcFlags & HALConfig_DisableBinaryShaders) == 0 && (caps & Cap_BinaryShaders);
    SingleBinaryShaderFile = (vmcFlags & HALConfig_MultipleShaderCacheFiles) == 0;

    SF_BINARYSHADER_DEBUG_MESSAGE1(1, "Using binary shaders: %s", ShouldUseBinaryShaders ? "true" : "false");

    // Disable separate pipelines if binary shaders are in use. It appears that these two extensions do not always 
    // interact with each other well. Certain drivers will not save separable programs that can be used as separable
    // programs when reloaded. TODO: investigate and re-enable if possible.
    if (ShouldUseBinaryShaders)
        SeparablePipelines = false;

    // Attempt to load binary shaders. If successful (all loaded), just finish now.
    if (ShouldUseBinaryShaders)
        loadBinaryShaders();

    // Check if platform supports dynamic looping. If it doesn't, blur-type filter shaders are dynamically generated.
    // This must come after the binary shader loading, because we may load the shader that has the dynamic loops.
    if ( !GetDynamicLoopSupport() )
        caps |= Cap_NoDynamicLoops;

    if ( (vmcFlags & HALConfig_DynamicShaderCompile) == 0)
    {
        bool multipass = (vmcFlags & HALConfig_DisableMultipassShaderCompile) != 0;
        bool shaderIsCompiled[UniqueShaderCombinations];
        memset(shaderIsCompiled, 0, sizeof(shaderIsCompiled));

        for (unsigned i = 0; i < UniqueShaderCombinations; i++)
        {
            // If the InitBinary succeeded, skip recompilation.
            if ( StaticShaders[i].IsInitialized() )
                continue;

            ShaderType shaderType = ShaderDesc::GetShaderTypeForComboIndex(i, GLSLVersion);
            if (shaderType == ShaderDesc::ST_None)
                continue;
            
            const FragShaderDesc* fdesc = FragShaderDesc::GetDesc(shaderType, GLSLVersion);
            const VertexShaderDesc* vdesc = VertexShaderDesc::GetDesc(shaderType, GLSLVersion);

            if (!fdesc || !vdesc )
                continue;

            // If the platform does not support dynamic loops, do not initialize shaders that use them.
            if ((fdesc->Flags & Shader_DynamicLoop) && (caps & Cap_NoDynamicLoops))
                continue;

#if !defined(GFX_ENABLE_VIDEO)
            // If video is not enabled, reject any video shaders.
            if ((vdesc->Flags & Shader_Video) || (fdesc->Flags & Shader_Video))
                continue;
#endif

            // If the platform doesn't have derivatives, do no initialize shaders that use them.
            if ((caps & Cap_NoDerivatives) != 0 && 
                ((vdesc->Flags & Shader_Derivatives) || (fdesc->Flags & Shader_Derivatives)))
            {
                continue;
            }

            // If the platform doesn't support instancing, do not initialize shaders that use it.
            if ( ((fdesc->Flags & Shader_Instanced) || (vdesc->Flags & Shader_Instanced)) && !HasInstancingSupport() )
                continue;

            if ( !StaticShaders[i].Init(phal, GLSLVersion, i, SeparablePipelines, CompiledShaderHash, false, !multipass))
                return false;

            shaderIsCompiled[i] = true;
            if (!multipass)
                addShaderToValidationQueue(i);
        }

        // Link and validate.
        if (multipass)
        {
            for (unsigned i = 0; i < UniqueShaderCombinations; ++i)
            {
                if (!shaderIsCompiled[i])
                    continue;
                if (!StaticShaders[i].link(CompiledShaderHash))
                    return false;
                addShaderToValidationQueue(i);
            }
        }

        // If we are precompiling all shaders (and it is presumably finished now), tell the shader compiler to
        // release its resources. 
#if defined(SF_RENDER_GLES)
        GLint hasShaderCompiler;
        glGetIntegerv(GL_SHADER_COMPILER, &hasShaderCompiler);
        if (hasShaderCompiler)
            glReleaseShaderCompiler();
#endif
    }

    // Now that all shaders have been compiled, save them to disk.
    if (ShouldUseBinaryShaders)
        saveBinaryShaders();

    return true;
}

unsigned ShaderManager::GetNumberOfUniforms() const
{
    unsigned maximumUniforms =(GetHAL()->GetCaps() & Cap_MaxUniforms) >> Cap_MaxUniforms_Shift;
    return maximumUniforms;
}

unsigned ShaderManager::SetupFilter(const Filter* filter, unsigned fillFlags, unsigned* passes) const
{
    return StaticShaderManagerType::GetFilterPasses(filter, fillFlags, passes);
}

bool ShaderManager::GetDynamicLoopSupport()
{
    // Check cached value. -1 indicates not calculated yet.
    if ( DynamicLoops < 0 )
    {
        // Just try to compile a shader we know has dynamic loops, and see if it fails.
        for ( int i = 0; i < FragShaderDesc::FSI_Count; ++i )
        {
            if ( FragShaderDesc::Descs[i] && (FragShaderDesc::Descs[i]->Flags & Shader_DynamicLoop) == Shader_DynamicLoop)
            {
                DynamicLoops = 0;
                
                unsigned comboIndex = FragShaderDesc::GetShaderComboIndex(FragShaderDesc::Descs[i]->Type, GLSLVersion);

                // Note: could already be initialized, due to binary shader loading.
                if (StaticShaders[comboIndex].IsInitialized() ||
                    StaticShaders[comboIndex].Init(pHal, GLSLVersion, comboIndex, SeparablePipelines, CompiledShaderHash, true ))
                {
                    DynamicLoops = 1;
                }
                break;
            }
        }
    }
    return DynamicLoops ? true : false;
}

bool ShaderManager::HasInstancingSupport() const
{
    // Caps generated on InitHAL.
    return (GetHAL()->GetCaps() & Cap_Instancing) != 0;
}

bool ShaderManager::UsingSeparateShaderObject() const
{
    return SeparablePipelines;
}

void ShaderManager::Reset(bool lost)
{
#if !defined(SF_RENDER_GLES)
    // Save binary shaders. In OpenGL, additional optimization may be done after a shader is actually used.
    // Thus, saving the binaries at this point, may yield additional benefits when reloading them. 
    if (ShouldUseBinaryShaders && !lost)
        saveBinaryShaders();
#endif

    if (!lost)
    {
        HashLH<unsigned, ShaderHashEntry>::ConstIterator it;
        for (it = CompiledShaderHash.Begin(); it != CompiledShaderHash.End(); ++it)
        {
            const ShaderHashEntry& e = it->Second;
            if (e.Program && glIsProgram(e.Program))
                glDeleteProgram(e.Program);
        }
    }
    CompiledShaderHash.Clear();

    // Clear the dynamically generated filter shaders as well.
    HashLH<DynamicLoopKey,ShaderObject*>::ConstIterator dit;
    for (dit = DynamicLoopShaderHash.Begin(); dit != DynamicLoopShaderHash.End(); ++dit)
    {
        ShaderObject* obj = dit->Second;
        if (!lost)
        {
            for (unsigned stage = 0; stage < ShaderStage_Count; ++stage)
            {
                if (obj->StagePrograms[ShaderStage_Vertex] && glIsProgram(obj->StagePrograms[ShaderStage_Vertex]))
                    glDeleteProgram(obj->StagePrograms[ShaderStage_Vertex]);
            }
        }
        delete obj;
    }
    DynamicLoopShaderHash.Clear();

    // Destroy the shader programs as well.
    for (unsigned i = 0; i < UniqueShaderCombinations; i++)
        StaticShaders[i].Shutdown();
}

void ShaderManager::BeginScene()
{
    // If we are using separated pipelines, make sure the current program is 0, otherwise it will
    // override any shader pipelines used with glBindProgramPipeline.
    if (SeparablePipelines)
        glUseProgram(0);
}

void ShaderManager::PerformShaderValidation()
{
    bool anyValidated = false;
    unsigned index = 0;
    while (ValidationQueue[index] >= 0)
    {
        ShaderObject::ValidationStatus status = StaticShaders[ValidationQueue[index]].validate(CompiledShaderHash, false);
        if (status != ShaderObject::Validation_NotInitialized)
        {
            anyValidated = true;
            ValidationQueue[index] = -1;
        }
        index++;
    }

    // Run through the list again, and consolidate the list of shaders which have not yet been initialized.
    unsigned index2 = 0, index3 = 0;
    while (index2 < index)
    {
        if (ValidationQueue[index2] >= 0)
            ValidationQueue[index3++] = ValidationQueue[index2];
        index2++;
    }
    NextValidationEntry = 0;

    // If we have a file-per-shader, save shaders after every validation.
    if (anyValidated &&
        pHal->GetConfigFlags() & HALConfig_MultipleShaderCacheFiles)
    {
        saveBinaryShaders();
    }
}

void ShaderManager::saveBinaryShaders()
{
#if defined(SF_GL_BINARY_SHADER)

    // If we support binary shaders, save them now.
    if (GetHAL()->GetCaps() & Cap_BinaryShaders)
    {
        SF_BINARYSHADER_DEBUG_MESSAGE(1,"Saving Binary Shaders...\n");

        // Before we do anything, run through all our shaders, and see if their binary sizes have changed. If not,
        // assume no further optimizations were done, and thus, do not actually re-save the file on shutdown.
        unsigned count = 0;
        bool needsResave = false;
        GLint maximumBinarySize = 0;
        HashLH<unsigned, ShaderHashEntry>::Iterator it = CompiledShaderHash.Begin();
        for (it = CompiledShaderHash.Begin(); it != CompiledShaderHash.End(); ++it)
        {
            Ptr<HALGLProgram> program  = it->Second.Program;
            if (!program || !glIsProgram(program))
                continue;
            count++;

            // Record the largest size, and see if it has changed.
            maximumBinarySize = Alg::Max(maximumBinarySize, it->Second.BinarySize);
            if (it->Second.BinarySize == 0)
            {
                GLint size;
                glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &size);
                maximumBinarySize = Alg::Max(maximumBinarySize, size);
                    SF_BINARYSHADER_DEBUG_MESSAGE3(1, "\tShader requires saving (hash=0x%08x, oldsize=%6d, newsize=%6d)\n", 
                        it->First, it->Second.BinarySize, size);
                needsResave = true;
            }
        }

        // If we don't need to resave, then just quit now.
        if (!needsResave)
            return;

        SF_DEBUG_ASSERT(maximumBinarySize > 0, "Maximum binary size reported as 0.");

        // Align the maximum binary size to 32k.
        maximumBinarySize = Alg::Align<32768>(maximumBinarySize);

        Ptr<File>  pfile = 0;
        GLsizei totalSize = 0;
        void* buffer = SF_ALLOC(maximumBinarySize, Stat_Default_Mem);
        
        if (SingleBinaryShaderFile)
        {
            String shpath = BinaryShaderPath + "GFxShaders.cache";
            pfile = *SF_NEW SysFile(shpath, File::Open_Write|File::Open_Create|File::Open_Truncate);
            if (!pfile->IsValid())
            {
                SF_DEBUG_WARNING2(1, "Error creating binary shader cache %s: %d", shpath.ToCStr(), pfile->GetErrorCode());
                SF_FREE(buffer);
                return;
            }

            // Now write the file header.
            pfile->Write((const UByte*) ShaderHeaderString, ShaderHeaderSize);
            pfile->WriteSInt64(SF_GFXSHADERMAKER_TIMESTAMP);
            pfile->WriteUInt32(count);
            pfile->WriteUInt32(SeparablePipelines ? 1 : 0);
            pfile->WriteUInt32(maximumBinarySize);
            totalSize += ShaderHeaderSize + sizeof(SInt64) + 3 * sizeof(UInt32);
        }

        // Iterate through the shader hash again, now saving all programs.
        for (it = CompiledShaderHash.Begin(); it != CompiledShaderHash.End(); ++it)
        {
            unsigned hashCode = it->First;
            Ptr<HALGLProgram> program  = it->Second.Program;
            if (!program || !glIsProgram(program))
                continue;

            GLsizei size;
            GLenum  format;
            glGetProgramBinary(program, maximumBinarySize, &size, &format, buffer);
            GLenum getBinaryError = glGetError();
            if (getBinaryError != 0)
            {
                SF_DEBUG_MESSAGE3(1, "glGetProgramBinary failure. Shaders may be corrupted, resetting all Shaders (error=%d, hash=%08x, program=%p).\n", 
                    getBinaryError, hashCode, program.GetPtr());
                Reset();
                pfile->Close();
                return;
            }

            if (!SingleBinaryShaderFile)
            {
                // Doesn't need saving.
                if (size == it->Second.BinarySize || size == 0)
                    continue;

                char shpath[1024];
                SFsprintf(shpath, 1024, "%sGFxShaders-%08x.cache", BinaryShaderPath.ToCStr(), hashCode);
                pfile = *SF_NEW SysFile(shpath, File::Open_Write|File::Open_Create|File::Open_Truncate);
                if (!pfile->IsValid())
                {
                    SF_DEBUG_WARNING2(1, "Error creating binary shader cache %s: %d", shpath, pfile->GetErrorCode());
                    continue;
                }

                // Write the header and shader timestamp (into every file).
                pfile->Write((const UByte*) ShaderHeaderString, ShaderHeaderSize);
                pfile->WriteSInt64(SF_GFXSHADERMAKER_TIMESTAMP);
                totalSize += ShaderHeaderSize + sizeof(SInt64);
            }

            pfile->WriteUInt32(hashCode);
            pfile->WriteUInt32(format);
            pfile->WriteUInt32(size);
            if (size > 0 && pfile->Write((UByte*)buffer, size) < size)
            {
                SF_DEBUG_MESSAGE(1, "Failed writing to binary shader file.");
                SF_FREE(buffer);
                return;
            }

            // Update the binary size of the shader, so we know it's been written.
            it->Second.BinarySize = size;

            SF_BINARYSHADER_DEBUG_MESSAGE3(1, "Wrote binary shader to file (hash=%08x, format=%8d, size=%8d)\n", hashCode, format, size);
            totalSize += size + 3 * sizeof(UInt32);

            if (!SingleBinaryShaderFile)
                pfile->Close();
        }

        SF_FREE(buffer);
        SF_BINARYSHADER_DEBUG_MESSAGE1(1, "Total bytes written to shader cache file(s): %d\n", totalSize);
        if (SingleBinaryShaderFile)
            pfile->Close();
    }
#endif // SF_GL_BINARY_SHADER
}

bool ShaderManager::loadBinaryShaders()
{
#if defined(SF_GL_BINARY_SHADER)

    if (GetHAL()->GetCaps() & Cap_BinaryShaders)
    {
        Ptr<File> pfile = 0;
        SInt32 count = 0;
        UInt32 maximumShaderSize = 0;

        if (SingleBinaryShaderFile)
        {
            String shpath = BinaryShaderPath + "GFxShaders.cache";
            pfile = *SF_NEW SysFile(shpath);
            if (!loadAndVerifyShaderCacheHeader(pfile))
                return false;

            // Read the rest of the parameters from the single-file version.
            count               = pfile->ReadUInt32();
            bool separate       = pfile->ReadUInt32() != 0;
            maximumShaderSize   = pfile->ReadUInt32();

            if (maximumShaderSize == 0)
            {
                SF_DEBUG_WARNING(1, "Binary shaders indicate the maximum shader size is 0 bytes. This is invalid. Using source shaders");
                return false;
            }

            // Detect if the file was saved with a different 'separated' pipeline state. If so, just ignore the binary
            // shaders. Likely, the HALConfig_DisableShaderPipelines has been modified for testing, so all shaders should
            // be recompiled.
            if (separate != SeparablePipelines)
            {
                SF_DEBUG_WARNING(!SeparablePipelines, "Binary shaders indicate that separate pipeline were "
                    "used when saved, but they are currently disabled. Ignoring binary shaders.");
                SF_DEBUG_WARNING(SeparablePipelines,  "Binary shaders indicate that separate pipeline were "
                    "not used when saved, but they are currently enabled. Ignoring binary shaders");
                return false;
            }
        }
        else
        {
            // If we are reading the shaders from disk in individual files, try to load all shaders.
            count = UniqueShaderCombinations;
            maximumShaderSize = 256 * 1024;     // should be large enough for even the largest shader.
        }

        void* buffer = SF_ALLOC(maximumShaderSize, Stat_Default_Mem);

        for (int i = 0; i < count; i++)
        {
            if (!SingleBinaryShaderFile)
            {
                char shpath[1024];
                // NOTE: always use ShaderStage_Vertex, binary shader means we are not using separable pipelines.
                unsigned hashCode = StaticShaders[i].getShaderPipelineHashCode(i, GLSLVersion, false, true, ShaderStage_Vertex);
                SFsprintf(shpath, 1024, "%sGFxShaders-%08x.cache", BinaryShaderPath.ToCStr(), hashCode);
                pfile = *SF_NEW SysFile(shpath);
                if (!loadAndVerifyShaderCacheHeader(pfile))
                    continue;
            }

            unsigned hashCode = pfile->ReadUInt32();
            GLenum format     = pfile->ReadUInt32();
            GLsizei size      = pfile->ReadUInt32();
                            
            SF_BINARYSHADER_DEBUG_MESSAGE3(1, "Loaded binary shader from file (hash=%08x, format=%8d, size=%8d)\n", hashCode, format, size);

            // Load the binary program, and put it in the hash.
            if (size > 0 )
            {
                if (pfile->Read((UByte*)buffer, size) < size)
                {
                    SF_DEBUG_WARNING(1, "Error reading from binary shader file (insufficient space remaining).");
                    SF_FREE(buffer);
                    return false;
                }

                // Create the program, and add it to the hash.
                ShaderHashEntry entry;
                entry.Program = *SF_NEW HALGLProgram();
                glCreateProgram(entry.Program);

                if (SeparablePipelineExtension)
                {
                    // NOTE: it is unclear in the spec whether it is possible to change the separable status of a binary program.
                    // Because we do not store the individual separable status of each program, we must query it afterwards, to see
                    // if it matches our current setup (whether we were able to modify it or not). If it does not match, fail loading 
                    // this binary.
                    if (SeparablePipelines)
                        glProgramParameteri(entry.Program, GL_PROGRAM_SEPARABLE, GL_TRUE);
                }

                // Load the binary shader.
                glProgramBinary(entry.Program, format, buffer, size);

                if (SeparablePipelineExtension)
                {
                    GLint separableFlag;
                    glGetProgramiv(entry.Program, GL_PROGRAM_SEPARABLE, &separableFlag);
                    if ((separableFlag == GL_TRUE) != SeparablePipelines)
                    {
                        SF_BINARYSHADER_DEBUG_MESSAGE2(1, "Loaded shader program's GL_PROGRAM_SEPARABLE value does not "
                            "match current state (hash=0x%08x, separable=%d). This shader will be ignored.\n",
                            hashCode, SeparablePipelines ? 1 : 0);
                        glDeleteProgram(entry.Program);
                        continue;
                    }
                }

                // Check to see if glProgramBinary failed, for instance because the driver has changed.
                GLint linkStatus;
                glGetProgramiv(entry.Program, GL_LINK_STATUS, &linkStatus);
                if (linkStatus != GL_TRUE)
                {
                    SF_DEBUG_WARNONCE(1, "Binary shader program failed. This might indicate a driver change since the last binary shader saving - recompiling.");
                    continue;
                }

                // Save the binary size, so we know this shader does not need resaving.
                entry.BinarySize = size;
                CompiledShaderHash.Add(hashCode, entry);
            }
        }

        SF_FREE(buffer);
        return true;
    }
#endif // SF_GL_BINARY_SHADER

    // We did not load any binary shaders.
    return false;
}

bool ShaderManager::loadAndVerifyShaderCacheHeader( File* pfile )
{
#if defined(SF_GL_BINARY_SHADER)
    if (!pfile || !pfile->IsValid())
    {
        if (SingleBinaryShaderFile)
        {
            SF_DEBUG_WARNING1(1, "Error reading binary shader cache, error code %d", pfile->GetErrorCode());
        }
        else
        {
            // With multiple files, don't spew a whole bunch of errors (unless in binary-shader-info-mode).
            // These may be expected errors, if this is the first time the app has compiled shaders.
            SF_BINARYSHADER_DEBUG_MESSAGE1(1, "Error reading binary shader cache, error code %d", pfile->GetErrorCode());
        }
        return false;
    }
    SF_BINARYSHADER_DEBUG_MESSAGE1(1, "Shader binary file is %d bytes\n", pfile->GetLength());

    char header[ShaderHeaderSize];
    SInt64 version = 0;

    if (pfile->Read((UByte*)header, ShaderHeaderSize) < (int)ShaderHeaderSize || strncmp(header, ShaderHeaderString, ShaderHeaderSize))
    {
        SF_DEBUG_WARNING1(1, "Binary shader file does not contain the required header (%s).", ShaderHeaderString);
        return false;
    }

    version             = pfile->ReadSInt64();
    if ( version != SF_GFXSHADERMAKER_TIMESTAMP )
    {
        SF_DEBUG_WARNING2(version != SF_GFXSHADERMAKER_TIMESTAMP, "Binary shaders timestamps do not match executable. "
            "(bin=%lld, exe=%lld)", version, SF_GFXSHADERMAKER_TIMESTAMP);
        return false;
    }

    return true;
#else
    return false;
#endif
}

void ShaderManager::addShaderToValidationQueue(unsigned shaderComboIndex)
{
    SF_DEBUG_ASSERT2(shaderComboIndex < UniqueShaderCombinations, "ShaderComboIndex is too high (%d, max =%d)", shaderComboIndex, UniqueShaderCombinations);
    SF_DEBUG_ASSERT(StaticShaders[shaderComboIndex].IsLinked, "Attempting to validate a shader which has not been linked.");
    if (!StaticShaders[shaderComboIndex].IsValidated)
    {
        ValidationQueue[NextValidationEntry++] = shaderComboIndex;
        ValidationQueue[NextValidationEntry] = -1;
    }
}

bool replaceFSize( char * shaderCode, const char* originalString, float * fsize )
{
    char scratchBuffer[MaxShaderCodeSize];
    const char* startLocation = originalString;
    const char* nextReplacement = SFstrchr(originalString, '%');
    char* nextWriteLocation = scratchBuffer;
    while (nextReplacement)
    {
        SFstrncpy(nextWriteLocation, MaxShaderCodeSize-(nextWriteLocation-scratchBuffer), startLocation, nextReplacement-startLocation );
        nextWriteLocation += nextReplacement-startLocation;
        if (!SFstrncmp(nextReplacement+1, "fsize.x", 7))
        {
            nextWriteLocation += SFsprintf(nextWriteLocation, MaxShaderCodeSize-(nextWriteLocation-scratchBuffer), "%.5f", fsize[0]);
            startLocation += nextReplacement-startLocation+1;
            startLocation += 8; // skip var and tail %
        }
        else if (!SFstrncmp(nextReplacement+1, "fsize.y", 7))
        {
            nextWriteLocation += SFsprintf(nextWriteLocation, MaxShaderCodeSize-(nextWriteLocation-scratchBuffer), "%.5f", fsize[1]);
            startLocation += nextReplacement-startLocation+1;
            startLocation += 8; // skip var and tail %
        }
        else
        {
            SF_DEBUG_ASSERT1(0, "Unexpected loop variable replacement. Dynamic loop shader replacement will fail. ShaderCode:\n%s\n", originalString);
            return false;
        }
        nextReplacement = SFstrchr(startLocation, '%');
    }
    SFstrcpy(nextWriteLocation, MaxShaderCodeSize-(nextWriteLocation-scratchBuffer), startLocation);
    SFstrcpy(shaderCode, MaxShaderCodeSize, scratchBuffer);

    return true;
}

ShaderObject* ShaderManager::getDynamicLoopShaderWithLiteralReplaced(const DynamicLoopKey& keyParams)
{
    // Try to get the hashed shader object.
    ShaderObject** shaderObject = DynamicLoopShaderHash.Get(keyParams);
    if (shaderObject != 0)
        return *shaderObject;

    // Wasn't already hashed, it must be created.
    const VertexShaderDesc* vdesc = VertexShaderDesc::GetDesc(keyParams.ShaderType, GLSLVersion);
    const FragShaderDesc* fdesc = FragShaderDesc::GetDesc(keyParams.ShaderType, GLSLVersion);

    char vshaderCodeBuffer[MaxShaderCodeSize];
    char fshaderCodeBuffer[MaxShaderCodeSize];
    const char* shaderCode[2] = {vshaderCodeBuffer, fshaderCodeBuffer};

    // Make the replacements.
    float fsize[4] = {keyParams.BlurX, keyParams.BlurY, 0.0f, 0.0f};
    if (!replaceFSize(vshaderCodeBuffer, vdesc->pSource, fsize) ||
        !replaceFSize(fshaderCodeBuffer, fdesc->pSource, fsize))
    {
        return 0;
    }

    // Create everything.
    Ptr<HALGLShader> shaders[2];
    shaders[0] = *SF_NEW HALGLShader(ShaderStage_Vertex);
    shaders[1] = *SF_NEW HALGLShader(ShaderStage_Frag);
    Ptr<HALGLProgram> program = *SF_NEW HALGLProgram();
    glCreateProgram(program);


    // Now compile the shader.
    char infoBuffer[MaxShaderCodeSize];
    for (unsigned i = 0; i < 2; ++i)
    {
        ShaderStages stage = (ShaderStages)i;
        glCreateShader(ShaderObject::getShaderTypeForStage(stage), shaders[stage]);
        glShaderSource(shaders[stage], 1, (const char**)&shaderCode[stage], 0);
        glCompileShader(shaders[stage]);
        GLint compileStatus;
        glGetShaderiv(shaders[stage], GL_COMPILE_STATUS, &compileStatus);
        if (compileStatus != GL_TRUE)
        {
            glGetShaderInfoLog(shaders[stage], sizeof(infoBuffer), 0, infoBuffer);
            glDeleteShader(shaders[stage]);
            SF_DEBUG_ASSERT1(0, "Shader failed to compile: %s\n", infoBuffer);
            return 0;
        }
        glAttachShader(program, shaders[stage]);
        glDeleteShader(shaders[stage]);
    }

    // Now link it.
    glLinkProgram(program);
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE)
    {
        glGetProgramInfoLog(program, MaxShaderCodeSize, 0, infoBuffer);
        SF_DEBUG_ASSERT1(0, "Shader failed to link: %s\n", infoBuffer);
        return 0;
    }

    ShaderObject* newObject = SF_NEW ShaderObject();
    newObject->Init(pHal, program, vdesc, fdesc);

    // Add the shader to the hash.
    DynamicLoopShaderHash.Set(keyParams, newObject);

    return newObject;
}

//*** ShaderInterface
void ShaderInterface::ResetContext()
{
}

void ShaderInterface::BeginScene()
{
    // Clear the current shader.
    CurShader.pShaderObj = 0;
    CurShader.pVDesc     = 0;
    CurShader.pFDesc     = 0;
}

}}}
