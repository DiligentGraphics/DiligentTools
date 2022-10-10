#define TEXTURE_FLOAT_CLEAR 0.1234
#define BUFFER_UINT_CLEAR   0

uint PackColor(float4 Color)
{
    return (uint(Color.r * 255) << 24) | (uint(Color.g * 255) << 16) | (uint(Color.b * 255) << 8) | uint(Color.a * 255);
}

float4 UnpackColor(uint Color)
{
    float4 Result;
    Result.r = float((Color >> 24) & 0x000000FF) / 255.0f;
    Result.g = float((Color >> 16) & 0x000000FF) / 255.0f;
    Result.b = float((Color >> 8) & 0x000000FF) / 255.0f;
    Result.a = float((Color >> 0) & 0x000000FF) / 255.0f;
    return saturate(Result);
}
