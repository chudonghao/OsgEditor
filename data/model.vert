#version 330 compatibility

uniform samplerBuffer u_posBuffer;

//out vec4 v_Color;
out vec2 v_TexCoord0;

void main() {
    vec3 dp = texelFetch(u_posBuffer,gl_InstanceID).xyz;
    vec4 pos = gl_ModelViewMatrix * gl_Vertex;
    pos.xyz += dp;
    gl_Position = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.xyz + dp,gl_Vertex.w);

//    gl_Position = ftransform();

//    vec4 pos = gl_Vertex;
//    pos.x += gl_InstanceID;
//    gl_Position = gl_ModelViewProjectionMatrix * pos;

//    v_Color = gl_Color;
    v_TexCoord0 = gl_MultiTexCoord0.xy;
}
