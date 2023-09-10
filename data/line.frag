#version 330 compatibility

in vec4 g_Normal;
in vec4 g_Color;

void main() {
    gl_FragColor = g_Color;
}
