#include "Common.hlsli"

Texture2D<float4> TextureSRV;
SamplerState TextureSRV_sampler;

void VSBlitTexture(uint Id: SV_VertexID, out float4 Position: SV_Position, out float2 Texcoord: TEXCOORD)
{
    Texcoord = float2((Id << 1) & 2, Id & 2);
    Position = float4(Texcoord * float2(2, -2) + float2(-1, 1), 1, 1);
}

float4 PSBlitTexture(float4 Position: SV_Position, float2 Texcoord: TEXCOORD): SV_TARGET0
{
    return TextureSRV.Sample(TextureSRV_sampler, Texcoord);
}

