#version 450

layout(push_constant) uniform PushConstant {
    vec4 scl;
    vec4 rot;
    vec4 trs;
    vec4 view_pos;
    vec4 view_rot;
    vec4 proj_param;
} constant;

layout(location=0) in vec3 in_pos;

mat4 my_scale(vec3 v) {
    return mat4(
        v.x, 0.0, 0.0, 0.0,
        0.0, v.y, 0.0, 0.0,
        0.0, 0.0, v.z, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

mat4 my_rotate_x(float ang) {
    float c = cos(ang);
    float s = sin(ang);
    return mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0,   c,  -s, 0.0,
        0.0,   s,   c, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

mat4 my_rotate_y(float ang) {
    float c = cos(ang);
    float s = sin(ang);
    return mat4(
          c, 0.0,   s, 0.0,
        0.0, 1.0, 0.0, 0.0,
         -s, 0.0,   c, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

mat4 my_rotate_z(float ang) {
    float c = cos(ang);
    float s = sin(ang);
    return mat4(
          c,  -s, 0.0, 0.0,
          s,   c, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

mat4 my_translate(vec3 v) {
    return mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        v.x, v.y, v.z, 1.0
    );
}

void main() {
    vec4 pos = vec4(in_pos, 1.0);
    pos = my_scale(constant.scl.xyz) * pos;
    pos = my_rotate_x(constant.rot.x) * pos;
    pos = my_rotate_y(constant.rot.y) * pos;
    pos = my_rotate_z(constant.rot.z) * pos;
    pos = my_translate(constant.trs.xyz) * pos;
    pos = my_rotate_x(-1.0 * constant.view_rot.x) * pos;
    pos = my_rotate_y(-1.0 * constant.view_rot.y) * pos;
    pos = my_rotate_z(-1.0 * constant.view_rot.z) * pos;
    pos = my_translate(-1.0 * (constant.view_pos.xyz)) * pos;
    float div_tanpov = 1.0 / tan(constant.proj_param.x);
    float div_depth = 1.0 / (constant.proj_param.w - constant.proj_param.z);
    pos = mat4(
        div_tanpov, 0.0, 0.0, 0.0,
        0.0, constant.proj_param.y * div_tanpov, 0.0, 0.0,
        0.0, 0.0, (constant.proj_param.w + constant.proj_param.z) * div_depth, 1.0,
        0.0, 0.0, 2.0 * constant.proj_param.w * constant.proj_param.z * div_depth, 0.0
    ) * pos;
    gl_Position = pos;
}
