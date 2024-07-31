#version 330 core

in vec3 texcoord;
out vec4 frag_color;

uniform samplerCube skybox;

void main() {
  frag_color = texture(skybox, texcoord);
}

