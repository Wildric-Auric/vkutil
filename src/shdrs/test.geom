#version 450 
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

layout(location = 0) in  vec2 uv[];
layout(location = 0) out vec2 outuv;

void main() {
    gl_Position = gl_in[0].gl_Position;
    outuv = uv[0];
    EmitVertex();

    gl_Position = gl_in[1].gl_Position;
    outuv = uv[1];
    EmitVertex();
    
    gl_Position = gl_in[2].gl_Position;
    outuv = uv[2];
    EmitVertex();

    EndPrimitive();

}
