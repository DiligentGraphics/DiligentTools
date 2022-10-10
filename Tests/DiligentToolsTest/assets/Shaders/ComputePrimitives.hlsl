#include "Common.hlsli"

RWTexture2D<float /*format = r32f*/> TextureUAV;
RWBuffer<uint /*format = r32ui*/> BufferUAV;

[numthreads(8, 8, 1)]
void CSClearUnorderedAccessViewUint(uint3 Id: SV_DispatchThreadID)
{
    TextureUAV[Id.xy] = TEXTURE_FLOAT_CLEAR;
}

[numthreads(1, 1, 1)]
void CSClearBufferCounter(uint3 Id: SV_DispatchThreadID)
{
    BufferUAV[0] = BUFFER_UINT_CLEAR;
}
