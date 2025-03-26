#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;


layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec2 fragTextureCoord;

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

const float AMBIENT_INDIRECT_LIGHT = 0.02;

vec3 rotateQuat(vec3 v, vec4 q)
{
	return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

void main() {
	

	vec4 positionWorld = push.modelMatrix * vec4(rotateQuat(position, push.quaturnian), 1.0);


	fragNormalWorld = normalize(mat3(push.normalMatix) * normal);
	fragPosWorld = positionWorld.xyz;
	fragColor = color;
	fragTextureCoord = uv;

	gl_Position = ubo.projectionMatrix * ubo.viewMatrix  * positionWorld;
//	fragTextureCoord = vertexTextureCoord
}