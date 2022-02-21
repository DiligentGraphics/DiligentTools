#include "Common.hlsli"

[numthreads(1, 1, 1)]
void BrokenShaderCS(uint3 Id: SV_DispatchThreadID) {
    BufferUAV[0] = BUFFER_UINT_CLEAR;
}
