cbuffer VSConstants {
  float4x4 WorldViewProjMatrix : packoffset(c0);
}
void main(in  float3 InPosition   : POSITION,
          in  float4 InVertColor  : COLOR,
          out float4 OutVertColor : COLOR,
          out float4 OutPosition  : SV_Position)
{
    OutPosition  = mul(WorldViewProjMatrix, float4(InPosition.xyz, 1.0f));
    OutVertColor = InVertColor;
};
