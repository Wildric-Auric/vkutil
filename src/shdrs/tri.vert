#version 450 

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out mat4 mvp;
layout(location = 5) out float subdiv;

layout(set = 0, binding = 0) uniform UBO {
    mat4 data;
    float subdivision;
} ubo;

void main() {
    gl_Position   = vec4(inPosition, 1.0);
    fragUV        = inUV; 
    mvp           = ubo.data;
    subdiv        = ubo.subdivision;
}


