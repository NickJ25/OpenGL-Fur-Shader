// textured.frag
#version 330

// Some drivers require the following
precision highp float;
in vec2 ex_TexCoord;
in vec3 ex_Normal;
//in int ex_furLength;
uniform  int current;
uniform float UVScale;
uniform int layers;
const int furNum = 8;

uniform sampler2D textureUnit0;
uniform sampler2D textureUnit1;

vec4 ambient = vec4(0.0, 0.0, 0.0, 0.0);
vec4 vecLightDir = vec4(0.8, 0.8, 1.0,0.0);

layout(location = 0) out vec4 out_Color;
vec4 furColour;
vec4 baseColour = texture(textureUnit1, ex_TexCoord);
void main(void) {
	// Make Base of object a texture
	if(current == 0)
	{
		baseColour = texture(textureUnit1, ex_TexCoord);
	} else {
		furColour = texture(textureUnit0, ex_TexCoord); //[current]
	// Tells OpenGL what to do with transparency.
		if(furColour.a < 0.1) discard;
		if(furColour.r < 0.1) discard;
		else furColour.r = baseColour.r;
		if(furColour.g < 0.1) discard;
		else furColour.g = baseColour.g;
		if(furColour.b < 0.1) discard;
		else furColour.b = baseColour.b;
		//vec4 finalColour = furColour;
	}
	//ambient = ambient * finalColour;
	//vec4 diffuse = finalColour;
	//finalColour = ambient + diffuse * dot(vecLightDir, vec4(ex_Normal,1.0));

	furColour.w = UVScale;
	//finalColour.w = UVScale;

	// Fragment colour
	out_Color = furColour * baseColour;
	//out_Color = finalColour;
	//out_Color = vec4(1.0f,1.0f,1.0f,1.0f); 
}