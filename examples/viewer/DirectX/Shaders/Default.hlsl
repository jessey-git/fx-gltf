// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------

#include "Common.hlsli"

struct VS_INPUT
{
    float3 Pos      : POSITION;
    float3 Normal   : NORMAL;
    float4 Tangent  : TANGENT;
    float2 TexC     : TEXCOORD;
};

struct PS_INPUT
{
    float4 PosH     : SV_POSITION;
    float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
    float4 TangentW : TANGENT;
    float2 TexC     : TEXCOORD;
};

//--------------------------------------------------------------------------------------

PS_INPUT StandardVS(VS_INPUT input)
{
    PS_INPUT output;

    // Transform to world space + homogeneous clip space...
    output.PosW = mul(input.Pos, (float3x3)World);
    output.PosH = mul(float4(input.Pos, 1.0f), WorldViewProj);

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    output.NormalW = mul(input.Normal, (float3x3)World);
    output.TangentW = mul(input.Tangent, World);
    output.TexC = input.TexC;

    return output;
}

//--------------------------------------------------------------------------------------
float4 UberPS(PS_INPUT input)
    : SV_Target
{
    MaterialData matData = MaterialDataBuffer[MaterialIndex];

    input.NormalW = normalize(input.NormalW);
    //input.TangentW = normalize(input.TangentW);

    float4 finalColor;

#ifdef USE_AUTO_COLOR
    finalColor = matData.MeshAutoColor;
    finalColor += saturate(dot((float3)AutoLightDir, input.NormalW) * AutoLightFactor);
#else
    float4 diffuseSample = Textures[matData.BaseColorIndex].Sample(SampAnisotropicWrap, input.TexC);
    float4 normalMapSample = Textures[matData.NormalIndex].Sample(SampAnisotropicWrap, input.TexC);
    float4 metalRoughSample = Textures[matData.MetalRoughIndex].Sample(SampAnisotropicWrap, input.TexC);

    float3 albedoColor = (diffuseSample * matData.BaseColorFactor).rgb;
    float roughness = 1.0f - (metalRoughSample.g * matData.RoughnessFactor);
    float metallic = metalRoughSample.b * matData.MetallicFactor;

    float3 baseColor = albedoColor.rgb;
    albedoColor.rgb = baseColor * (1.0f - metallic);
    float3 specular = lerp(0.04f, baseColor, metallic);

    normalMapSample.g = 1.0f - normalMapSample.g;
    float3 N = NormalSampleToWorldSpace(normalMapSample.rgb, input.NormalW, input.TangentW);

    float3 V = normalize((float3)Eye - input.PosW);

    float3 diffuseTerm = 0;
    float3 specularTerm = 0;

    float NdV = clamp(dot(N, V), 0, 1);
    float3 fresnelTerm = F_Schlick(NdV, specular);

    for (int i = 0; i < 1; i++)
    {
        float3 lightPosition = PointLights[i].Position;
        float3 lc = 1;
        float range = PointLights[i].FalloffEnd - PointLights[i].FalloffStart;

        float3 L = lightPosition - input.PosW;

        // Calculate point light attenuation
        float dist = length(L);
        float attenuation = LightAttenuation(dist, range);
        L /= dist;
        float3 H = normalize(L + V);
        float NdL = clamp(dot(N, L), 0.0, 1.0);
        float NdH = clamp(dot(N, H), 0.0, 1.0);

        float shadowContrib = 1.0;

        float3 li = lc * NdL * attenuation * shadowContrib;
        diffuseTerm += li;
        specularTerm += li * fresnelTerm * D_Phong(roughness, NdH);
    }

    finalColor.rgb = albedoColor * max(diffuseTerm, 0.0) + max(specularTerm, 0.0);
    finalColor.a = 1.0f;

#endif

    return finalColor;
}
