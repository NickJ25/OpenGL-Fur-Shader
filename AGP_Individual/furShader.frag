// textured.frag
#version 330

// Some drivers require the following
precision highp float;
in vec2 ex_TexCoord;
in vec3 ex_Normal;
in int ex_furLength;
uniform  int current;
uniform float UVScale;
const int furNum = 8;

uniform sampler2D textureUnit0[furNum];

vec4 ambient = vec4(0.3, 0.3, 0.3, 0.0);
vec4 vecLightDir = vec4(0.8, 0.8, 1.0,0.0);

layout(location = 0) out vec4 out_Color;
 
void main(void) {
	vec4 furColour = texture(textureUnit0[current], ex_TexCoord);
	// Tells OpenGL what to do with transparency.
	if(furColour.a < 0.1)
        discard;
	furColour.rgb = vec3(0.1, 0.1, 0.1);
	vec4 finalColour = furColour;
    
	ambient = ambient * finalColour;
	vec4 diffuse = finalColour;
	finalColour = ambient + diffuse * dot(vecLightDir, vec4(ex_Normal,1.0));

	// Fragment colour
	furColour.w = UVScale;
	//out_Color = texture(textureUnit0[current], ex_TexCoord);
	out_Color = furColour;
	//out_Color = vec4(1.0f,1.0f,1.0f,1.0f); 
}