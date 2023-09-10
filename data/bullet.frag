#version 330 compatibility

uniform float u_StartTime;
uniform float u_V;
uniform float u_Size;
uniform float osg_FrameTime;

in float v_S;
in vec4 v_Color;

void main() {
    float factor = min(1, 1 - pow(v_S - u_V * (osg_FrameTime - u_StartTime), 2)/u_Size);
    gl_FragColor = v_Color;
    gl_FragColor.a *= factor;
}
