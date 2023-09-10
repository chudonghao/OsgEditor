#version 330 compatibility

uniform sampler2D u_DiffuseMap;
//in vec4 v_Color;
in vec2 v_TexCoord0;

void main() {
//    gl_FragColor = v_Color;
    vec4 color = texture(u_DiffuseMap,v_TexCoord0.xy);
    if(color.a <= 0.5)
    {
        discard;
    }

    gl_FragColor = color;
}
