#version 330 compatibility
in vec2 v_LPos;
void main() {
    vec2 lPos = normalize(v_LPos);
    float a = acos(dot(lPos,vec2(1,0)));
    a = mod(a,0.2);

    gl_FragColor = vec4(step(a,0.1), 0.0, 0.0, 1.0);
}
