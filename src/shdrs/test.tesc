#version 450 
layout (vertices=4) out; //Number of vertices of the patch, the same as specified in the pipeline tessellation state
//The tsc shader runs on every vertex of the patch,
//which can be retrieved via gl_in[gl_InvocationID].  gl_InvocationID < gl_MaxPatchVertices

layout(location = 1) in  mat4 mvp[];
layout(location = 0) in  vec2  uv[];
layout(location = 5) in  float subdiv[];

layout(location = 0) out vec2 outuv[];
layout(location = 1) out mat4 outMvp[];

void main() {
    #define idd gl_InvocationID
    gl_out[idd].gl_Position = gl_in[idd].gl_Position;
    outMvp[idd] = mvp[idd];
    outuv[idd] = uv[idd];

    if (idd == 0) {
        //gl_TessLevelOuter[0] = int(subdiv[0]);
        //gl_TessLevelOuter[1] = int(subdiv[0]);
        //gl_TessLevelOuter[2] = int(subdiv[0]);
        //gl_TessLevelOuter[3] = int(subdiv[0]); 

        gl_TessLevelInner[0] = int(subdiv[0]);
        gl_TessLevelInner[1] = int(subdiv[0]);
    }
}
