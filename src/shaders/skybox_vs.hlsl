struct VS_Input
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD0;
};

struct VS_Output
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

cbuffer VS_TransformConstants : register(b0)
{
    float4x4 modelMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

VS_Output vs_main(VS_Input input)
{
    VS_Output output;

    // Remove translation from view matrix for skybox effect
    float4x4 viewNoTranslation = viewMatrix;
    viewNoTranslation._41 = 0.0;
    viewNoTranslation._42 = 0.0;
    viewNoTranslation._43 = 0.0;

    // Transform position
    output.position = mul(float4(input.position, 1.0), modelMatrix);
    output.position = mul(output.position, viewNoTranslation);
    output.position = mul(output.position, projectionMatrix);

    // Set z = w to ensure the skybox is rendered at maximum depth
    output.position.z = output.position.w * 0.9999f; // Slightly closer than far plane

    // Pass through texture coordinates for the sphere
    output.texCoord = input.texCoord;

    return output;
}