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
out vec4 FragColor;

in vec2 TexCoords;
uniform sampler2D myTex;
uniform bool grayscale;

void main()
{
    FragColor = texture(myTex, TexCoords);
    if (grayscale)
    {
        FragColor = vec4(FragColor.r, FragColor.r, FragColor.r, 1.0);
    }
}