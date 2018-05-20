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
    float4 Eye;

    float4 AutoLightDir;
    float4 AutoLightFactor;

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

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float4 tangentW)
{
    // Uncompress each component from [0,1] to [-1,1].
    float3 normalT = 2.0f * normalMapSample - 1.0f;
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

float3 F_Schlick(float NdV, float3 specular)
{
    return specular + (1.0 - specular) * pow(1.0 - NdV, 5.0);
}

float D_Phong(float g, float NdH)
{
    float a = pow(8192.0, g);
    return (a + 2.0) / 8.0 * pow(NdH, a);
}

float LightAttenuation(float dist, float range)
{
    float attenuation = 1.0;
    attenuation = dist * dist / (range * range + 1.0);
    float att_s = 5;
    attenuation = 1.0 / (attenuation * att_s + 1.0);
    att_s = 1.0 / (att_s + 1.0);
    attenuation = attenuation - att_s;
    attenuation /= 1.0 - att_s;
    return clamp(attenuation, 0.0, 1.0);
}
