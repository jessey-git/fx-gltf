// ------------------------------------------------------------
// Copyright(c) 2018 Jesse Yurkovich
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// See the LICENSE file in the repo root for full license information.
// ------------------------------------------------------------

cbuffer SceneConstants : register(b0)
{
    float4  Eye;

    float4  LightDir[2];
    float4  LightColor[2];
};

cbuffer MeshConstants : register(b1)
{
    float4x4    WorldViewProj;
    float4x4    World;

    int         MaterialIndex;
};

struct MaterialData
{
    float4 MeshAutoColor;

    int DiffuseIndex;
    float4 DiffuseFactor;

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

Texture2D Textures[64]: register(t0);
SamplerState SampAnisotropicWrap : register(s0);

//--------------------------------------------------------------------------------------
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
    float3 Normal   : NORMAL;
    float4 Tangent  : TANGENT;
    float2 TexC     : TEXCOORD;
};

//--------------------------------------------------------------------------------------

PS_INPUT TriangleVS(VS_INPUT input)
{
    PS_INPUT output;

    // Transform to world space + homogeneous clip space...
    output.PosW = mul(input.Pos, (float3x3)World);
    output.PosH = mul(float4(input.Pos, 1.0f), WorldViewProj);

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    output.Normal = mul(input.Normal, (float3x3)World);
    output.Tangent = mul(input.Tangent, World);
    output.TexC = input.TexC;

    return output;
}

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

struct MaterialProperty
{
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float Shininess;
};

struct Light
{
    float3 Strength;
    float FalloffStart; // point/spot light only
    float3 Direction;   // directional/spot light only
    float FalloffEnd;   // point/spot light only
    float3 Position;    // point light only
    float SpotPower;    // spot light only
};

float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec)
{
    float cosIncidentAngle = saturate(dot(normal, lightVec));

    float f0 = 1.0f - cosIncidentAngle;
    float3 reflectPercent = R0 + (1.0f - R0) * (f0 * f0 * f0 * f0 * f0);

    return reflectPercent;
}

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal, float3 toEye, MaterialProperty mat)
{
    const float m = mat.Shininess * 256.0f;
    float3 halfVec = normalize(toEye + lightVec);

    float roughnessFactor = (m + 8.0f) * pow(max(dot(halfVec, normal), 0.0f), m) / 8.0f;
    float3 fresnelFactor = SchlickFresnel(mat.FresnelR0, halfVec, lightVec);

    float3 specAlbedo = fresnelFactor * roughnessFactor;

    // Our spec formula goes outside [0,1] range, but we are 
    // doing LDR rendering.  So scale it down a bit.
    specAlbedo = specAlbedo / (specAlbedo + 1.0f);

    return (mat.DiffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    // Linear falloff.
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

float3 ComputePointLight(Light L, MaterialProperty mat, float3 pos, float3 normal, float3 toEye)
{
    // The vector from the surface to the light.
    float3 lightVec = L.Position - pos;

    // The distance from surface to light.
    float d = length(lightVec);

    // Range test.
    if (d > L.FalloffEnd)
        return 0.0f;

    // Normalize the light vector.
    lightVec /= d;

    // Scale light down by Lambert's cosine law.
    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.Strength * ndotl;

    // Attenuate light by distance.
    float att = CalcAttenuation(d, L.FalloffStart, L.FalloffEnd);
    lightStrength *= att;

    return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

float4 ComputeLighting(Light gLights[1], MaterialProperty mat, float3 pos, float3 normal, float3 toEye, float3 shadowFactor)
{
    float3 result = 0.0f;

    int i = 0;

    result += ComputePointLight(gLights[i], mat, pos, normal, toEye);

    return float4(result, 0.0f);
}

float4 UberPS(PS_INPUT input) : SV_Target
{
    MaterialData matData = MaterialDataBuffer[MaterialIndex];

    input.Normal = normalize(input.Normal);

    float4 finalColor;

#ifdef USE_AUTO_COLOR
    finalColor = matData.MeshAutoColor;
    finalColor += saturate(dot((float3)LightDir[0], input.Normal) * LightColor[0]);
#else
    float4 diffuseAlbedo = matData.DiffuseFactor;
    float roughness = 0.9f; //matData.RoughnessFactor;

    float3 fresnelR0 = float3(0.1f, 0.1f, 0.1f);

    float4 normalMapSample = Textures[matData.NormalIndex].Sample(SampAnisotropicWrap, input.TexC);
    normalMapSample.g = 1.0f - normalMapSample.g;
    float3 bumpedNormal = NormalSampleToWorldSpace(normalMapSample.rgb, input.Normal, input.Tangent);

    diffuseAlbedo *= Textures[matData.DiffuseIndex].Sample(SampAnisotropicWrap, input.TexC);

    float3 toEye = normalize((float3)Eye - input.PosW);

    float4 ambient = float4(0.8f, 0.8f, 0.8f, 1) * diffuseAlbedo;

    const float shininess = (1.0f - roughness) * normalMapSample.a;
    MaterialProperty mat = { diffuseAlbedo, fresnelR0, shininess };
    Light lights[1];
    lights[0].Position = float3(-3, 2, 5);
    lights[0].Strength = 0.25f;
    lights[0].FalloffEnd = 20.0f;
    float3 shadowFactor = 1.0f;
    float4 directLight = ComputeLighting(lights, mat, input.PosW, bumpedNormal, toEye, shadowFactor);

    finalColor = ambient + directLight;

    //// Add in specular reflections.
    //float3 r = reflect(-toEye, bumpedNormal);
    //float4 reflectionColor = float4(1, 1, 1, 1);// gCubeMap.Sample(gsamLinearWrap, r);
    //float3 fresnelFactor = SchlickFresnel(fresnelR0, bumpedNormal, r);
    //finalColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;

    // Common convention to take alpha from diffuse albedo.
    finalColor.a = diffuseAlbedo.a;
#endif

    return finalColor;
}

