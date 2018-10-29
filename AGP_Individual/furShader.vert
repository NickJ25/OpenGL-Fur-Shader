// textured.vert
// use textures, but no lighting
#version 330

uniform mat4 modelview;
uniform mat4 projection;

in  vec3 in_Position;
in  vec3 in_Normal;
in  vec2 in_TexCoord;
uniform  int furLength;

out vec2 ex_TexCoord;
out vec3 ex_Normal;
out int ex_furLength;


// Fur Settings
//float furLength = 0;
float UVScale = 1.0f;
float layer = 0;


// multiply each vertex position by the MVP matrix
void main(void) {
	ex_furLength = furLength;
	//ex_TexCoord = (normalize(in_Position)).xy;

	vec3 Pos = in_Position.xyz + (in_Normal * (furLength * 0.01) );
	vec4 P = modelview * vec4(Pos,1.0);
	//ex_Normal = normalize(vec4(in_Normal,1.0) * modelview).xyz;
	mat3 normalmatrix = transpose(inverse(mat3(modelview)));
	ex_Normal = normalize(normalmatrix * in_Normal);
	float k = pow(layer, 3);
	// vertex into eye coordinates
	//vec4 vertexPosition = modelview * vec4(in_Position,1.0);

	ex_TexCoord = in_TexCoord;

    gl_Position = projection * P;


}