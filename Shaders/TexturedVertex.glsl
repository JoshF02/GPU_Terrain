#version 330 core
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 textureMatrix;

uniform bool apply;
uniform float xCoord;
uniform float zCoord;

in vec3 position;
in vec2 texCoord;
//in vec4 colour;

out Vertex {
	vec2 texCoord;
	//vec4 colour;
} OUT;

void main(void){
	mat4 mvp = projMatrix * viewMatrix * modelMatrix;

	float dist = sqrt(((position.x - xCoord) * (position.x - xCoord)) + ((position.z - zCoord) * (position.z - zCoord)));

	vec3 newPos;
	//if (dist < 1000) newPos = position + vec3(0, 8000 / dist, 0);
	//else newPos = position;
	newPos = position + vec3(0, 5000 / (dist * 0.1), 0);

	newPos = position;


	gl_Position = mvp * vec4(newPos, 1.0);
	OUT.texCoord = (textureMatrix * vec4(texCoord, 0.0, 1.0)).xy;
	//OUT.colour = colour;
}