#version 450

layout(binding=1) uniform sampler2D diffuse_map;

layout(location=0) in vec2 in_uv;

layout(location=0) out vec4 out_color;

void main() {
    out_color = texture(diffuse_map, in_uv);
}
