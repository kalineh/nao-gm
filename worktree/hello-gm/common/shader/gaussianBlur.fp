#version 120

uniform sampler2D tex;
uniform vec4 uvDelta;

void main(void)
{
   vec2 uv = gl_TexCoord[0].xy;	
   vec2 uvStep = vec2(uvDelta.xy);
   vec4 sum = vec4(0.0f);
 
   // 9 samples
   sum += texture2D(tex, uv - 4.0f*uvStep) * 0.05f;
   sum += texture2D(tex, uv - 3.0f*uvStep) * 0.09f;
   sum += texture2D(tex, uv - 2.0f*uvStep) * 0.12f;
   sum += texture2D(tex, uv - uvStep) * 0.15f;
   sum += texture2D(tex, uv) * 0.16f;
   sum += texture2D(tex, uv + uvStep) * 0.15f;
   sum += texture2D(tex, uv + 2.0f*uvStep) * 0.12f;
   sum += texture2D(tex, uv + 3.0f*uvStep) * 0.09f;
   sum += texture2D(tex, uv + 4.0f*uvStep) * 0.05f;
 
   gl_FragColor = sum;
}