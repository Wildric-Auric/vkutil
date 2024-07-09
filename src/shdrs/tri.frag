#version 450

layout(location = 0) in  vec2 uv;
layout(location = 1) in  float color;

layout(location = 0) out vec4 fragColor;

void main() {
	fragColor = vec4(vec3(color), 1.0);
}

