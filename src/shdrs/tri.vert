#version 450 

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 fragUV;

layout(set = 0, binding = 0) uniform UBO {
    mat4 data;
} ubo;

void main() {
    gl_Position   = ubo.data * vec4(inPosition, 1.0);
    fragUV        = inUV; 
}


