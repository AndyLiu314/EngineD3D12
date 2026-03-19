#include "RootSignature.hlsl"
#include "Pipeline.hlsli"

Correction correction : register(b1);

[RootSignature(ROOTSIG)]
void main(
    // == IN ==
    in float2 pos : Position,
    in float2 uv : Texcoord,

    // == OUT ==
    out float2 o_uv : Texcoord,
    out float4 o_pos : SV_Position
)
{
    // Rules of transformation: Model -> View -> Projection
    float2 px;
    px.x = (pos.x * correction.cosAngle) - (pos.y * correction.sinAngle); // Model
    px.y = (pos.x * correction.sinAngle) + (pos.y * correction.cosAngle);
    px *= correction.zoom; // View
    px.x *= correction.aspecRatio; // Projection
    
    o_pos = float4(px, 0.0f, 1.0f);
    o_uv = uv;
}
