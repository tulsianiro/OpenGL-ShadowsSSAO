#shader vertex
#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNorm;
layout(location = 2) in vec2 vertUV;

out vec3 pixelPos;
out vec3 lightSpacePos;
out vec2 pixelUV;
out vec3 pixelNorm;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
	mat4 modelView = view * model;
	mat4 normalMatrix = transpose(inverse(modelView));
	vec4 worldPos = model * vec4(vertPos, 1.0);
	vec4 viewPos = view * worldPos;

	lightSpacePos = (lightSpaceMatrix * worldPos).xyz;
	pixelPos = viewPos.xyz;
	pixelUV = vertUV;

	// inverse transpose for normal matrix
	vec4 vertNorm4 = vec4(vertNorm, 0.0);
	pixelNorm = (normalMatrix * vertNorm4).xyz;

	gl_Position = projection * viewPos;
}


#shader fragment
#version 330 core

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gAlbedo;
layout(location = 3) out vec3 gLightSpacePosition;

in vec2 pixelUV;
in vec3 pixelPos;
in vec3 pixelNorm;
in vec3 lightSpacePos;

uniform mat4 view;
uniform sampler2D normalMap;
uniform bool useNormalMap;

void main()
{
	// store the fragment position vector in the first gbuffer texture
	gPosition = pixelPos;
	gLightSpacePosition = lightSpacePos;
	// also store the per-fragment normals into the gbuffer
	gNormal = normalize(pixelNorm);
	if (useNormalMap)
	{
		vec3 tmpNorm = texture(normalMap, pixelUV).xyz;
		vec4 correctedNorm = vec4(tmpNorm.x, tmpNorm.z, -tmpNorm.y, 0.0);
		gNormal = normalize(view * correctedNorm).xyz;
	}
	// and the diffuse per-fragment color
	gAlbedo.rgb = vec3(1.0); // 1.0 white diffuse for everything
}
