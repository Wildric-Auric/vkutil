#version 450

layout(location = 0) in  vec3 color;
layout(location = 1) in  vec2 uv;

layout(binding  = 1) uniform sampler2D uTex;


layout(location = 0) out vec4 fragColor;

void main() {
    vec3 texel = texture(uTex, uv).xyz;
	fragColor = vec4(texel, 1.0);
}
