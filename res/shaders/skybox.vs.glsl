#version 330 core

layout (location = 0) in vec3 att_coords;
out vec3 texcoord;

uniform mat4 proj;
uniform mat4 view;

void main() {
  vec4 pos = proj * view * vec4(att_coords, 1.0f);
  gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
  texcoord = vec3(att_coords.x, att_coords.y, -att_coords.z);
}


