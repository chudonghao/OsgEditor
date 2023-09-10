#version 330 compatibility

layout (lines) in;
layout (triangle_strip, max_vertices = 6) out;

uniform float u_Width;

in vec4 v_Pos[];
in vec4 v_Color[];
in vec3 v_Normal[];

out vec4 g_Pos;
out vec3 g_Normal;
out vec4 g_Color;

void main(){
    vec3 tNormal;
    vec3 bNormal;
    vec3 nNormal = vec3(0, 0, 1);
    vec3 nextTNormal = normalize(v_Normal[1]);
    vec3 nextBNormal;
    vec4 pos[6];
    vec4 color[6];

    tNormal = normalize(vec3(v_Pos[1].xy-v_Pos[0].xy, 0));
    bNormal = u_Width*normalize(cross(nNormal, tNormal));
    nextBNormal = u_Width*normalize(cross(nNormal, nextTNormal));

    // all vertex todo
    g_Normal = nNormal;

    pos[0] = v_Pos[1];
    pos[0].xyz += nextBNormal;
    color[0] = vec4(v_Color[1].rgb, 0);
    pos[1] = v_Pos[0];
    pos[1].xyz += bNormal;
    color[1] = vec4(v_Color[0].rgb, 0);
    pos[2] = v_Pos[1];
    pos[2].xyz;
    color[2] = v_Color[1];
    pos[3] = v_Pos[0];
    pos[3].xyz;
    color[3] = v_Color[0];
    pos[4] = v_Pos[1];
    pos[4].xyz -= nextBNormal;
    color[4] = vec4(v_Color[1].rgb, 0);
    pos[5] = v_Pos[0];
    pos[5].xyz -= bNormal;
    color[5] = vec4(v_Color[0].rgb, 0);

    for (int i = 0; i < 6; ++i)
    {
        g_Pos = pos[i];
        g_Color = color[i];
        gl_Position = gl_ProjectionMatrix * g_Pos;
        EmitVertex();
    }
    EndPrimitive();
}
