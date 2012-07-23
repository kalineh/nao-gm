#version 120

varying vec2 uv;
varying vec4 color;

void main(void)
{
	vec2 uvNorm = uv * 2.0f - vec2(1.0f);
	float len = length(uvNorm);
	float alpha = 1.0f-smoothstep( 0.0f, 1.0f, len );
	
	vec4 finalColor = color;
	finalColor.w *= alpha;
	gl_FragColor = finalColor;
}