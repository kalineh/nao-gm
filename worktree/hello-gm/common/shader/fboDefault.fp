#version 120

uniform sampler2D tex;

void main()
{		
	vec2 uv = gl_TexCoord[0].xy;	
	vec3 diffuse = texture2D(tex, uv).xyz;
	
	gl_FragColor.xyz = vec3(diffuse);
	gl_FragColor.w = 1.0f;
}