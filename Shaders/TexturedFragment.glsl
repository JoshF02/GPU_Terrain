#version 330 core
uniform sampler2D diffuseTex;
uniform sampler2D differentTex;
uniform float blend;

in Vertex {
	vec2 texCoord;
	//vec4 colour;
} IN;

out vec4 fragColour;
void main(void){
	vec4 t0 = texture2D(diffuseTex, IN.texCoord);
	//vec4 t1 = texture2D(differentTex, IN.texCoord);
	//fragColour = mix(t0, t1, t1.a)  * IN.colour;


	vec2 f = vec2(0.726, 0.0857);

	vec2 u = f * f * (3.0 - 2.0 * f);               // FADE

    //vec2 uTest = f * f * f * (f * (f * 6 - vec2(15,15)) + vec2(10,10));

	vec2 uTest2;

	float t = f.x;
	float t2 = f.y;
	uTest2 = vec2( (t * t * t * (t * (t * 6 - 15) + 10)) , (t2 * t2 * t2 * (t2 * (t2 * 6 - 15) + 10)) );

	//if (u == uTest2) fragColour = t0;
	//else fragColour = vec4(0.0,0.0,0.0,1.0);

	fragColour = t0;
}