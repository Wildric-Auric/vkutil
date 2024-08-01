#version 450

layout(location = 0) in  vec2 uv;

layout(location = 0) out vec4 colorAtt;
layout(location = 1) out vec4 testAtt;

layout(set = 0, binding = 1) uniform sampler2D uTex;

void main() {
    float c = 0.7;
    if (uv.x > 0.99 || uv.y > 0.99 || uv.x < 0.01 || uv.y < 0.01)  c = 0.2;
	colorAtt = vec4(texture(uTex,uv).xyz * c, 1.0);
    testAtt  = colorAtt * 0.5;
}

