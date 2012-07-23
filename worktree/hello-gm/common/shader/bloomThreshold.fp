#version 120

uniform sampler2D tex;
uniform vec4 thresholdLumin;

void main(void)
{
   vec2 uv = gl_TexCoord[0].xy;	
   vec3 diffuse =  texture2D(tex, uv).xyz;
   
   float lumin = dot( diffuse, vec3( 0.3f, 0.59f, 0.11f ) );
   float thresCoeff = step( thresholdLumin.x, lumin );
 
   gl_FragColor = vec4( diffuse*thresCoeff, 1.0f );
}