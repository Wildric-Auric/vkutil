#version 450 

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out float color;

layout(set = 0, binding = 0) uniform UBO {
    float c;
} ubo;

void main() {
    gl_Position   = vec4(inPosition, 1.0);
    fragUV        = inUV; 
    color         = ubo.c;
}


//layout(binding = 0) uniform UniformBufferObject {
//    mat4 model;
//    mat4 view;
//    mat4 proj;
//} ubo;
