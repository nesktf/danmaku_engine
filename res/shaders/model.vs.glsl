#version 330 core

layout (location = 0) in vec3 att_coords;
layout (location = 1) in vec3 att_normals;
layout (location = 2) in vec2 att_texcoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 normal;
out vec2 texcoord;

void main() {
  gl_Position = proj * view * model * vec4(att_coords, 1.0f);
  normal = att_normals;
  texcoord = att_texcoords;
}

