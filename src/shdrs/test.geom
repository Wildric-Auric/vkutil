#version 450 
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in  vec2 uv[];
layout(location = 0) out vec2 outuv;
layout(location = 1) out float depth;

void main() {
    gl_Position = gl_in[0].gl_Position;
    outuv = uv[0];
    depth = gl_Position.z / gl_Position.w;
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    outuv = uv[1];
    depth = gl_Position.z / gl_Position.w;
    EmitVertex();
    
    gl_Position = gl_in[2].gl_Position;
    outuv = uv[2];
    depth = gl_Position.z / gl_Position.w;
    EmitVertex();

    EndPrimitive();

}
