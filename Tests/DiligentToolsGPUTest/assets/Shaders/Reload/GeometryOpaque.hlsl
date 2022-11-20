Texture2D    g_Tex;
SamplerState g_Tex_sampler;

void VSMain(uint VertexID       : SV_VertexID, 
            uint InstanceID     : SV_InstanceID,
            out float4 Position : SV_Position,
            out float4 Color    : VERTCOLOR,
            out float2 TexCoord : TEXCOORD)
{
    float2 VertexPositions[] = { float2(-0.5, -0.5), float2(+0.5, -0.5), float2(+0.0, +0.5) };

    float4 VertexColors[] = { float4(1.0, 0.0, 0.0, 1.0), float4(0.0, 1.0, 0.0, 1.0), float4(0.0, 0.0, 1.0, 1.0) };
    float2 InstancePositionOffsets[] = { float2(.0, 0.0), float2(0.5, 0.5), float2(-0.5, -0.5), float2(-0.5, 0.5), float2(0.5, -0.5) };

    Color = VertexColors[VertexID];
    Position = float4(VertexPositions[VertexID] + InstancePositionOffsets[InstanceID], 0.8, 1.0f);
    TexCoord = VertexPositions[VertexID] + float2(0.5, 0.5);
}

void PSMain(in  float4 Position : SV_Position, 
            in  float4 Color    : VERTCOLOR,
            in  float2 TexCoord : TEXCOORD,
            out float4 Color0   : SV_Target0,
            out float4 Color1   : SV_Target1)
{
    Color0 = Color;
    Color1 = g_Tex.Sample(g_Tex_sampler, TexCoord);
}
