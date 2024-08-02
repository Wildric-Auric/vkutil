#version 450 
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) out vec2 outuv;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec3 fragPos;

layout(location = 0) in  vec2 uv[];
layout(location = 1) in UBO { 
    mat4 view;
    mat4 model;
    mat4 proj;
    float subdivision;
} ubo[];  

void main() {
    mat4 mvp = ubo[0].proj * ubo[0].view * ubo[0].model;
    mat4 m   = ubo[0].model;

    vec4 a = m * gl_in[0].gl_Position;
    vec4 b = m * gl_in[1].gl_Position;
    vec4 c = m * gl_in[2].gl_Position;

    vec3 x = a.xyz;
    vec3 y = b.xyz;
    vec3 z = c.xyz;

    vec3 n = cross(y - x, z - x);

    gl_Position = mvp * gl_in[0].gl_Position;
    fragPos     = gl_Position.xyz;
    outuv  = uv[0];
    normal = n;
    EmitVertex();

    gl_Position = mvp * gl_in[1].gl_Position;
    fragPos     = gl_Position.xyz;
    outuv = uv[1];
    normal = n;
    EmitVertex();
    
    gl_Position = mvp * gl_in[2].gl_Position;
    fragPos     = gl_Position.xyz;
    outuv = uv[2];
    normal = n;
    EmitVertex();

    EndPrimitive();

}
