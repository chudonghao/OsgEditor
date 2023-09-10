#version 330 compatibility

////////////////////////////////////////////////////////////////////////////////
// 旋转矩阵
mat3 rotate(float angle, vec3 v);

const float PI = 3.14159265359;

mat3 rotate(float angle, vec3 v)
{
    float a = angle;
    float c = cos(a);
    float s = sin(a);

    vec3 axis = normalize(v);
    vec3 temp = (1. - c) * axis;

    mat3 Rotate;
    Rotate[0][0] = c + temp[0] * axis[0];
    Rotate[0][1] = temp[0] * axis[1] + s * axis[2];
    Rotate[0][2] = temp[0] * axis[2] - s * axis[1];

    Rotate[1][0] = temp[1] * axis[0] - s * axis[2];
    Rotate[1][1] = c + temp[1] * axis[1];
    Rotate[1][2] = temp[1] * axis[2] + s * axis[0];

    Rotate[2][0] = temp[2] * axis[0] + s * axis[1];
    Rotate[2][1] = temp[2] * axis[1] - s * axis[0];
    Rotate[2][2] = c + temp[2] * axis[2];

    return Rotate;
}

out vec4 v_Color;
void main(){

    // rotate matrix
    mat4 m = mat4(1.);
    mat3 r = mat3(1.);
    vec3 c = vec3(1.);
    switch (gl_InstanceID)
    {
        case 0:                                                             c = vec3(1,0,0);break;
        case 1:r = rotate(PI/2, vec3(1, 0, 0));                             c = vec3(1,0,0);break;
        case 2:r =                             rotate(PI/2, vec3(0, 0, 1)); c = vec3(0,1,0);break;
        case 3:r = rotate(PI/2, vec3(0, 1, 0))*rotate(PI/2, vec3(0, 0, 1)); c = vec3(0,1,0);break;
        case 4:r =                             rotate(PI/2, vec3(0, -1, 0));c = vec3(0,0,1);break;
        case 5:r = rotate(PI/2, vec3(0, 0, 1))*rotate(PI/2, vec3(0, -1, 0));c = vec3(0,0,1);break;
    }
    // normal gl_Position
    // rotate gl_Position
    m[0].xyz = r[0];
    m[1].xyz = r[1];
    m[2].xyz = r[2];

    gl_Position = gl_ModelViewProjectionMatrix * m * gl_Vertex;
    v_Color.xyz = c;
    v_Color.w = 1.;
}
