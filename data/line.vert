#version 330 compatibility

out vec3 v_Normal;
out vec4 v_Pos;
out vec4 v_Color;

void main() {
    v_Normal = gl_NormalMatrix * gl_Normal.xyz;
    v_Pos = gl_ModelViewMatrix * gl_Vertex;
    v_Color = gl_Color;
    gl_Position = gl_ProjectionMatrix * v_Pos;
}
