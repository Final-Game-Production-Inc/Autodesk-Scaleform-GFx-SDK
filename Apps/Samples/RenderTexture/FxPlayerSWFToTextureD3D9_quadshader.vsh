float4x4 WorldViewProjMatrix : register(c0);
void main(in  float4 InPosition   : POSITION,
          in  float2 InUV         : TEXCOORD0,
          out float2 OutUV        : TEXCOORD0,
          out float4 OutPosition  : POSITION)
{
    OutPosition  = mul(WorldViewProjMatrix, InPosition);
    OutUV = InUV;
};
