#version 450

layout(location = 0) in  vec2 uv;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 1) uniform sampler2D uTex;

void main() {
	fragColor = vec4(texture(uTex,uv).xyz, 1.0);
}

