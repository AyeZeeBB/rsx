struct VS_Output
{
    float4 position : SV_POSITION;
    float3 worldPosition : POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

Texture2D baseTexture : register(t0);
SamplerState texSampler : register(s0);

cbuffer CameraConstants : register(b2)
{
    float c_gameTime;
    float3 c_cameraOrigin;
    float4x4 c_cameraRelativeToClip;
    int c_frameNum;
    float3 c_cameraOriginPrevFrame;
    float4x4 c_cameraRelativeToClipPrevFrame;
    float4 c_clipPlane;

    // Skip fog params (20 floats)
    float4 fogK0, fogK1, fogK2, fogK3, fogK4;

    float3 c_skyColor;
    float c_shadowBleedFudge;
    float c_envMapLightScale;
    float3 c_sunColor;
    float3 c_sunDir;
    float c_minShadowVariance;

    // Skip CSM constants (large block)
    float _csmPadding[64]; // Approximate size - we'll skip to lighting params

    // Lighting parameters at the end
    float c_sunIntensity;
    float c_ambientIntensity;
    float3 c_ambientColor;
    float c_specularPower;
    float c_enableLighting;
};

float4 ps_main(VS_Output input) : SV_Target
{
    float4 baseColor = baseTexture.Sample(texSampler, input.uv);

    // If lighting is disabled, return unlit texture
    if (c_enableLighting < 0.5)
    {
        return baseColor;
    }

    // Normalize the normal
    float3 normal = normalize(input.normal);

    // Calculate basic directional lighting
    float3 lightDir = normalize(-c_sunDir);
    float NdotL = max(0.0, dot(normal, lightDir));

    // Diffuse lighting
    float3 diffuse = c_sunColor * c_sunIntensity * NdotL;

    // Simple specular
    float3 viewDir = normalize(c_cameraOrigin - input.worldPosition);
    float3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), c_specularPower);
    float3 specular = c_sunColor * spec * 0.3; // Reduced specular intensity

    // Ambient lighting
    float3 ambient = c_ambientColor * c_ambientIntensity;

    // Combine lighting
    float3 finalColor = baseColor.rgb * (ambient + diffuse) + specular;

    return float4(finalColor, baseColor.a);
}