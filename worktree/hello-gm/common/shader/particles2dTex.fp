#version 120

uniform sampler2D tex;

varying vec2 uv;
varying vec4 color;

void main(void)
{	
	vec4 finalColor = color;
	vec4 texColor = texture2D(tex, uv);
	gl_FragColor = texColor*finalColor;
}