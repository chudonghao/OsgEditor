#version 330 compatibility

uniform mat4 u_scale;

void main() {
    gl_Position = gl_ModelViewProjectionMatrix * u_scale * gl_Vertex;
}
