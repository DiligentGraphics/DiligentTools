#include "Common.hlsli"

[numthreads(1, 1, 1)]
void CSClearBufferCounter(uint3 Id: SV_DispatchThreadID) {
    BufferUAV[0] = BUFFER_UINT_CLEAR;
}
