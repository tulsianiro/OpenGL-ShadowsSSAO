#shader vertex
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 lightSpacePos;
out vec2 TexCoords;
out vec3 Normal;

uniform bool invertedNormals;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
	vec4 worldPos = model * vec4(aPos, 1.0);
	vec4 viewPos = view * worldPos;
	lightSpacePos = (lightSpaceMatrix * worldPos).xyz;
	FragPos = viewPos.xyz;
	TexCoords = aTexCoords;

	// inverse transpose for normal matrix
	mat3 normalMatrix = transpose(inverse(mat3(view * model)));
	Normal = normalMatrix * (invertedNormals ? -aNormal : aNormal);

	gl_Position = projection * viewPos;
}


#shader fragment
#version 330 core

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gAlbedo;
layout(location = 3) out vec3 gLightSpacePosition;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 lightSpacePos;

uniform mat4 view;
uniform sampler2D normalMap;
uniform bool useNormalMap;

void main()
{
	// store the fragment position vector in the first gbuffer texture
	gPosition = FragPos;
	gLightSpacePosition = lightSpacePos;
	// also store the per-fragment normals into the gbuffer
	gNormal = normalize(Normal);
	if (useNormalMap)
	{
		vec3 tmpNorm = texture(normalMap, TexCoords).xyz;
		vec4 correctedNorm = vec4(tmpNorm.x, tmpNorm.z, -tmpNorm.y, 0.0);
		gNormal = normalize(view * correctedNorm).xyz;
	}
	// and the diffuse per-fragment color
	gAlbedo.rgb = vec3(0.95); // 0.95 diffuse for everything
}
