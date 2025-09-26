struct VS_Output
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
};

Texture2D skyboxTexture : register(t0);
SamplerState skyboxSampler : register(s0);

float4 ps_main(VS_Output input) : SV_Target
{
    // Sample the 2D texture using UV coordinates
    return skyboxTexture.Sample(skyboxSampler, input.texCoord);
}