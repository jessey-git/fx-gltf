// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------

struct Light
{
    float3 Strength;
    float FalloffStart; // point/spot light only
    float3 Direction;   // directional/spot light only
    float FalloffEnd;   // point/spot light only
    float3 Position;    // point light only
    float SpotPower;    // spot light only
};

cbuffer SceneConstants : register(b0)
{
    float4x4 ViewProj;
    float4 Camera;

    Light DirectionalLight;
    Light PointLights[2];
};

cbuffer MeshConstants : register(b1)
{
    float4x4 WorldViewProj;
    float4x4 World;

    int MaterialIndex;
};

struct MaterialData
{
    float4 MeshAutoColor;

    int BaseColorIndex;
    float4 BaseColorFactor;

    int NormalIndex;
    float NormalScale;

    int MetalRoughIndex;
    float RoughnessFactor;
    float MetallicFactor;

    int AOIndex;
    float AOStrength;

    int EmissiveIndex;
    float3 EmissiveFactor;
};

StructuredBuffer<MaterialData> MaterialDataBuffer : register(t0, space1);

Texture2D Environment  : register(t0);
Texture2D Textures[64] : register(t1);

SamplerState SampAnisotropicWrap : register(s0);

//--------------------------------------------------------------------------------------
static const float M_PI = 3.141592653589793f;
static const float MinRoughness = 0.04f;

float3 NormalToWorld(float3 normalMapSample, float normalScale, float3 unitNormalW, float4 tangentW)
{
    // Uncompress each component from [0,1] to [-1,1].
    float3 normalT = (2.0f * normalMapSample - 1.0f) * float3(normalScale, normalScale, 1.0f);
    float3 tangent3 = tangentW.xyz;

    // Build orthonormal basis.
    float3 N = unitNormalW;
    float3 T = normalize(tangent3 - dot(tangent3, N) * N);
    float3 B = cross(N, T) * tangentW.w;

    float3x3 TBN = float3x3(T, B, N);

    // Transform from tangent space to world space.
    float3 bumpedNormalW = mul(normalT, TBN);

    return bumpedNormalW;
}

float4 SRGBtoLINEAR(float4 srgbColor)
{
#if MANUAL_SRGB
    return float4(pow(srgbColor.xyz, 2.2), srgbColor.w);
#else
    return srgbColor;
#endif
}

float3 LINEARtoSRGB(float3 linearColor)
{
#if MANUAL_SRGB
    return pow(linearColor, 1.0 / 2.2);
#else
    return linearColor;
#endif

}

float3 Diffuse_Lambert(float3 diffuseColor)
{
    return diffuseColor / M_PI;
}

float3 F_Schlick(float3 r0, float3 f90, float VdH)
{
    return r0 + (f90 - r0) * pow(clamp(1.0 - VdH, 0.0, 1.0), 5.0);
}

float G_Smith(float NdL, float NdV, float alphaRoughness)
{
    float r = alphaRoughness;

    float attenuationL = 2.0 * NdL / (NdL + sqrt(r * r + (1.0 - r * r) * (NdL * NdL)));
    float attenuationV = 2.0 * NdV / (NdV + sqrt(r * r + (1.0 - r * r) * (NdV * NdV)));
    return attenuationL * attenuationV;
}

float D_GGX(float NdH, float alphaRoughness)
{
    float roughnessSq = alphaRoughness * alphaRoughness;
    float f = (NdH * roughnessSq - NdH) * NdH + 1.0;
    return roughnessSq / (M_PI * f * f);
}
