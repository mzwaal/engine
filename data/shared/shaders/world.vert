// attributes from the VAOs
$in uvec3 a_pos;
$in uvec3 a_info;

#ifdef INSTANCED
// instanced rendering
$in vec3 a_offset;
#endif

#ifndef MATERIALOFFSET
#define MATERIALOFFSET 0
#endif
int materialoffset = MATERIALOFFSET;

uniform mat4 u_model;
uniform mat4 u_viewprojection;
uniform sampler2D u_texture;
#define MATERIALCOLORS 256
layout(std140) uniform u_materialblock {
	// use the size here directly to let the shadertool be able
	// to generate the setters with the correct sizes
	vec4 u_materialcolor[256];
};

$out vec4 v_pos;
$out vec4 v_color;
$out float v_ambientocclusion;
$out float v_debug_color;

#include "_shadowmap.vert"

#if cl_deferred == 0
$out vec3 v_fogcolor;
#endif

void main(void) {
	uint a_ao = a_info[0];
	uint a_colorindex = a_info[1];
	uint a_material = a_info[2];
#ifdef INSTANCED
	v_pos = vec4(a_offset, 0.0) + u_model * vec4(a_pos, 1.0);
#else
	v_pos = u_model * vec4(a_pos, 1.0);
#endif

	int materialColorIndex = int(a_colorindex) + materialoffset;
	vec3 materialColor = u_materialcolor[materialColorIndex % MATERIALCOLORS].rgb;
	vec3 colornoise = texture(u_texture, abs(v_pos.xz) / 256.0 / 10.0).rgb;
	float alpha = u_materialcolor[a_colorindex].a;
	if (a_material == 1u) {
		alpha = 0.6;
	}
	v_color = vec4(materialColor * colornoise * 1.8, alpha);
	v_color = clamp(v_color, 0.0, 1.0);

	const float aovalues[] = float[](0.15, 0.6, 0.8, 1.0);
	v_ambientocclusion = aovalues[a_ao];

#if cl_shadowmap == 1
	v_lightspacepos = u_light * v_pos;
#endif

#if cl_deferred == 0
	// use the air color as fog color, too
	v_fogcolor = u_materialcolor[0].rgb;
#endif

	gl_Position = u_viewprojection * v_pos;
}
