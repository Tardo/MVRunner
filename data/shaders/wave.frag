uniform sampler2D texture;
uniform float wave_phase;

float randv(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float acc = 0.0;
void main()
{
	acc += randv(gl_TexCoord[0].xy);

	vec2 offset;
	offset.x += cos(offset.x+wave_phase+acc*100)*0.0014;
	offset.y += sin(offset.y+wave_phase+acc*80)*0.00078;
	vec4 pixelBlur = texture2D(texture, gl_TexCoord[0].xy + offset);

	gl_FragColor = gl_Color * pixelBlur;
}
