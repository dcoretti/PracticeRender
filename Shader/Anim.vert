#version 430 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec4 normal;
layout(location = 3) in int jointIndex;
// todo switch weigth to float[3] weight for 4 weighted bones or something

// per-bone skinning matrix for current pose
uniform mat4[32] skinningPalette;	

uniform mat4 mvp = mat4(1.0);

out vec2 texCoordUv;
out vec3 norm;

void main() {
	texCoordUv = uv;
	norm = normal.xyz;
	gl_Position = pos * skinningPalette[jointIndex] * mvp;
}