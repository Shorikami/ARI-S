#version 430 core

layout(location = 0) out vec4 sDepth;

in vec4 shadowPos;

vec4 Moments(float fragDepth)
{
	float sq = fragDepth * fragDepth;
	return vec4(fragDepth, sq, sq * fragDepth, sq * sq);
}

void main()
{
	float depth = shadowPos.z / shadowPos.w;
	depth = depth * 0.5f + 0.5f;
	
	sDepth = Moments(depth);
	
	//float depth = ((gl_DepthRange.diff * (shadowPos.z / shadowPos.w)) +
	//gl_DepthRange.near + gl_DepthRange.far) / 2.0f;
	//
	//sDepth = vec4(vec3(gl_FragCoord.z, pow(gl_FragCoord.z, 2), pow(gl_FragCoord.z, 3)), pow(gl_FragCoord.z, 4));
	//
	//sDepth = vec4(depth, pow(depth, 2), pow(depth, 3), pow(depth, 4));
}