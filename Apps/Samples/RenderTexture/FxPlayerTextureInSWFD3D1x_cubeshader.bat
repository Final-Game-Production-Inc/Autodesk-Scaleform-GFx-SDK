REM This compiles the Vertex and Pixel shaders used in rendering the rotating cube. Note that fxc.exe must be in your path.
fxc /T vs_4_0 /Vn CubeVertexShader /Fh FxPlayerTextureInSWFD3D1x_cubeshader.vsh.h FxPlayerTextureInSWFD3D1x_cubeshader.vsh
fxc /T ps_4_0 /Vn CubePixelShader /Fh FxPlayerTextureInSWFD3D1x_cubeshader.psh.h FxPlayerTextureInSWFD3D1x_cubeshader.psh
type FxPlayerTextureInSWFD3D1x_cubeshader.???.h > FxPlayerTextureInSWFD3D1x_cubeshader.h
del FxPlayerTextureInSWFD3D1x_cubeshader.???.h
