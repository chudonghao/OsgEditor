#version 330 compatibility

out vec2 v_LPos;
void main() {
    v_LPos = gl_Vertex.xy;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
