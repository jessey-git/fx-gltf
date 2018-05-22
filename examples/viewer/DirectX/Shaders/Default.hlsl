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

#if USE_AUTO_COLOR
    finalColor = matData.MeshAutoColor;
    finalColor += saturate(dot(float4(DirectionalLight.Direction, 1.0f), input.NormalW) * float4(DirectionalLight.Strength, 1.0f));
#else

#if HAS_BASECOLORMAP
    float4 baseColor = SRGBtoLINEAR(Textures[matData.BaseColorIndex].Sample(SampAnisotropicWrap, input.TexC)) * matData.BaseColorFactor;
#else
    float4 baseColor = matData.BaseColorFactor;
#endif

#if HAS_METALROUGHNESSMAP
    float4 metalRoughSample = Textures[matData.MetalRoughIndex].Sample(SampAnisotropicWrap, input.TexC);
    float perceptualRoughness = metalRoughSample.g * matData.RoughnessFactor;
    float metallic = metalRoughSample.b * matData.MetallicFactor;
#else
    float perceptualRoughness = matData.RoughnessFactor;
    float metallic = matData.MetallicFactor;
#endif

    perceptualRoughness = clamp(perceptualRoughness, MinRoughness, 1.0);
    metallic = clamp(metallic, 0.0, 1.0);

    float alphaRoughness = perceptualRoughness * perceptualRoughness;

    float3 f0 = 0.04;
    float3 diffuseColor = baseColor.rgb * (1.0 - f0);
    diffuseColor *= 1.0 - metallic;
    float3 specularColor = lerp(f0, baseColor.rgb, metallic);

    // Compute reflectance.
    float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);

    // For typical incident reflectance range (between 4% to 100%) set the grazing reflectance to 100% for typical fresnel effect.
    // For very low reflectance range on highly diffuse objects (below 4%), incrementally reduce grazing reflecance to 0%.
    float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
    float3 specularEnvironmentR0 = specularColor.rgb;
    float3 specularEnvironmentR90 = reflectance90;

#if HAS_NORMALMAP
    float4 normalMapSample = Textures[matData.NormalIndex].Sample(SampAnisotropicWrap, input.TexC);
    normalMapSample.g = 1.0f - normalMapSample.g;
    float3 N = NormalToWorld(normalMapSample.rgb, matData.NormalScale, input.NormalW, input.TangentW);
#else
    float3 N = input.NormalW;
#endif

    float3 V = normalize(Camera.xyz - input.PosW); // Vector from surface point to camera
    float3 L = normalize(DirectionalLight.Direction); // Vector from surface point to light
    float3 H = normalize(L + V); // Half vector between both l and v
    float3 reflection = -normalize(reflect(V, N));

    float NdL = clamp(dot(N, L), 0.001, 1.0);
    float NdV = abs(dot(N, V)) + 0.001;
    float NdH = clamp(dot(N, H), 0.0, 1.0);
    float LdH = clamp(dot(L, H), 0.0, 1.0);
    float VdH = clamp(dot(V, H), 0.0, 1.0);

    // Calculate the shading terms for the microfacet specular shading model
    float3 F = F_Schlick(specularEnvironmentR0, specularEnvironmentR90, VdH);
    float G = G_Smith(NdL, NdV, alphaRoughness);
    float D = D_GGX(NdH, alphaRoughness);

    // Calculation of analytical lighting contribution
    float3 diffuseContrib = (1.0 - F) * Diffuse_Lambert(diffuseColor);
    float3 specContrib = F * G * D / (4.0 * NdL * NdV);
    // Obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cosine law)
    float3 color = NdL * 3 * (diffuseContrib + specContrib);

    // Calculate lighting contribution from image based lighting source (IBL)
#if USE_IBL
    color += getIBLContribution(pbrInputs, n, reflection);
#endif

    // Apply optional PBR terms for additional (optional) shading
#if HAS_OCCLUSIONMAP
#if HAS_OCCLUSIONMAP_COMBINED
    float ao = metalRoughSample.r;
#else
    float ao = Textures[matData.AOIndex].Sample(SampAnisotropicWrap, input.TexC).r;
#endif

    color = lerp(color, color * ao, matData.AOStrength);
#endif

#if HAS_EMISSIVEMAP
    float3 emissive = SRGBtoLINEAR(Textures[matData.EmissiveIndex].Sample(SampAnisotropicWrap, input.TexC)).rgb * matData.EmissiveFactor;
    color += emissive;
#endif

    finalColor = float4(LINEARtoSRGB(color), baseColor.a);
#endif

    return finalColor;
}
