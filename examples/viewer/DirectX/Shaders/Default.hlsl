// ------------------------------------------------------------
// Copyright(c) 2018-2021 Jesse Yurkovich
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
    float3 TangentW : TANGENT;
    float3 BinormalW: BINORMAL;
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
    output.TangentW = mul(input.Tangent.xyz, (float3x3)World);
    output.BinormalW = cross(output.NormalW, output.TangentW) * input.Tangent.w;
    output.TexC = input.TexC;

    return output;
}

//--------------------------------------------------------------------------------------

float4 UberPS(PS_INPUT input)
    : SV_Target
{
    MaterialData matData = MaterialDataBuffer[MaterialIndex];

    input.NormalW = normalize(input.NormalW);

    float4 finalColor;

#if USE_AUTO_COLOR
    finalColor = matData.MeshAutoColor;
    finalColor += float4(saturate(dot(DirectionalLight.Direction, input.NormalW) * DirectionalLight.Strength), 1.0f);
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

    float3 f0 = MinRoughness;
    float3 diffuseColor = baseColor.rgb * (1.0 - f0) * (1.0 - metallic);
    float3 specularColor = lerp(f0, baseColor.rgb, metallic);

    float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);

    // For typical incident reflectance range (between 4% to 100%) set the grazing reflectance to 100% for typical fresnel effect.
    // For very low reflectance range on highly diffuse objects (below 4%), incrementally reduce grazing reflecance to 0%.
    float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
    float3 specularEnvironmentR0 = specularColor.rgb;
    float3 specularEnvironmentR90 = reflectance90;

#if HAS_TANGENTS
    float3x3 TBN = float3x3(normalize(input.TangentW), normalize(input.BinormalW), input.NormalW);
#else
    float3 pos_dx = ddx(input.PosW);
    float3 pos_dy = ddy(input.PosW);
    float3 tex_dx = ddx(float3(input.TexC, 0.0));
    float3 tex_dy = ddy(float3(input.TexC, 0.0));
    float3 t = (tex_dy.y * pos_dx - tex_dx.y * pos_dy) / (tex_dx.x * tex_dy.y - tex_dy.x * tex_dx.y);

    float3 n = input.NormalW;
    t = normalize(t - n * dot(n, t));
    float3 b = normalize(cross(n, t));

    float3x3 TBN = float3x3(t, b, n);
#endif

#if HAS_NORMALMAP
    float4 normalMapSample = Textures[matData.NormalIndex].Sample(SampAnisotropicWrap, input.TexC);
    normalMapSample.g = 1.0f - normalMapSample.g;

    float3 normal = (2.0f * normalMapSample.rgb - 1.0f) * float3(matData.NormalScale, matData.NormalScale, 1.0f);
    float3 N = normalize(mul(normal, TBN));
#else
    float3 N = TBN[2].xyz;
#endif

    float3 V = normalize(Camera.xyz - input.PosW); // Vector from surface point to camera
    float3 L = normalize(DirectionalLight.Direction); // Vector from surface point to light
    float3 H = normalize(L + V); // Half vector between both l and v
    float3 reflection = -normalize(reflect(V, N));

    float NdL = clamp(dot(N, L), 0.001, 1.0);
    float NdV = clamp(abs(dot(N, V)), 0.001, 1.0);
    float NdH = clamp(dot(N, H), 0.0, 1.0);
    float LdH = clamp(dot(L, H), 0.0, 1.0);

    // Calculate the shading terms for the microfacet specular shading model
    float3 F = F_Schlick(specularEnvironmentR0, specularEnvironmentR90, LdH);
    float G = G_Smith(NdL, NdV, alphaRoughness);
    float D = D_GGX(NdH, alphaRoughness);

    // Calculation of analytical lighting contribution
    float3 diffuseContrib = (1.0 - F) * Diffuse_Lambert(diffuseColor);
    float3 specContrib = F * (G * D);
    float3 color = NdL * (diffuseContrib + specContrib);

    // Calculate lighting contribution from image based lighting source (IBL)
#if USE_IBL
    color += IBLContribution(diffuseColor, specularColor, perceptualRoughness, NdV, N, reflection);
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

float4 GroundPS(PS_INPUT input)
    : SV_Target
{
    const float3 gridColorMajor = 0.0f;
    const float3 gridColorMinor = 0.3f;
    const float gridSizeMajor = 4;
    const float gridSizeMinor = 1;
    const float epsilon = 0.05f;

    float4 finalColor = 0.98f;

    float wx = input.PosW.x;
    float wz = input.PosW.z;
    float x0 = abs(frac(wx / gridSizeMajor - 0.5) - 0.5) / fwidth(wx) * gridSizeMajor / 2.0;
    float z0 = abs(frac(wz / gridSizeMajor - 0.5) - 0.5) / fwidth(wz) * gridSizeMajor / 2.0;

    float x1 = abs(frac(wx / gridSizeMinor - 0.5) - 0.5) / fwidth(wx) * gridSizeMinor;
    float z1 = abs(frac(wz / gridSizeMinor - 0.5) - 0.5) / fwidth(wz) * gridSizeMinor;

    float v0 = 1.0 - clamp(min(x0, z0), 0.0, 1.0);
    float v1 = 1.0 - clamp(min(x1, z1), 0.0, 1.0);

    if (v0 > epsilon)
    {
        finalColor.rgb = lerp(finalColor.rgb, gridColorMajor, v0);
    }
    else
    {
        finalColor.rgb = lerp(finalColor.rgb, gridColorMinor, v1);
    }

    float awx = abs(wx);
    float awz = abs(wz);
    if (awx < epsilon)
    {
        finalColor.rgb = lerp(float3(0, 1, 0), finalColor.rgb, awx / epsilon);
    }
    else if (awz < epsilon)
    {
        finalColor.rgb = lerp(float3(1, 0, 0), finalColor.rgb, awz / epsilon);
    }

    finalColor.a *= 1.0 - clamp((length(input.PosW.xz) - 20) / 25.0, 0.1, 1.0);
    return finalColor;
}
