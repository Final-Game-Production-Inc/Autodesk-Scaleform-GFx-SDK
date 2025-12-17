/**************************************************************************

Filename    :   GL_Shader.h
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2011 Autodesk, Inc. All Rights reserved.

Use of this software is subject to the terms of the Autodesk license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

**************************************************************************/

#ifndef INC_SF_GL_Shader_H
#define INC_SF_GL_Shader_H

#include "Render/GL/GL_Common.h"

#if defined(GL_ES_VERSION_2_0)
#include "Render/GL/GLES_ShaderDescs.h"
#else
#include "Render/GL/GL_ShaderDescs.h"
#endif

#include "Render/Render_Shader.h"
#include "Render/Render_FiltersLE.h"

namespace Scaleform { namespace Render { namespace GL {

class HAL;
class Texture;
class ShaderInterface;
                              
// Bookeeping structure for organizing shader programs.
struct ShaderHashEntry
{
    ShaderHashEntry() : Program(0), BinarySize(0) { }

    GLuint  Program;        // The program name.
    GLint   BinarySize;     // The binary size of the program, when it was last checked.
};

// Represents a unique combination of shader programs to acheive a specific rendering technique.
struct ShaderObject
{
    friend class ShaderInterface;
    friend class ShaderManager;

    ShaderObject();
    ~ShaderObject();

    // Initializes the shader, first attempting to read its program from the shader hash. If it cannot 
    // be located in the shader hash, then it is compiled from source, and inserted into the hash. 
    bool Init(HAL* phal, ShaderDesc::ShaderVersion ver, unsigned comboIndex, bool separable, 
              HashLH<unsigned, ShaderHashEntry>& shaderHash, bool testCompilation = false);

    void Shutdown();
    bool IsInitialized() const;

    // Applies the shader to the rendering pipeline.
    void ApplyShader() const;

    // Returns the uniform definition information for the given variable (assumes that the uniform is only
    // used within a single pipeline stage). If the variable is not used in any pipeline, NULL is returned.
    const UniformVar* GetUniformVariable(unsigned var) const;

    // Returns the GL program associated with the given uniform variable. This is only useful in separable shader objects,
    // which use the glProgramUniform* calls, so they know which program to update the uniforms for.
    GLuint GetUniformVariableProgram(unsigned var) const;

    inline  HAL* GetHAL() const { return pHal; }

protected:

    // Structure representing the location of the uniform (wrt GL's shader compiler).
    struct UniformLocation
    {
        GLuint Program;     // The program which contains the uniform.
        GLint  Location;    // The index of the uniform.
    };

    HAL*                    pHal;
    const VertexShaderDesc* pVDesc;
    const FragShaderDesc*   pFDesc;
    ShaderDesc::ShaderVersion   ShaderVer;                          // The ShaderVersion that this shader uses.
    int                         ComboIndex;                         // The shader combo index for this object. Only used in non-separated pipeline.
    bool                        Separated;                          // true if this shader object is a separable program, false otherwise.
    GLuint                      Pipeline;                           // A shader pipeline object (only used if Separated = true).
    GLuint                      StagePrograms[ShaderStage_Count];   // Array of shader programs, for each shader stage.
    UniformLocation             Uniforms[Uniform::SU_Count];            // The program and location of the uniforms.

    // Inspects the linked program/pipeline, and extracts the required uniform information.
    bool initUniforms();

    // Creates either a GL shader or a separable GL program, whose type and contents are given by the stage 
    // and shaderCode parameters. If any failure occurs, debug output is generated within this function,
    // but surpressed if testCompilation is true.
    GLuint createShaderOrProgram(ShaderStages stage, const char* shaderCode, bool separable, bool testCompilation);

    // Creates either an uber-program or a pipeline, from the array shaders (or programs) in the shaders array.
    bool createProgramOrPipeline(ShaderHashEntry* shaders, bool separable, HashLH<unsigned, ShaderHashEntry>& shaderHash);

    // Gets the shader code for the shader stage, given by 'stage'. The shaderModificationBuffer is be required 
    // if the shader could be modified (for example, to reduce batch counts to match available uniform registers). 
    // Returns the actual source shader code, which may either be the modified source, or a pointer to the original.
    const char* getShaderPipelineCode(ShaderStages stage, unsigned maxUniforms, char* shaderModificationBuffer);

    // Gets the hash code for the given pipeline for this shader. If searching for a program (instead of a shader), 
    // pass true from 'program'.
    unsigned getShaderPipelineHashCode(bool program, ShaderStages stage);
    static unsigned getShaderPipelineHashCode(unsigned comboIndex, ShaderDesc::ShaderVersion ver, bool separated, 
        bool program, ShaderStages stage);

    // Returns the shader type, given the shader stage.
    static GLenum getShaderTypeForStage(ShaderStages stage);
    // Returns the shader 'bit' for the given shader stage (eg. GL_VERTEX_SHADER_BIT).
    static GLenum getShaderBitForStage(ShaderStages stage);

    // Clears this object to all references of shader programs.
    void releasePrograms();

    // Prints out the uniforms for the input program to the debug output. Note that this requires a special define
    // change to SF_GL_BINARY_SHADER_DEBUG, to determine the level of verbosity. If the input name is zero, the
    // shader program(s) for this object are output.
    void dumpUniforms(GLuint shader = 0);
};

struct ShaderPair
{
    const ShaderObject*     pShaderObj;
    const VertexShaderDesc* pVDesc;
    const FragShaderDesc*   pFDesc;
        
    ShaderPair() : pShaderObj(0), pVDesc(0), pFDesc(0) {}
        
    operator bool() const { return pShaderObj && pVDesc && pFDesc; }
    const ShaderPair* operator->() const { return this; }
};

typedef BlurFilterShaderDesc<GLint> BlurFilterShader;

class ShaderInterface : public ShaderInterfaceBase<Uniform,ShaderPair>
{
    HAL*       pHal;
    ShaderPair CurShader;

    // Holds the current stages used by samplers.
    struct TextureSamplerUniform
    {
        int     UniformVar;
        int     StagesUsed;
        GLint   SamplerStages[FragShaderDesc::MaxTextureSamplers];
    };
    TextureSamplerUniform   TextureUniforms[FragShaderDesc::MaxTextureSamplers];

    inline  HAL* GetHAL() const { return pHal; }

public:
    typedef ShaderPair Shader;

    ShaderInterface(Render::HAL* phal);
    ~ShaderInterface() { ResetContext(); }

    const Shader&       GetCurrentShaders() const { return CurShader; }
    bool                SetStaticShader(ShaderDesc::ShaderType shader, const VertexFormat* pvf);

    void                SetTexture(Shader sp, unsigned stage, Render::Texture* ptexture, ImageFillMode fm, unsigned index = 0);

    void                Finish(unsigned batchCount = 0);

    // Returns the maximum number of uniform vectors required to render a single element in a batch or instance.
    static unsigned     GetMaximumRowsPerInstance();
    // Returns the number of rows per instance for the given VertexShaderDesc (0 if not batched).
    static unsigned     GetRowsPerInstance(const VertexShaderDesc* pvdesc);
    // Returns the number of entries per instance of the given uniform type.
    static unsigned     GetCountPerInstance(const VertexShaderDesc* pvdesc, Uniform::UniformType arrayType);

    const BlurFilterShader* GetBlurShader(const BlurFilterShaderKey& key);

    void                ResetContext();

    // Called on BeginScene (resets redundancy checks)
    void                BeginScene();

protected:
    static unsigned     MaxRowsPerInstance;  // Cached value from GetMaximumRowsPerInstance.

    typedef Hash<BlurFilterShaderKey, const BlurFilterShader*> BlurShadersHash;
    BlurShadersHash BlurShaders;
};

typedef StaticShaderManager<ShaderDesc, VertexShaderDesc, Uniform, ShaderInterface, Texture> StaticShaderManagerType;
class ShaderManager : public StaticShaderManagerType
{
    friend struct ShaderObject;
    friend class ShaderInterface;
    friend class HAL;

public:
    typedef StaticShaderManager<ShaderDesc, VertexShaderDesc, Uniform, ShaderInterface, Texture> Base;
    typedef Uniform UniformType;

    ShaderManager(ProfileViews* prof);

    bool Initialize(HAL* phal, unsigned vmcFlags);

    // Sets the path in which the file containing binary shader definitions will be read/written.
    void SetBinaryShaderPath(const String& path) { BinaryShaderPath = path; }

    // Returns the number of (vertex) uniforms available on the system. Overridden in GL, because GLES frequently
    // only supports 128 uniforms, and some buggy device drivers can support as little as 32.
    virtual unsigned GetNumberOfUniforms() const;

    // Uses either standard GetFilterPasses or BlurFilterState::Setup if non-looping mode is needed.
    unsigned SetupFilter(const Filter* filter, unsigned fillFlags, unsigned* passes, BlurFilterState& leBlur) const;

    // By default, systems will report that they have dynamic loops (in shaders).
    bool    GetDynamicLoopSupport();

    // GL can support instancing in OpenGL 3.0+, or with the GL_ARB_draw_instanced extension. 
    bool    HasInstancingSupport() const;

    // Whether we are using separate shader objects (GL_EXT_separate_shader_objects)
    bool    UsingSeparateShaderObject() const;

    // Called on shutdown, or ResetContext.
    void    Reset();

    // Called on BeginScene.
    void    BeginScene();

    static unsigned GetDrawableImageFlags() { return ShaderManager::CPF_InvertedViewport; }

    // Hide warning.
    ShaderManager& operator= (const ShaderManager&) { return *this; };

    // Retrieve the HAL that owns this shader manager
    HAL* GetHAL() const { return pHal; }

protected:

    // Saves binary shaders to disk.
    void saveBinaryShaders();
    // Loads binary shaders from disk.
    bool loadBinaryShaders();
    // Verifies that a shader file has a matching header and version. Returns true 
    // on success, false if an error occured, or the header did not match.
    bool loadAndVerifyShaderCacheHeader( File* pfile);

    HAL*                                pHal;                                       // HAL object that owns this ShaderManager.
    unsigned                            Caps;                                       // See Render::GL::CapFlags, set in InitHAL.
    ShaderDesc::ShaderVersion           GLSLVersion;                                // Holds the GLSL version that we are currently using.
    ShaderObject                        StaticShaders[UniqueShaderCombinations];    // The set of VS/FS combinations used by the ShaderManager.
    HashLH<unsigned, ShaderHashEntry>   CompiledShaderHash;                         // Hash of source shader text to compiled shader index.
    String                              BinaryShaderPath;                           // Filename to store binary shaders.
    int                                 DynamicLoops;                               // Cached value from GetDynamicLoopSupport.
    bool                                ShouldUseBinaryShaders;                     // Whether to use binary shaders or not.
    bool                                SingleBinaryShaderFile;                     // Whether to use a monolithic shader binary (or individual files)
    bool                                SeparablePipelines;                         // Whether shader programs are separable (GL_EXT_separate_shader_objects)
    bool                                SeparablePipelineExtension;                 // Set to true if this platform is capable of separable pipelines.
};

}}}

#endif
