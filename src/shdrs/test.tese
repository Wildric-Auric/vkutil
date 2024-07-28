#version 450 
layout(quads, equal_spacing, cw) in;
layout(location = 0) in vec2 uv[];
layout(location = 1) in  mat4 mvp[];

layout(location = 0) out vec2 outuv;

void main() {
    vec2 localCoord = vec2(gl_TessCoord);

    outuv   = uv[0];
    
    vec4 x0 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
    vec4 x1 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
    vec4 y  = mix(x0,x1, gl_TessCoord.y);
    
    outuv           = vec2(gl_TessCoord.x, gl_TessCoord.y);

    vec3 n = normalize(y.xyz);

    gl_Position     = mvp[0] * vec4(n, 1.0); 
}
