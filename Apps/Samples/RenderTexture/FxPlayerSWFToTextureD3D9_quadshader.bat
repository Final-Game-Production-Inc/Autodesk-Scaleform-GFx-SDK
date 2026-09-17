REM This compiles the Vertex and Pixel shaders used in rendering the on-screen quad. Note that fxc.exe must be in your path.
fxc /T vs_2_0 /Vn QuadVertexShader /Fh FxPlayerSWFToTextureD3D9_quadshader.vsh.h FxPlayerSWFToTextureD3D9_quadshader.vsh
fxc /T ps_2_0 /Vn QuadPixelShader /Fh FxPlayerSWFToTextureD3D9_quadshader.psh.h FxPlayerSWFToTextureD3D9_quadshader.psh
type FxPlayerSWFToTextureD3D9_quadshader.???.h > FxPlayerSWFToTextureD3D9_quadshader.h
del FxPlayerSWFToTextureD3D9_quadshader.???.h
