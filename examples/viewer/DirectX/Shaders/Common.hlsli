// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
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

TextureCube DiffuseEnvMap : register(t0);
TextureCube SpecularEnvMap : register(t1);
Texture2D BRDF_LUT : register(t2);

Texture2D Textures[64] : register(t3);

SamplerState SampAnisotropicWrap : register(s0);
SamplerState SampLinearClamp : register(s1);

//--------------------------------------------------------------------------------------
static const float M_PI = 3.141592653589793f;
static const float MinRoughness = 0.04f;

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

float3 F_Schlick(float3 r0, float3 f90, float LdH)
{
    return r0 + (f90 - r0) * pow(clamp(1.0 - LdH, 0.0, 1.0), 5.0);
}

float G_Smith(float NdL, float NdV, float alphaRoughness)
{
    float a2 = alphaRoughness * alphaRoughness;

    float gl = NdL + sqrt(a2 + (1.0 - a2) * (NdL * NdL));
    float gv = NdV + sqrt(a2 + (1.0 - a2) * (NdV * NdV));
    return 1.0f / (gl * gv); // The division by (4.0 * NdL * NdV) is unneeded with this form
}

float D_GGX(float NdH, float alphaRoughness)
{
    float a2 = alphaRoughness * alphaRoughness;
    float f = (NdH * a2 - NdH) * NdH + 1.0;
    return a2 / (M_PI * f * f);
}

float3 IBLContribution(float3 diffuseColor, float3 specularColor, float perceptualRoughness, float NdV, float3 N, float3 reflection)
{
    float mipCount = 7.0f;
    float lod = (perceptualRoughness * mipCount);
    float3 brdf = SRGBtoLINEAR(BRDF_LUT.Sample(SampLinearClamp, float2(NdV, 1.0 - perceptualRoughness))).rgb;
    float3 diffuseLight = SRGBtoLINEAR(DiffuseEnvMap.Sample(SampAnisotropicWrap, N)).rgb;

    float3 specularLight = SRGBtoLINEAR(SpecularEnvMap.SampleLevel(SampAnisotropicWrap, reflection, lod)).rgb;

    float3 diffuse = diffuseLight * diffuseColor;
    float3 specular = specularLight * (specularColor * brdf.x + brdf.y);

    return diffuse + specular;
}