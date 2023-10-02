cbuffer ModelViewProjecion : register(b0)
{
    float4x4  MVP;
};

struct PSInput
{
    float4 position : SV_POSITION;
};

PSInput VSMain(float4 position : POSITION, float2 coord : COORD)
{
    PSInput result;

    result.position = mul(position, MVP);

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(1.f,1.f,0.f,1.f);
}