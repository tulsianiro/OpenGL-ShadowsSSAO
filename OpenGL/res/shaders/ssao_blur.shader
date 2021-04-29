#shader vertex
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
out vec2 pixelUV;

void main()
{
    gl_Position = vec4(position, 1.0);
    pixelUV = uv;
}

#shader fragment
#version 330 core
in vec2 pixelUV;
out float pixelCol;

uniform sampler2D initialSSAOTex;

void main()
{
    vec2 texSize = textureSize(initialSSAOTex, 0);
    vec2 texDim = vec2(1.0) / texSize;
    float accum = 0.0;
    for (int y = -2; y < 2; ++y) { for (int x = -2; x < 2; ++x)
    {
        // offset between [-2, -2], [1, 1]. gives us a 4x4 block, which is same as noise texture
        // this makes sense since we want to remove the 4x4 noise pattern. (centered around pixel)
        vec2 sampleUV = pixelUV + vec2(x * texDim.x, y * texDim.y);
        accum += texture(initialSSAOTex, sampleUV).x;
    }}
    // 16 is hardcoded since that is the value we initially chose based on resources like LearnOpenGL,
    // and never changed since the result looked quite good.
    pixelCol = accum / 16.0; 
}