#version 330 core

in vec3 normal;
in vec2 texcoord;
out vec4 frag_color;

struct Material {
  sampler2D diffuse;
  sampler2D specular;
  float shiny;
};
uniform Material material;

void main() {
  vec4 out_color = texture(material.diffuse, texcoord);

  if (out_color.a < 0.1)
    discard;

  frag_color = out_color;
}

