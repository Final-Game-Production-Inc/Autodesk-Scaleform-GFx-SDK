REM This compiles the Vertex and Pixel shaders used in rendering the rotating cube. Note that fxc.exe must be in your path.
fxc /T vs_2_0 /Vn CubeVertexShader /Fh FxPlayerTextureInSWFD3D9_cubeshader.vsh.h FxPlayerTextureInSWFD3D9_cubeshader.vsh
fxc /T ps_2_0 /Vn CubePixelShader /Fh FxPlayerTextureInSWFD3D9_cubeshader.psh.h FxPlayerTextureInSWFD3D9_cubeshader.psh
type FxPlayerTextureInSWFD3D9_cubeshader.???.h > FxPlayerTextureInSWFD3D9_cubeshader.h
del FxPlayerTextureInSWFD3D9_cubeshader.???.h
