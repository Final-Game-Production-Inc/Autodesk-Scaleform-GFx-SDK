float4x4 WorldViewProjMatrix       : register(c0);
void main(in float4 InPosition     : POSITION,
          in float4 InVertColor    : COLOR,    
          out float4 OutPosition   : POSITION,
          out float4 OutVertColor  : COLOR)
{
    OutPosition  = mul(WorldViewProjMatrix, InPosition);
    OutVertColor = InVertColor;
};
