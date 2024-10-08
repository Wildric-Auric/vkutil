#version 450 
layout (vertices=4) out; //Number of vertices of the patch, the same as specified in the pipeline tessellation state
//The tsc shader runs on every vertex of the patch,
//which can be retrieved via gl_in[gl_InvocationID].  gl_InvocationID < gl_MaxPatchVertices

layout(location = 0) in vec2 fragUV[];
layout(location = 1) in UBO { 
    mat4 view;
    mat4 model;
    mat4 proj;
    float subdivision;
} outUBO[];


layout(location = 0) out vec2 ofragUV[];
layout(location = 1) out UBO { 
    mat4 view;
    mat4 model;
    mat4 proj;
    float subdivision;
} ooutUBO[];

void main() {
    int subdiv = int(outUBO[0].subdivision);
    #define idd gl_InvocationID
    gl_out[idd].gl_Position = gl_in[idd].gl_Position;
    ofragUV[idd]            = fragUV[idd];
    
    ooutUBO[idd].subdivision        = outUBO[idd].subdivision;
    ooutUBO[idd].proj               = outUBO[idd].proj;
    ooutUBO[idd].view               = outUBO[idd].view ;
    ooutUBO[idd].model              = outUBO[idd].model;

    if (idd == 0) {
        gl_TessLevelOuter[0] = int(subdiv);
        gl_TessLevelOuter[1] = int(subdiv);
        gl_TessLevelOuter[2] = int(subdiv);
        gl_TessLevelOuter[3] = int(subdiv); 

        gl_TessLevelInner[0] = int(subdiv);
        gl_TessLevelInner[1] = int(subdiv);
    }
}
