// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------

#include "Common.hlsli"

struct VS_INPUT
{
    float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
    float2 TexC    : TEXCOORD;
};

struct PS_INPUT
{
    float4 PosH : SV_POSITION;
    float3 PosL : POSITION;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;

    // Use local vertex position as cubemap lookup vector.
    output.PosL = input.PosL;

    // Transform to world space.
    float4 posW = mul(float4(input.PosL, 1.0f), World);

    // Always center sky about camera.
    posW.xyz += Camera.xyz;

    // Set z = w so that z/w = 1 (i.e., skydome always on far plane).
    output.PosH = mul(posW, ViewProj).xyww;

    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
    return Environment.Sample(SampAnisotropicWrap, input.PosL.xy);
}
