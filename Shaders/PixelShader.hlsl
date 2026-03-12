#include "RootSignature.hlsl"

float3 colour : register(b0);

[RootSignature(ROOTSIG)]
float4 main() : SV_Target
{
    return float4(colour, 1.0f);
}
