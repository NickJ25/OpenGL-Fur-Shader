// furShader.frag
// use textures, but no lighting or materials
#version 330

// Some drivers require the following
precision highp float;
in vec2 ex_TexCoord;
in vec3 ex_Normal;
uniform  int current;
uniform float UVScale;
uniform float layers;
uniform float currentLayer;

uniform sampler2D textureUnit0;
uniform sampler2D textureUnit1;

vec4 ambient = vec4(0.0, 0.0, 0.0, 0.0);
vec4 vecLightDir = vec4(0.8, 0.8, 1.0,0.0);

layout(location = 0) out vec4 out_Color;
vec4 furColour;
vec4 baseColour = texture(textureUnit1, ex_TexCoord);
void main(void) {
	// Make Base of object a texture
	if(currentLayer > 0)
	{
		furColour = texture(textureUnit0, ex_TexCoord);
		// Tells OpenGL what to do with transparency.
		if(furColour.a < 0.1) discard;
		if(furColour.r < 0.1) discard;
		else furColour.r = baseColour.r;
		if(furColour.g < 0.1) discard;
		else furColour.g = baseColour.g;
		if(furColour.b < 0.1) discard;
		else furColour.b = baseColour.b;
	}

	furColour.w = UVScale;

	// Fragment colour
	out_Color = furColour;
	if(currentLayer == 0) out_Color = vec4(0.0,0.0,0.0,1.0);
}