#version 450

layout (location = 0) in  vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;


layout (location = 0) out  vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec4 ambientLightColor;
	vec3 lightPosition;
	vec4 lightColor;
} ubo;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatix; // mat4 for memory alignment reasons
} push;

void main() {

	vec3 directionToLight = ubo.lightPosition - fragPosWorld;
	float attenuation = 1.0 / dot(directionToLight, directionToLight); // |directionToLight|^2

	vec3 lightColor = ubo.lightColor.xyx * ubo.lightColor.w * attenuation;
	vec3 ambientLightColor = ubo.ambientLightColor.xyx * ubo.ambientLightColor.w;

	vec3 diffuseLight = lightColor * max(dot(normalize(fragNormalWorld), normalize(directionToLight)), 0);

	outColor = vec4((ambientLightColor + diffuseLight) * fragColor, 1.0);
}