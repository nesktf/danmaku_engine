#pragma once

#include <string_view>

namespace okuu::render {

constexpr std::string_view vert_sprite = R"glsl(
#version 460 core

layout (location = 0) in vec3 att_coords;
layout (location = 1) in vec3 att_normals;
layout (location = 2) in vec2 att_texcoords;
out vec2 tex_coord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

uniform vec4 u_offset;

void main() {
  tex_coord.x = att_texcoords.x*u_offset.x + u_offset.z;
  tex_coord.y = att_texcoords.y*u_offset.y + u_offset.w;

  gl_Position = u_proj * u_view * u_model * vec4(att_coords, 1.0f);
}

)glsl";

constexpr std::string_view frag_sprite = R"glsl(
#version 460 core

in vec2 tex_coord;
out vec4 frag_color;

uniform sampler2D u_sprite_sampler;
uniform vec4 u_sprite_color;

void main() {
  vec4 out_color = u_sprite_color * texture(u_sprite_sampler, tex_coord);

  if (out_color.a < 0.1) {
    discard;
  }

  frag_color = out_color;
}

)glsl";

constexpr std::string_view vert_fbo = R"glsl(
#version 460 core

layout (location = 0) in vec3 att_coords;
layout (location = 1) in vec3 att_normals;
layout (location = 2) in vec2 att_texcoords;
out vec2 tex_coord;

uniform mat4 model;
uniform mat4 proj;

void main() {
  gl_Position = proj * model * vec4(att_coords, 1.0f);
  tex_coord = vec2(att_texcoords.x, 1-att_texcoords.y);
}

)glsl";

constexpr std::string_view frag_fbo = R"glsl(
#version 460 core

in vec2 tex_coord;
out vec4 frag_color;

uniform sampler2D fb_sampler;

void main() {
  frag_color = texture(fb_sampler, tex_coord);
}

)glsl";

constexpr std::string_view frag_back = R"glsl(
#version 460 core

#define TILES 8.0

#define RESOLUTION vec2(1280.f, 720.f)

in vec2 tex_coord;
out vec4 frag_color;

uniform float time;
uniform sampler2D tex;

float Hash(vec2 p, float scale) {
	// This is tiling part, adjusts with the scale...
	p = mod(p, scale);
	return fract(sin(dot(p, vec2(27.16898, 38.90563))) * 5151.5473453);
}

float Noise(vec2 p, float scale ) {
	vec2 f;
	
	p *= scale;

	
	f = fract(p);		// Separate integer from fractional
    p = floor(p);
	
    f = f*f*(3.0-2.0*f);	// Cosine interpolation approximation
	
    float res = mix(mix(Hash(p, 				 scale),
						Hash(p + vec2(1.0, 0.0), scale), f.x),
					mix(Hash(p + vec2(0.0, 1.0), scale),
						Hash(p + vec2(1.0, 1.0), scale), f.x), f.y);
    return res;
}

float fBm(vec2 p) {
  p += vec2(sin(time * .7), cos(time * .45))*(.1);
	float f = 0.0;
	// Change starting scale to any integer value...
	float scale = 10.;
    p = mod(p, scale);
	float amp   = 0.6;
	
	for (int i = 0; i < 5; i++)
	{
		f += Noise(p, scale) * amp;
		amp *= .5;
		// Scale must be multiplied by an integer value...
		scale *= 2.;
	}
	// Clamp it just in case....
	return min(f, 1.0);
}

void main() {
  vec2 uv = gl_FragCoord.xy*TILES / RESOLUTION;
  vec3 col = vec3(fBm(uv))*vec3(0.8);

  vec2 coords = tex_coord*TILES + time*0.05*vec2(sqrt(2),-sqrt(2));
  frag_color = vec4(col, 1.0)*texture(tex, coords);
}
)glsl";

} // namespace okuu::render
