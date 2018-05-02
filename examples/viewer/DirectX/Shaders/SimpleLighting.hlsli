//--------------------------------------------------------------------------------------
// SimpleLighting.hlsl
//
// Shader demonstrating Lambertian lighting from multiple sources
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer SceneConstants : register(b0)
{
    float4   lightDir[2];
    float4   lightColor[2];
};

cbuffer MeshConstants : register(b1)
{
    float4x4 mWorldViewProj;
    float4x4 mWorld;
    float3 mMeshColor;
};


//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos      : POSITION;
    float3 Normal   : NORMAL;
    float2 TexC     : TEXCOORD;
};

struct PS_INPUT
{
    float4 Pos      : SV_POSITION;
    float3 Normal   : NORMAL;
};


//--------------------------------------------------------------------------------------
// Name: TriangleVS
// Desc: Vertex shader
//--------------------------------------------------------------------------------------
PS_INPUT TriangleVS(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul(input.Pos, mWorldViewProj);
    output.Normal = mul(input.Normal, ((float3x3) mWorld));

    return output;
}


//--------------------------------------------------------------------------------------
// Name: TrianglePS
// Desc: Pixel shader applying Lambertian lighting from two lights
//--------------------------------------------------------------------------------------
float4 LambertPS(PS_INPUT input) : SV_Target
{
    float4 finalColor = float4(mMeshColor, 1.0f);

    input.Normal = normalize(input.Normal);
    finalColor += saturate(dot((float3) lightDir[0], input.Normal) * lightColor[0]);
    //finalColor += saturate(dot((float3) lightDir[1], input.Normal) * lightColor[1]);

    return finalColor;
}

