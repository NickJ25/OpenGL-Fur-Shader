// textured.vert
// use textures, but no lighting
#version 330

uniform mat4 modelview;
uniform mat4 projection;

in  vec3 in_Position;
in  vec3 in_Normal;
in  vec2 in_TexCoord;
uniform  int currentLayer;
uniform  int layers;
uniform	 float furLength;

out vec2 ex_TexCoord;
out vec3 ex_Normal;
out int ex_furLength;


// Fur Settings
float UVScale = 1.0f;
float layer = 0;
vec4 vGravity = vec4(0.0f, -2.0f, 0.0f, 1.0f);


// multiply each vertex position by the MVP matrix
void main(void) {
	//ex_TexCoord = (normalize(in_Position)).xy;


	vec3 Pos = in_Position.xyz + (in_Normal * (currentLayer * (furLength / layers)));
	vec4 P = (modelview * vec4(Pos,1.0));
	//ex_Normal = normalize(vec4(in_Normal,1.0) * modelview).xyz;
	mat3 normalmatrix = transpose(inverse(mat3(modelview)));
	ex_Normal = normalize(normalmatrix * in_Normal);

	float layerNormalize = (currentLayer / layers);
	vGravity = (vGravity * modelview);
	float k = layerNormalize * layerNormalize * layerNormalize; //pow(layerNormalize, 3);
	P = P + vGravity * k;
	
	// As the layers gets closer to the tip, bend more
	
	// vertex into eye coordinates
	//vec4 vertexPosition = modelview * vec4(in_Position,1.0);

	ex_TexCoord = in_TexCoord;
    gl_Position = projection * P;


}