#shader vertex 
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

out vec2 TexCoords;

void main()
{
    TexCoords = uv;
    gl_Position = vec4(position, 1.0);
}

#shader fragment
#version 330 core
out vec4 pixCol;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gLightSpacePosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

uniform sampler2D ssaoScalarTex;
uniform sampler2D shadowMap;

uniform int useSSAO;

uniform vec3 lPos;
uniform vec3 lCol;

void main()
{
    // retrieve data from gbuffer
    vec3 pixPos = texture(gPosition, TexCoords).rgb;
    vec3 pixNorm = texture(gNormal, TexCoords).rgb;
    //basically tells me where we have geometry at all. 
    vec3 pixAlbedo = texture(gAlbedo, TexCoords).rgb;
    vec3 pixPosLightSpace = texture(gLightSpacePosition, TexCoords).rgb;

    vec3 normal = normalize(pixNorm);

    // ambient occlusion from texture
    float AmbientOcclusion = texture(ssaoScalarTex, TexCoords).r;
    if (useSSAO == 0)
    {
        AmbientOcclusion = 1.0;
    }

    // then calculate lighting as usual
    vec3 ambient = vec3(0.3 * pixAlbedo * AmbientOcclusion);
    vec3 lighting = ambient;
    vec3 viewDir = normalize(-pixPos); // viewpos is (0.0.0)
    // diffuse
    vec3 lightDir = normalize(lPos - pixPos);
    vec3 diffuse = max(dot(pixNorm, lightDir), 0.0) * pixAlbedo * lCol;
    // specular (Blinn-Phong Shading Model)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(pixNorm, halfwayDir), 0.0), 16.0);
    vec3 specular = lCol * spec;

    float shadowFactor = 0.0;
    float minShadowBias = 0.003; float maxShadowBiasFactor = 0.005;
    vec3 lightUV = pixPosLightSpace * 0.5 + 0.5;
    float lightNearest = texture(shadowMap, lightUV.xy).r;
    float bias = max(maxShadowBiasFactor * (1.0 - dot(normal, lightDir)), minShadowBias);
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -2; x <= 2; ++x) { for (int y = -2; y <= 2; ++y)
    {
        float pcfDepth = texture(shadowMap, lightUV.xy + vec2(x, y) * texelSize).r;
        shadowFactor += lightUV.z - bias > pcfDepth ? 1.0 : 0.0;
    }}
    shadowFactor /= 25.0;
    // outside far clip plane
    shadowFactor = lightUV.z <= 1.0 ? shadowFactor : 0.0;
    shadowFactor = 1.0 - shadowFactor;

    lighting += shadowFactor * (diffuse + specular);
    pixCol = vec4(lighting, 1.0);
}