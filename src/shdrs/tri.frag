#version 450

layout(location = 0) in  vec2 uv;
layout(location = 0) out vec4 fragColor;

void main() {
	fragColor = vec4(vec3(1.0), 1.0);
}

//layout(binding  = 1) uniform sampler2D uTex;
