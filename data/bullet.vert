#version 330 compatibility

out float v_S;
out vec4 v_Color;

void main() {
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    v_S = gl_MultiTexCoord0.x;
    v_Color = gl_Color;
}
