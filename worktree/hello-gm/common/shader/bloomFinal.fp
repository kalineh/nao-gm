#version 120

uniform sampler2D texRaw;
uniform sampler2D texBloom;
uniform vec4 bloomData;

void main(void)
{
	vec2 uv = gl_TexCoord[0].xy;	
	vec3 diffuse =  texture2D(texRaw, uv).xyz;
	vec3 bloom =  texture2D(texBloom, uv).xyz;
	
	float bloomCoeff = bloomData.x;
	vec3 bloomPower = vec3(bloomData.y, bloomData.y, bloomData.y);
	bloom = pow(bloom,bloomPower) * bloomCoeff;

	gl_FragColor = vec4( (vec3(1.0f)-bloom)*diffuse + bloom, 1.0f );
//	gl_FragColor = vec4( diffuse + bloom, 1.0f );
}