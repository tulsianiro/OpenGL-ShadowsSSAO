#shader vertex
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

out vec2 pixelUV;

void main()
{
    pixelUV = uv;
    gl_Position = vec4(position, 1.0);
}

#shader fragment
#version 330 core
out vec4 pixelCol;

in vec2 pixelUV;
uniform sampler2D myTex;
uniform bool grayscale;

void main()
{
    pixelCol = texture(myTex, pixelUV);
    if (grayscale)
    {
        pixelCol = vec4(pixelCol.r, pixelCol.r, pixelCol.r, 1.0);
    }
}