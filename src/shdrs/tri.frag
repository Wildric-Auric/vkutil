#version 450

layout(location = 0) in  vec2 uv;
layout(location = 1) in  vec3 normal;
layout(location = 2) in  vec3 fragPos;

layout(location = 0) out vec4 colorAtt;
layout(location = 1) out vec4 testAtt;

layout(set = 0, binding = 1) uniform sampler2D uTex;


vec3 lightPos = vec3(0., -1.3, 3.5);

void main() {
    float c = 0.7;
    if (uv.x > 0.99 || uv.y > 0.99 || uv.x < 0.01 || uv.y < 0.01)  c = 0.2;
    vec3 tex     = texture(uTex, uv).xyz;
    vec3 n       = normalize(normal);
    vec3  ldir   = normalize(lightPos - fragPos);
    float diff   = dot(ldir, n);
    diff         = max(diff, 0.0);
    vec3 diffCol = (diff + vec3(0.005)) * vec3(1.0);
    
    vec3 col = diffCol * tex;
	colorAtt = vec4(col,1.0);
    testAtt  = vec4(vec3(n.x, -n.y, n.z) * c, 1.0); //Visualize in a gfx debugger 
}

