#version 430 core
layout (location = 0) out vec3 gPos;
layout (location = 1) out vec3 gNorm;
layout (location = 2) out vec2 gUVs;
layout (location = 3) out vec3 gAlbedo;
layout (location = 4) out vec3 gSpecular;
layout (location = 5) out float gDepth;

in vec3 outPos;
in vec3 outNorm;
in vec2 outTexCoord;

uniform sampler2D diffTex;
uniform sampler2D specTex;

void main()
{
	gPos = outPos;
	gNorm = normalize(outNorm);
	gUVs = outTexCoord;
	gAlbedo = texture(diffTex, outTexCoord).rgb;
	gSpecular = texture(specTex, outTexCoord).rrr;
	gDepth = gl_FragCoord.z;
}