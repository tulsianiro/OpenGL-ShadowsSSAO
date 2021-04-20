#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}

#shader fragment
#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];

// parameters (you'd probably want to use them as uniforms to more easily tweak the effect)
int kernelSize = 64; // size of sample kernel
float radius = 0.5; // ssao hemisphere radius
float bias = 0.025; // bias to avoid acne

// tile noise texture over screen based on screen dimensions divided by noise size
const vec2 noiseScale = vec2(1280.0 / 4.0, 720.0 / 4.0);

uniform mat4 projection;

void main()
{
    // get fragment position from the GBuffer
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    // get the fragment normal from the GBuffer
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    // take the TexCoords then scale by noiseScale which makes us tile every 4 pixels per dimension
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

    // create Frenet Frame / TBN matrix: from tangent-space to view-space
    // the TBN basis is slightly rotated each time
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // iterate over the sample kernel and calculate occlusion factor between 0 and 1
    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i)
    {
        vec3 samplePos = TBN * samples[i]; // from tangent to view-space, since normal comes into shader in View Space
        samplePos = fragPos + samplePos * radius; // radius of the hemisphere, samplePos is pos assuming pixel as origin

        // we want the uv offset in our GBuffer textures for this fragment sample
        vec4 offset = vec4(samplePos, 1.0); // initially put into vector
        offset = projection * offset; // from view to clip-space by multiplying by perspective
        offset.xyz /= offset.w; // perspective divide to get the NDC coordinates
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

        // get the depth for this particular fragment sample
        float sampleDepth = texture(gPosition, offset.xy).z;

        // make sure that the z value is within the sample hemisphere. also weigh less depending on dist
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        // if GBuffer depth of the pixel corresponding to sample is greater than the sample's z, 
        // then occluded by nearby obj.
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }

    // sub from 1 so we can just use as a scaling factor for ambient light. more occluded less light.
    occlusion = 1.0 - (occlusion / kernelSize); // average occlusion for the pixel, and sub from 1
    FragColor = occlusion;
}
