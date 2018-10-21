// textured.frag
#version 330

// Some drivers require the following
precision highp float;

uniform samplerCube textureUnit0;

in vec3 ex_TexCoord;
layout(location = 0) out vec4 out_Color;
 
void main(void) {
    
	// Fragment colour
	out_Color = texture(textureUnit0, ex_TexCoord);
}