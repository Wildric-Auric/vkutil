#version 450 
layout(quads, equal_spacing, cw) in;

layout(location = 0) in vec2 ofragUV[];
layout(location = 1) in UBO { 
    mat4 view;
    mat4 model;
    mat4 proj;
    float subdivision;
} ooutUBO[];

layout(location = 0) out vec2 outuv;
layout(location = 1) out UBO { 
    mat4 view;
    mat4 model;
    mat4 proj;
    float subdivision;
} outUBO;

void main() {
    vec2 localCoord = vec2(gl_TessCoord);

    outuv   = ofragUV[0];
    
    outUBO.subdivision        = ooutUBO[0].subdivision;
    outUBO.proj               = ooutUBO[0].proj;
    outUBO.view               = ooutUBO[0].view ;
    outUBO.model              = ooutUBO[0].model;
    
    vec4 x0 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
    vec4 x1 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
    vec4 y  = mix(x0,x1, gl_TessCoord.y);
    
    outuv           = vec2(gl_TessCoord.x, gl_TessCoord.y);

    vec3 n = normalize(y.xyz);
    gl_Position     = vec4(n, 1.0); 
}
