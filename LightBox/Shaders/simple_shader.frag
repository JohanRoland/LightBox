#version 450

layout (location = 0) in  vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragTextureCoord;


layout (location = 0) out  vec4 outColor;
layout (set = 1, binding = 0) uniform sampler2D material;

layout(set = 0, binding = 0) uniform GlobalUbo {
	mat4 projectionMatrix;
	mat4 viewMatrix;
	vec4 ambientLightColor;
	vec3 lightPosition;
	vec3 cameraPosition;
	vec4 lightColor;
} ubo;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatix; // mat4 for memory alignment reasons
	vec4 quaturnian;
} push;

void main() {

	vec3 directionToLight = normalize(ubo.lightPosition - fragPosWorld);
	float attenuation = 1.0 / dot(directionToLight, directionToLight); // |directionToLight|^2

	vec3 lightColor = ubo.lightColor.xyx * ubo.lightColor.w * attenuation;
	vec3 ambientLightColor = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;

	vec3 diffuseLight = lightColor * max(dot(normalize(fragNormalWorld), normalize(directionToLight)), 0);

	vec3 viewDir    = normalize(ubo.cameraPosition - fragPosWorld);
	vec3 halfwayDir = normalize(directionToLight + viewDir);
	vec3 specularHighlight =  lightColor * pow(max(dot(fragNormalWorld, halfwayDir), 0.0), 100);

	//outColor = vec4((ambientLightColor + diffuseLight + specularHighlight) * fragColor * texture(material, fragTextureCoord), 1.0);
	outColor = vec4((ambientLightColor + diffuseLight + specularHighlight) * fragNormalWorld, 1.0);
}