cbuffer VSConstants {
  float4x4 WorldViewProjMatrix : packoffset(c0);
}
void main(in  float3 InPosition   : POSITION,
          in  float2 InUV         : TEXCOORD0,
          out float2 OutUV        : TEXCOORD0,
          out float4 OutPosition  : SV_Position)
{
    OutPosition  = mul(WorldViewProjMatrix, float4(InPosition.xyz, 1.0f));
    OutUV = InUV;
};
