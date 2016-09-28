#version 430 core
in vec2 texCoordUv;
in vec3 normal;
out vec4 fragColor;

uniform sampler2D texSampler;


void main() {
	fragColor = texture(texSampler, texCoordUv);

	//fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}