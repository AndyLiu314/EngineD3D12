/*
* Root Signature Layout (CPU)
* - 0: float3 "Colour
* - 1: Correction "Correction Data"
* - 2: Descriptor Table "Textures"
*
* Root Signature Layout (GPU)
* - b0: float3 "Colour"
* - t0: Texture2D<float4> "Textures"
* - s0: Sampler for "Textures"
*/

#define ROOTSIG \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
"RootConstants(num32BitConstants=4, b0)," \
"RootConstants(num32BitConstants=4, b1)," \
"DescriptorTable(" \
    "SRV(t0, numDescriptors = unbounded)" \
")," \
"StaticSampler(s0)"