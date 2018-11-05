// furShader.vert
// use textures, but no lighting or materials
#version 330

uniform mat4 modelview;
uniform mat4 projection;

in  vec3 in_Position;
in  vec3 in_Normal;
in  vec2 in_TexCoord;
uniform  float furFlowOffset;
uniform  float currentLayer;
uniform  float layers;
uniform	 float furLength;

out vec2 ex_TexCoord;

vec4 vGravity = vec4(0.0f, -2.0f, 0.0f, 1.0f);

void main(void) {
	// Extrude the surface by the normal by the gap
	vec3 Pos = in_Position.xyz + (in_Normal * (currentLayer * (furLength / layers)));
	// Translate into worldspace
	vec4 P = (modelview * vec4(Pos,1.0));

	// As the layers gets closer to the tip, bend more
	float layerNormalize = (currentLayer / layers);
	vGravity = (vGravity * modelview);
	float k = pow(layerNormalize, 3) * 0.08;
	P = P + vGravity * k;
	if(currentLayer != 0){
		P = P + vec4(1.0f, 1.0f, 1.0f, 1.0f) * (furFlowOffset);
	}

	ex_TexCoord = in_TexCoord;
    gl_Position = projection * P;
}

