/* Copyright 1993-2007 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO USER:
 *
 * This source code is subject to NVIDIA ownership rights under U.S. and
 * international Copyright laws.
 *
 * This software and the information contained herein is PROPRIETARY and
 * CONFIDENTIAL to NVIDIA and is being provided under the terms and
 * conditions of a Non-Disclosure Agreement.  Any reproduction or
 * disclosure to any third party without the express written consent of
 * NVIDIA is prohibited.
 *
 * NVIDIA MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS SOURCE
 * CODE FOR ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR
 * IMPLIED WARRANTY OF ANY KIND.  NVIDIA DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOURCE CODE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OR PERFORMANCE OF THIS SOURCE CODE.
 *
 * U.S. Government End Users.  This source code is a "commercial item" as
 * that term is defined at 48 C.F.R. 2.101 (OCT 1995), consisting  of
 * "commercial computer software" and "commercial computer software
 * documentation" as such terms are used in 48 C.F.R. 12.212 (SEPT 1995)
 * and is provided to the U.S. Government only as a commercial end item.
 * Consistent with 48 C.F.R.12.212 and 48 C.F.R. 227.7202-1 through
 * 227.7202-4 (JUNE 1995), all U.S. Government End Users acquire the
 * source code with only those rights set forth herein.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <GL/glew.h>
#include <GL/glut.h>

#define NV_REPORT_COMPILE_ERRORS    // Defined to print compiler errors.

#include "nvShaderUtils.h"
#include "nvGlutManipulators.h"

// GFx modification: Global instance of the GFxPlayer object.
#include "GFxPlayerGL.h"
static Scaleform::GFxPlayer* gfxplayer = 0;

// This example shows how to use bindable uniform buffers and draw instanced in order to 
// render tessellated curved surfaces. The vertices and the indices are stored in buffer 
// objects and bound to the shader. Patches are drawn by rendering a single tessellated
// quad multiple times, using instancing to do this in a single call and the instance id
// to index the index buffer. Indices are stored as integers using the integer support
// in gpu shader4.
//
// This is idea is based on this paper:
// http://www.labri.fr/publications/is/2005/BS05/


// TODO:
// - Add dynamic lod.
// - Add more bezier surfaces.

// Vertex shader.
static const char * s_vertexShaderText =
    "#version 120\n"
    "#extension GL_EXT_bindable_uniform : enable\n"
    "#extension GL_EXT_gpu_shader4 : enable\n"
    "\n"
    "bindable uniform vec4 vertices[%d];\n"
    "bindable uniform ivec4 indices[%d/4];\n"
    "\n"
    "varying vec3 v_Normal;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "    vec3 p[16];\n"
    "    for(int i = 0; i < 4; i++) {\n"
    "        p[4*i+0] = vertices[indices[gl_InstanceID * 4 + i].x].xyz;\n"
    "        p[4*i+1] = vertices[indices[gl_InstanceID * 4 + i].y].xyz;\n"
    "        p[4*i+2] = vertices[indices[gl_InstanceID * 4 + i].z].xyz;\n"
    "        p[4*i+3] = vertices[indices[gl_InstanceID * 4 + i].w].xyz;\n"
    "    }\n"
    "    \n"
    "    vec2 uv = gl_Vertex.xy;\n"
    "    \n"
    "    vec2 B0 = (1 - uv) * (1 - uv) * (1 - uv);\n"
    "    vec2 B1 = 3 * uv * (1 - uv) * (1 - uv);\n"
    "    vec2 B2 = 3 * uv * uv * (1 - uv);\n"
    "    vec2 B3 = uv * uv * uv;\n"
    "    \n"
    "    vec3 pos = \n"
    "        (B0.x * p[ 0] + B1.x * p[ 1] + B2.x * p[ 2] + B3.x * p[ 3]) * B0.y +\n"
    "        (B0.x * p[ 4] + B1.x * p[ 5] + B2.x * p[ 6] + B3.x * p[ 7]) * B1.y +\n"
    "        (B0.x * p[ 8] + B1.x * p[ 9] + B2.x * p[10] + B3.x * p[11]) * B2.y +\n"
    "        (B0.x * p[12] + B1.x * p[13] + B2.x * p[14] + B3.x * p[15]) * B3.y;\n"
    "    \n"
    "    vec2 T0 = (1 - uv) * (1 - uv);\n"
    "    vec2 T1 = 2 * uv * (1 - uv);\n"
    "    vec2 T2 = uv * uv;\n"
    "    \n"
    "    vec3 dv = \n"
    "        (B0.x * (p[ 4]-p[0]) + B1.x * (p[ 5]-p[1]) + B2.x * (p[ 6]-p[ 2]) + B3.x * (p[ 7]-p[ 3])) * T0.y +\n"
    "        (B0.x * (p[ 8]-p[4]) + B1.x * (p[ 9]-p[5]) + B2.x * (p[10]-p[ 6]) + B3.x * (p[11]-p[ 7])) * T1.y +\n"
    "        (B0.x * (p[12]-p[8]) + B1.x * (p[13]-p[9]) + B2.x * (p[14]-p[10]) + B3.x * (p[15]-p[11])) * T2.y;\n"
    "    \n"
    "    vec3 du = \n"
    "        (T0.x * (p[ 1]-p[ 0]) + T1.x * (p[ 2]-p[ 1]) + T2.x * (p[ 3]-p[ 2])) * B0.y +\n"
    "        (T0.x * (p[ 5]-p[ 4]) + T1.x * (p[ 6]-p[ 5]) + T2.x * (p[ 7]-p[ 6])) * B1.y +\n"
    "        (T0.x * (p[ 9]-p[ 8]) + T1.x * (p[10]-p[ 9]) + T2.x * (p[11]-p[10])) * B2.y +\n"
    "        (T0.x * (p[13]-p[12]) + T1.x * (p[14]-p[13]) + T2.x * (p[15]-p[14])) * B3.y;\n"
    "    \n"
    "    gl_Position = gl_ModelViewProjectionMatrix * vec4(pos, 1.0);\n"
    "    vec3 nor = cross(du, dv);\n"
    "    nor = (length(nor) != 0) ? normalize(nor) : vec3(0.0);\n"
    "    v_Normal = gl_NormalMatrix * nor;\n"
    "}\n"
    "\n";

// Default fragment shader.
static const char * s_fragmentShaderText =
    "varying vec3 v_Normal;\n"
    "\n"
    "uniform vec3 color;\n"
    "\n"
    "void main() {\n"
    "    vec3 N = normalize(v_Normal);\n"
    "    vec3 L = normalize(gl_LightSource[0].position.xyz);\n"
    "    vec3 H = normalize(gl_LightSource[0].halfVector.xyz);\n"
    "    \n"
    "    vec3 ambient = color * 0.1;\n"
    "    vec3 diffuse = color * 0.9 * max(dot(L, N), 0.0);\n"
    "    vec3 specular = vec3(1.0, 1.0, 1.0) * pow(max(dot(H, N), 0.0), 16.0);\n"
    "    \n"
    "    gl_FragColor = vec4(ambient + diffuse + specular, 1.0);\n"
    "}\n";

// Window properties.
static int s_width = 800;
static int s_height = 600;

// Manipulators.
static nv::GlutExamine s_trackball;

// Handles.
static GLuint s_program = 0;
static GLuint s_vbHandle = 0;
static GLuint s_ibHandle = 0;

// Geometry.
#include "teapot.h"

static int s_patchVertexCount;
static int s_patchIndexCount;
const int c_teapotVertexCount = sizeof(teapotVertices) / (3 * sizeof(float));
const int c_teapotIndexCount = sizeof(teapotIndices) / sizeof(int);
const int c_teapotPatchCount = c_teapotIndexCount / 16;

// Rendering options.
// GFx modification: removed 'static' from these, so they can be exposed to the GFxPlayer callback system.
bool s_drawWireframe = false;
bool s_drawControlMesh = false;
bool s_drawGlutTeapot = false;
bool s_animate = true;




// Mouse event handler.
void mouse(int button, int state, int x, int y)
{
    // GFx modification: If the GFx content passes a hit-test for the given mouse-click event coordinates, 
    // translate this to a GFx event type, and pass it to the gfxplayer for processing.
    if (gfxplayer->IsHit(x,y))
    {
        Scaleform::GFx::Event::EventType type = Scaleform::GFx::Event::MouseDown;
        switch(state)
        {
        default:        
            SF_DEBUG_MESSAGE1(true, "Unexpected glutMouseFunc state (%d)\n", state);
            break;
        case GLUT_DOWN: 
            type = Scaleform::GFx::Event::MouseDown; 
            break;
        case GLUT_UP:   
            type = Scaleform::GFx::Event::MouseUp; 
            break;
        }
        gfxplayer->OnMouseEvent(type, button, x, y);
    }
    else
    {
        s_trackball.mouse(button, state, x, y);
    }

    glutPostRedisplay();
}

// Mouse motion event handler.
void motion(int x, int y)
{
    // GFx modification: Notify the gfxplayer of any mouse movement.
    gfxplayer->OnMouseEvent(Scaleform::GFx::Event::MouseMove, 0, x, y);

    s_trackball.motion(x, y);
    glutPostRedisplay();
}

// Key event handler.
void keyboard(unsigned char c, int x, int y)
{
    if (c == 27 || c == 'q' || c == 'Q')
    {
        exit(0);
    }
    else if (c == 'w' || c == 'W')
    {
        s_drawWireframe = !s_drawWireframe;
    }
    else if (c == '1')
    {
        s_drawControlMesh = !s_drawControlMesh;
    }
    else if (c == '2')
    {
        s_drawGlutTeapot = !s_drawGlutTeapot;
    }
    else if (c == ' ')
    {
		s_animate = !s_animate;
    }

    glutPostRedisplay();
}

// Display event handler.
void display()
{
    // GFx modification: Before clearing buffers, ensure that all of the write masks are enabled. GFx rendering may 
    // have disabled them. Note that in general, GFx does not preserve GL states before or after rendering. The user is 
    // responsible for resetting all required states after GFx rendering is complete.
    glDepthMask(GL_TRUE);
    glStencilMask(GL_TRUE);
    glColorMask(1,1,1,1);

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(20.0f, (double)s_width/s_height, 0.25f, 5.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    s_trackball.applyTransform();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);

    if (s_drawWireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glScalef(0.2, 0.2, 0.2);
    glColor3f(1, 1, 1);

    if (s_drawGlutTeapot)
    {
        // Draw reference teapot.
        glutWireTeapot(1);
    }
    else
    {
        glPushMatrix();

        // Apply the standard teapot transform.
        glRotated( 270.0, 1.0, 0.0, 0.0 );
        glScaled( 0.5, 0.5, 0.5 );
        glTranslated( 0.0, 0.0, -1.5 );

        // Select program.
        glUseProgram(s_program);

        GLint colorUniform = glGetUniformLocation(s_program, "color");
        glUniform3f(colorUniform, 1.0f, 0.0f, 0.0f);

        glEnableClientState(GL_VERTEX_ARRAY);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, s_vbHandle);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, s_ibHandle);
        glVertexPointer(2, GL_FLOAT, 0, 0);

        // Draw patches.
        glDrawElementsInstancedEXT(GL_TRIANGLES, s_patchIndexCount, GL_UNSIGNED_SHORT, 0, c_teapotPatchCount);

        glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
        glDisableClientState(GL_VERTEX_ARRAY);

        glUseProgram(0);
        
        glPopMatrix();
    }

    if (s_drawControlMesh)
    {
        glPushMatrix();

        // Apply the standard teapot transform.
        glRotated( 270.0, 1.0, 0.0, 0.0 );
        glScaled( 0.5, 0.5, 0.5 );
        glTranslated( 0.0, 0.0, -1.5 );

        glColor3f(0,0,1);

        // Draw control mesh using inmediate mode.
        glBegin(GL_LINES);

        for(int p = 0; p < c_teapotPatchCount; p++)
        {
            int v[16];
            for(int i = 0; i < 16; i++)
            {
                v[i] = 3 * teapotIndices[p * 16 + i];
            }

            for(int i = 0; i < 4; i++)
            {
                glVertex3fv(teapotVertices + v[4*i+0]);
                glVertex3fv(teapotVertices + v[4*i+1]);

                glVertex3fv(teapotVertices + v[4*i+1]);
                glVertex3fv(teapotVertices + v[4*i+2]);

                glVertex3fv(teapotVertices + v[4*i+2]);
                glVertex3fv(teapotVertices + v[4*i+3]);

                glVertex3fv(teapotVertices + v[i+0]);
                glVertex3fv(teapotVertices + v[i+4]);

                glVertex3fv(teapotVertices + v[i+4]);
                glVertex3fv(teapotVertices + v[i+8]);

                glVertex3fv(teapotVertices + v[i+8]);
                glVertex3fv(teapotVertices + v[i+12]);
            }
        }

        glEnd();

        glPopMatrix();
    }

    // GFx modification: Now advance and render GFx content.
    gfxplayer->AdvanceAndDisplay();

    glutSwapBuffers();
}

// Reshape event handler.
void reshape(int w, int h)
{
    // GFx modification: send resize events to the GFxPlayer, so that it can resize the viewport.
    gfxplayer->OnResize(w,h);

    s_width = w;
    s_height = h;
    glViewport(0, 0, s_width, s_height);
    s_trackball.reshape(w, h);
}

// Idle event handler.
static void idle()
{
    if (s_animate) {
        s_trackball.idle();
        glutPostRedisplay();
    }
}

// Main menu event handler.
static void mainMenu(int i)
{
	keyboard((unsigned char) i, 0, 0);
}


// Compile shaders and link program.
bool initShaders()
{
    char * vertexShaderText = new char[strlen(s_vertexShaderText) + 1024];
    sprintf(vertexShaderText, s_vertexShaderText, c_teapotVertexCount, c_teapotIndexCount);

    GLuint vertexShader = nv::CompileGLSLShader(GL_VERTEX_SHADER, vertexShaderText);
    if (vertexShader == 0) 
    {
        printf("Vertex shader failed to compile\n");
        return false;
    }

    delete [] vertexShaderText;

    GLuint fragmentShader = nv::CompileGLSLShader(GL_FRAGMENT_SHADER, s_fragmentShaderText);
    if (fragmentShader == 0) 
    {
        printf("Fragment shader failed to compile\n");
        return false;
    }

    s_program = nv::LinkGLSLProgram(vertexShader, fragmentShader);
    if (s_program == 0) 
    {
        printf("Program failed to link\n");
        return false;
    }

    return true;
}


// Allocate bindable uniform buffers for teapot control mesh.
void initUniformBuffers()
{
    // zero based indices.
    for(int i = 0; i < c_teapotIndexCount; i++)
    {
        --teapotIndices[i];
    }

    int maxVertexBindableUniforms;
    glGetIntegerv(GL_MAX_VERTEX_BINDABLE_UNIFORMS_EXT, &maxVertexBindableUniforms);
    assert(maxVertexBindableUniforms >= 2);

    int maxBindableUniformSize;
    glGetIntegerv(GL_MAX_BINDABLE_UNIFORM_SIZE_EXT, &maxBindableUniformSize);
    assert(4 * sizeof(float) * c_teapotVertexCount <= maxBindableUniformSize);
    assert(sizeof(teapotIndices) <= maxBindableUniformSize);

    // create vertex buffer.
    GLint vlocation = glGetUniformLocation(s_program, "vertices");
    GLint vsize = glGetUniformBufferSizeEXT(s_program, vlocation);
    assert(vsize == 4 * sizeof(float) * c_teapotVertexCount);

    // In the present implementation all constants must be 128-bit aligned, so an array of 
    // vec3s is padded to vec4s. A future driver version may remove this limitation.
    float * tmp = new float[c_teapotVertexCount * 4];
    for(int i = 0; i < c_teapotVertexCount; i++)
    {
        tmp[4 * i + 0] = teapotVertices[3 * i + 0];
        tmp[4 * i + 1] = teapotVertices[3 * i + 1];
        tmp[4 * i + 2] = teapotVertices[3 * i + 2];
        tmp[4 * i + 3] = 1.0f;    // pad
    }

    GLuint vbuffer;
    glGenBuffers(1, &vbuffer);
    glBindBuffer(GL_UNIFORM_BUFFER_EXT, vbuffer);
    glBufferData(GL_UNIFORM_BUFFER_EXT, vsize, tmp, GL_STATIC_READ);
    glUniformBufferEXT(s_program, vlocation, vbuffer);

    delete [] tmp;

    // create index buffer.
    GLint ilocation = glGetUniformLocation(s_program, "indices");
    GLint isize = glGetUniformBufferSizeEXT(s_program, ilocation);
    assert(isize == sizeof(teapotIndices));

    GLuint ibuffer;
    glGenBuffers(1, &ibuffer);
    glBindBuffer(GL_UNIFORM_BUFFER_EXT, ibuffer);
    glBufferData(GL_UNIFORM_BUFFER_EXT, isize, teapotIndices, GL_STATIC_READ);
    glUniformBufferEXT(s_program, ilocation, ibuffer);
}


// Allocate vertex and index buffer for rectangular patch.
void initPatchGeometry()
{
    int density = 15;
    s_patchVertexCount = (density + 1) * (density + 1);
    s_patchIndexCount = density * density * 6;

    float * const vertexPtr = new float[2 * s_patchVertexCount];
    short * const indexPtr = new short[s_patchIndexCount];

    int idx = 0;
    for(int i = 0; i <= density; i++)
    {
        float v = float(i) / density;
            
        for(int e = 0; e <= density; e++)
        {
            float u = float(e) / density;

            vertexPtr[idx++] = u;
            vertexPtr[idx++] = v;
        }
    }
    assert(idx == 2 * s_patchVertexCount);

    idx = 0;
    for(int i = 0; i < density; i++)
    {
        for(int e = 0; e < density; e++)
        {
            indexPtr[idx++] = (density+1) * (i+0) + (e+0);
            indexPtr[idx++] = (density+1) * (i+0) + (e+1);
            indexPtr[idx++] = (density+1) * (i+1) + (e+0);

            indexPtr[idx++] = (density+1) * (i+1) + (e+0);
            indexPtr[idx++] = (density+1) * (i+0) + (e+1);
            indexPtr[idx++] = (density+1) * (i+1) + (e+1);
        }
    }
    assert(idx == s_patchIndexCount);

    glGenBuffersARB(1, &s_vbHandle);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, s_vbHandle);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, s_patchVertexCount * sizeof(float) * 2, vertexPtr, GL_STATIC_DRAW_ARB);

    glGenBuffersARB(1, &s_ibHandle);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, s_ibHandle);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, s_patchIndexCount * sizeof(short), indexPtr, GL_STATIC_DRAW_ARB);

    delete [] vertexPtr;
    delete [] indexPtr;
}

static void initMenus()
{
    glutCreateMenu(mainMenu);
    glutAddMenuEntry("Toggle drawing of the control mesh [1]", '1');
    glutAddMenuEntry("Toggle drawing of the glut reference teapot [2]", '2');
    glutAddMenuEntry("Toggle wireframe rendering [w]", 'w');
    glutAddMenuEntry("Quit (esc)", '\033');
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}



// Program entrypoint.
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(s_width, s_height);
    glutCreateWindow("Tessellation example");

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);

    s_trackball.setDollyActivate(GLUT_LEFT_BUTTON, GLUT_ACTIVE_CTRL);
    s_trackball.setPanActivate(GLUT_LEFT_BUTTON, GLUT_ACTIVE_SHIFT);
    s_trackball.setDollyPosition(-2.0f);

	initMenus();

    printf("This example draws a mesh composed of bezier patches using intancing to emulate tessellation.\n\n");
    printf("Use the following keys to toggle options:\n");
    printf("[1] Toggle drawing of the control mesh.\n");
    printf("[2] Toggle drawing of the glut reference teapot.\n");
    printf("[w] Toggle wireframe rendering.\n");
    printf("[q] Exit the application.\n\n");
    printf("Use the mouse to manipulate camera:\n");
    printf("Left mouse button to rotate around the object.\n");
    printf("Left mouse button + Shift to dolly.\n");
    printf("Left mouse button + Ctrl to pan.\n\n");

    glewInit();
    if (!GLEW_VERSION_2_0 || !GLEW_ARB_vertex_buffer_object || !GLEW_EXT_gpu_shader4 || !GLEW_EXT_bindable_uniform || !GLEW_EXT_draw_instanced)
    {
        printf( "Required extensions not supported.\n");
        printf( "This sample requires:\n");
        printf( "  OpenGL version 2.0 or later\n");
        printf( "  GL_ARB_vertex_buffer_object\n");
        printf( "  GL_EXT_gpu_shader4\n");
        printf( "  GL_EXT_bindable_uniform\n");
        printf( "  GL_EXT_draw_instanced\n");
        return 1;
    }

     if (!initShaders())
    {
        return 1;
    }

    initUniformBuffers();
    initPatchGeometry();

    // Set GL state.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);

    // GFx modification: Initialize GFx system, and the GFxPlayer instance.
    Scaleform::SysAllocMalloc a;
    Scaleform::GFx::System gfxInit(&a);
    gfxplayer = new Scaleform::GFxPlayer();
    int gfxInitError = gfxplayer->Init();
    if (gfxInitError)
        return gfxInitError;

    // Run the main loop.
    glutMainLoop();

    // // GFx modification: Destroy GFxPlayer (GFxSystem will shutdown when gfxInit object goes out of scope).
    delete gfxplayer;
    gfxplayer = 0;

    return 0;
}
