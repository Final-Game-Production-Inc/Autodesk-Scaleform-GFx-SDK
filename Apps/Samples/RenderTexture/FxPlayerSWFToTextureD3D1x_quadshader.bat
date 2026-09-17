REM This compiles the Vertex and Pixel shaders used in rendering the on-screen quad. Note that fxc.exe must be in your path.
fxc /T vs_4_0 /Vn QuadVertexShader /Fh FxPlayerSWFToTextureD3D1x_quadshader.vsh.h FxPlayerSWFToTextureD3D1x_quadshader.vsh
fxc /T ps_4_0 /Vn QuadPixelShader /Fh FxPlayerSWFToTextureD3D1x_quadshader.psh.h FxPlayerSWFToTextureD3D1x_quadshader.psh
type FxPlayerSWFToTextureD3D1x_quadshader.???.h > FxPlayerSWFToTextureD3D1x_quadshader.h
del FxPlayerSWFToTextureD3D1x_quadshader.???.h
